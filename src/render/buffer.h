#pragma once

#if defined(RENDER_API_VULKAN)
    #include "engine/render/vulkan/vk_types.h"
#endif

namespace AT::render {
    
    // Represents a GPU buffer abstraction used for storing vertex, index, or general data,
    // depending on the active rendering backend (OpenGL or Vulkan).
    class buffer {
    public:
    
    #if defined(RENDER_API_OPENGL)

        // Enum defining buffer types for OpenGL.
        // - VERTEX: Vertex buffer object (VBO).
        // - INDEX:  Index buffer object (EBO/IBO).
        enum class type { VERTEX, INDEX };

        // Enum defining buffer usage patterns for OpenGL.
        // - STATIC: Data is set once and used many times.
        // - DYNAMIC: Data is updated frequently.
        // - STREAM: Data is updated every frame.
        enum class usage { STATIC, DYNAMIC, STREAM };

        // Constructs an OpenGL buffer of the given type, usage, and size, uploading initial data.
        // @param type The type of buffer (VERTEX or INDEX).
        // @param usage The expected usage pattern (STATIC, DYNAMIC, STREAM).
        // @param data Pointer to the initial data to upload to the buffer.
        // @param size Size of the buffer in bytes.
        buffer(type type, usage usage, const void* data, size_t size);

        // Returns the OpenGL buffer ID.
        DEFAULT_GETTER_SETTER(u32, ID)

        // Returns the type of the OpenGL buffer (VERTEX or INDEX).
        DEFAULT_GETTER(type, type)


    #elif defined(RENDER_API_VULKAN)

        // Default constructor for Vulkan buffers.
        // Initializes an empty buffer without allocation.
        buffer() = default;

        // Constructs a Vulkan buffer and allocates GPU memory using VMA.
        // @param usage Vulkan buffer usage flags (e.g., VK_BUFFER_USAGE_VERTEX_BUFFER_BIT).
        // @param memory_usage VMA memory usage hint (e.g., VMA_MEMORY_USAGE_CPU_TO_GPU).
        // @param alloc_size The size of the buffer in bytes.
        // @param data Pointer to the initial data to upload to the buffer.
        buffer(VkBufferUsageFlags usage, VmaMemoryUsage memory_usage, size_t alloc_size, void* data);

        // Returns the Vulkan buffer handle.
        DEFAULT_GETTER(VkBuffer, buffer)

        // Returns a const reference to the Vulkan buffer handle.
        DEFAULT_GETTER_REF(VkBuffer, buffer)

    #endif

        // Destructor.
        // Releases the allocated GPU resources (OpenGL buffer or Vulkan buffer + memory).
        ~buffer();
            
        // Returns the size of the buffer in bytes.
        DEFAULT_GETTER(size_t, size)

    protected:
        size_t              m_size = 0;     // Size of the buffer in bytes.
    
    #if defined(RENDER_API_OPENGL)
        type                m_type;         // OpenGL buffer type (VERTEX or INDEX).
        usage               m_usage;        // OpenGL buffer usage pattern.
        u32                 m_ID = 0;       // OpenGL buffer object ID.

    #elif defined(RENDER_API_VULKAN)
        VkBuffer            m_buffer;       // Vulkan buffer handle.
        VmaAllocation       m_allocation;   // VMA memory allocation handle.
        VmaAllocationInfo   m_info;         // VMA allocation info (contains mapped memory pointer).
    #endif
    
    };

}
