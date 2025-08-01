#include "texture.h"

#include <application/renderer/device.h>
#include <application/core/utils/engine_assert.h>

#include <stb_image.h>

//-------------------------------------------------------------------------------------------------
VulkanTexture::VulkanTexture(std::string_view path, std::shared_ptr<VkGraphicDevice> device) : m_device(device)
{
	engineAssert(m_device != nullptr, "Device is not initialized yet");

	loadFromFile(path);
	createVulkanResources();
	createDescriptorSet();
}

VulkanTexture::~VulkanTexture()
{
	cleanup();
}

//-------------------------------------------------------------------------------------------------
void VulkanTexture::loadFromFile(std::string_view path)
{
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixels = stbi_load(path.data(), &width, &height, &channels, STBI_rgb_alpha);

	engineAssert(pixels != nullptr, std::format("Failed to load texture: {}", path));

	m_width = static_cast<uint32_t>(width);
	m_height = static_cast<uint32_t>(height);

	size_t imageSize = width * height * 4; // RGBA
	m_pixelData.resize(imageSize);
	memcpy(m_pixelData.data(), pixels, imageSize);

	stbi_image_free(pixels);
}

void VulkanTexture::createVulkanResources()
{
	VkDeviceSize imageSize = m_pixelData.size();

	// Create staging buffer
	vk::Buffer       stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	m_device->createBuffer(imageSize
		, vk::BufferUsageFlagBits::eTransferSrc
		, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		, stagingBuffer
		, stagingBufferMemory);

	// Copy data to staging buffer
	void* data;
	[[maybe_unused]] auto res = m_device->getLogicalDevice().
		mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);

	memcpy(data, m_pixelData.data(), imageSize);
	m_device->getLogicalDevice().unmapMemory(stagingBufferMemory);

	// Create VkImage
	vk::Format vkFormat = vk::Format::eR8G8B8A8Unorm;

	vk::ImageCreateInfo imageInfo = {};
	imageInfo.setImageType(vk::ImageType::e2D)
		.setExtent(vk::Extent3D(m_width, m_height, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(vkFormat)
		.setTiling(vk::ImageTiling::eOptimal)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setSharingMode(vk::SharingMode::eExclusive);

	auto [imageRes, image] = m_device->getLogicalDevice().createImage(imageInfo);
	m_image = image;

	// Allocate memory for texture
	vk::MemoryRequirements memRequirements = m_device->getLogicalDevice().getImageMemoryRequirements(m_image);

	vk::MemoryAllocateInfo allocInfo = {};
	allocInfo.setAllocationSize(memRequirements.size)
		.setMemoryTypeIndex(m_device->findMemoryType(memRequirements.memoryTypeBits
		, vk::MemoryPropertyFlagBits::eDeviceLocal));

	auto [allocRes, imageMem] = m_device->getLogicalDevice().allocateMemory(allocInfo);
	m_imageMemory = imageMem;
	m_device->getLogicalDevice().bindImageMemory(m_image, m_imageMemory, 0);

	// Changing layout to once we need
	m_device->transitionImage(m_image, vkFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	m_device->copyBufferToImage(stagingBuffer, m_image, m_width, m_height);
	m_device->transitionImage(m_image
		, vkFormat
		, vk::ImageLayout::eTransferDstOptimal
		, vk::ImageLayout::eShaderReadOnlyOptimal);

	// Cleaning staging
	m_device->getLogicalDevice().destroyBuffer(stagingBuffer);
	m_device->getLogicalDevice().freeMemory(stagingBufferMemory);

	// Creating image view
	vk::ImageViewCreateInfo viewInfo = {};
	viewInfo.setImage(m_image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vkFormat)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	auto [imageViewRes, imageView] = m_device->getLogicalDevice().createImageView(viewInfo);
	m_imageView = imageView;

	// Cleaning pixel data
	m_pixelData.clear();
	m_pixelData.shrink_to_fit();
}

//-------------------------------------------------------------------------------------------------
void VulkanTexture::createDescriptorSet()
{
	m_descriptorSet = m_device->createTextureDescriptorSet(m_image, m_imageView);
}

void VulkanTexture::cleanup()
{
	if (m_device && m_device->getLogicalDevice())
	{
		auto& device = m_device->getLogicalDevice();

		m_device->freeDescriptorSetFromPool(m_descriptorSet);

		if (m_imageView != VK_NULL_HANDLE)
		{
			device.destroyImageView(m_imageView);
			m_imageView = VK_NULL_HANDLE;
		}

		if (m_image != VK_NULL_HANDLE)
		{
			device.destroyImage(m_image);
			m_image = VK_NULL_HANDLE;
		}

		if (m_imageMemory != VK_NULL_HANDLE)
		{
			device.freeMemory(m_imageMemory);
			m_imageMemory = VK_NULL_HANDLE;
		}
	}
}
