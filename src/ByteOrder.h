#pragma once

#if defined(IMP_PLATFORM_WINDOWS)
#    include <stdlib.h>
#    define ByteSwap16(x) _byteswap_ushort(x)
#    define ByteSwap32(x) _byteswap_ulong(x)
#    define ByteSwap64(x) _byteswap_uint64(x)
#elif defined(IMP_PLATFORM_APPLE)
#    include <libkern/OSByteOrder.h>
#    define ByteSwap16(x) OSSwapInt16(x)
#    define ByteSwap32(x) OSSwapInt32(x)
#    define ByteSwap64(x) OSSwapInt64(x)
#elif defined(IMP_PLATFORM_LINUX)
#    include <byteswap.h>
#    define ByteSwap16(x) bswap_16(x)
#    define ByteSwap32(x) bswap_32(x)
#    define ByteSwap64(x) bswap_64(x)
#else
#    error "ByteSwap not implemented for this platform"
#endif
#include <cassert>
#include <cstdint>

namespace imp {
template<typename T>
struct ByteOrder {
    static void swap(T&)
    {
        assert(false && "ByteOrder::swap not implemented for this type");
    }
};

template<>
struct ByteOrder<int16_t> {
    static void swap(int16_t& value)
    {
        value = ByteSwap16(value);
    }
};

template<>
struct ByteOrder<uint16_t> {
    static void swap(uint16_t& value)
    {
        value = ByteSwap16(value);
    }
};

template<>
struct ByteOrder<int32_t> {
    static void swap(int32_t& value)
    {
        value = ByteSwap32(value);
    }
};

template<>
struct ByteOrder<uint32_t> {
    static void swap(uint32_t& value)
    {
        value = ByteSwap32(value);
    }
};

template<>
struct ByteOrder<int64_t> {
    static void swap(int64_t& value)
    {
        value = ByteSwap64(value);
    }
};

template<>
struct ByteOrder<uint64_t> {
    static void swap(uint64_t& value)
    {
        value = ByteSwap64(value);
    }
};

template<>
struct ByteOrder<float> {
    static void swap(float& value)
    {
        int32_t v = ByteSwap32(*reinterpret_cast<int32_t*>(&value));
        value = *reinterpret_cast<float*>(&v);
    }
};

static constexpr bool IsLittleEndian()
{
#if defined(IMP_ENDIANNESS_LITTLE)
    return IMP_ENDIANNESS_LITTLE;
#else
    return false;
#endif
}

static constexpr bool IsBigEndian()
{
#if defined(IMP_ENDIANNESS_BIG)
    return IMP_ENDIANNESS_BIG;
#else
    return false;
#endif
}

template<typename T>
static void DoByteReorder(T& value)
{
    ByteOrder<T>::swap(value);
}

template<typename T>
static void ReorderFromBE(T& value)
{
    if constexpr (IsLittleEndian() && sizeof(T) > 1) {
        DoByteReorder(value);
    }
}

template<typename T>
static void ReorderToBE(T& value)
{
    if constexpr (IsLittleEndian() && sizeof(T) > 1) {
        DoByteReorder(value);
    }
}

template<typename T>
static void ReorderFromLE(T& value)
{
    if constexpr (IsBigEndian() && sizeof(T) > 1) {
        DoByteReorder(value);
    }
}

template<typename T>
static void ReorderArrFromLE(T* arr, uint64_t count)
{
    if constexpr (IsBigEndian() && sizeof(T) > 1) {
        for (uint64_t i = 0; i < count; ++i) {
            DoByteReorder(arr[i]);
        }
    }
}

template<typename T>
static void ReorderArrFromBE(T* arr, uint64_t count)
{
    if constexpr (IsLittleEndian() && sizeof(T) > 1) {
        for (uint64_t i = 0; i < count; ++i) {
            DoByteReorder(arr[i]);
        }
    }
}

}
