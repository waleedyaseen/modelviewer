#include "VertexBuffer.h"

#include "GLUtils.h"

namespace imp {
bool VertexBuffer::Create(uint32_t size, BufferCreateFlags flags, void const* data)
{
    return CreateInternal(GL_ARRAY_BUFFER, GL_STATIC_DRAW, GL_DYNAMIC_DRAW, size, flags, data);
}

void VertexBuffer::Destroy()
{
    DestroyInternal();
}

void VertexBuffer::Update(uint32_t offset, uint32_t size, void const* data)
{
    UpdateInternal(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, offset, size, data);
}
}
