#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <cstddef>

template <typename T, size_t N>
class CircularBuffer {

    class Iterator {
    public:
        Iterator(CircularBuffer& buffer, size_t pos, size_t count) : m_buffer{buffer}, m_pos{pos}, m_count{count} {};

        T& operator*() const { return m_buffer[m_pos]; }

        Iterator& operator++() {
            m_pos = ++m_pos % N;
            ++m_count;
            return *this;
        }

        bool operator!=(Iterator& other) const { return m_count != other.m_count; }
        
    private:
        CircularBuffer& m_buffer;
        size_t m_pos;
        size_t m_count;
    };

public:
    CircularBuffer(T t) : m_array{t}, m_head{0}, m_size{0} {
        for (int i=0; i < N; ++i) {
            push(t);
        }
    }

    void push(T t) {
        m_array[m_head] = t;
        m_head = (++m_head) % N;
        if (m_size < N)
            ++m_size;
    }

    inline size_t capacity() { return N; }

    T& operator[](std::ptrdiff_t index) { return m_array[index]; }

    Iterator begin() { return Iterator(*this, m_head, 0); }
    Iterator end() { return Iterator(*this, m_head, m_size); }

private:
    T m_array[N];
    size_t m_head;
    size_t m_size;
};

#endif