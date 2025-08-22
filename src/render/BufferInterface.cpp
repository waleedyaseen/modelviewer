#include "BufferInterface.h"

#include "GLUtils.h"

namespace imp {
bool BufferInterface::CreateInternal(uint32_t target, uint32_t staticUsage, uint32_t dynamicUsage, uint32_t size, BufferCreateFlags flags, void const* data)
{
    IMP_DEBUG_ASSERT(size > 0);
    IMP_DEBUG_ASSERT(m_id == 0);
    GLCALL(glGenBuffers(1, &m_id));
    if (m_id == 0) {
        IMP_LOG_WARN("Unable to create buffer!");
        return false;
    }
    m_size = size;
    m_flags = flags;
    GLCALL(glBindBuffer(target, m_id));
    GLCALL(glBufferData(target, size, data, (flags & BUFFER_FLAG_DYNAMIC) ? dynamicUsage : staticUsage));
    GLCALL(glBindBuffer(target, 0));
    return true;
}

void BufferInterface::DestroyInternal()
{
    if (m_id == 0) {
        return;
    }
    glDeleteBuffers(1, &m_id);
    m_id = 0;
}

void BufferInterface::UpdateInternal(int32_t target, uint32_t usage, uint32_t offset, uint32_t size, void const* data)
{
    IMP_DEBUG_ASSERT(m_id != 0);
    IMP_DEBUG_ASSERT(data != nullptr);
    IMP_DEBUG_ASSERT(offset + size <= m_size);

    GLCALL(glBindBuffer(target, m_id));
    GLCALL(glBufferSubData(target, offset, size, data));
    GLCALL(glBindBuffer(target, 0));
}

}
