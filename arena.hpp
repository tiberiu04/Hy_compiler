#pragma once

#include <cstddef>
#include <memory>
#include <utility>

class ArenaAllocator {
public:
    explicit ArenaAllocator(const size_t max_num_bytes)
        : m_size { max_num_bytes } // the buffer size
        , m_buffer { new std::byte[max_num_bytes] } // allocate the buffer
        , m_offset { m_buffer } // set the offset to the beginning of the buffer
    {
    }

    // copying is deactivated, because the buffer would cause conflicts
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    // move constructor and move assignment operator
    ArenaAllocator(ArenaAllocator&& other) noexcept
        : m_size { std::exchange(other.m_size, 0) }
        , m_buffer { std::exchange(other.m_buffer, nullptr) }
        , m_offset { std::exchange(other.m_offset, nullptr) }
    {
    }

    // move assignment operator
    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept
    {
        std::swap(m_size, other.m_size);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_offset, other.m_offset);
        return *this;
    }

    // here we allocate the memory for the object, in the generic way
    template <typename T>
    [[nodiscard]] T* alloc()
    {
        // we calculate the remaining number of bytes
        size_t remaining_num_bytes = m_size - static_cast<size_t>(m_offset - m_buffer);
        // we align the address of the memory
        auto pointer = static_cast<void*>(m_offset);
        const auto aligned_address = std::align(alignof(T), sizeof(T), pointer, remaining_num_bytes);
        // if the address is not aligned, we throw an exception
        if (aligned_address == nullptr) {
            throw std::bad_alloc {};
        }
        // we move the offset to the next available memory
        m_offset = static_cast<std::byte*>(aligned_address) + sizeof(T);
        // we return the address of the memory
        return static_cast<T*>(aligned_address);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        // here we allocate the memory for an object using the constructor
        const auto allocated_memory = alloc<T>();
        return new (allocated_memory) T { std::forward<Args>(args)... };
    }

    ~ArenaAllocator()
    {
        // No destructors are called for the stored objects. Thus, memory
        // leaks are possible (e.g. when storing std::vector objects or
        // other non-trivially destructable objects in the allocator).
        // Although this could be changed, it would come with additional
        // runtime overhead and therefore is not implemented.
        delete[] m_buffer;
    }

private:
    size_t m_size;
    std::byte* m_buffer;
    std::byte* m_offset;
};