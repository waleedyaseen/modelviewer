#pragma once

#include "../Utils.h"
#include <cinttypes>

namespace imp {
enum BufferCreateFlags : uint8_t {
    BUFFER_FLAG_NONE = 0x0,
    BUFFER_FLAG_DYNAMIC = BIT(0),
};
class BufferInterface {
    MAKE_NON_COPYABLE(BufferInterface);

public:
    BufferInterface() = default;
    virtual ~BufferInterface() = default;

    uint32_t GetNativeBuffer() const
    {
        return m_id;
    }

protected:
    bool CreateInternal(uint32_t target, uint32_t staticUsage, uint32_t dynamicUsage, uint32_t size, BufferCreateFlags flags, void const* data);
    void DestroyInternal();

    void UpdateInternal(int32_t target, uint32_t usage, uint32_t offset, uint32_t size, void const* data);

    uint32_t m_id { 0 };
    uint32_t m_size { 0 };
    uint8_t m_flags { 0x0 };
};
}
