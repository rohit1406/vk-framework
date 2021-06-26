//
//  VulkanRenderer.cpp
//  VulkanTesting
//
//  Created by Apple on 18/05/21.
//

#include "VulkanRenderer.hpp"

// constructor
VulkanRenderer::VulkanRenderer(){
    
}

// vulkan initialization
int VulkanRenderer::init(GLFWwindow *newWindow){
    printf(">>> Welcome to Init!\n");
    this->window = newWindow;
    try{
        createInstance();
        printf(">>> createInstance!\n");
        createSurface();
        printf(">>> createSurface!\n");
        getPhysicalDevice();
        printf(">>> getPhysicalDevice!\n");
        createLogicalDevice();
        printf(">>> createLogicalDevice!\n");
        createSwapChain();
        printf(">>> createSwapChain!\n");
        createDepthBufferImage();
        printf(">>> createDepthBufferImage!\n");
        createRenderPass();
        printf(">>> createRenderPass!\n");
        createDescriptorSetLayout();
        printf(">>> createDescriptorSetLayout!\n");
        createPushConstantRange();
        printf(">>> createPushConstantRange!\n");
        createGraphicsPipeline();
        printf(">>> createGraphicsPipeline!\n");
        createFramebuffers();
        printf(">>> createFramebuffers!\n");
        createCommandPool();
        printf(">>> createCommandPool!\n");
        createCommandBuffers();
        printf(">>> createCommandBuffers!\n");
        createTextureSampler();
        printf(">>> createTextureSampler!\n");
        // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
        //allocateDynamicBufferTransferSpace();
        //printf(">>> allocateDynamicBufferTransferSpace!\n");
        createUniformBuffers();
        printf(">>> createUniformBuffers!\n");
        createDescriptorPool();
        printf(">>> createDescriptorPool!\n");
        createDescriptorSets();
        printf(">>> createDescriptorSets!\n");
        createSynchronization();
        printf(">>> createSynchronization!\n");
        
        
        uboViewProjection.projection = glm::perspective(glm::radians(45.0f), (float) swapchainExtent.width / (float) swapchainExtent.height, 0.1f, 100.0f);
        uboViewProjection.view = glm::lookAt(
                               glm::vec3(10.0f, 0.0f, 20.0f), // eye - where the camera is
                               glm::vec3(0.0f, 0.0f, -2.0f), // target - where the camera is looking at
                               glm::vec3(0.0f, 1.0f, 0.0f)  // up vector
                               );
        uboViewProjection.projection[1][1] *= -1;

        // Create a default "no texture" texture
        createTexture("plain.png");
        printf(">>> Welcome to Vulkan, Rohit!\n");
    }catch(const std::runtime_error &e){
        printf(">>> Error Ocurred!\n");
        printf("ERROR: %s\n", e.what());
        return EXIT_FAILURE;
    }
    return 0;
}

// destructor
VulkanRenderer::~VulkanRenderer(){
    
}

// initialize vulkan
void VulkanRenderer::createInstance(){
    // information about the application itself
    // most of the data here doesn't affect the program and is for a developer convenience
    // e.g. debugging, output to error logs, etc
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName ="Vulkan App";                 // custom name of the application
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // custom version of the application
    appInfo.pEngineName = "No Engine";                      // custom engine name
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // custom engine version
    appInfo.apiVersion = VK_API_VERSION_1_0;                // The Vulkan version
    
    // creation information for a VkInstance (Vulkan Instance)
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    // create list to hold instance extensions
    std::vector<const char*> instanceExtensions = std::vector<const char*>();
    
    // set up extensions instance will use
    uint32_t glfwExtensionCount = 0;            // GLFW may require multiple extensions
    const char** glfwExtensions;                // Extensions passed as array of cstrings, so need pointer (the array) of pointer (the cstring)
    
    // Get GLFW extentions
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    // add GLFW extensions to the list of extensions
    for(size_t i=0; i<glfwExtensionCount; i++){
        instanceExtensions.push_back(glfwExtensions[i]);
    }
    
    // Check instance extensions supported...
    if(!checkInstanceExtensionsSupport(&instanceExtensions)){
        throw std::runtime_error("VkInstance does not support required extensions!");
    }
    
    createInfo.enabledExtensionCount = static_cast<u_int32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();
    
    // TODO: Set up validation layers that instance will use
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    
    // create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Vulkan Instance!");
    }
}

bool VulkanRenderer::checkInstanceExtensionsSupport(std::vector<const char *> *checkExtensions){
    // Need to get number of extensions to create array of correct size to hold extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    // Create list of vkExtensionProperties using count
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    
    // Check if given extensions are in the list of available extensions
    for(const auto &checkExtension: *checkExtensions){
        bool hasExtension = false;
        for(const auto &extension: extensions){
            if(strcmp(checkExtension, extension.extensionName) == 0){
                hasExtension = true;
                break;
            }
        }
        
        if(!hasExtension){
            return false;
        }
    }
    
    return true;
}

void VulkanRenderer::cleanUp(){
    // Wait until no actions being run on device before destroying
    vkDeviceWaitIdle(mainDevice.logicalDevice);
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    //aligned_free(modelTransferSpace);
    
    for(size_t i=0; i<modelList.size(); i++){
        modelList[i].destroyMeshModel();
    }
    
    vkDestroyDescriptorPool(mainDevice.logicalDevice, samplerDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, samplerSetLayout, nullptr);
    
    vkDestroySampler(mainDevice.logicalDevice, textureSampler, nullptr);
    
    for(size_t i=0; i<textureImages.size(); i++){
        vkDestroyImageView(mainDevice.logicalDevice, textureImageView[i], nullptr);
        vkDestroyImage(mainDevice.logicalDevice, textureImages[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, textureImageMemory[i], nullptr);
    }
    
    vkDestroyImageView(mainDevice.logicalDevice, depthBufferImageView, nullptr);
    vkDestroyImage(mainDevice.logicalDevice, depthBufferImage, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, depthBufferImageMemory, nullptr);
    
    vkDestroyDescriptorPool(mainDevice.logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(mainDevice.logicalDevice, descriptorSetLayout, nullptr);
    for(size_t i=0; i<swapchainImages.size();i++){
        vkDestroyBuffer(mainDevice.logicalDevice, vpUniformBuffer[i], nullptr);
        vkFreeMemory(mainDevice.logicalDevice, vpUniformBufferMemory[i], nullptr);
        //vkDestroyBuffer(mainDevice.logicalDevice, modelDUniformBuffer[i], nullptr);
        //vkFreeMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[i], nullptr);
    }
    
    for(size_t i=0; i<MAX_FRAME_DRAWS; i++){
        vkDestroySemaphore(mainDevice.logicalDevice, renderFinished[i], nullptr);
        vkDestroySemaphore(mainDevice.logicalDevice, imageAvailable[i], nullptr);
        vkDestroyFence(mainDevice.logicalDevice, drawFences[i], nullptr);
    }
    vkDestroyCommandPool(mainDevice.logicalDevice, graphicsCommandPool, nullptr);
    for(auto framebuffer: swapchainFramebuffers){
        vkDestroyFramebuffer(mainDevice.logicalDevice, framebuffer, nullptr);
    }
    vkDestroyPipeline(mainDevice.logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(mainDevice.logicalDevice, renderpass, nullptr);
    for(auto image: swapchainImages){
        vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
    }
    vkDestroySwapchainKHR(mainDevice.logicalDevice, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(mainDevice.logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::getPhysicalDevice(){
    // Enumerate physical devices that vkInstance can access
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    // if no devices are available, then none support Vulkan!
    if(deviceCount == 0){
        throw std::runtime_error("Can't find GPU instance that supports Vulkan!");
    }
    
    // Get list of physical devices
    std::vector<VkPhysicalDevice> deviceList(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());
    
    // TEMP: just pick the first device
    // mainDevice.physicalDevice = deviceList[0];
    for(const auto &device: deviceList){
        if(doCheckDeviceSuitable(device)){
            mainDevice.physicalDevice = device;
            break;
        }
    }
    
    // Get properties of our new device
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(mainDevice.physicalDevice, &deviceProperties);
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    //minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
}

bool VulkanRenderer::doCheckDeviceSuitable(VkPhysicalDevice device){
    /*
    // Information about the device itself (ID, name, type, vendor, etc)
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
     */
    // Information about what the device can do (geo shader, tess shader, wide lines, etc)
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    QueueFamilyIndices indices = getQueueFamilies(device);
    
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainValid = false;
    if(extensionsSupported){
        SwapChainDetails swapChainDetails = getSwapChainDetails(device);
        swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
    }
    return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;
    
    // Get all queue family info for the given device
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());
    
    // Go through each queue family and check if it has at least 1 of the required types of queue
    int i=0;
    for(const auto &queueFamily: queueFamilyList){
        // First check if queue family has at least 1 queue in that family (could have no queues)
        // Queue can be multiple types defined through bitfield. Need to bitwise AND with VK_QUEUE_*_BIT to check if has required type
        if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i; // if queue family is valid then get index
        }
        
        // Check if Queue Family supports presentation
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport);
        // Check if queue is presentation type (can be both graphics and presentation)
        if(queueFamily.queueCount > 0 && presentationSupport){
            indices.presentationFamily = i;
        }
        
        // Check if queue family indices are in a valid state, stop searching if so
        if(indices.isValid()){
            break;
        }
        i++;
    }
    
    return indices;
}

void VulkanRenderer::createLogicalDevice(){
    // Get the queue family indices for the choosen Physical Device
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);
    
    // Vector for queue creation information, and set for family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };
    
    // Queues the logical device needs to create and info to do so
    for(int queueFamilyIndex: queueFamilyIndices){
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;      // The index of the family to create the index from
        queueCreateInfo.queueCount = 1;                                 // Number of queues to create
    
        float priotity = 1.0f;
        queueCreateInfo.pQueuePriorities=&priotity;                     // Vulkan needs to know how to handle multiple queues, so decide priorities (1 = Highest Priority)
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Information to create logical device (sometime called "device")
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());     // Number of queue create infos
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();                               // List of queue create infos so device can create required queues
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());    // Number of enabled logical device extensions
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();                         // List of enabled logical device extensions
    
    // Physical device features the logical device will be using
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;                     // Enabling Anisotropy
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;            // Physical device features logical device will use
    
    // Create the logical device for the given physical device
    VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo, nullptr, &mainDevice.logicalDevice);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create the logical device!");
    }
    
    // Queues are created at the same time as the device...
    // so we want to handle to queues
    // From given logical device, of give queue family, of given queue index (0 since only one queue), place reference in give vkQueue
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void VulkanRenderer::createSurface(){
    // Create surface (creates a surface create info struct, run the create surface function, returns result)
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a surface!");
    }
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device){
    // Get device extensions count
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    
    // if no extensions found, then return failure
    if(extensionCount == 0){
        return false;
    }
    
    // Populate list of extensions
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
    
    // Check for extension
    for(const auto &deviceExtension: deviceExtensions){
        bool hasExtension = false;
        
        for(const auto &extension: extensions){
            if(strcmp(deviceExtension, extension.extensionName) == 0){
                hasExtension = true;
                break;
            }
        }
        
        if(!hasExtension){
            return false;
        }
    }
    
    return true;
}

SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device){
    SwapChainDetails swapChainDetails;
    
    // -- CAPABILITIES --
    // Get the surface capabilities for the give surface on the give physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainDetails.surfaceCapabilities);
    
    // -- FORMATS --
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    
    // If formats returned, get list of formats
    if(formatCount != 0){
        swapChainDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
    }
    
    // -- PRESENTATION MODES --
    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
    
    // If presentation modes count returned, get list of presentation modes
    if(presentationCount != 0){
        swapChainDetails.presentationModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainDetails.presentationModes.data());
    }
    return swapChainDetails;
}

void VulkanRenderer::createSwapChain(){
    // Get swap chain details so we can pick best settings
    SwapChainDetails swapChainDetails = getSwapChainDetails(mainDevice.physicalDevice);
    
    // Find optimal surface values for our swap chain
    // 1. CHOOSE BEST SURFACE FORMAT
    VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
    // 2. CHOOSE BEST PRESENTATION MODE
    VkPresentModeKHR presentMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
    // 3. CHOOSE SWAP CHAIN IMAGE RESOLUTION
    VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);
    
    // How many images are in the swap chain? Get 1 more than the minimum to allow tripple buffering
    uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;
    
    // If imageCount higher that max, then clamp down to max
    // If 0, then limitless
    if(swapChainDetails.surfaceCapabilities.maxImageCount > 0
            && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount){
        imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
    }
    
    // Creation information for swap chain
    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = surface;                                                      // Swapchain surface
    swapChainCreateInfo.imageFormat = surfaceFormat.format;                                     // Swapchain format
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;                             // Swapchain color space
    swapChainCreateInfo.presentMode = presentMode;                                              // Swapchain presentation mode
    swapChainCreateInfo.imageExtent = extent;                                                   // Swapchain image extents
    swapChainCreateInfo.minImageCount = imageCount;                                             // Minimum images in swapchain
    swapChainCreateInfo.imageArrayLayers = 1;                                                   // Number of layers for each image in chain
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;                       // What attachments images will be used as
    swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;   // Transform to perform on swap chain images
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                     // How to handle blending images with external graphics (e.g. other windows)
    swapChainCreateInfo.clipped = VK_TRUE;                                                      // Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)
    
    // Get Queue Famiy Indices
    QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);
    
    // If Graphics and Presentation families are different, then swapchain must let images be shared between families
    if(indices.graphicsFamily != indices.presentationFamily){
        
        // Queues to share between
        uint32_t queueFamilyIndices[] = {
            (uint32_t)indices.graphicsFamily,
            (uint32_t)indices.presentationFamily
        };
        
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;      // Image share handling
        swapChainCreateInfo.queueFamilyIndexCount = 2;                          // Number of queues to share images between
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;           // Array of queues to share between
    }else{
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    
    // If old swap chain been destroyed and this one replaces it, then link old one to quickly handover responsibilities
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    
    // Create Swapchain
    VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice, &swapChainCreateInfo, nullptr, &swapchain);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Swapchain!");
    }
    
    // Store for later reference
    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
    
    // Get swap chain images (first count, then values)
    uint32_t swapchainImageCount;
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapchainImageCount, nullptr);
    std::vector<VkImage> images(swapchainImageCount);
    vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapchain, &swapchainImageCount, images.data());
    
    for(VkImage image: images){
        // Store image handle
        SwapchainImage swapchainImage = {};
        swapchainImage.image = image;
        
        // CREATE IMAGE VIEW HERE
        swapchainImage.imageView = createImageView(image, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        
        // Add to swapchain image list
        swapchainImages.push_back(swapchainImage);
    }
}

// Best format is subjective, but ours will be:
// format       :   VK_FORMAT_R8G8B8A8_UNORM (VK_FORMAT_B8G8R8A8_UNORM as backup)
// colorSpace   :   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats){
    // If only one format available and is underfined, then this means ALL formats are available (no restrictions)
    if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
        return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }
    
    // If restricted, search for optimal format
    for(const auto &format : formats){
        if( (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
           && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            return format;
        }
    }
    
    // If can't find optimal format, then just return first format
    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes){
    // Look for Mailbox presentation mode
    for(const auto &presentationMode: presentationModes){
        if(presentationMode == VK_PRESENT_MODE_MAILBOX_KHR){
            return presentationMode;
        }
    }
    
    // If can't find, use FIFO as Vulkan spec says it must be present
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities){
    // if current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
    if(surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return surfaceCapabilities.currentExtent;
    }else{
        // If value can vary, need to set it manually
        
        // Get window size
        int width,height;
        glfwGetFramebufferSize(window, &width, &height);
        
        // Create new extent using window size
        VkExtent2D newExtent = {};
        newExtent.width = static_cast<uint32_t>(width);
        newExtent.height = static_cast<uint32_t>(height);
        
        // Surface also defines max and min, so make sure within boundaries by clamping value
        newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
        newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));
        
        return newExtent;
    }
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags){
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;                                   // Image to create view for
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                // Type of image (1D, 2D, 3D, Cube, etc)
    viewCreateInfo.format = format;                                 // Format of image data
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;    // Allows remapping of rgba components to other rgba values
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    
    // Subresources allow the view to view only a part of an image
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;        // Which aspect of image to view (e.g. COLOR_BIT for veiwing color)
    viewCreateInfo.subresourceRange.baseMipLevel = 0;                // Start mipmap level to view from
    viewCreateInfo.subresourceRange.levelCount = 1;                  // Number of mipmap levels to view
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;              // Start array layer to view from
    viewCreateInfo.subresourceRange.layerCount = 1;                  // Number of array levels to view
    
    // Create image view and return it
    VkImageView imageView;
    VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create an Image view!");
    }
    
    return imageView;
}

void VulkanRenderer::createGraphicsPipeline(){
    // Read in SPIR-V code of shaders
    //std::string shaderDirectory = "Shaders/";
    auto vertexShaderCode = readFile("vert.spv");
    auto fragmentShaderCode = readFile("frag.spv");
    
    // Build Shader Modules to link to Graphics Pipeline
    // Create shader modules
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);
    
    // -- SHADER STAGE CREATION INFORMATION --
    // Vertex Stage creation information
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;                  // Shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule;                         // Shader module to be used by stage
    vertexShaderCreateInfo.pName = "main";                                      // Entry point to the shader
    
    // Fragment Stage creation information
    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
    fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;              // Shader stage name
    fragmentShaderCreateInfo.module = fragmentShaderModule;                     // Shader module to be used by stage
    fragmentShaderCreateInfo.pName = "main";                                    // Entry point to the shader
    
    // Put shader stage creation info in to array
    // Graphics Pipeline creation info requires array of shader stage creates
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertexShaderCreateInfo, fragmentShaderCreateInfo
    };
    
    // How the data for a single vertex (including info such as position, color, tex coords, normals, etc) is as a whole
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;                                 // Can bind multiple streams of data, this defines which one
    bindingDescription.stride = sizeof(Vertex);                     // Size of a single vertex object
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;     // How to move between data after each vertex
                                                                    // VK_VERTEX_INPUT_RATE_VERTEX      : Move on to the next vertex
                                                                    // VK_VERTEX_INPUT_RATE_INSTANCE    : Move to a vertex for the next instance
    
    // How the data for an attribute is defined within a vertex
    std::array<VkVertexInputAttributeDescription, 3> attributeDescription;
    
    // Position attribute
    attributeDescription[0].binding = 0;                            // Which binding the data is at (should be same as above)
    attributeDescription[0].location = 0;                           // Location in shader where data will be read from
    attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;    // Format the data will take (also helps define the size of data)
    attributeDescription[0].offset = offsetof(Vertex, pos);         // Where this attribute is defined in the data for a single vertex
    
    // Color attribute
    attributeDescription[1].binding = 0;                            // Which binding the data is at (should be same as above)
    attributeDescription[1].location = 1;                           // Location in shader where data will be read from
    attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;    // Format the data will take (also helps define the size of data)
    attributeDescription[1].offset = offsetof(Vertex, col);         // Where this attribute is defined in the data for a single vertex
    
    // Texture Attributes
    attributeDescription[2].binding = 0;                            // Which binding the data is at (should be same as above)
    attributeDescription[2].location = 2;                           // Location in shader where data will be read from
    attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;    // Format the data will take (also helps define the size of data)
    attributeDescription[2].offset = offsetof(Vertex, tex);         // Where this attribute is defined in the data for a single vertex
    
    // -- VERTEX INPUT --
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;                 // List of vertex binding descriptions (data spacing, stride info, etc)
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescription.data();               // List of vertex attribute descriptions (data format and where to bind to/from)
    
    // -- INPUT ASSEMBLY --
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;               // Primitive type to assemble vertices as
    inputAssembly.primitiveRestartEnable = VK_FALSE;                            // Allow overriding of "strip" topology to start new primitives
    
    // -- VIEWPORT & SCISSOR --
    // Create a view port info struct
    VkViewport viewport = {};
    viewport.x = 0.0f;                                  // x start coordinate
    viewport.y = 0.0f;                                  // y start coordinate
    viewport.width = (float) swapchainExtent.width;     // width of view port
    viewport.height = (float) swapchainExtent.height;   // height of view port
    viewport.minDepth = 0.0f;                           // min framebuffer depth
    viewport.maxDepth = 1.0f;                           // max framebuffer depth
    
    // Create s SCISSOR infor struct
    VkRect2D scissor = {};
    scissor.offset = { 0,0 };                           // Offset to use region from
    scissor.extent = swapchainExtent;                   // Extent to describe region to use, starting at offset
    
    VkPipelineViewportStateCreateInfo viewportStageCreateInfo = {};
    viewportStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStageCreateInfo.viewportCount = 1;
    viewportStageCreateInfo.pViewports = &viewport;
    viewportStageCreateInfo.scissorCount = 1;
    viewportStageCreateInfo.pScissors = &scissor;
    
    // -- DYNAMIC STATES --
    /*
    // Note: If you are resizing a window, also recreate swapchain, swapchain images and any images associated with the swapchain so that they can actually fit
    // Dynamic states to enable
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);   // Dynamic Viewport: can resize in command buffer with vkCommandSetViewport(commandBuffer, 0, 1, &viewport)
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);    // Dynamic Scissor: can resize in command buffer with vkCommandSetScissor(commandBuffer, 0, 1, &scissor)
    
    // Dynamic state creation info
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
    */
    
    // -- RASTERIZER --
    // Converts the triangle into individual fragments on the screen to be used in fragment shader
    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;            // Change if fragments boyond near/far planes are clipped (default) or clamped to plane
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;     // Whether to discard data and skip rasterizer. Never creates fragment, only suitable for pipeline without framebuffer output
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;     // How to handle filling points between vertices
    rasterizationCreateInfo.lineWidth = 1.0f;                       // How thick lines should be when drawn
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;       // Which face of the tri to cull
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;    // Winding to detetmine which side is front
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;             // Whether to add depth bias to fragment (good for stopping "shadow acne" in shadow mapping)
    
    // -- MULTISAMPLING --
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;                 // Enable multisample shading or not
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;   // Number of sample to use per fragment
    
    // -- BLENDING --
    // Blending decides how to blend a new color being written to a fragment, with the old value
    
    // Blend attachment state (How blending is handled)
    VkPipelineColorBlendAttachmentState colorBlendingAttachmentState = {};
    colorBlendingAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;      // Colors to apply blending to
    colorBlendingAttachmentState.blendEnable = VK_TRUE;                             // Enable blending
    
    // Blending uses equations: (srcColorBlendFactor * newColor) colorBlendOp (dstColorBlendFactor * oldColor)
    colorBlendingAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendingAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendingAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    
    // Summarized: (VK_BLEND_FACTOR_SRC_ALPHA * newColor) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * oldColor)
    //             (new color alpha * new color) + ((1 - new color alpha) * old color)
    
    colorBlendingAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendingAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendingAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    // Sumarised: (1 * new alpha) + (0 * old alpha) = new alpha
    
    VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
    colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingCreateInfo.logicOpEnable = VK_FALSE;                       // Alternative to calculations is to use logical operations
    // colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;                  // Default
    colorBlendingCreateInfo.attachmentCount = 1;
    colorBlendingCreateInfo.pAttachments = &colorBlendingAttachmentState;
    
    // -- PIPELINE LAYOUT --
    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = { descriptorSetLayout, samplerSetLayout};
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    
    // Create pipeline layout
    VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    
    // -- DEPTH STENCIL TESTING --
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
    depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilCreateInfo.depthTestEnable = VK_TRUE;               // Enable checking depth to determine fragment write
    depthStencilCreateInfo.depthWriteEnable = VK_TRUE;              // Enable writing to depth buffer (to replace all values)
    depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;     // Comparison operation that allows an overwrite (is in front)
    depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;        //  Depth Bounds Test: Does the depth value exists between two bounds
    // TODO: Enable stencil testing as well to avoid error on MacOS
    depthStencilCreateInfo.stencilTestEnable = VK_FALSE;            // Enable Stencil Test
    depthStencilCreateInfo.flags = 0;                               // ROHIT: Set to 0 to make it work on MacOS
    depthStencilCreateInfo.pNext = nullptr;                         // ROHIT: needed to make it null explicitly to run on MacOS

    
    // -- Graphics Pipeline Creation --
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;                                  // Number of shader stages
    pipelineCreateInfo.pStages = shaderStages;                          // List of shader stages
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;      // All the fixed funtion pipeline states
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    pipelineCreateInfo.pViewportState = &viewportStageCreateInfo;
    pipelineCreateInfo.pDynamicState = nullptr;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;                         // Pipeline layout pipeline should use
    pipelineCreateInfo.renderPass = renderpass;                         // Renderpass description the pipeline is compatible with
    pipelineCreateInfo.subpass = 0;                                     // Subpass of render pass to use with pipeline
    
    // Pipeline derivatives : can create multiple pipelines that derive from one another for optimization
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;             // Existing pipeline to derive from
    pipelineCreateInfo.basePipelineIndex = -1;                         // or index of pipeline being created to derive from (in case creating multiple at once)
        
    // Create Graphics Pipeline
    result = vkCreateGraphicsPipelines(mainDevice.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Graphics Pipeline!");
    }
    
    // Destroy shder module, no longer needed after Pipeline creation
    vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule, nullptr);
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char> &code){
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();                                  // Size of code
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data()); // Pointer to code (of uint32_t pointer type)
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a shader module!");
    }
    
    return shaderModule;
}

void VulkanRenderer::createRenderPass(){
    // ATTACHMENTS
    // Color attachment of the render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainImageFormat;                  // Format to use for attachment
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                // Number of samples to write for multisampling
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;           // Describes what to do with attachment before rendering (what glClear does)
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;         // Describes what to do with attachment after rendering
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;// Describes what to do with stencil before rendering
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;// Describes what to do with stencil after rendering
    
    // Framebuffer data will be stored as an image, but images can be given different data layouts
    // to give optimal use for certain operations
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Image data layout before render pass starts
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Image data layout after render pass (to change to)
    
    // Depth attachment of render pass
    VkAttachmentDescription depthAttachment = {};
    
    // Choose supported formats for depth buffer
    depthBufferFormat = chooseSupportedFormat(
          {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT},
                                                  VK_IMAGE_TILING_OPTIMAL,
                                                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                  );
    depthAttachment.format = depthBufferFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    // REFERENCES
    // Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // Depth attachment reference
    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    // Information about a particular subpass the Render pass is using
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;        // Pipeline type subpass is to be bound to
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
    
    // Need to determine when layout transition occur using subpass dependencies
    std::array<VkSubpassDependency, 2> subpassDependencies;
    
    // Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    // Transition must happen after...
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;                            // Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;         // Pipeline stage
    subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;                   // Stage access mask (memory access)
    // But must happen before...
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = 0;
    
    // Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // Transition must happen after...
    subpassDependencies[1].srcSubpass = 0;                                                  // Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;    // Pipeline stage
    subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;                     // Stage access mask (memory access)
    // But must happen before...
    subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[1].dependencyFlags = 0;
    
    std::array<VkAttachmentDescription, 2> renderPassAttachment = {colorAttachment, depthAttachment};
    
    // Create info for render pass
    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachment.size());
    renderPassCreateInfo.pAttachments = renderPassAttachment.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassCreateInfo.pDependencies = subpassDependencies.data();
    
    VkResult result = vkCreateRenderPass(mainDevice.logicalDevice, &renderPassCreateInfo, nullptr, &renderpass);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Renderpass!");
    }
}

void VulkanRenderer::createFramebuffers(){
    // resize framebuffer count to equals swapchain image count
    swapchainFramebuffers.resize(swapchainImages.size());
    
    // create a framebuffer for each swapchain image
    for(size_t i=0; i< swapchainFramebuffers.size(); i++){
        std::array<VkImageView, 2> attachments = {
            swapchainImages[i].imageView,
            depthBufferImageView
        };
        
        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderpass;                                          // Render pass layout the Framebuffer will be used with
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();                                // List of attachments (1:1 with renderpass)
        framebufferCreateInfo.width = swapchainExtent.width;                                    // Framebuffer width
        framebufferCreateInfo.height = swapchainExtent.height;                                  // Framebuffer height
        framebufferCreateInfo.layers = 1;                                                       // Framebuffer layers
        framebufferCreateInfo.pNext = nullptr;                                                  // ROHIT: Needed to explicitly set it to null to avoid program crash
        
        VkResult result = vkCreateFramebuffer(mainDevice.logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]);
        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to create a Framebuffer!");
        }
    }
}


void VulkanRenderer::createCommandPool(){
    // Get indices of Queue Family from device
    QueueFamilyIndices queueFamilyIndices = getQueueFamilies(mainDevice.physicalDevice);
    
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;          // Queue Family type that buffers from this command pool will use
    
    // Create a Graphics Queue Family for Command pool
    VkResult result = vkCreateCommandPool(mainDevice.logicalDevice, &poolInfo, nullptr, &graphicsCommandPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Command Pool!");
    }
}

void VulkanRenderer::createCommandBuffers(){
    // Resize command buffer count to have one for each framebuffer
    commandBuffers.resize(swapchainFramebuffers.size());
    
    VkCommandBufferAllocateInfo cbAllocInfo = {};
    cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo.commandPool = graphicsCommandPool;
    cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                    // VK_COMMAND_BUFFER_LEVEL_PRIMARY - Buffers you submit to the queue, can't be called by other buffers.
                                                                            // VK_COMMAND_BUFFER_LEVEL_SECONDARY - Buffers can't be called directly. Can be called from other buffers via vkCmdExecuteCommands(buffer) when recording commands in primary buffers
    cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    
    // Allocate command buffers and allocate handles in array of buffers
    VkResult result = vkAllocateCommandBuffers(mainDevice.logicalDevice, &cbAllocInfo, commandBuffers.data());
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate a Command Buffer!");
    }
}

void VulkanRenderer::recordCommands(uint32_t currentImage){
    // Information about how to begin with each command buffer
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Buffer can be resubmitted when it has already been submitted and is awaiting execution
    
    // Information about how to begin a render pass (only needed for graphical operation)
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderpass;                            // render pass to begin
    renderPassBeginInfo.renderArea.offset = { 0,0 };                        // Start point of render pass in pixels
    renderPassBeginInfo.renderArea.extent = swapchainExtent;                // Size of region to run render pass on (starting at offset)
    
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.6f, 0.65f, 0.4f, 1.0f};
    clearValues[1].depthStencil.depth = 1.0f;
    
    renderPassBeginInfo.pClearValues = clearValues.data();                         // List of clear values
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    
        renderPassBeginInfo.framebuffer = swapchainFramebuffers[currentImage];
        
        // Start recording commands to command buffer!
        VkResult result = vkBeginCommandBuffer(commandBuffers[currentImage], &bufferBeginInfo);
        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to start recording a command buffer!");
        }
        
        // Begin render pass: this is going to call the clear function
        vkCmdBeginRenderPass(commandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        // Bind pipeline to be used in render pass
        vkCmdBindPipeline(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        
        for(size_t j=0; j<modelList.size(); j++){
            MeshModel thisModel = modelList[j];
            glm::mat4 thisModelsModel = thisModel.getModel();
            // "Push" constants to give shader stage directly (no buffer)
            vkCmdPushConstants(commandBuffers[currentImage], pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT,  // Stage to push constant to
                               0,                           // Offset of push constant to update
                               sizeof(Model),               // Size of data being pushed
                               &thisModelsModel        // Actual data being pushed
                               );
            
            for(size_t k=0; k<thisModel.getMeshCount(); k++){
                VkBuffer vertexBuffers[] = { thisModel.getMesh(k)->getVertexBuffer() };             // Buffers to bind
                VkDeviceSize offsets[] = { 0 };                                         // Offsets into buffer being bound
                vkCmdBindVertexBuffers(commandBuffers[currentImage], 0, 1, vertexBuffers, offsets);// Command to bind vertex buffer before drawing with them
                
                // Bind mesh index buffer, with 0 offset and using the uint32 format
                vkCmdBindIndexBuffer(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
                
                // Dynamic Offset Amount
                //uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * j;
                Model model = thisModel.getMesh(k)->getModel();
                
                std::array<VkDescriptorSet, 2> descriptorSetGroup = { descriptorSets[currentImage], samplerDescriptorSets[thisModel.getMesh(k)->getTexId()] };
                
                // Bind descriptor sets
                vkCmdBindDescriptorSets(commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 0, nullptr);
                
                // Execute pipeline
                //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(firstMesh.getVertexCount()), 1, 0, 0);
                vkCmdDrawIndexed(commandBuffers[currentImage], thisModel.getMesh(k)->getIndexCount(), 1, 0, 0, 0);
            }
        }
        // End render pass
        vkCmdEndRenderPass(commandBuffers[currentImage]);
        
        // Stop recording to command buffer
        result = vkEndCommandBuffer(commandBuffers[currentImage]);
        if(result != VK_SUCCESS){
            throw std::runtime_error("Failed to stop recording a command buffer!");
        }
    
}

void VulkanRenderer::draw(){
    // 1. Get next available image to draw to and set something to signal when we are finished with image (a semaphore)
    // Wait for give fence to signal (open) from last draw before continuing
    vkWaitForFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    // Manually reset (close) fences
    vkResetFences(mainDevice.logicalDevice, 1, &drawFences[currentFrame]);
    // Wait for the device to become idle
    vkDeviceWaitIdle(mainDevice.logicalDevice);
    
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mainDevice.logicalDevice, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    
    recordCommands(imageIndex);
    updateUniformBuffers(imageIndex);
    
    // 2. Submit a command buffer to queue for execution, make sure it waits for the image to be signaled as available before drawing and singals when it has finished rendering
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;                      // Number of semaphores to wait on
    submitInfo.pWaitSemaphores = &imageAvailable[currentFrame];           // List of semaphores to wait on
    VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.pWaitDstStageMask = waitStages;              // Stages to wait the semaphore at
    submitInfo.commandBufferCount = 1;                      // Number of command buffers submit
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];// Command buffer to submit
    submitInfo.signalSemaphoreCount = 1;                    // Number of semaphores to submit
    submitInfo.pSignalSemaphores = &renderFinished[currentFrame];         // Semaphore to signal when command buffer finishes
    
    // Submit command buffer to queue
    VkResult result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to submit command to Queue!");
    }
    
    // 3. Present image to screen when it has signal finished rendering
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;                     // Number of semaphores to wait on
    presentInfo.pWaitSemaphores = &renderFinished[currentFrame];          // Semaphores to wait on
    presentInfo.swapchainCount = 1;                         // Number of swapchains to present to
    presentInfo.pSwapchains = &swapchain;                   // Swapchains to present image to
    presentInfo.pImageIndices = &imageIndex;                // Index of images in swapchain to present
    
    // Present image to screen
    result = vkQueuePresentKHR(presentationQueue, &presentInfo);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to present rendered image to Screen!");
    }
    
    // Get next frame (use % MAX_FRAME_DRAWS to keep value below MAX_FRAME_DRAWS)
    currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanRenderer::createSynchronization(){
    imageAvailable.resize(MAX_FRAME_DRAWS);
    renderFinished.resize(MAX_FRAME_DRAWS);
    drawFences.resize(MAX_FRAME_DRAWS);
    
    // Semaphore creation information
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    // Fence creation infor
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for(size_t i=0; i<MAX_FRAME_DRAWS; i++){
        if(vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS
           || vkCreateSemaphore(mainDevice.logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinished[i]) != VK_SUCCESS
           || vkCreateFence(mainDevice.logicalDevice, &fenceCreateInfo, nullptr, &drawFences[i]) != VK_SUCCESS){
            throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
        }
    }
}

void VulkanRenderer::createDescriptorSetLayout(){
    // UNIFORM VALUES DESCRIPTOR SET LAYOUT
    // MVP Binding info
    VkDescriptorSetLayoutBinding vpLayoutBinding = {};
    vpLayoutBinding.binding = 0;                                           // Binding point in shader designated by binding number in shader
    vpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;    // Type of descriptor (uniform, dynamic uniform, image sampler, etc)
    vpLayoutBinding.descriptorCount = 1;                                   // Number of descriptors for binding
    vpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;               // Shader stage to bind to
    vpLayoutBinding.pImmutableSamplers = nullptr;                          // For Textures: Can make sampler data unchangeable (immutable) by specifying in layout
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    // Note: add modelLayoutBinding to layoutBindings vector when below code is needed back
    /*
    // Model binding info
    VkDescriptorSetLayoutBinding modelLayoutBinding = {};
    modelLayoutBinding.binding = 1;
    modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    modelLayoutBinding.descriptorCount = 1;
    modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    modelLayoutBinding.pImmutableSamplers = nullptr;
     */
    
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {vpLayoutBinding};
    
    // Create Descriptor set layout with give bindings
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());                  // Number of binding infos
    layoutCreateInfo.pBindings = layoutBindings.data();     // Array of binding infos
    
    // Create Descriptor set layout
    VkResult result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Descriptor Set Layout!");
    }
    
    
    // CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
    // Texture Binding Info
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    
    // Create Descriptor set layout with given bindings for texture
    VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = {};
    textureLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutCreateInfo.bindingCount = 1;
    textureLayoutCreateInfo.pBindings = &samplerLayoutBinding;
    
    // Create Descriptor Set Layout
    result = vkCreateDescriptorSetLayout(mainDevice.logicalDevice, &textureLayoutCreateInfo, nullptr, &samplerSetLayout);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Texture Sampler Descriptor Set Layout!");
    }
}

void VulkanRenderer::createUniformBuffers(){
    // ViewProjection buffer size
    VkDeviceSize vpBufferSize = sizeof(UBOViewProjection);
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    // Model buffer size
    //VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;
    
    // One uniform buffer for each image  (and by extension, command buffer)
    vpUniformBuffer.resize(swapchainImages.size());
    vpUniformBufferMemory.resize(swapchainImages.size());
    //modelDUniformBuffer.resize(swapchainImages.size());
    //modelDUniformBufferMemory.resize(swapchainImages.size());
    
    // Create Uniform buffers
    for(size_t i=0; i<swapchainImages.size();i++){
        createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);
        
        /*
         createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i], &modelDUniformBufferMemory[i]);
         */
    }
}

void VulkanRenderer::createDescriptorPool(){
    // CREATE UNIFORM DESCRIPTOR POOL
    
    // Type of descriptors + how many DESCRIPTORS, not Descriptor Sets (combined makes the pool size)
    // ViewProjection Pool
    VkDescriptorPoolSize vpPoolSize = {};
    vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size());
    
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    // Note: add modelPoolSize to descriptorPoolSizes vector when below code is needed back
    /*
    // Modelpool (DYNAMIC)
    VkDescriptorPoolSize modelPoolSize = {};
    modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());
    */
    // List of pool sizes
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {vpPoolSize};
    
    // Data to create Descriptor pool
    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.maxSets = static_cast<uint32_t>(swapchainImages.size());                     // Maximum number of descriptor sets that can be created from pool
    poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());           // Amount of pool sizes being passed
    poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();                                     // Pool sizes to create pool with
    
    // Create descriptor pool
    VkResult result = vkCreateDescriptorPool(mainDevice.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Descriptor Pool!");
    }
    
    // CREATE SAMPLER DESCRIPTOR POOL
    // Texture Sampler Pool
    VkDescriptorPoolSize samplerPoolSize = {};
    samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerPoolSize.descriptorCount = MAX_OBJECTS;
    
    VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
    samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
    samplerPoolCreateInfo.poolSizeCount = 1;
    samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;
    
    result = vkCreateDescriptorPool(mainDevice.logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Sampler Descriptor Pool!");
    }
}

void VulkanRenderer::createDescriptorSets(){
    // Resize Descriptor set list so one for every buffer
    descriptorSets.resize(swapchainImages.size());
    
    std::vector<VkDescriptorSetLayout> setLayouts(swapchainImages.size(), descriptorSetLayout);
    
    // Descriptor set allocation info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = descriptorPool;                                   // Pool to allocate descriptor sets from
    setAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapchainImages.size());// Number of sets to allocate
    setAllocInfo.pSetLayouts = setLayouts.data();                                   // Layout to use to allocate sets (1:1 relationship)
    
    // Allocate Descriptor Sets (multiple)
    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, descriptorSets.data());
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate Descriptor Sets!");
    }
    
    // Update all of descriptor set buffer bindings
    for(size_t i=0; i<swapchainImages.size(); i++){
        // VIEW PROJECTION DESCRIPTOR
        // Buffer info and data offset info
        VkDescriptorBufferInfo vpBufferInfo = {};
        vpBufferInfo.buffer = vpUniformBuffer[i];           // Buffer to get data from
        vpBufferInfo.offset = 0;                            // Position of start of the data
        vpBufferInfo.range = sizeof(UBOViewProjection);     // Size of data
        
        // Data about connection between binding and buffer
        VkWriteDescriptorSet vpSetWrite = {};
        vpSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vpSetWrite.dstSet = descriptorSets[i];                             // Descriptor set to update
        vpSetWrite.dstBinding = 0;                                         // Binding to update (matches with binding on layout/shader)
        vpSetWrite.dstArrayElement = 0;                                    // Index in array to update
        vpSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;     // Type of descriptor (should match with type of descriptor set)
        vpSetWrite.descriptorCount = 1;                                    // Amount to update
        vpSetWrite.pBufferInfo = &vpBufferInfo;                            // Information about data to bind
        
        // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
        // Note: add modelSetWrite to setWrites vector when below code is needed back
        /*
        // MODEL DESCRIPTOR
        // Model Buffer binding infor
        VkDescriptorBufferInfo modelBufferInfo = {};
        modelBufferInfo.buffer = modelDUniformBuffer[i];
        modelBufferInfo.offset = 0;
        modelBufferInfo.range = modelUniformAlignment;
        
        VkWriteDescriptorSet modelSetWrite = {};
        modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        modelSetWrite.dstSet = descriptorSets[i];
        modelSetWrite.dstBinding = 1;
        modelSetWrite.dstArrayElement = 0;
        modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        modelSetWrite.descriptorCount = 1;
        modelSetWrite.pBufferInfo = &modelBufferInfo;*/
        
        // List of Descriptor Set Writes
        std::vector<VkWriteDescriptorSet> setWrites = {vpSetWrite};
        
        // Update the descriptor sets with new buffer binding info
        vkUpdateDescriptorSets(mainDevice.logicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
    }
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex){
    // Copy VP data
    void *data;
    vkMapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex], 0, sizeof(UBOViewProjection), 0, &data);
    memcpy(data, &uboViewProjection, sizeof(UBOViewProjection));
    vkUnmapMemory(mainDevice.logicalDevice, vpUniformBufferMemory[imageIndex]);
    
    // Copy Model data
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    /*for(size_t i=0; i<meshList.size(); i++){
        UBOModel *thisModel = (UBOModel *)((uint64_t) modelTransferSpace + (i * modelUniformAlignment));
        *thisModel = meshList[i].getModel();
    }
    
    // Map the list of model data
    vkMapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex], 0, modelUniformAlignment * meshList.size(), 0, &data);
    memcpy(data, modelTransferSpace, modelUniformAlignment * meshList.size());
    vkUnmapMemory(mainDevice.logicalDevice, modelDUniformBufferMemory[imageIndex]);*/
}

void VulkanRenderer::updateModel(int modelId, glm::mat4 newModel){
    if(modelId >= modelList.size()){
        return;
    }
    modelList[modelId].setModel(newModel);
}

void VulkanRenderer::allocateDynamicBufferTransferSpace(){
    // Below commented code is for DYNAMIC UNIFORM BUFFERS and is kept for futher references
    /*
    // Calculate allignment of model data
    modelUniformAlignment = (sizeof(UBOModel) + minUniformBufferOffset - 1) & ~(minUniformBufferOffset - 1);
    
    // Create space in memory to hold dynamic buffer that is aligned to our required alignment and holds MAX_OBJECTS
    modelTransferSpace = (UBOModel *)aligned_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
     */
}

void VulkanRenderer::createPushConstantRange(){
    // Define push constant values (No "create" needed!)
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;      // Shader stage push constant will go to
    pushConstantRange.offset = 0;                                   // Offset to given data to pass to push constant
    pushConstantRange.size = sizeof(Model);                         // Size of data being passed
}

VkImage VulkanRenderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propFlags, VkDeviceMemory *imageMemory){
    // CREATE IMAGE
    // Image creation info
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;                   // Type of image (1D, 2D or 3D)
    imageCreateInfo.extent.width = width;                           // Width of image extent
    imageCreateInfo.extent.height = height;                         // Height of image extent
    imageCreateInfo.extent.depth = 1;                               // Depth of image extent (just 1, no 3D aspect)
    imageCreateInfo.mipLevels = 1;                                  // Number of mipmap levels
    imageCreateInfo.arrayLayers = 1;                                // Number of levels in image array
    imageCreateInfo.format = format;                                // Format type of image
    imageCreateInfo.tiling = tiling;                                // How image data should be tiled (arranged for optimal reading)
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Layout of image data on creation
    imageCreateInfo.usage = useFlags;                              // Bit flags defining what image will be used for
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;                // Number of samples for multisampling
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;        // Whether image can be shared between queues
    
    // Create Image
    VkImage image;
    VkResult result = vkCreateImage(mainDevice.logicalDevice, &imageCreateInfo, nullptr, &image);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create an Image!");
    }
    // CREATE MEMORY FOR IMAGE
    
    // Get memory requirement for the type of image
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mainDevice.logicalDevice, image, &memoryRequirements);
    
    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = findMemoryTypeIndex(mainDevice.physicalDevice, memoryRequirements.memoryTypeBits, propFlags);
    
    // Allocate memory using image requirements and user defined properties
    result = vkAllocateMemory(mainDevice.logicalDevice, &memoryAllocInfo, nullptr, imageMemory);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate memory for an Image!");
    }
    
    // Connect memory to image
    vkBindImageMemory(mainDevice.logicalDevice, image, *imageMemory, 0);
    
    return image;
}

void VulkanRenderer::createDepthBufferImage(){
    // Get supported format for depth buffer
    
    // depthBufferFormat is set in createRenderPass() and used here
    depthBufferFormat = chooseSupportedFormat(
    {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT},
                                            VK_IMAGE_TILING_OPTIMAL,
                                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                                            );
    
    // Create depth buffer image
    depthBufferImage = createImage(swapchainExtent.width, swapchainExtent.height, depthBufferFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &depthBufferImageMemory);
    
    // Create depth buffer image view
    depthBufferImageView = createImageView(depthBufferImage, depthBufferFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat VulkanRenderer::chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags){
    // Loop through the options and find compatible one
    for(VkFormat format: formats){
        // Get properties for a given format on this device
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(mainDevice.physicalDevice, format, &properties);
        
        // Depending on tiling choice, need to check for different bit flag
        if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags){
            return format;
        }else if(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags){
            return format;
        }
    }
    throw std::runtime_error("Failed to find a matching format!");
}

stbi_uc* VulkanRenderer::loadTextureFile(std::string fileName, int *width, int *height, VkDeviceSize *imageSize){
    // Number of channels image uses
    int channels;
    
    // Load pixel data for image
    std::string fileLoc = "/Textures/" + fileName;
    std::string fullFilePath = std::string(getcwd(NULL, 0)) + fileLoc;
    // Validate if file exists
    // Open stream from give file
    // std::ios::binary tells stream to read file as binary
    // std::ios::ate tell stream to start reading file from end of the file
    std::ifstream file(fullFilePath, std::ios::binary | std::ios::ate);
    
    // Check if file stream successfully opened
    if(!file.is_open()){
        throw std::runtime_error("Failed to open a file! ("+fullFilePath+")");
    }else{
        file.close();
    }
    
    stbi_uc* image = stbi_load(fullFilePath.c_str(), width, height, &channels, STBI_rgb_alpha);
    
    if(!image){
        throw std::runtime_error("Failed to load a texture file ("+fullFilePath+").");
    }
    
    // Calculate image size using given and known data
    *imageSize = *width * *height * 4;
    
    return image;
}

int VulkanRenderer::createTextureImage(std::string fileName){
    // Load image file
    int width, height;
    VkDeviceSize imageSize;
    
    stbi_uc* imageData = loadTextureFile(fileName, &width, &height, &imageSize);
    
    // Create staging buffer to hold loaded data, ready to copy to device
    VkBuffer imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;
    createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);
    
    // Copy image data to staging buffer
    void* data;
    vkMapMemory(mainDevice.logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, imageData, static_cast<size_t>(imageSize));
    vkUnmapMemory(mainDevice.logicalDevice, imageStagingBufferMemory);
    
    // Free original image data as we have already copied it to staging buffer
    stbi_image_free(imageData);
    
    // Create image to hold final texture
    VkImage texImage;
    VkDeviceMemory texImageMemory;
    texImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texImageMemory);
    
    // COPY DATA TO IMAGE
    // Transition image to be DST for copy operation
    transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    // Copy image data
    copyImageBuffer(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, imageStagingBuffer, texImage, width, height);
    
    // Transition image to be shader readable for shader usage
    transitionImageLayout(mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    // Add texture data to vector for reference
    textureImages.push_back(texImage);
    textureImageMemory.push_back(texImageMemory);
    
    // Destroy staging buffers
    vkDestroyBuffer(mainDevice.logicalDevice, imageStagingBuffer, nullptr);
    vkFreeMemory(mainDevice.logicalDevice, imageStagingBufferMemory, nullptr);
    
    // Return index of new texture image
    return textureImages.size() - 1;
}

int VulkanRenderer::createTexture(std::string fileName){
    // Create texture image and get it's location in an array
    int textureImageLoc = createTextureImage(fileName);
    
    // Create Image View and add to list
    VkImageView imageView = createImageView(textureImages[textureImageLoc], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    textureImageView.push_back(imageView);
    
    // Create Texture Descriptor
    int descriptorLoc = createTextureDescriptor(imageView);
    
    // Return location of set with texture
    return descriptorLoc;
}

void VulkanRenderer::createTextureSampler(){
    // Sampler creation info
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;                     // How to render if image is Magnified on screen
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;                     // How to render if Image is Minified on screen
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;    // How to handle Texture wrap in U(x) direction
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;    // How to handle Texture wrap in V(y) direction
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;    // How to handle Texture wrap in W(z) direction
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;   // Border beyond Texture (only works for border clamp)
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;               // Whether coords should be normalized between 0 and 1
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;       // Mipmap interpolation mode
    samplerCreateInfo.mipLodBias = 0.0f;                                // Level of Details bias for mip level
    samplerCreateInfo.minLod = 0.0f;                                    // Minimum level of detail to pick mip level
    samplerCreateInfo.maxLod = 0.0f;                                    // Maximum level of detail to pick mip level
    samplerCreateInfo.anisotropyEnable = VK_TRUE;                       // Enable Anisotropy
    samplerCreateInfo.maxAnisotropy = 16;                               // Anisotropy sample level
    
    VkResult result = vkCreateSampler(mainDevice.logicalDevice, &samplerCreateInfo, nullptr, &textureSampler);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to create a Texture Sampler!");
    }
}

int VulkanRenderer::createTextureDescriptor(VkImageView textureImage){
    VkDescriptorSet descriptorSet;
    
    // Descriptor Set Allocation Info
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.descriptorPool = samplerDescriptorPool;
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = &samplerSetLayout;
    
    // Allocate Descriptor Sets
    VkResult result = vkAllocateDescriptorSets(mainDevice.logicalDevice, &setAllocInfo, &descriptorSet);
    if(result != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate Texture Descriptor Set!");
    }
    
    // Texture Image info
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;       // Image layout when use
    imageInfo.imageView = textureImage;                                     // Image to bind to set
    imageInfo.sampler = textureSampler;                                     // Sampler to use for set
    
    
    // Descriptor Write Info
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    
    // Update new descriptor set
    vkUpdateDescriptorSets(mainDevice.logicalDevice, 1, &descriptorWrite, 0, nullptr);
    
    // Add descriptor set to list
    samplerDescriptorSets.push_back(descriptorSet);
    
    // Return descriptor set location
    return samplerDescriptorSets.size() - 1;
}

int VulkanRenderer::createMeshModel(std::string modelFile){
    std::string fullFilePath = std::string(getcwd(NULL, 0))+"/Models/" + modelFile;
    // Validate if file exists
    // Open stream from give file
    // std::ios::binary tells stream to read file as binary
    // std::ios::ate tell stream to start reading file from end of the file
    std::ifstream file(fullFilePath, std::ios::binary | std::ios::ate);
    
    // Check if file stream successfully opened
    if(!file.is_open()){
        throw std::runtime_error("Failed to open a file! ("+fullFilePath+")");
    }else{
        file.close();
    }
    
    // Import model scene
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fullFilePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    if(!scene){
        throw std::runtime_error("Failed to load model! ("+ fullFilePath +")");
    }
    
    // Get vector of all materials with 1:1 ID placement
    std::vector<std::string> textureNames = MeshModel::LoadMaterials(scene);
    
    // Conversion from the materials list IDs to our Descriptor Array IDs
    std::vector<int> matToTex(textureNames.size());
    
    // Loop over textureNames and create textures for them
    for(size_t i=0; i<textureNames.size(); i++){
        // If material has no texture, set 0 to indicate no texture, texture 0 will be reserved for a default texture
        if(textureNames[i].empty()){
            matToTex[i] = 0;
        }else{
            // Otherwise, create texture and set value to index of new texture
            matToTex[i] = createTexture(textureNames[i]);
        }
    }
    
    // Load in all our meshes
    std::vector<Mesh> modelMeshes = MeshModel::LoadNode(mainDevice.physicalDevice, mainDevice.logicalDevice, graphicsQueue, graphicsCommandPool, scene->mRootNode, scene, matToTex);
    
    // Create mesh model and add to list
    MeshModel meshModel = MeshModel(modelMeshes);
    modelList.push_back(meshModel);
    return modelList.size() - 1;
}
