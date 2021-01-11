#ifndef RPC_UNSERIALIZER_HH
#define RPC_UNSERIALIZER_HH

#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <type_traits>

namespace rpc
{

    template <typename T>
    struct unserializable
    {
        unserializable() = delete;
    };

    /*
     * template <>
     * struct unserializable<Object> {
     *     static bool unserialize(Object* out, Unserializer& s) { ... }
     * };
     */

    template <typename T>
    constexpr bool is_unserializable_v = std::is_constructible_v<unserializable<T>>;

    class Unserializer
    {
    public:
        Unserializer(std::vector<std::uint8_t>&& buffer)
            : data_(std::move(buffer)), index_(0), handle_index_(0)
        {}

        Unserializer(std::vector<std::uint8_t>&& buffer, const std::vector<int>& handles)
            : data_(std::move(buffer)), handles_(handles), index_(0), handle_index_(0)
        {}

        Unserializer(const std::vector<std::uint8_t>& buffer)
            : data_(buffer), index_(0), handle_index_(0)
        {}

        Unserializer(const std::vector<std::uint8_t>& buffer, const std::vector<int>& handles)
            : data_(buffer), handles_(handles), index_(0), handle_index_(0)
        {}

        template <typename T>
        bool unserialize(T* output)
        {
            return unserialize_into(output);
        }

        /**
         * Unserialize the next handle. Returns true on success.
         */
        bool next_handle(int* out)
        {
            if (handle_index_ >= handles_.size())
                return false;

            *out = handles_[handle_index_++];
            return true;
        }

        std::vector<std::uint8_t> get_remaining()
        {
            std::vector<std::uint8_t> result(data_.begin() + index_, data_.end());
            index_ = data_.size();

            return result;
        }

    private:

        template <typename T>
        std::enable_if_t<std::is_arithmetic_v<T>, bool>
        unserialize_into(T* output)
        {
            if (index_ + sizeof(T) > data_.size())
            {
                // We invalidate the stream if the unserialization failed.
                index_ = data_.size();
                return false;
            }

            std::uint8_t* start = data_.data() + index_;
            std::copy(start, start + sizeof(T), reinterpret_cast<std::uint8_t*>(output));
            index_ += sizeof(T);

            return true;
        }

        bool unserialize_into(std::string* output)
        {
            std::size_t size = 0;

            if (!unserialize<std::size_t>(&size))
                return false;

            if (index_ + size > data_.size())
                return false;

            auto* str_start = data_.data() + index_;
            *output = std::string(str_start, str_start + size);
            index_ += size;

            return true;
        }

        template <typename T>
        std::enable_if_t<std::is_default_constructible_v<T>, bool>
        unserialize_into(std::vector<T>* output)
        {
            std::size_t size;

            if (!unserialize<std::size_t>(&size))
                return false;

            for (std::size_t i = 0; i < size; i++)
            {
                // XXX: For now any contained element must be default
                //      constructible.
                T element;

                if (!unserialize<T>(&element))
                    return false;

                output->push_back(std::move(element));
            }

            return true;
        }

        template <typename T>
        std::enable_if_t<is_unserializable_v<T>, bool>
        unserialize_into(T* output)
        {
            return unserializable<T>::unserialize(output, *this);
        }

        template <typename T>
        bool unserialize_into(std::optional<T>* output)
        {
            std::uint8_t has_element = 0;

            if (!unserialize(&has_element))
                return false;

            if (has_element)
            {
                T element;

                if (!unserialize(&element))
                    return false;

                *output = std::move(element);
            }
            else
            {
                *output = std::nullopt;
            }

            return true;
        }

    private:
        std::vector<std::uint8_t> data_;
        std::vector<int> handles_;
        std::size_t index_;
        std::size_t handle_index_;
    };

}

#endif
