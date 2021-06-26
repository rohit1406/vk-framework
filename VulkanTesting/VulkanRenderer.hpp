//
//  VulkanRenderer.hpp
//  VulkanTesting
//
//  Created by Apple on 18/05/21.
//

#ifndef VulkanRenderer_hpp
#define VulkanRenderer_hpp

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// This header defines a set of standard exceptions that both the library and programs can use to report common errors
#include <stdexcept>
#include <vector>
#include <set>
#include <algorithm>                // used in choosing swapExtent
#include <array>
#include "stb_image.h"              // For image loading
#include <stdlib.h>
#include <stdio.h>
#include <malloc/malloc.h>
#include "Utilities.h"
#include "Mesh.hpp"
#include "MeshModel.hpp"

#include <unistd.h>

class VulkanRenderer{
public:
    VulkanRenderer();
    int init(GLFWwindow *window);
    int createMeshModel(std::string modelFile);
    void updateModel(int modelId, glm::mat4 newModel);
    void draw();
    void cleanUp();
    ~VulkanRenderer();
    
private:
    GLFWwindow *window;
    
    int currentFrame = 0;
    
    // Scene Objects
    std::vector<MeshModel> modelList;
    
    // Scene settings
    struct UBOViewProjection{
        glm::mat4 projection;
        glm::mat4 view;
    }uboViewProjection;
    
    
    // Vulkan components
    // - Main
    VkInstance instance;
    struct{
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } mainDevice;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    
    std::vector<SwapchainImage> swapchainImages;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    
    std::vector<VkImage> colorBufferImage;
    std::vector<VkDeviceMemory> colorBufferImageMemory;
    std::vector<VkImageView> colorBufferImageView;
    
    std::vector<VkImage> depthBufferImage;
    std::vector<VkDeviceMemory> depthBufferImageMemory;
    std::vector<VkImageView> depthBufferImageView;
    
    VkFormat depthBufferFormat;
    
    VkSampler textureSampler;
    
    // - Descriptors
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout samplerSetLayout;
    VkDescriptorSetLayout inputSetLayout;
    VkPushConstantRange pushConstantRange;
    
    VkDescriptorPool descriptorPool;
    VkDescriptorPool samplerDescriptorPool;
    VkDescriptorPool inputDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDescriptorSet> samplerDescriptorSets;
    std::vector<VkDescriptorSet> inputDescriptorSets;
    
    std::vector<VkBuffer> vpUniformBuffer;
    std::vector<VkDeviceMemory> vpUniformBufferMemory;
    std::vector<VkBuffer> modelDUniformBuffer;
    std::vector<VkDeviceMemory> modelDUniformBufferMemory;
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    //VkDeviceSize minUniformBufferOffset;
    //size_t modelUniformAlignment;
    //UBOModel *modelTransferSpace;
    
    // - Assets
    
    std::vector<VkImage> textureImages;
    std::vector<VkDeviceMemory> textureImageMemory;
    std::vector<VkImageView> textureImageView;
    
    // - Pipeline
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkPipeline secondPipeline;
    VkPipelineLayout secondPipelineLayout;
    
    VkRenderPass renderpass;
    
    // - Pools
    VkCommandPool graphicsCommandPool;
    
    // - Utility
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    
    // -- Synchronization
    std::vector<VkSemaphore> imageAvailable;
    std::vector<VkSemaphore> renderFinished;
    std::vector<VkFence> drawFences;
    
    // Vulkan functions
    // - Create functions
    void createInstance();
    void createLogicalDevice();
    void createSurface();
    void createSwapChain();
    void createRenderPass();
    void createDescriptorSetLayout();
    void createPushConstantRange();
    void createGraphicsPipeline();
    void createColorBufferImage();
    void createDepthBufferImage();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSynchronization();
    void createTextureSampler();
    
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createInputDescriptorSets();
    
    void updateUniformBuffers(uint32_t imageIndex);
    
    // - Allocate functions
    void allocateDynamicBufferTransferSpace();
    
    // - Record functions
    void recordCommands(uint32_t currentImage);
    
    // - Get functions
    void getPhysicalDevice();
    
    // - Support functions
    // -- Checker functions
    bool checkInstanceExtensionsSupport(std::vector<const char*> *checkExtensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool doCheckDeviceSuitable(VkPhysicalDevice device);
    
    // -- Getter functions
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
    
    // -- Choose functions
    VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);
    VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
    
    // -- Create functions
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkShaderModule createShaderModule(const std::vector<char> &code);
    
    int createTextureImage(std::string fileName);
    int createTexture(std::string fileName);
    int createTextureDescriptor(VkImageView textureImage);
    
    // -- Loader Functions
    stbi_uc* loadTextureFile(std::string fileName, int *width, int *height, VkDeviceSize *imageSize);
};
#endif /* VulkanRenderer_hpp */
