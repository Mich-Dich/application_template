#pragma once

#include "util/core_config.h"
#include "imgui.h"

#if defined(RENDER_API_VULKAN)
    #include "engine/render/vulkan/vk_types.h"
#elif defined(RENDER_API_OPENGL)
    #include <GL/glew.h>
#endif

struct extent_3D { u32 width, height, depth; };

namespace AT {
    
    // Represents supported image formats for textures.
    enum class image_format {
        None = 0,
        RGBA,       // 8-bit per channel RGBA format
        RGBA32F     // 32-bit floating-point per channel RGBA format
    };

    class image {
    public:

        // Default constructor.
        // Creates an empty, uninitialized image object.
        // @return None.
        image() = default;

        // Constructs an image from raw pixel data.
        // @param data Pointer to the image pixel data.
        // @param size 3D extent of the image (width, height, depth).
        // @param format Pixel format of the image.
        // @param mipmapped Whether mipmaps should be generated.
        image(void* data, extent_3D size, image_format format, bool mipmapped = false);

        // Constructs a 2D image from raw pixel data.
        // @param data Pointer to the image pixel data.
        // @param width Width of the image in pixels.
        // @param height Height of the image in pixels.
        // @param format Pixel format of the image.
        // @param mipmapped Whether mipmaps should be generated.
        image(void* data, u32 width, u32 height, image_format format, bool mipmapped = false);

        // Constructs an image from a file path.
        // Loads image data from disk and creates a GPU texture.
        // @param image_path Path to the image file.
        // @param format Desired image format.
        // @param mipmapped Whether mipmaps should be generated.
        image(std::filesystem::path image_path, image_format format, bool mipmapped = false);

        // Destructor.
        // Automatically releases GPU resources associated with this image.
        ~image();

        // Returns the width of the image in pixels.
        // @return Width as a 32-bit unsigned integer.
        u32 get_width() const;

        // Returns the height of the image in pixels.
        // @return Height as a 32-bit unsigned integer.
        u32 get_height() const;

        // Releases GPU resources associated with this image.
        // After calling this, the image becomes invalid.
        void release();

        // Returns a unified texture handle for ImGui rendering.
        // On Vulkan, this returns a descriptor set; on OpenGL, a bindless texture handle.
        // @return ImGui-compatible texture identifier.
        ImTextureID get();

#if defined(RENDER_API_VULKAN)

        // Vulkan-specific getters and setters for internal objects.
        DEFAULT_GETTER_SETTER_ALL(VkImage,          image);
        DEFAULT_GETTER_SETTER_ALL(VkImageView,      image_view);
        DEFAULT_GETTER_SETTER_ALL(VmaAllocation,    allocation);
        DEFAULT_GETTER_SETTER_ALL(extent_3D,        image_extent);
        DEFAULT_GETTER_SETTER_ALL(VkFormat,         image_format);

        // Generates a Vulkan descriptor set for sampling this image in shaders.
        // @param sampler Vulkan sampler used for this texture.
        // @param layout Image layout, defaults to shader-read-only.
        // @return Vulkan descriptor set handle.
        VkDescriptorSet generate_descriptor_set(VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

#elif defined(RENDER_API_OPENGL)

        // OpenGL-specific getters for texture ID and bindless handle.
        DEFAULT_GETTER_C(GLuint,                    textureID)
        DEFAULT_GETTER_C(GLuint64,                  bindless_handle)

        // Creates a bindless texture handle and makes it resident.
        // Required for modern OpenGL bindless texturing.
        void create_bindless_handel();
        
#endif

        // Decodes an image from memory into raw RGBA pixel data.
        // @param data Pointer to compressed image data (e.g., PNG, JPEG).
        // @param length Size of the compressed data in bytes.
        // @param outWidth Output parameter for image width.
        // @param outHeight Output parameter for image height.
        // @return Pointer to decoded RGBA pixel data. Caller must free this.
        static void* decode(const void* data, u64 length, u32& outWidth, u32& outHeight);

    private:
    
        extent_3D                                   m_image_extent{};
        bool                                        m_is_initialized = false;
        bool                                        m_mipmapped = false;

#if defined(RENDER_API_VULKAN)
    
        // Allocates GPU memory and uploads pixel data to a Vulkan image.
        // @param data Raw image data.
        // @param size Image dimensions.
        // @param format Pixel format.
        // @param mipmapped Whether mipmaps should be generated.
        // @param usage Vulkan usage flags for the image.
        void allocate_memory(void* data, extent_3D size, image_format format, bool mipmapped, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT);

        // Allocates a Vulkan image object and creates an image view.
        // @param size Image dimensions.
        // @param format Vulkan image format.
        // @param usage Usage flags for the Vulkan image.
        // @param mipmapped Whether mipmaps should be generated.
        void allocate_image(extent_3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

        bool                                        m_is_initalized = false;
        VkImage                                     m_image = VK_NULL_HANDLE;
        VkImageView                                 m_image_view = VK_NULL_HANDLE;
        VmaAllocation                               m_allocation = nullptr;
        VkFormat                                    m_image_format{};
        VkDescriptorSet                             m_descriptor_set = VK_NULL_HANDLE;

#elif defined(RENDER_API_OPENGL)

        // Allocates an OpenGL texture and uploads image data.
        // @param data Raw image data.
        // @param size Image dimensions.
        // @param format Image format.
        // @param mipmapped Whether mipmaps should be generated.
        void allocate_memory(void* data, extent_3D size, image_format format, bool mipmapped);

        GLuint                                      m_textureID = 0;
        GLuint64                                    m_bindless_handle = 0;
        image_format                                m_format = image_format::None;
#endif
    };
}
