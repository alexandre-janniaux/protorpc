#include "cprotorpc/serializer.h"
#include "cprotorpc/unserializer.h"
#include "gtest/gtest.h"

TEST(serializer, simple_serialization_1)
{
    sidl_serializer_t s;
    sidl_serializer_init(&s);

    sidl_serializer_write_u8(&s, 42);
    sidl_serializer_write_u8(&s, 43);
    sidl_serializer_write_u32(&s, 0xdeadbeef);

    ASSERT_EQ(s.data_size, 6);

    sidl_serializer_destroy(&s);
}

TEST(serializer, round_trip_1)
{
    sidl_serializer_t s;
    sidl_serializer_init(&s);

    const char* text = "Hello world !";
    uint8_t v1 = 0x7f;
    uint64_t v2 = 0xdeadbeefdeadbeef;

    sidl_serializer_write_u64(&s, v2);
    sidl_serializer_write_u8(&s, v1);
    sidl_serializer_write_string(&s, text);

    const char* text_u = NULL;
    uint8_t v1_u = 0;
    uint64_t v2_u = 0;

    sidl_unserializer_t u;
    sidl_unserializer_init(&u, s.data, s.data_size, s.fds, s.fd_count);

    ASSERT_NE(sidl_unserializer_read_u64(&u, &v2_u), -1);
    ASSERT_NE(sidl_unserializer_read_u8(&u, &v1_u), -1);
    ASSERT_NE(sidl_unserializer_read_string(&u, &text_u), -1);

    ASSERT_EQ(v1_u, v1);
    ASSERT_EQ(v2_u, v2);
    ASSERT_EQ(strcmp(text, text_u), 0);

    free((void*)text_u);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
