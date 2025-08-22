#pragma once

#include "BufferInterface.h"

namespace imp {
class IndexBuffer final : public BufferInterface {
    MAKE_NON_COPYABLE(IndexBuffer);

public:
    IndexBuffer() = default;
    ~IndexBuffer() override = default;

    bool Create(uint32_t size, BufferCreateFlags flags, void const* data);
    void Destroy();

    void Update(uint32_t offset, uint32_t size, void const* data);
};
}
