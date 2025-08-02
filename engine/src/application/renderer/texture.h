#define STB_IMAGE_IMPLEMENTATION
#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <string>
#include <vector>

class VkGraphicDevice;

//-------------------------------------------------------------------------------------------------
class VulkanTexture
{
public:
	VulkanTexture(std::string_view path, std::shared_ptr<VkGraphicDevice> device);
	~VulkanTexture();

	uint32_t getWidth() const { return m_width; }
	uint32_t getHeight() const { return m_height; }
	std::string_view getPath() const { return m_path; }
	size_t getMemoryUsage() const { return size_t(); }

	bool isValid() const { return m_image != VK_NULL_HANDLE; }

	vk::Image getVkImage() const { return m_image; }
	vk::ImageView getVkImageView() const { return m_imageView; }
	vk::DescriptorSet getDescriptorSet() const { return m_descriptorSet; }

private:
	void loadFromFile(std::string_view path);
	void createVulkanResources();
	void createDescriptorSet();
	void cleanup();

private:
	std::shared_ptr<VkGraphicDevice> m_device;
	std::string      m_path;
	uint32_t         m_width = 0;
	uint32_t         m_height = 0;

	// Vulkan resources
	vk::Image         m_image = VK_NULL_HANDLE;
	vk::DeviceMemory  m_imageMemory = VK_NULL_HANDLE;
	vk::ImageView     m_imageView = VK_NULL_HANDLE;
	vk::DescriptorSet m_descriptorSet = VK_NULL_HANDLE;

	// Texture data
	std::vector<uint8_t> m_pixelData;
};
