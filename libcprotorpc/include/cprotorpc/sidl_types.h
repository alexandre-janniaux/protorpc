#ifndef CPROTORPC_SIDL_TYPES_H
#define CPROTORPC_SIDL_TYPES_H

#define XM_SIDL_TYPES(FN) \
    FN(u8, uint8_t) \
    FN(u16, uint16_t) \
    FN(u32, uint32_t) \
    FN(u64, uint64_t) \
    FN(i8, int8_t) \
    FN(i16, int16_t) \
    FN(i32, int32_t) \
    FN(i64, int64_t) \
    FN(usize, size_t)

#endif
