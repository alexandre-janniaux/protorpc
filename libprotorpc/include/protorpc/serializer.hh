#ifndef RPC_SERIALIZER_HH
#define RPC_SERIALIZER_HH

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <iostream>
#include <typeinfo>
#include <type_traits>

namespace rpc
{

    template <typename T>
    struct serializable
    {
        serializable() = delete;
    };

    /*
     * template <>
     * struct serializable<Object> {
     *     // Serialize fields to bytes
     *     static void serialize(Object& obj, Serializer& s) { ... };
     * };
     */

    template <typename T>
    constexpr bool is_serializable_v = std::is_constructible_v<serializable<T>>;

    class Serializer
    {
    public:
        template <typename T>
        void serialize(T value)
        {
            serialize_into(value);
        }

        void serialize(void* data, std::size_t size)
        {
            std::uint8_t* data_ptr = reinterpret_cast<std::uint8_t*>(data);
            data_.insert(data_.end(), data_ptr, data_ptr + size);
        }

        void add_handle(int handle)
        {
            handles_.push_back(handle);
        }

        std::vector<std::uint8_t> get_payload()
        {
            auto ret = std::move(data_);
            data_.clear();

            return ret;
        }

        std::vector<int> get_handles()
        {
            auto ret = std::move(handles_);
            handles_.clear();

            return ret;
        }

    private:
        template <typename T>
        std::enable_if_t<std::is_arithmetic_v<T>>
        serialize_into(T value)
        {
            std::uint8_t* start = reinterpret_cast<std::uint8_t*>(&value);
            data_.insert(data_.end(), start, start + sizeof(T));
        }


        void serialize_into(std::string str)
        {
            serialize<std::size_t>(str.size());
            data_.insert(data_.end(), str.c_str(), str.c_str() + str.size());
        }

        template <typename T>
        void serialize_into(std::vector<T> v)
        {
            serialize<std::size_t>(v.size());

            for (auto& e : v)
                serialize<T>(e);
        }

        template <typename T>
        std::enable_if_t<is_serializable_v<T>>
        serialize_into(T obj)
        {
            serializable<T>::serialize(obj, *this);
        }

        template <typename T>
        void serialize_into(std::optional<T> obj)
        {
            if (obj)
            {
                serialize<std::uint8_t>(1);
                serialize(*obj);
            }
            else
            {
                serialize<std::uint8_t>(0);
            }
        }


        std::vector<uint8_t> data_;
        std::vector<int> handles_;
    };

}

#endif
