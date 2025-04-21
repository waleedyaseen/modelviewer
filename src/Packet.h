#pragma once

#include "ByteOrder.h"
#include <assert.h>
#include <cinttypes>
#include <cstring>

namespace imp {
class Packet {
public:
    explicit Packet(size_t size)
        : m_data(new int8_t[size])
        , m_pos(0)
        , m_size(size)
        , m_owned(true)
    {
        memset(m_data, 0, size);
    }

    Packet(int8_t* data, size_t size)
        : m_data(data)
        , m_pos(0)
        , m_size(size)
        , m_owned(false)
    {
        assert(m_data != nullptr);
    }

    ~Packet()
    {
        if (m_owned) {
            delete[] m_data;
        }
    }

    int8_t g1s()
    {
        assert(m_pos + 1 <= m_size);
        return m_data[m_pos++];
    }

    uint8_t g1()
    {
        assert(m_pos + 1 <= m_size);
        return static_cast<uint8_t>(m_data[m_pos++] & 0xff);
    }

    void p1(uint8_t value)
    {
        assert(m_pos + 1 <= m_size);
        m_data[m_pos++] = static_cast<int8_t>(value);
    }

    int16_t g2s()
    {
        return gT<int16_t>();
    }

    uint16_t g2()
    {
        return gT<uint16_t>();
    }

    void p2(uint16_t value)
    {
        pT<uint16_t>(value);
    }

    int32_t g4s()
    {
        return gT<int32_t>();
    }

    uint32_t g4()
    {
        return gT<uint32_t>();
    }

    void p4(uint32_t value)
    {
        pT<uint32_t>(value);
    }

    int32_t gSmart1or2s()
    {
        assert(m_pos + 1 <= m_size);
        uint8_t value = m_data[m_pos] & 0xff;
        if (value < 128) {
            return g1() - 64;
        } else {
            return g2() - 49152;
        }
    }

    int32_t gSmart1or2()
    {
        assert(m_pos + 1 <= m_size);
        uint8_t value = m_data[m_pos] & 0xff;
        if (value < 128) {
            return g1();
        } else {
            return g2() - 32768;
        }
    }

    void pSmart1or2(int32_t value)
    {
        if (value >= 0 && value < 128) {
            p1(value);
        } else if (value >= 0 && value < 32768) {
            p2(value + 32768);
        } else {
            assert(false);
        }
    }

    void pSmart1or2s(int32_t value)
    {
        if (value >= -64 && value < 64) {
            p1(value + 64);
        } else if (value >= -16384 && value < 16384) {
            p2(value + 49152);
        } else {
            assert(false);
        }
    }
    void pArr(int8_t const* data, size_t size)
    {
        assert(m_pos + size <= m_size);
        memcpy(m_data + m_pos, data, size);
        m_pos += size;
        ReorderArrFromBE(m_data + m_pos - size, size);
    }

    template<typename T>
    T gT()
    {
        m_pos += sizeof(T);
        assert(m_pos <= m_size);
        T value = *reinterpret_cast<T const*>(m_data + m_pos - sizeof(T));
        ReorderFromBE(value);
        return value;
    }

    template<typename T>
    T gTLE()
    {
        m_pos += sizeof(T);
        assert(m_pos <= m_size);
        T value = *reinterpret_cast<T const*>(m_data + m_pos - sizeof(T));
        ReorderFromLE(value);
        return value;
    }

    template<typename T>
    void pT(T value)
    {
        assert(m_pos + sizeof(T) <= m_size);
        ReorderToBE(value);
        *reinterpret_cast<T*>(m_data + m_pos) = value;
        m_pos += sizeof(T);
    }

    Packet View() const
    {
        assert(m_pos <= m_size);
        return Packet(m_data, m_size);
    }

    void SetPos(size_t pos)
    {
        assert(pos <= m_size);
        m_pos = pos;
    }

    size_t GetPos() const
    {
        return m_pos;
    }

    size_t GetSize() const
    {
        return m_size;
    }

    int8_t const* GetData() const
    {
        return m_data;
    }

    int8_t* GetData()
    {
        return m_data;
    }

private:
    int8_t* m_data;
    size_t m_pos;
    size_t m_size;
    bool m_owned;
};
}
