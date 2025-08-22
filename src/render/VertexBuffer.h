#pragma once

#include "BufferInterface.h"

namespace imp {
class VertexBuffer final : public BufferInterface {
    MAKE_NON_COPYABLE(VertexBuffer);

public:
    VertexBuffer() = default;
    ~VertexBuffer() override = default;

    bool Create(uint32_t size, BufferCreateFlags flags, void const* data);
    void Destroy();

    void Update(uint32_t offset, uint32_t size, void const* data);
};
}
