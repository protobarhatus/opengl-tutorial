#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include "linear_algebra.h"
#include "primitives.h"
#include "Scene.h"
#include "GLSL_structures.h"
#include "ComposedObject.h"
#include "objects_fabric.h"
#include "parser.h"
#include "gl_utils.h"
#include "GLSL_structures.h"
#include <array>


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    inline bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};




class VulkanApp {
    const uint32_t WIDTH = 1000;
    const uint32_t HEIGHT = 1000;
    const uint32_t INTERSECTION_STACK_SIZE = 100;
    const uint32_t BATCHES_COUNT = 1;
    //под нее надо так же множить отдельные буфера для компьют шейдера и отдельные параметры камеры и тп, нафиг
    const int MAX_FRAMES_IN_FLIGHT = 1;
    //без учета юниформ буфера
    int BUFFERS_NUM = 7;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        //все снизу - FOR RAYTRACING
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    void setScene(const GlslSceneMemory& scene);
public:
    void run();
    
    void setScene(const std::unique_ptr<Object>& obj);
private:

    Vector<3> camera = { 0, -5, 0 };

    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkPipelineLayout computeLayout;
    VkPipeline computePipeline;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkCommandPool computeCommandPool;
    std::vector<VkCommandBuffer> computeCommandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;


    uint32_t currentFrame = 0;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    VkImageView textureImageView;
    VkSampler textureSampler;

    VkDescriptorSetLayout descriptorSetLayout;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkDescriptorPool computeDescriptorPool;
    std::vector<VkDescriptorSet> computeDescriptorSets;

    std::vector<VkBuffer> compute_buffers;
    std::vector<VkDeviceMemory> compute_memory;
    std::vector<void*> compute_mapped;

    GlslSceneMemory scene;
    std::unique_ptr<Object> scene_object;

    
    std::array<VkBuffer, 4> intersection_stack_buffers;
    std::array<VkDeviceMemory, 4> intersection_stack_buffers_memory;

    bool framebufferResized = false;

   

    void initWindow();

    void initVulkan();

    void mainLoop();

    void cleanup();

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSwapChain();

    void createImageViews();
    void createRenderPass();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();

    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createSyncObjects();

    void drawFrame();

    void cleanupSwapChain();

    void recreateSwapChain();

    void createDescriptorSetLayout();

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createDescriptorPool();

    void createDescriptorSets();

    void doComputeShadersSetup();
    void createSsbos();
    int findComputeQueue(VkPhysicalDevice device);
    void loadComputeShader();
    void createDescriptorSetsLayoutForComputeShader();
    void createDescriptorPoolForCompute();
    void createDescriptorSetsForCompute();

    void setUpCommandPoolForCompute();
    void dispatchCompute();

    void createTextureImage();
    void createTextureImageView();
    VkImageView createImageView(VkImage image, VkFormat format);
    void createTextureSampler();
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);



    VkDescriptorSetLayout rtxDescriptorSetLayout;
    VkDescriptorPool rtxDescriptorPool;
    std::vector<VkDescriptorSet> rtxDescriptorSets;

    VkPipelineLayout raytrace_pipeline_layout;
    VkPipeline raytrace_pipeline;
    VkCommandPool rtxCommandPool;
    VkCommandBuffer rtxCommandBuffer;
    //это временное решение
    VkFence rtxFence;
    VkAccelerationStructureKHR tlas;
    VkStridedDeviceAddressRegionKHR raygen_shader_binding_table;
    VkStridedDeviceAddressRegionKHR inter_shader_binding_table;
    VkStridedDeviceAddressRegionKHR miss_shader_binding_table;

    void doRaytraceSetup();
    void createRaytracePipeline();
    void createRaytraceCommandBuffer();
    void createAccelerationStructure();
    void makeShaderBindingTable();
    void createDescriptorPoolAndSetForRaytrace();
    void prepareCommandBufferForRtx();
    VkAccelerationStructureKHR createBottomLevelAccelerationStructure();
};