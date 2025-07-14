module;

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

export module graphic_device;

import <vector>;
import <array>;
import <memory>;
import <ranges>;
import <set>;
import <string>;
import <iostream>;
import <format>;
import <algorithm>;
import <filesystem>;
import <cstdio>;
import <fstream>;
import <shaderc/shaderc.hpp>;
import <cstdlib>;
import <print>;
import <format>;
import <numeric>;

import <backends/imgui_impl_vulkan.h>;
import <backends/imgui_impl_glfw.h>;

import renderer_manager;
import imgui_integration;
import engine_assert;

constexpr int	C_MAX_FRAMES_IN_FLIGHT = 2;

//-------------------------------------------------------------------------------------------------
//-- Helper functions, maybe need to move in an another module
std::vector<uint32_t> compileShaderFromSource(const std::string& source,
	shaderc_shader_kind kind,
	const std::string& name)
{
	shaderc::Compiler		compiler;
	shaderc::CompileOptions	options;

	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	auto result = compiler.CompileGlslToSpv(source, kind, name.c_str(), options);

	engineAssert(result.GetCompilationStatus() == shaderc_compilation_status_success
		, std::format("Shader: '{}' compilation failed", name));

	// Spirv binary code
	return { result.cbegin(), result.cend() };
}

//-------------------------------------------------------------------------------------------------
std::string readFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	engineAssert(file && file.is_open()
		, std::format("Wasn't able to open file: '{}'", path));

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string buffer;
	buffer.resize(size);

	if (!file.read(buffer.data(), size))
	{
		engineAssert(false, std::format("Wasn't able to read file: '{}'", path));
	}

	return buffer;
}

//-------------------------------------------------------------------------------------------------
vk::VertexInputBindingDescription getBindingDescription()
{
	vk::VertexInputBindingDescription bindingDescription = {};
	bindingDescription.setBinding(0)
		.setStride(sizeof(VertexData))
		.setInputRate(vk::VertexInputRate::eVertex);
	return bindingDescription;
}

//-------------------------------------------------------------------------------------------------
std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
{
	std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};
	attributeDescriptions[0].setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(0)
		.setOffset(offsetof(VertexData, m_vertex));

	attributeDescriptions[1].setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setLocation(1)
		.setOffset(offsetof(VertexData, m_color));

	attributeDescriptions[2].setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(2)
		.setOffset(offsetof(VertexData, m_texCoord));

	return attributeDescriptions;
}

//-------------------------------------------------------------------------------------------------
struct QueueFamilies
{
	int	m_graphicQueue = -1;
	int	m_presentationQueue = -1;

	bool isValid() const
	{
		return m_graphicQueue >= 0
			&& m_presentationQueue >= 0;
	}
};

//-------------------------------------------------------------------------------------------------
struct Queues
{
	vk::Queue	m_graphicQueue;
	vk::Queue	m_presentationQueue;
};

//-------------------------------------------------------------------------------------------------
struct SwapchainImage
{
	vk::Image		m_image = VK_NULL_HANDLE;
	vk::ImageView	m_imageView = VK_NULL_HANDLE;
};

//-------------------------------------------------------------------------------------------------
struct SwapChainDetails
{
	//-- Surface props such as size and buffer images count
	vk::SurfaceCapabilitiesKHR			m_surfaceCapabilities;
	//-- Format such as RGBA8
	std::vector<vk::SurfaceFormatKHR>	m_surfaceSupportedFormats;
	//-- Supported presentation modes to choose
	std::vector<vk::PresentModeKHR>		m_presentMode;
};

//-------------------------------------------------------------------------------------------------
struct PhysicalDeviceData
{
	int					m_score = 0;
	QueueFamilies		m_queueFamilies;
	SwapChainDetails	m_swapchainDetails;
};

//-------------------------------------------------------------------------------------------------
struct UniformBufferObject
{
	glm::mat4	m_view;
	glm::mat4	m_proj;
};

//-------------------------------------------------------------------------------------------------
struct VulkanBufferMemory
{
	vk::Buffer			m_buffer;
	vk::DeviceMemory	m_bufferMem;
};

//-------------------------------------------------------------------------------------------------
export struct TexturedGeometry
{
	VulkanBufferMemory	m_memory;
	vk::DescriptorSet	m_textureDescriptorSet;
	uint32_t			m_spritesCount;
};

export using TexturedGeometryBatch = std::vector<TexturedGeometry>;
export using BatchIndecies = std::vector<VulkanBufferMemory>;

//-------------------------------------------------------------------------------------------------
export class VkGraphicDevice
{
public:
	//-------------------------------------------------------------------------------------------------
	~VkGraphicDevice()
	{
		shutdown();
	}

	//-------------------------------------------------------------------------------------------------
	void init(GLFWwindow* window)
	{
		engineAssert(window != nullptr, "GLFW Window not initialized");
		m_window = window;

		try
		{
			createVkInstance();
			//-- Create surface earlier than other devices types since we need it
			//-- in checking queue that can support presentation operations
			std::println("createSurface");
			createSurface();
			std::println("setupPhysicalDevice");
			setupPhysicalDevice();
			std::println("createLogicalDevice");
			createLogicalDevice();
			std::println("createSwapchain");
			createSwapchain();
			std::println("createShaderModule");
			createShaderModule();
			std::println("createRenderPass");
			createRenderPass();
			std::println("createDescriptorSetLayout");
			createDescriptorSetLayout();
			std::println("createPipeline");
			createPipeline();
			std::println("createFramebuffer");
			createFramebuffer();
			std::println("createCommandPool");
			createCommandPool();
			std::println("createTextureSampler");
			createTextureSampler();
			std::println("createUniformBuffers");
			createUniformBuffers();
			std::println("createDescriptorPool");
			createDescriptorPool();
			std::println("createDescriptorSets");
			createDescriptorsSets();
			std::println("createCommandBuffer");
			createCommandBuffer();
			std::println("createSyncObjects");
			createSyncObjects();
			std::println("Vulkan objects initialized");

			ImGuiInitInfo imGuiIntegrationInfo{
				.m_apiVersion = apiVersion(),
				.m_instance = instance(),
				.m_physicalDevice = physicalDevice(),
				.m_device = device(),
				.m_queueFamily = getGraphicQueueFamily(),
				.m_queue = graphicQueue(),
				.m_descriptorPool = descriptorPool(),
				.m_renderPass = renderPass(),
				.m_minImageCount = minImageCount(),
				.m_imageCount = imageCount(),
				.m_window = m_window
			};

			m_imGuiIntegration = ImGuiIntegration(imGuiIntegrationInfo);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error during initialization: " << e.what() << std::endl;
			throw; // Re-throw to allow proper cleanup
		}
	}

	//-------------------------------------------------------------------------------------------------
	void shutdown()
	{
		m_imGuiIntegration.shutdown();

		m_queues.m_graphicQueue.waitIdle();
		m_queues.m_presentationQueue.waitIdle();

		for (int i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_logicalDevice.destroySemaphore(m_imageAvailableSemaphores[i]);
			m_logicalDevice.destroySemaphore(m_renderFinishedSemaphores[i]);
			m_logicalDevice.destroyFence(m_inFlightFences[i]);
		}
		for (const auto& buf : m_uniformBuffers)
		{
			m_logicalDevice.destroyBuffer(buf);
		}
		for (const auto& mem : m_uniformBuffersMemory)
		{
			m_logicalDevice.freeMemory(mem);
		}
		m_logicalDevice.destroyDescriptorPool(m_descriptorPool);
		m_logicalDevice.destroyDescriptorSetLayout(m_uniformsSetLayout);
		m_logicalDevice.destroyDescriptorSetLayout(m_texturesSetLayout);
		m_logicalDevice.freeCommandBuffers(m_commandPool, m_commandBuffers);
		m_logicalDevice.destroyCommandPool(m_commandPool);
		cleanupSwapchain();

		m_logicalDevice.destroySampler(m_textureSampler);

		m_logicalDevice.destroyPipeline(m_graphicsPipeline);
		m_logicalDevice.destroyPipelineLayout(m_pipelineLayout);
		m_logicalDevice.destroyRenderPass(m_renderPass);
		m_logicalDevice.destroyShaderModule(m_vertexShaderModule);
		m_logicalDevice.destroyShaderModule(m_fragmentShaderModule);
		m_logicalDevice.destroy();

		m_vkInstance.destroySurfaceKHR(m_surface);
		m_vkInstance.destroy();
	}

	//-------------------------------------------------------------------------------------------------
	void resizedWindow()
	{
		m_framebufferResized = true;
	}

	//-------------------------------------------------------------------------------------------------
	void beginFrame(float /*dt*/)
	{
		auto res = m_logicalDevice.waitForFences(m_inFlightFences[m_currFrame]
			, vk::True
			, UINT64_MAX);

		auto [result, imageIndex] = m_logicalDevice.acquireNextImageKHR(m_swapchain
			, UINT64_MAX
			, m_imageAvailableSemaphores[m_currFrame]);

		m_currImageIndex = imageIndex;

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			recreateSwapChain();
			return;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		//-- Only reset the fence if we are submitting work
		m_logicalDevice.resetFences(m_inFlightFences[m_currFrame]);

		m_commandBuffers[m_currFrame].reset();
	}

	//-------------------------------------------------------------------------------------------------
	void endFrame(const TexturedGeometryBatch& geometryBatch, const BatchIndecies& indicesBatch)
	{
		recordCommandBuffer(m_commandBuffers[m_currFrame]
			, m_currImageIndex
			, geometryBatch
			, indicesBatch);
		updateUniformBuffer();

		//-- Submitting command buffer
		vk::SubmitInfo submitInfo = {};
		vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submitInfo.setWaitSemaphoreCount(1)
			.setWaitSemaphores(m_imageAvailableSemaphores[m_currFrame])
			.setWaitDstStageMask(waitStages)
			.setSignalSemaphoreCount(1)
			.setSignalSemaphores(m_renderFinishedSemaphores[m_currFrame])
			.setCommandBufferCount(1)
			.setCommandBuffers(m_commandBuffers[m_currFrame]);

		m_queues.m_graphicQueue.submit(submitInfo, m_inFlightFences[m_currFrame]);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.setWaitSemaphoreCount(1)
			.setWaitSemaphores(m_renderFinishedSemaphores[m_currFrame])
			.setSwapchainCount(1)
			.setSwapchains(m_swapchain)
			.setImageIndices(m_currImageIndex);

		try
		{
			[[maybe_unused]] auto result = m_queues.m_presentationQueue.presentKHR(presentInfo);
			if (result == vk::Result::eSuboptimalKHR)
			{
				recreateSwapChain();
				m_framebufferResized = false;
			}
		}
		catch (const vk::OutOfDateKHRError&)
		{
			recreateSwapChain();
			m_framebufferResized = false;
		}
		catch (const vk::SystemError& e)
		{
			throw std::runtime_error(std::format("Failed to present swap chain image: {}", e.what()));
		}

		m_currFrame = (m_currFrame + 1) % C_MAX_FRAMES_IN_FLIGHT;
	}

	//-------------------------------------------------------------------------------------------------
	auto createIndexBuffer(uint16_t spriteCount) -> VulkanBufferMemory
	{
		std::vector<uint16_t> indicies;
		indicies.resize(spriteCount * 6);
		constexpr uint32_t C_VERTICES_IN_QUAD = 4;
		uint32_t offset = 0;

		for (uint32_t sprite = 0; sprite < spriteCount; ++sprite)
		{
			uint32_t baseIndex = sprite * 6;

			indicies[baseIndex + 0] = offset + 0;
			indicies[baseIndex + 1] = offset + 1;
			indicies[baseIndex + 2] = offset + 2;

			indicies[baseIndex + 3] = offset + 2;
			indicies[baseIndex + 4] = offset + 3;
			indicies[baseIndex + 5] = offset + 0;

			offset += C_VERTICES_IN_QUAD;
		}

		VulkanBufferMemory resultMemory;

		auto bufferSize = indicies.size() * sizeof(uint16_t);

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer(bufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer,
			stagingBufferMemory);

		void* data = nullptr;
		auto res = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
		memcpy(data, indicies.data(), bufferSize);

		m_logicalDevice.unmapMemory(stagingBufferMemory);

		createBuffer(bufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			resultMemory.m_buffer,
			resultMemory.m_bufferMem);

		copyBuffer(stagingBuffer, resultMemory.m_buffer, bufferSize);
		m_logicalDevice.destroyBuffer(stagingBuffer);
		m_logicalDevice.freeMemory(stagingBufferMemory);

		return resultMemory;
	}

	//-------------------------------------------------------------------------------------------------
	void clearBuffer(VulkanBufferMemory memory)
	{
		m_logicalDevice.destroyBuffer(memory.m_buffer);
		m_logicalDevice.freeMemory(memory.m_bufferMem);
	}

	//-------------------------------------------------------------------------------------------------
	VulkanBufferMemory createCombinedVertexBuffer(const std::vector<std::array<VertexData, 4>>& sprites)
	{
		size_t totalVertices = sprites.size() * 4;
		auto bufferSize = totalVertices * sizeof(VertexData);

		VulkanBufferMemory resultMemory;
		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		createBuffer(bufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer,
			stagingBufferMemory);

		void* data = nullptr;
		auto mapResult = m_logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);

		VertexData* vertexData = static_cast<VertexData*>(data);
		size_t vertexIndex = 0;

		memcpy(data, sprites.data(), bufferSize);

		m_logicalDevice.unmapMemory(stagingBufferMemory);

		createBuffer(bufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			resultMemory.m_buffer,
			resultMemory.m_bufferMem);

		copyBuffer(stagingBuffer, resultMemory.m_buffer, bufferSize);

		m_logicalDevice.destroyBuffer(stagingBuffer);
		m_logicalDevice.freeMemory(stagingBufferMemory);

		return resultMemory;
	}
	
	//-------------------------------------------------------------------------------------------------
	uint8_t maxFrames() const
	{
		return C_MAX_FRAMES_IN_FLIGHT;
	}

	//-------------------------------------------------------------------------------------------------
	uint8_t currFrame() const
	{
		return m_currFrame;
	}

	//-------------------------------------------------------------------------------------------------
	void waitGraphicIdle()
	{
		m_queues.m_graphicQueue.waitIdle();
	}

	//-------------------------------------------------------------------------------------------------
	void updateUniformBuffer()
	{
		int w = 0, h = 0;
		glfwGetFramebufferSize(m_window, &w, &h);
		if (w == 0)
		{
			w = 1;
		}
		if (h == 0)
		{
			h = 1;
		}

		UniformBufferObject ubo = {};
		ubo.m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f)
			, glm::vec3(0.0f, 0.0f, 0.0f)
			, glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.m_proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 10.0f);
		ubo.m_proj[1][1] *= -1;
		memcpy(m_uniformBuffersMapped[m_currFrame], &ubo, sizeof(UniformBufferObject));
	}

	//-------------------------------------------------------------------------------------------------
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties physDeviceMemProps = m_physicalDevice.getMemoryProperties();
		std::vector<uint32_t> indices(physDeviceMemProps.memoryTypeCount);
		std::iota(indices.begin(), indices.end(), 0);

		auto it = std::find_if(indices.begin(), indices.end(),
			[&](uint32_t i)
			{
				return (typeFilter & (1 << i)) &&
					(physDeviceMemProps.memoryTypes[i].propertyFlags & properties) == properties;
			});

		engineAssert(it != indices.end(), std::format("Didn't find suitable memory type"));
		return *it;
	}

	//-------------------------------------------------------------------------------------------------
	vk::Device&	getLogicalDevice()
	{
		return m_logicalDevice;
	}

	//-------------------------------------------------------------------------------------------------
	void createVkInstance()
	{
		//-- Instance creation info will need info about application
		//-- Most info here for developer
		vk::ApplicationInfo appInfo = {};
		appInfo.setPApplicationName("Vulkan Study")
			.setApiVersion(VK_API_VERSION_1_3);

		m_apiVersion = VK_API_VERSION_1_3;

		//-- Creation info for a vkInstance
		vk::InstanceCreateInfo createInfo = {};
		createInfo.setPApplicationInfo(&appInfo);

		//-- Create collection to hold instance extensions
		std::vector<const char*> instanceextensions;

		uint32_t extensionsCount = 0;
		const char** glfwExtensionsList = glfwGetRequiredInstanceExtensions(&extensionsCount);
		instanceextensions.reserve(extensionsCount);
		for (uint32_t i = 0; i < extensionsCount; ++i)
		{
			instanceextensions.push_back(glfwExtensionsList[i]);
		}

		//-- Check if all requested extensions supported
		checkExtensionsSupport(instanceextensions);

		//-- Set extensions we received from glfw
		createInfo.setEnabledExtensionCount(static_cast<uint32_t>(instanceextensions.size()))
			.setPEnabledExtensionNames(instanceextensions);

		//-- This variable we will use to enable validation layers
		std::vector<const char*> validationLayers;
		if (m_enableValidationLayers)
		{
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			checkValidationLayerSupport(validationLayers);
			createInfo.setEnabledLayerCount(validationLayers.size());
		}
		else
		{
			createInfo.setEnabledLayerCount(0);
		}
		createInfo.setPEnabledLayerNames(validationLayers);

		//-- Create instance
		m_vkInstance = vk::createInstance(createInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createLogicalDevice()
	{
		//-- Vector for queue creation information and set to avoid duplication same queue
		std::vector<vk::DeviceQueueCreateInfo> queuesCreateInfos;
		std::set<int> queueFamilyIdices = {
			m_physicalDeviceData.m_queueFamilies.m_graphicQueue
			, m_physicalDeviceData.m_queueFamilies.m_presentationQueue
		};
		queuesCreateInfos.reserve(queueFamilyIdices.size());

		for (int queueIndex : queueFamilyIdices)
		{
			//-- Queue creation info
			const float priority = 1.0f;
			vk::DeviceQueueCreateInfo queueCreateInfo({}, queueIndex, 1, &priority);
			queuesCreateInfos.push_back(queueCreateInfo);
		}
		//-- Device info itself
		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.setSamplerAnisotropy(VK_TRUE);
		vk::DeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.setQueueCreateInfos(queuesCreateInfos)
			.setPEnabledFeatures(&deviceFeatures)
			.setEnabledExtensionCount(C_DEVICE_EXTENSIONS.size())
			.setPEnabledExtensionNames(C_DEVICE_EXTENSIONS);

		//-- Creating logical device
		m_logicalDevice = m_physicalDevice.createDevice(deviceCreateInfo);

		//-- Queues are created automatically, we save it
		m_logicalDevice.getQueue(m_physicalDeviceData.m_queueFamilies.m_graphicQueue
			, 0
			, &m_queues.m_graphicQueue);

		m_logicalDevice.getQueue(m_physicalDeviceData.m_queueFamilies.m_presentationQueue
			, 0
			, &m_queues.m_presentationQueue);
	}

	//-------------------------------------------------------------------------------------------------
	void createSurface()
	{
		VkSurfaceKHR rawSurface = {};
		VkResult result = glfwCreateWindowSurface(m_vkInstance, m_window, nullptr, &rawSurface);
		engineAssert(result == VK_SUCCESS, "Create surface failed");
		m_surface = vk::SurfaceKHR(rawSurface);
	}

	//-------------------------------------------------------------------------------------------------
	void recreateSwapChain()
	{
		m_logicalDevice.waitIdle();

		cleanupSwapchain();

		m_swapchainImages.clear();
		m_swapChainFramebuffers.clear();
		m_physicalDeviceData.m_swapchainDetails = swapchainDetails(m_physicalDevice);

		createSwapchain();
		createFramebuffer();
	}

	//-------------------------------------------------------------------------------------------------
	void createSwapchain()
	{
		const SwapChainDetails& swaphainDetails = m_physicalDeviceData.m_swapchainDetails;

		const vk::SurfaceFormatKHR	surfaceFormat = chooseSurfaceFormat(swaphainDetails.m_surfaceSupportedFormats);
		const vk::PresentModeKHR	presentMode = choosePresentMode(swaphainDetails.m_presentMode);
		const vk::Extent2D			extent = chooseSwapChainExtent(swaphainDetails.m_surfaceCapabilities);

		vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
		swapChainCreateInfo.setImageFormat(surfaceFormat.format)
			.setImageColorSpace(surfaceFormat.colorSpace)
			.setPresentMode(presentMode)
			.setImageExtent(extent)
			.setMinImageCount(std::clamp(swaphainDetails.m_surfaceCapabilities.minImageCount + 1
			, swaphainDetails.m_surfaceCapabilities.minImageCount
			, swaphainDetails.m_surfaceCapabilities.maxImageCount))
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setPreTransform(swaphainDetails.m_surfaceCapabilities.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setClipped(VK_TRUE);

		const QueueFamilies& families = m_physicalDeviceData.m_queueFamilies;

		if (families.m_graphicQueue != families.m_presentationQueue)
		{
			const std::array<uint32_t, 2> indicies = {
				static_cast<uint32_t>(families.m_graphicQueue),
				static_cast<uint32_t>(families.m_presentationQueue)
			};
			swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
				.setQueueFamilyIndexCount(2)
				.setPQueueFamilyIndices(indicies.data());
		}
		else
		{
			swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndexCount(0)
				.setPQueueFamilyIndices(nullptr);
		}
		swapChainCreateInfo.setOldSwapchain(VK_NULL_HANDLE)
			.setSurface(m_surface);

		m_swapchain = m_logicalDevice.createSwapchainKHR(swapChainCreateInfo);

		auto images = m_logicalDevice.getSwapchainImagesKHR(m_swapchain);
		for (auto& image : images)
		{
			m_swapchainImages.push_back({
				.m_image = image,
				.m_imageView = createImageView(image, surfaceFormat.format, vk::ImageAspectFlagBits::eColor)
				});
		}

		m_surfaceFormat = surfaceFormat;
		m_presentMode = presentMode;
		m_imageExtent = extent;
	}

	//-------------------------------------------------------------------------------------------------
	void createShaderModule()
	{
		constexpr auto C_V_SHADER = "shaders/hello.vert";
		constexpr auto C_F_SHADER = "shaders/hello.frag";

		auto curr_path = std::filesystem::current_path();
		auto root_path = curr_path.parent_path();

		auto full_vertex_shader_path = root_path / C_V_SHADER;
		auto full_fragment_shader_path = root_path / C_F_SHADER;

		std::cout << std::format("Vertex Shader path: '{}'", full_vertex_shader_path.generic_string()) << std::endl;
		std::cout << std::format("Fragment Shader path: '{}'", full_fragment_shader_path.generic_string()) << std::endl;

		std::vector<uint32_t> compiled_vertex_shader = compileShaderFromSource(
			readFile(full_vertex_shader_path.generic_string()), shaderc_vertex_shader, "test_vertex_shader"
		);
		std::vector<uint32_t> compiled_fragment_shader = compileShaderFromSource(
			readFile(full_fragment_shader_path.generic_string()), shaderc_fragment_shader, "test_fragment_shader"
		);

		std::cout << "Successfully compiled shaders" << std::endl;

		vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
		vertexShaderModuleCreateInfo.setCodeSize(compiled_vertex_shader.size() * sizeof(uint32_t))
			.setPCode(compiled_vertex_shader.data());

		m_vertexShaderModule = m_logicalDevice.createShaderModule(vertexShaderModuleCreateInfo);

		vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
		fragmentShaderModuleCreateInfo.setCodeSize(compiled_fragment_shader.size() * sizeof(uint32_t))
			.setPCode(compiled_fragment_shader.data());

		m_fragmentShaderModule = m_logicalDevice.createShaderModule(fragmentShaderModuleCreateInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createDescriptorSetLayout()
	{
		{
			vk::DescriptorSetLayoutBinding uniformLayoutBinding = {};
			uniformLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBinding(0)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex);

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindingCount(1)
				.setBindings(uniformLayoutBinding);
			{
				m_uniformsSetLayout = m_logicalDevice.createDescriptorSetLayout(createInfo);
			}
		}

		{
			vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setBinding(0)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vk::DescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.setBindingCount(1)
				.setBindings(samplerLayoutBinding);
			{
				m_texturesSetLayout = m_logicalDevice.createDescriptorSetLayout(createInfo);
			}
		}
	}

	//-------------------------------------------------------------------------------------------------
	void createPipeline()
	{
		vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
		vertexShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(m_vertexShaderModule)
			.setPName("main");

		vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
		fragmentShaderStageCreateInfo.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(m_fragmentShaderModule)
			.setPName("main");

		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

		//-- Create pipline now
		//-- Vertex input
		auto bindingDescriptions = getBindingDescription();
		auto atributeDescriptions = getAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.setVertexBindingDescriptions(bindingDescriptions)
			.setVertexAttributeDescriptions(atributeDescriptions);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
		inputAssemblyCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		vk::Viewport viewport = {};
		viewport.setX(0.0f)
			.setY(0.0f)
			.setWidth(m_imageExtent.width)
			.setHeight(m_imageExtent.height)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		vk::Rect2D scissor = {};
		scissor.setOffset(vk::Offset2D(0, 0))
			.setExtent(m_imageExtent);

		vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor);

		//-- DYNAMIC STATE
		const std::array<vk::DynamicState, 2> dynamicStateEnables = {
			vk::DynamicState::eViewport, //-- allow to resize in command buffer with vkCmdSetViewport
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.setDynamicStateCount(dynamicStateEnables.size())
			.setPDynamicStates(dynamicStateEnables.data());

		//-- RASTERIZER
		vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
		rasterizerCreateInfo.setDepthClampEnable(VK_FALSE)
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(vk::PolygonMode::eFill) //-- Try line
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(VK_FALSE);

		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.setSampleShadingEnable(VK_FALSE)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA)
			.setBlendEnable(VK_TRUE)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);

		//-- TODO: Check how to use it
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.setLogicOpEnable(VK_FALSE)
			.setLogicOp(vk::LogicOp::eCopy) // Optional
			.setAttachmentCount(1)
			.setPAttachments(&colorBlendAttachment);

		//-- We will need layout for uniforms
		std::array<vk::DescriptorSetLayout, 2> layouts = { m_uniformsSetLayout, m_texturesSetLayout };
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.setSetLayouts(layouts)
			.setSetLayoutCount(2);

		m_pipelineLayout = m_logicalDevice.createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		//-- Static part of pypline
		pipelineInfo.setStageCount(2)
			.setPStages(shaderStages.data())
			.setPVertexInputState(&vertexInputCreateInfo)
			.setPInputAssemblyState(&inputAssemblyCreateInfo)
			.setPViewportState(&viewportStateCreateInfo)
			.setPRasterizationState(&rasterizerCreateInfo)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(nullptr) // Optional
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(&dynamicStateCreateInfo)
			//-- Layout, describing uniforms
			.setLayout(m_pipelineLayout)
			//-- Render pass
			.setRenderPass(m_renderPass)
			.setSubpass(0);

		auto res = m_logicalDevice.createGraphicsPipelines(VK_NULL_HANDLE
			, 1
			, &pipelineInfo
			, nullptr
			, &m_graphicsPipeline);
	}

	//-------------------------------------------------------------------------------------------------
	void createRenderPass()
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.setFormat(m_surfaceFormat.format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass = {};
		subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachmentCount(1)
			.setPColorAttachments(&colorAttachmentRef);

		vk::SubpassDependency dependency = {};
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.setAttachmentCount(1)
			.setAttachments(colorAttachment)
			.setSubpassCount(1)
			.setSubpasses(subpass)
			.setDependencyCount(1)
			.setDependencies(dependency);

		m_renderPass = m_logicalDevice.createRenderPass(renderPassInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createFramebuffer()
	{
		m_swapChainFramebuffers.resize(m_swapchainImages.size());
		int i = 0;
		for (const auto& [_, imageView] : m_swapchainImages)
		{
			std::array<vk::ImageView, 1> attachments = { imageView };

			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.setRenderPass(m_renderPass)
				.setAttachmentCount(1)
				.setPAttachments(attachments.data())
				.setWidth(m_imageExtent.width)
				.setHeight(m_imageExtent.height)
				.setLayers(1);

			m_swapChainFramebuffers[i] = m_logicalDevice.createFramebuffer(framebufferInfo);
			++i;
		}
	}

	//-------------------------------------------------------------------------------------------------
	void createCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo = {};
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(m_physicalDeviceData.m_queueFamilies.m_graphicQueue);

		m_commandPool = m_logicalDevice.createCommandPool(poolInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createTextureSampler()
	{
		const auto props = m_physicalDevice.getProperties();
		const auto maxAnisotropy = props.limits.maxSamplerAnisotropy;
		vk::SamplerCreateInfo createInfo = {};
		createInfo.setMagFilter(vk::Filter::eNearest)
			.setMinFilter(vk::Filter::eNearest)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(maxAnisotropy)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eNearest)
			.setMipLodBias(0.0f)
			.setMinLod(0.0f)
			.setMaxLod(0.0f);

		m_textureSampler = m_logicalDevice.createSampler(createInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createUniformBuffers()
	{
		auto bufferSize = sizeof(UniformBufferObject);

		m_uniformBuffers.resize(C_MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMemory.resize(C_MAX_FRAMES_IN_FLIGHT);
		m_uniformBuffersMapped.resize(C_MAX_FRAMES_IN_FLIGHT);

		for (uint32_t i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			createBuffer(bufferSize,
				vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				m_uniformBuffers[i],
				m_uniformBuffersMemory[i]);

			auto res = m_logicalDevice.mapMemory(m_uniformBuffersMemory[i],
				0,
				bufferSize,
				{},
				&m_uniformBuffersMapped[i]);
		}
	}

	//-------------------------------------------------------------------------------------------------
	void createDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].setDescriptorCount(C_MAX_FRAMES_IN_FLIGHT)
			.setType(vk::DescriptorType::eUniformBuffer);
		poolSizes[1].setDescriptorCount(C_MAX_FRAMES_IN_FLIGHT)
			.setType(vk::DescriptorType::eCombinedImageSampler);

		vk::DescriptorPoolCreateInfo createInfo = {};
		createInfo.setPoolSizeCount(poolSizes.size())
			.setPoolSizes(poolSizes)
			.setMaxSets(C_MAX_FRAMES_IN_FLIGHT + m_maxTextures)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

		m_descriptorPool = m_logicalDevice.createDescriptorPool(createInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createDescriptorsSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(C_MAX_FRAMES_IN_FLIGHT, m_uniformsSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.setDescriptorPool(m_descriptorPool)
			.setSetLayouts(layouts);

		m_descriptorSets = m_logicalDevice.allocateDescriptorSets(allocInfo);
		for (uint32_t i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk::DescriptorBufferInfo bufferInfo = {};
			bufferInfo.setBuffer(m_uniformBuffers[i])
				.setOffset(0)
				.setRange(sizeof(UniformBufferObject));

			vk::WriteDescriptorSet descriptorsWrite = {};
			descriptorsWrite.setBufferInfo(bufferInfo)
				.setDstSet(m_descriptorSets[i])
				.setDstBinding(0)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1);

			m_logicalDevice.updateDescriptorSets(descriptorsWrite, {});
		}
	}

	//-------------------------------------------------------------------------------------------------
	void freeDescriptorSetFromPool(vk::DescriptorSet& descriptorSet)
	{
		m_logicalDevice.freeDescriptorSets(m_descriptorPool, descriptorSet);
	}

	//-------------------------------------------------------------------------------------------------
	void createCommandBuffer()
	{
		vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.setCommandBufferCount(C_MAX_FRAMES_IN_FLIGHT)
			.setCommandPool(m_commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		m_commandBuffers = m_logicalDevice.allocateCommandBuffers(commandBufferAllocateInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createSyncObjects()
	{
		m_imageAvailableSemaphores.reserve(C_MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.reserve(C_MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.reserve(C_MAX_FRAMES_IN_FLIGHT);

		vk::FenceCreateInfo fenceInfo = {};
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (int i = 0; i < C_MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_imageAvailableSemaphores.push_back(m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo()));
			m_renderFinishedSemaphores.push_back(m_logicalDevice.createSemaphore(vk::SemaphoreCreateInfo()));
			m_inFlightFences.push_back(m_logicalDevice.createFence(fenceInfo));
		}

	}

	//-------------------------------------------------------------------------------------------------
	void setupPhysicalDevice()
	{
		auto physDevices = m_vkInstance.enumeratePhysicalDevices();

		int bestScore = 0;
		for (vk::PhysicalDevice& device : physDevices)
		{
			//-- Looking for the best device
			PhysicalDeviceData physDeviceData = checkIfPhysicalDeviceSuitable(device);

			if (physDeviceData.m_score > bestScore)
			{
				bestScore = physDeviceData.m_score;
				m_physicalDevice = device;
				m_physicalDeviceData = physDeviceData;
			}
		}

		{
			vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
			std::cout << std::format("Chosen physical device name name: {}"
				, properties.deviceName.operator std::string()) << std::endl;
		}

		engineAssert(bestScore > 0, "No suitable videocard found");
		engineAssert(m_physicalDevice != VK_NULL_HANDLE, "Physical device is NULL");
		engineAssert(m_physicalDeviceData.m_queueFamilies.isValid(), "Queue families are not valid");
		//-- Maybe check if it supports specific modes we wanna see like mailbox & RGB8UNORM
		engineAssert(!m_physicalDeviceData.m_swapchainDetails.m_presentMode.empty(), "Present mode is empty");
		engineAssert(!m_physicalDeviceData.m_swapchainDetails.m_surfaceSupportedFormats.empty(), "Surface formats empty");
	}


	//-------------------------------------------------------------------------------------------------
	vk::DescriptorSet createTextureDescriptorSet(vk::Image& image, vk::ImageView& imageView)
	{
		vk::DescriptorSetAllocateInfo allocInfo = {};
		allocInfo.setDescriptorPool(m_descriptorPool)
			.setDescriptorSetCount(1)
			.setSetLayouts(m_texturesSetLayout);

		auto allocatedDescriptors = m_logicalDevice.allocateDescriptorSets(allocInfo);

		vk::DescriptorImageInfo imageInfo = {};
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(imageView)
			.setSampler(m_textureSampler);

		vk::WriteDescriptorSet descriptorsWrite = {};
		descriptorsWrite.setImageInfo(imageInfo)
			.setDstSet(allocatedDescriptors[0])
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);

		m_logicalDevice.updateDescriptorSets(descriptorsWrite, {});

		return allocatedDescriptors[0];
	}

	//-------------------------------------------------------------------------------------------------
	void recordCommandBuffer(vk::CommandBuffer commandBuffer
		, uint32_t imageIndex
		, const TexturedGeometryBatch& geometryBatch
		, const BatchIndecies& indicesBatch)
	{
		vk::CommandBufferBeginInfo cmdBBeginfo = {};
		commandBuffer.begin(cmdBBeginfo);

		vk::RenderPassBeginInfo renderPassInfo = {};
		vk::Rect2D renderArea = {};
		vk::ClearValue clearValue = { vk::ClearColorValue(0.2f, 0.2f, 0.2f, 1.0f) };
		renderArea.setOffset({ 0 , 0 }).setExtent(m_imageExtent);
		renderPassInfo.setRenderPass(m_renderPass)
			.setFramebuffer(m_swapChainFramebuffers[imageIndex])
			.setRenderArea(renderArea)
			.setClearValueCount(1)
			.setClearValues({ clearValue });

		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);

		vk::Viewport viewport = {};
		viewport.setX(0.0f).setY(0.0f)
			.setWidth(m_imageExtent.width)
			.setHeight(m_imageExtent.height)
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);
		commandBuffer.setViewport(0, viewport);

		vk::Rect2D scissor = {};
		scissor.setExtent(m_imageExtent).setOffset({ 0, 0 });
		commandBuffer.setScissor(0, scissor);

		for (uint32_t i = 0; i < geometryBatch.size(); ++i)
		{
			commandBuffer.bindVertexBuffers(0, geometryBatch[i].m_memory.m_buffer, { 0 });
			commandBuffer.bindIndexBuffer(indicesBatch[i].m_buffer, 0, vk::IndexType::eUint16);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
				m_pipelineLayout,
				0,
				{ m_descriptorSets[m_currFrame], geometryBatch[i].m_textureDescriptorSet },
				{});

			commandBuffer.drawIndexed(geometryBatch[i].m_spritesCount * 6, 1, 0, 0, 0);
		}

		m_imGuiIntegration.update(m_commandBuffers[m_currFrame], m_imGuiDrawCallbacks);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	//-------------------------------------------------------------------------------------------------
	//-- const char* here because glfw returns const char** as extentions list
	void checkExtensionsSupport(const std::vector<const char*>& instanceExtentionsAppNeed) const
	{
		auto extensionsProps = vk::enumerateInstanceExtensionProperties();
		for (const char* extension : instanceExtentionsAppNeed)
		{
			auto itRes = std::find_if(extensionsProps.begin(), extensionsProps.end(), [extension](const vk::ExtensionProperties& vkInst)
				{
					if (strcmp(vkInst.extensionName, extension) == 0)
					{
						return true;
					}
					return false;
				});
			engineAssert(itRes != extensionsProps.end(), std::format("Extention: {} is not supported", extension));
		}
	}

	//-------------------------------------------------------------------------------------------------
	void checkValidationLayerSupport(const std::vector<const char*>& validationLayerAppNeed) const
	{
		auto availableLayers = vk::enumerateInstanceLayerProperties();

		for (const char* layer : validationLayerAppNeed)
		{
			auto itRes = std::find_if(availableLayers.begin(), availableLayers.end(), [&](const vk::LayerProperties& vkInst)
				{
					if (strcmp(vkInst.layerName, layer) == 0)
					{
						return true;
					}
					return false;
				});
			engineAssert(itRes != availableLayers.end(), std::format("Layer: {} is not supported", layer));
		}
	}

	//-------------------------------------------------------------------------------------------------
	bool checkDeviceExtensionsSupport(const std::vector<const char*>& deviceExtentions, vk::PhysicalDevice physicalDevice) const
	{
		auto extensionsProps = physicalDevice.enumerateDeviceExtensionProperties();

		for (const char* extension : deviceExtentions)
		{
			auto itRes = std::ranges::find_if(extensionsProps, [&](const vk::ExtensionProperties& vkInst)
				{
					if (strcmp(vkInst.extensionName, extension) == 0)
					{
						return true;
					}
					return false;
				});
			if (itRes == extensionsProps.end())
			{
				return false;
			}
		}
		return true;
	}

	//-------------------------------------------------------------------------------------------------
	PhysicalDeviceData checkIfPhysicalDeviceSuitable(vk::PhysicalDevice device) const
	{
		PhysicalDeviceData data;

		vk::PhysicalDeviceProperties properties = device.getProperties();

		data.m_queueFamilies = checkQueueFamilies(device);
		if (data.m_queueFamilies.isValid())
		{
			data.m_score += 10;
		}

		//-- Device extensions
		if (checkDeviceExtensionsSupport(C_DEVICE_EXTENSIONS, device))
		{
			data.m_score += 10;
		}

		data.m_swapchainDetails = swapchainDetails(device);
		bool swapchainValid = !data.m_swapchainDetails.m_presentMode.empty()
			&& !data.m_swapchainDetails.m_surfaceSupportedFormats.empty();
		if (swapchainValid)
		{
			data.m_score += 10;
		}

		//-- Prefer discrete video card but only if it fits by queues reqs
		if (data.m_score > 0 && properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			data.m_score += 1;
		}

		return data;
	}

	//-------------------------------------------------------------------------------------------------
	QueueFamilies checkQueueFamilies(vk::PhysicalDevice device) const
	{
		QueueFamilies queueFamilies = {};
		std::vector<vk::QueueFamilyProperties> queueFamilyProps = device.getQueueFamilyProperties();

		int index = 0;
		for (auto& familyProp : queueFamilyProps)
		{
			if (familyProp.queueCount > 0)
			{
				//-- Check if queue has graphic family and queue count is not zero
				if (!!(familyProp.queueFlags & vk::QueueFlagBits::eGraphics))
				{
					//-- Remember that queue index to work with that later
					queueFamilies.m_graphicQueue = index;
				}

				vk::Bool32 presentSupport = device.getSurfaceSupportKHR(index, m_surface);
				if (presentSupport == VK_TRUE)
				{
					queueFamilies.m_presentationQueue = index;
				}
			}
			if (queueFamilies.isValid())
			{
				//-- Already found needed queues
				break;
			}

			++index;
		}

		return queueFamilies;
	}

	//-------------------------------------------------------------------------------------------------
	SwapChainDetails swapchainDetails(vk::PhysicalDevice device) const
	{
		SwapChainDetails details;

		details.m_surfaceCapabilities = device.getSurfaceCapabilitiesKHR(m_surface);
		details.m_surfaceSupportedFormats = device.getSurfaceFormatsKHR(m_surface);
		details.m_presentMode = device.getSurfacePresentModesKHR(m_surface);
		return details;
	}

	//-------------------------------------------------------------------------------------------------
	vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& supportedFormats)
	{
		for (const auto& availableFormat : supportedFormats)
		{
			if ((availableFormat.format == vk::Format::eB8G8R8A8Unorm || availableFormat.format == vk::Format::eR8G8B8A8Unorm)
				&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return availableFormat;
			}
		}
		return supportedFormats[0];
	}

	//-------------------------------------------------------------------------------------------------
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
	{
		//-- Looking for MAILBOX (best for games)
		//-- Tripple buffering, update on vertical blank and no tearing can be observed
		for (const auto& mode : availablePresentModes)
		{
			if (mode == vk::PresentModeKHR::eMailbox)
			{
				return mode;
			}
		}
		//-- If there's no MAILBOX use FIFO (always available by specification)
		//-- "This is the only value of presentMode that is required to be supported."
		return vk::PresentModeKHR::eFifo;
	}

	//-------------------------------------------------------------------------------------------------
	vk::Extent2D chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
	{
		//-- If currExtent is max - than it will be handeled by surface
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		int width = 0;
		int height = 0;;
		glfwGetFramebufferSize(m_window, &width, &height);
		vk::Extent2D ret = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		ret.width = std::clamp(ret.width
			, capabilities.minImageExtent.width
			, capabilities.maxImageExtent.width);
		ret.height = std::clamp(ret.height
			, capabilities.minImageExtent.height
			, capabilities.maxImageExtent.height);

		return ret;
	}

	//-------------------------------------------------------------------------------------------------
	vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
	{
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(format);

		createInfo.subresourceRange.setAspectMask(aspectFlags)
			.setBaseMipLevel(0)
			.setLayerCount(1)
			.setLevelCount(1)
			.setBaseArrayLayer(0);

		return m_logicalDevice.createImageView(createInfo);
	}

	//-------------------------------------------------------------------------------------------------
	void createBuffer(vk::DeviceSize size
		, vk::BufferUsageFlags usageFlags
		, vk::MemoryPropertyFlags memPropFlags
		, vk::Buffer& buffer
		, vk::DeviceMemory& deviceMemory)
	{
		vk::BufferCreateInfo bufferInfo = {};
		bufferInfo.setSize(size)
			.setUsage(usageFlags)
			.setSharingMode(vk::SharingMode::eExclusive);

		buffer = m_logicalDevice.createBuffer(bufferInfo);

		vk::MemoryRequirements memReq = m_logicalDevice.getBufferMemoryRequirements(buffer);
		uint32_t memType = findMemoryType(memReq.memoryTypeBits, memPropFlags);

		vk::MemoryAllocateInfo allocInfo = {};
		allocInfo.setMemoryTypeIndex(memType)
			.setAllocationSize(memReq.size);

		deviceMemory = m_logicalDevice.allocateMemory(allocInfo);
		m_logicalDevice.bindBufferMemory(buffer, deviceMemory, 0);
	}

	//-------------------------------------------------------------------------------------------------
	void transitionImage(vk::Image image
		, vk::Format format
		, vk::ImageLayout oldLayout
		, vk::ImageLayout newLayout)
	{
		auto commandBuffer = beginSingleTimeCommands();

		vk::AccessFlags srcAccess = {};
		vk::AccessFlags dstAccess = {};
		vk::PipelineStageFlags srcStage = {};
		vk::PipelineStageFlags dstStage = {};

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			srcAccess = vk::AccessFlagBits::eNone;
			dstAccess = vk::AccessFlagBits::eTransferWrite;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else
		{
			srcAccess = vk::AccessFlagBits::eTransferWrite;
			dstAccess = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}

		vk::ImageMemoryBarrier barrier = {};
		barrier.setOldLayout(oldLayout)
			.setNewLayout(newLayout)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setImage(image)
			.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 })
			.setSrcAccessMask(srcAccess)
			.setDstAccessMask(dstAccess);

		commandBuffer.pipelineBarrier(srcStage
			, dstStage
			, vk::DependencyFlagBits::eByRegion
			, {}
			, {}
		, barrier);

		endSingleTimeCommand(commandBuffer);
	}

	//-------------------------------------------------------------------------------------------------
	void copyBufferToImage(vk::Buffer buffer
		, vk::Image image
		, uint32_t width
		, uint32_t height)
	{
		auto commandBuffer = beginSingleTimeCommands();

		vk::BufferImageCopy region = {};
		region.setBufferOffset(0)
			.setBufferRowLength(0)
			.setBufferImageHeight(0)
			.setImageSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 })
			.setImageOffset({ 0, 0, 0 })
			.setImageExtent({ width, height, 1 });

		commandBuffer.copyBufferToImage(buffer
			, image
			, vk::ImageLayout::eTransferDstOptimal
			, { region });

		endSingleTimeCommand(commandBuffer);
	}

	//-------------------------------------------------------------------------------------------------
	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
	{
		auto commandBuffer = beginSingleTimeCommands();

		vk::BufferCopy bufferCopy = {};
		bufferCopy.setSize(size)
			.setSrcOffset(0)
			.setDstOffset(0);

		commandBuffer.copyBuffer(srcBuffer, dstBuffer, { bufferCopy });

		endSingleTimeCommand(commandBuffer);
	}

	//-------------------------------------------------------------------------------------------------
	vk::CommandBuffer beginSingleTimeCommands()
	{
		vk::CommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(m_commandPool)
			.setCommandBufferCount(1);

		vk::CommandBuffer commandBuffer = {};
		auto commandBuffers = m_logicalDevice.allocateCommandBuffers(allocateInfo);
		commandBuffer = *commandBuffers.begin();

		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		commandBuffer.begin(beginInfo);
		return commandBuffer;
	}

	//-------------------------------------------------------------------------------------------------
	void endSingleTimeCommand(vk::CommandBuffer commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo = {};
		submitInfo.setCommandBufferCount(1)
			.setCommandBuffers({ commandBuffer });

		m_queues.m_graphicQueue.submit(submitInfo);
		m_queues.m_graphicQueue.waitIdle();

		m_logicalDevice.freeCommandBuffers(m_commandPool, { commandBuffer });
	}

	//-------------------------------------------------------------------------------------------------
	void cleanupSwapchain()
	{
		for (auto& framebuffer : m_swapChainFramebuffers)
		{
			m_logicalDevice.destroyFramebuffer(framebuffer);
		}
		for (auto& [_, imageView] : m_swapchainImages)
		{
			m_logicalDevice.destroyImageView(imageView);
		}
		m_logicalDevice.destroySwapchainKHR(m_swapchain);
	}

	//-------------------------------------------------------------------------------------------------
	void setMaxTextures(uint32_t maxPossibleTextures)
	{
		m_maxTextures = maxPossibleTextures;
	}

	//-- Info getters
	//-------------------------------------------------------------------------------------------------
	uint32_t apiVersion() const { return m_apiVersion; }
	//-------------------------------------------------------------------------------------------------
	uint32_t getGraphicQueueFamily() const { return m_physicalDeviceData.m_queueFamilies.m_graphicQueue; }
	//-------------------------------------------------------------------------------------------------
	uint32_t minImageCount() const { return m_swapchainImages.size(); }
	//-------------------------------------------------------------------------------------------------
	uint32_t imageCount() const { return m_swapchainImages.size(); }
	//-------------------------------------------------------------------------------------------------
	vk::Instance instance() const { return m_vkInstance; }
	//-------------------------------------------------------------------------------------------------
	vk::PhysicalDevice physicalDevice() const { return m_physicalDevice; }
	//-------------------------------------------------------------------------------------------------
	vk::Device device() const { return m_logicalDevice; }
	//-------------------------------------------------------------------------------------------------
	vk::Queue graphicQueue() const { return m_queues.m_graphicQueue; }
	//-------------------------------------------------------------------------------------------------
	vk::DescriptorPool descriptorPool() const { return m_descriptorPool; }
	//-------------------------------------------------------------------------------------------------
	vk::RenderPass renderPass() const { return m_renderPass; }

	void setImGuiDrawCallbacks(std::vector<RendererManager::ImGuiDrawCallback>& imGuiDrawCallbacks)
	{
		m_imGuiDrawCallbacks = imGuiDrawCallbacks;
	}

private:
	const std::vector<const char*> C_DEVICE_EXTENSIONS
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

private:
	PhysicalDeviceData				m_physicalDeviceData;
	std::vector<SwapchainImage>		m_swapchainImages;
	Queues							m_queues;
	GLFWwindow*						m_window = nullptr;
	vk::Instance					m_vkInstance;
	vk::PhysicalDevice				m_physicalDevice;
	vk::Device						m_logicalDevice;
	vk::SurfaceKHR					m_surface;
	vk::SwapchainKHR				m_swapchain;
	vk::ShaderModule				m_vertexShaderModule;
	vk::ShaderModule				m_fragmentShaderModule;
	vk::RenderPass					m_renderPass;
	vk::DescriptorSetLayout			m_uniformsSetLayout;
	vk::DescriptorSetLayout			m_texturesSetLayout;
	vk::DescriptorPool				m_descriptorPool;
	std::vector<vk::DescriptorSet>	m_descriptorSets;
	vk::PipelineLayout				m_pipelineLayout;
	vk::Pipeline					m_graphicsPipeline;

	std::vector<vk::Buffer>			m_uniformBuffers;
	std::vector<vk::DeviceMemory>	m_uniformBuffersMemory;
	std::vector<void*>				m_uniformBuffersMapped;

	vk::Sampler						m_textureSampler;

	vk::CommandPool					m_commandPool;
	std::vector<vk::CommandBuffer>	m_commandBuffers;
	std::vector<vk::Framebuffer>	m_swapChainFramebuffers;

	vk::SurfaceFormatKHR			m_surfaceFormat;
	vk::Extent2D					m_imageExtent;
	vk::PresentModeKHR				m_presentMode = {};

	std::vector<vk::Semaphore>		m_imageAvailableSemaphores;
	std::vector<vk::Semaphore>		m_renderFinishedSemaphores;
	std::vector<vk::Fence>			m_inFlightFences;

	ImGuiIntegration								m_imGuiIntegration;
	std::vector<RendererManager::ImGuiDrawCallback>	m_imGuiDrawCallbacks;

	uint32_t						m_apiVersion = 0;
	uint32_t						m_currFrame = 0;
	uint32_t						m_currImageIndex = 0;
	uint32_t						m_maxTextures = 200;

	bool							m_framebufferResized = false;
#ifdef NDEBUG
	const bool						m_enableValidationLayers = false;
#else
	const bool						m_enableValidationLayers = true;
#endif
};
