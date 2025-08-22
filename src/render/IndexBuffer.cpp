#include "IndexBuffer.h"

#include "GLUtils.h"

namespace imp {
bool IndexBuffer::Create(uint32_t size, BufferCreateFlags flags, void const* data)
{
    return CreateInternal(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, GL_DYNAMIC_DRAW, size, flags, data);
}

void IndexBuffer::Destroy()
{
    DestroyInternal();
}

void IndexBuffer::Update(uint32_t offset, uint32_t size, void const* data)
{
    UpdateInternal(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, offset, size, data);
}
}
