/*********************************************************************
 * file:   vkapp_fns.cpp
 * author: lawrence.winters (lawrence.winters@digipen.edu)
 * date:   June 1, 2024
 * Copyright © 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Holds all of the functions related to the vulkan function calls
 *********************************************************************/

#include <array>
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream

#include <unordered_set>
#include <unordered_map>

#include "vkapp.h"
#include "app.h"
#include "extensions_vk.hpp"

#ifdef GUI
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#endif

/*********************************************************************
 *
 * 
 * brief:  All of resources created using vk func calls destroyed here
 **********************************************************************/
void VkApp::destroyAllVulkanResources()
{
    // @@
    vkDeviceWaitIdle(m_device);  // Uncomment this when you have an m_device created.

    // Destroy ImGUI
    vkDestroyDescriptorPool(m_device, m_imguiDescPool, nullptr);
    ImGui_ImplVulkan_Shutdown();

    // Destroy all vulkan objects.
    // ...  All objects created on m_device must be destroyed before m_device.

    // Project 3 Destroy
    m_shaderBindingTableBW.destroy(m_device);

    vkDestroyPipelineLayout(m_device, m_rtPipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_rtPipeline, nullptr);

    m_rtDesc.destroy(m_device);
    m_rtBuilder.destroy();

    m_rtColCurrBuffer.destroy(m_device);
    m_rtColPrevBuffer.destroy(m_device);

    m_rtKdCurrBuffer.destroy(m_device);
    m_rtKdPrevBuffer.destroy(m_device);

    m_rtNdCurrBuffer.destroy(m_device);
    m_rtNdPrevBuffer.destroy(m_device);

    // Project 2 Destroy
    vkDestroyPipelineLayout(m_device, m_scanlinePipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_scanlinePipeline, nullptr);
    m_scDesc.destroy(m_device);
    vkDestroyRenderPass(m_device, m_scanlineRenderPass, nullptr);
    vkDestroyFramebuffer(m_device, m_scanlineFramebuffer, nullptr);

    m_objDescriptionBW.destroy(m_device);
    m_matrixBW.destroy(m_device);

    for (auto& ob : m_objData) 
    {
      ob.vertexBuffer.destroy(m_device); 
      ob.matIndexBuffer.destroy(m_device);
      ob.matColorBuffer.destroy(m_device);
      ob.indexBuffer.destroy(m_device);
    }

    for (auto& tex : m_objText) 
    {
      tex.destroy(m_device);
    }

    m_postDesc.destroy(m_device);
    m_scImageBuffer.destroy(m_device);

    // Project 1 Destroy
    vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr);
    vkDestroyPipeline(m_device, m_postPipeline, nullptr);

    for (uint32_t i = 0; i < m_imageCount; i++)
    {
      vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
    }

    vkDestroyRenderPass(m_device, m_postRenderPass, nullptr);
    m_depthImage.destroy(m_device);
    destroySwapchain();
    vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

/*********************************************************************
 * param:  size
 *
 * 
 * brief:  
 **********************************************************************/
void VkApp::recreateSizedResources(VkExtent2D size)
{
  assert(false && "Not ready for onResize events.");

  //vkDeviceWaitIdle(m_device);

  //int width = 0,
  //  height = 0;
  //
  //glfwGetWindowSize(app->GLFW_window, &width, &height);
  //

  //if (width != size.width || height != size.height)
  //{
  //  printf("Old window : (%i, %i)\n", size.width, size.height);
  //  printf("New window : (%i, %i)\n", width, height);
  //}
  //else return;

  //// assert(false && "Not ready for onResize events.");
  //// Destroy everything related to the window size
  //m_rtColCurrBuffer.destroy(m_device);
  //vkDestroyPipelineLayout(m_device, m_scanlinePipelineLayout, nullptr);
  //vkDestroyPipeline(m_device, m_scanlinePipeline, nullptr);
  //vkDestroyRenderPass(m_device, m_scanlineRenderPass, nullptr);
  //vkDestroyFramebuffer(m_device, m_scanlineFramebuffer, nullptr);
  //vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr);
  //vkDestroyPipeline(m_device, m_postPipeline, nullptr);
  //m_scImageBuffer.destroy(m_device);
  //for (uint32_t i = 0; i < m_imageCount; i++)
  //{
  //  vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
  //}
  //m_depthImage.destroy(m_device);
  //destroySwapchain();

  //// (RE)Create them all at the new size
  //createSwapchain();
  //createDepthResource();
  //createPostFrameBuffers();
  //createScBuffer();
  //createPostPipeline();
  //createScanlineRenderPass();
  //createScPipeline();
  //createRtBuffers();
}

 /*********************************************************************
  * param:  doApiDump, Lunar dump with tons of information (true if wanted)
  *
  * 
  * brief:  Initialize vulkan library by creating an instance
  **********************************************************************/
void VkApp::createInstance(bool doApiDump)
{
    uint32_t countGLFWextensions{0};
    const char** reqGLFWextensions = glfwGetRequiredInstanceExtensions(&countGLFWextensions);

    // @@
    // Append each GLFW required extension in reqGLFWextensions to reqInstanceExtensions
    // Print them out while your are at it (DONE)
    printf("GLFW required extensions:\n");
    for (int i = 0; i < countGLFWextensions; ++i)
    {
      reqInstanceExtensions.push_back(reqGLFWextensions[i]);

      // printf("%s\n", reqGLFWextensions[i]);
    }

    // Suggestion: Parse a command line argument to set/unset doApiDump
    // If included, the api_dump layer should be first on reqInstanceLayers
    if (doApiDump)
        reqInstanceLayers.insert(reqInstanceLayers.begin(), "VK_LAYER_LUNARG_api_dump");
  
    uint32_t count;
    // The two step procedure for getting a variable length list
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());

    // @@
    // Print out the availableLayers (DONE)
    printf("InstanceLayer count: %d\n", count);
    // ...  use availableLayers[i].layerName
    /*for (const VkLayerProperties& layers : availableLayers)
    {
       printf("%s\n", layers.layerName);
    }*/

    // Another two step dance
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());

    // @@
    // Print out the availableExtensions (DONE)
    printf("InstanceExtensions count: %d\n", count);
    // ...  use availableExtensions[i].extensionName
    /*for (const VkExtensionProperties& extension : availableExtensions)
    {
       printf("%s\n", extension.extensionName);
    }*/

    VkApplicationInfo applicationInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    applicationInfo.pApplicationName = "rtrt";
    applicationInfo.pEngineName      = "no-engine";
    applicationInfo.apiVersion       = VK_MAKE_VERSION(1, 3, 0);

    VkInstanceCreateInfo instanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceCreateInfo.pNext                   = nullptr;
    instanceCreateInfo.pApplicationInfo        = &applicationInfo;
    
    instanceCreateInfo.enabledExtensionCount   = reqInstanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = reqInstanceExtensions.data();
    
    instanceCreateInfo.enabledLayerCount       = reqInstanceLayers.size();
    instanceCreateInfo.ppEnabledLayerNames     = reqInstanceLayers.data();

    auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

    // @@
    // Verify VkResult is VK_SUCCESS (DONE)
    // Document with a cut-and-paste of the three list printouts above. (DONE)
    // To destroy: vkDestroyInstance(m_instance, nullptr); (DONE)
    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create instance!");
    }
}

/*********************************************************************
 *
 * 
 * brief:  Look for a select a graphics card in the system
 **********************************************************************/
void VkApp::createPhysicalDevice()
{
    // Get the GPU list;  Another two-step list retrieval procedure:
    uint physicalDevicesCount;
    vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, physicalDevices.data());

    std::vector<uint32_t> compatibleDevices;
  
    printf("%d devices\n", physicalDevicesCount);

    // For each GPU:
    for (auto physicalDevice : physicalDevices) {

        // Get the GPU's properties
        VkPhysicalDeviceProperties GPUproperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &GPUproperties);

        // Get the GPU's extension list;  Another two-step list retrieval procedure:
        uint extCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> extensionProperties(extCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr,
                                             &extCount, extensionProperties.data());

        // @@ This code is in a loop iterating variable physicalDevice
        // through a list of all physicalDevices(GPUs).  The
        // physicalDevice's properties (GPUproperties) and a list of
        // its extension properties (extensionProperties) are retrieved
        // above, and here we judge if the physicalDevice
        // is compatible with our requirements. We consider a GPU to be
        // compatible if it satisfies both:
        
        //    GPUproperties.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        
        // and
        
        //    All reqDeviceExtensions can be found in the GPUs extensionProperties list
        //      That is: for all i, there exists a j such that:
        //                 reqDeviceExtensions[i] == extensionProperties[j].extensionName

        //  If a GPU is found to be compatible save it in m_physicalDevice.

        //  If none are found, declare failure and abort
        //     (And then find a better GPU for this class.)
        //  If several are found, tell me all about your system
        // (DONE)

        // Discrete GPU compatibility
        bool discrete = (GPUproperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

        // All reqDeviceExtensions are found
        bool extensions = true;
        for (const char* reqExtenstions : reqDeviceExtensions)
        {
          bool result = false;
          for (const VkExtensionProperties& properties : extensionProperties)
          {
            if (strcmp(reqExtenstions, properties.extensionName) == 0)
            {
              result = true;
              break;
            }
          }

          if (!result)
          {
            extensions = false;
            break;
          }
        }

        // GPU was compatible
        if (discrete && extensions)
        {
          //printf("GPU Accepted\n");
          //printf("%s\n", GPUproperties.deviceName);
          m_physicalDevice = physicalDevice;
        }
        // GPU was not compatible
        /*else
        {
          printf("GPU Rejected\n");
          printf("%s\n", GPUproperties.deviceName);
        }*/


        // Hint: Instead of a double nested pair of loops consider
        // making an std::unordered_set of all the device's
        // extensionNames and using its "find" method to search for
        // each required extension.
    }
    
    // @@ Document the GPU accepted, and any GPUs rejected. (DONE)
    // Oddly, there is nothing to destroy here.
  
}

/*********************************************************************
 *
 * 
 * brief:  Navigates list of queue families
 **********************************************************************/
void VkApp::chooseQueueIndex()
{
  VkQueueFlags requiredQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

  // Retrieve the list of queue families. (2-step.)
  uint32_t mpCount;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &mpCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueProperties(mpCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &mpCount, queueProperties.data());

  // @@ How many queue families does your Vulkan offer?  Which of
  // the three flags does each offer?  Which of them, by index, has
  // the above three required flags? (DONE)
  // printf("QueueFamily count: %d\n", mpCount);

  bool found = false;
  for (int i = 0; i < mpCount; ++i)
  {
    // @@ Search the list for (the index of) the first queue family
    // that has the required flags in queueProperties[i].queueFlags.  Record the index in
    // m_graphicsQueueIndex. (DONE)
    VkQueueFlags result = queueProperties[i].queueFlags & requiredQueueFlags;
    // printf("Queue Index: %i\n", i);
    // printf("%d\n", result);

    if (result == 0x7 && !found)
    {
      m_graphicsQueueIndex = i;
      found = true;
    }
  }

  if (!found)
  {
    throw std::runtime_error("queue family with required flags not found!");
  }

  // Nothing to destroy as m_graphicsQueueIndex is just an integer.
}

/*********************************************************************
 *
 * 
 * brief:  Set up logical device to interface with physical device
 **********************************************************************/
void VkApp::createDevice()
{
    // @@
    // Build a pNext chain of the following six "feature" structures:
    //   features2->features11->features12->features13->accelFeature->rtPipelineFeature->NULL (DONE)

    // Hint: Keep it simple; add a second parameter (the pNext
    // pointer) to each structure's initializer pointing up to the
    // previous structure.
    
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &rtPipelineFeature};
    
    VkPhysicalDeviceVulkan13Features features13{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, &accelFeature};
    
    VkPhysicalDeviceVulkan12Features features12{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &features13};
    
    VkPhysicalDeviceVulkan11Features features11{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &features12};
    
    VkPhysicalDeviceFeatures2 features2{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &features11};
    // Let Vulkan fill in all structures on the pNext chain
    vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features2);
    // @@ If you are curious, document the whole filled in pNext chain
    // using an api_dump and examine all the many features.  (DONE)

    float priority = 1.0;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueInfo.queueFamilyIndex = m_graphicsQueueIndex;
    queueInfo.queueCount       = 1;
    queueInfo.pQueuePriorities = &priority;
    
    VkDeviceCreateInfo deviceCreateInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.pNext            = &features2; // This is the whole pNext chain
  
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos    = &queueInfo;
    
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(reqDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = reqDeviceExtensions.data();

    VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    
    // @@
    // Verify VK_SUCCESS (DONE)
    // To destroy: vkDestroyDevice(m_device, nullptr); (DONE)

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create logical device!");
    }
}

/*********************************************************************
 *
 * 
 * brief:  Retrieve queue handles for each queue family
 **********************************************************************/
void VkApp::getCommandQueue()
{
    vkGetDeviceQueue(m_device, m_graphicsQueueIndex, 0, &m_queue);
    // Returns void -- nothing to verify
    // Nothing to destroy -- the queue is owned by the device.
}

/*********************************************************************
 *
 * 
 * brief:  Calling load_VK_EXTENSIONS from extensions_vk.cpp. A Python script 
 *         from NVIDIA created extensions_vk.cpp from the current Vulkan spec 
 *         for the purpose of loading the symbols for all registered extension. 
 *         This be (indistinguishable from) magic.
 **********************************************************************/
void VkApp::loadExtensions()
{
    load_VK_EXTENSIONS(m_instance, vkGetInstanceProcAddr, m_device, vkGetDeviceProcAddr);
}

/*********************************************************************
 *
 * 
 * brief:  VkSurface is Vulkan's name for the screen. Since GLFW creates 
 *         and manages the window, it creates the VkSurface at our request.
 **********************************************************************/
void VkApp::getSurface()
{
    VkBool32 isSupported;   // Supports drawing(presenting) on a screen

    VkResult glfwResult = glfwCreateWindowSurface(m_instance, app->GLFW_window, nullptr, &m_surface);
    VkResult vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, m_graphicsQueueIndex,
                                         m_surface, &isSupported);

    // @@ Verify VK_SUCCESS from both the glfw... and the vk... calls. (DONE)
    // @@ Verify isSupported==VK_TRUE, meaning that Vulkan supports presenting on this surface. (DONE)
    // To destroy: vkDestroySurfaceKHR(m_instance, m_surface, nullptr); (DONE)
    if (glfwResult != VK_SUCCESS || vkResult != VK_SUCCESS)
    {
      throw std::runtime_error("unsuccessful creation of surface!");
    }

    if (isSupported != VK_TRUE)
    {
      throw std::runtime_error("vulkan does not support presenting on this surface!");
    }
}

/*********************************************************************
 *
 * 
 * brief:  Create a command pool, used to allocate command buffers, which in 
 *         turn are use to gather and send commands to the GPU.  The flag 
 *         makes it possible to reuse command buffers.  The queue index 
 *         determines which queue the command buffers can be submitted to.
 *         Use the command pool to also create a command buffer.
 **********************************************************************/
void VkApp::createCommandPool()
{
    VkCommandPoolCreateInfo poolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = m_graphicsQueueIndex;
    VkResult result = vkCreateCommandPool(m_device, &poolCreateInfo, nullptr, &m_cmdPool);
    // @@ Verify VK_SUCCESS (DONE)
    // To destroy: vkDestroyCommandPool(m_device, m_cmdPool, nullptr); (DONE)

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create command pool!");
    }
    
    // Create a command buffer
    VkCommandBufferAllocateInfo allocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool        = m_cmdPool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    result = vkAllocateCommandBuffers(m_device, &allocateInfo, &m_commandBuffer);
    // @@ Verify VK_SUCCESS (DONE)
    // Nothing to destroy -- the pool owns the command buffer.

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate command buffers!");
    }
}
 
/*********************************************************************
 *
 * 
 * brief:  Creates the swap chain images (3)
 **********************************************************************/
void VkApp::createSwapchain()
{
    VkResult       err;
    VkSwapchainKHR oldSwapchain = m_swapchain;

    vkDeviceWaitIdle(m_device);  // Probably unnecessary

    // Get the surface's capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    // @@  Roll your own two step process to retrieve a list of present mode into
    //    std::vector<VkPresentModeKHR> presentModes;
    //  by making two calls to
    //    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, ...);
    // For an example, search above for vkGetPhysicalDeviceQueueFamilyProperties
    // Retrieve the list of queue families. (2-step.) (DONE)
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
    
    // @@ Document your present modes. I especially want to know if
    // your system offers VK_PRESENT_MODE_MAILBOX_KHR mode.  My
    // high-end windows desktop does; My higher-end Linux laptop
    // doesn't. (DONE)
    /*printf("Present Mode Count: %i\n", presentModeCount);
    for (const VkPresentModeKHR& mode : presentModes)
    {
      printf("%i\n", mode);
    }*/

    // Choose VK_PRESENT_MODE_FIFO_KHR as a default (this must be supported)
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR; // Support is required.
    // @@ But choose VK_PRESENT_MODE_MAILBOX_KHR if it can be found in
    // the retrieved presentModes. Several Vulkan tutorials opine that
    // MODE_MAILBOX is the premier mode, but this may not be best for
    // us -- expect more about this later. (DONE)
  

    // Get the list of VkFormat's that are supported:
    //@@ Do the two step process to retrieve a list of surface formats in
    //   std::vector<VkSurfaceFormatKHR> formats;
    // with two calls to
    //   vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, ...); (DONE)

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    // @@ Document the list you get. (DONE)

    /*printf("Format Count: %i\n", formatCount);
    for (const VkSurfaceFormatKHR& format : formats)
    {
      printf("%i, %i\n", format.format, format.colorSpace);
    }*/

    // @@ Replace the above two temporary lines with the following two
    // to choose the first format and its color space as defaults: (DONE)
    VkFormat surfaceFormat = formats[0].format;
    VkColorSpaceKHR surfaceColor  = formats[0].colorSpace;

    // @@ Then search the formats (from several lines up) to choose
    // format VK_FORMAT_B8G8R8A8_UNORM (and its color space) if such
    // exists.  Document your list of formats/color-spaces, and your
    // particular choice.(DONE)
    for (const VkSurfaceFormatKHR& format : formats)
    {
      if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
      {
        surfaceFormat = format.format;
        surfaceColor = format.colorSpace;

        break;
      }
    }
    
    // Get the swap chain extent
    VkExtent2D swapchainExtent = capabilities.currentExtent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        swapchainExtent = capabilities.currentExtent; }
    else {
        // Does this case ever happen?
        int width, height;
        glfwGetFramebufferSize(app->GLFW_window, &width, &height);

        swapchainExtent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        swapchainExtent.width = std::clamp(swapchainExtent.width,
                                           capabilities.minImageExtent.width,
                                           capabilities.maxImageExtent.width);
        swapchainExtent.height = std::clamp(swapchainExtent.height,
                                            capabilities.minImageExtent.height,
                                            capabilities.maxImageExtent.height); }

    // Test against valid size, typically hit when windows are minimized.
    // The app must prevent triggering this code in such a case
    assert(swapchainExtent.width && swapchainExtent.height);
    // @@ If this assert fires, we have some work to do to better deal
    // with the situation.

    // Choose the number of swap chain images, within the bounds supported.
    uint imageCount = capabilities.minImageCount + 1; // Recommendation: minImageCount+1
    if (capabilities.maxImageCount > 0
        && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount; }
    
    // assert (imageCount == 3);
    // If this triggers, disable the assert, BUT help me understand
    // the situation that caused it.  

    // Create the swap chain
    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                                 | VK_IMAGE_USAGE_STORAGE_BIT
                                 | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    VkSwapchainCreateInfoKHR _i = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    _i.surface                  = m_surface;
    _i.minImageCount            = imageCount;
    _i.imageFormat              = surfaceFormat;
    _i.imageColorSpace          = surfaceColor;
    _i.imageExtent              = swapchainExtent;
    _i.imageUsage               = imageUsage;
    _i.preTransform             = capabilities.currentTransform;
    _i.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    _i.imageArrayLayers         = 1;
    _i.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    _i.queueFamilyIndexCount    = 1;
    _i.pQueueFamilyIndices      = &m_graphicsQueueIndex;
    _i.presentMode              = swapchainPresentMode;
    _i.oldSwapchain             = oldSwapchain;
    _i.clipped                  = true;

    VkResult result = vkCreateSwapchainKHR(m_device, &_i, nullptr, &m_swapchain);
    // @@ Verify VK_SUCCESS (DONE)
    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create swapchain!");
    }
    
    //@@ Do the two step process to retrieve the list of (3) swapchain images (DONE)
    //   m_swapchainImages (of type std::vector<VkImage>)
    // with two calls to
    //   vkGetSwapchainImagesKHR(m_device, m_swapchain, ...);
    // Verify success (DONE)
    // Verify and document that you retrieved the correct number of images. (DONE)
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, nullptr);
    m_swapchainImages.resize(m_imageCount);
    result = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_imageCount, m_swapchainImages.data());

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to get swapchain images!");
    }
    // printf("Swapchain Image Count: %i\n", m_imageCount);
    
    // Set the image count to the proper value and resize vectors to that
    m_barriers.resize(m_imageCount);
    m_imageViews.resize(m_imageCount);

    // Create an VkImageView for each swap chain image.
    for (uint i=0;  i<m_imageCount;  i++) {
        VkImageViewCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            createInfo.image = m_swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surfaceFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            vkCreateImageView(m_device, &createInfo, nullptr, &m_imageViews[i]); }

    // Create three VkImageMemoryBarrier structures (one for each swap
    // chain image) and specify the desired
    // layout (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) for each.
    for (uint i=0;  i<m_imageCount;  i++) {
        VkImageSubresourceRange range = {0};
        range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel            = 0;
        range.levelCount              = VK_REMAINING_MIP_LEVELS;
        range.baseArrayLayer          = 0;
        range.layerCount              = VK_REMAINING_ARRAY_LAYERS;
        
        VkImageMemoryBarrier memBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        memBarrier.dstAccessMask        = 0;
        memBarrier.srcAccessMask        = 0;
        memBarrier.oldLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        memBarrier.newLayout            = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memBarrier.image                = m_swapchainImages[i];
        memBarrier.subresourceRange     = range;
        m_barriers[i] = memBarrier;
    }

    // Create a temporary command buffer. submit the layout conversion
    // command, submit and destroy the command buffer.
    VkCommandBuffer cmd = createTempCmdBuffer();
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0,
                         nullptr, m_imageCount, m_barriers.data());
    submitTempCmdBuffer(cmd);

    // Create the three synchronization objects.  These are not
    // technically part of the swap chain, but they are used
    // exclusively for synchronizing the swap chain, so I include them
    // here.
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_waitFence);
    
    VkSemaphoreCreateInfo semCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(m_device, &semCreateInfo, nullptr, &m_readSemaphore);
    vkCreateSemaphore(m_device, &semCreateInfo, nullptr, &m_writtenSemaphore);
    //NAME(m_readSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "m_readSemaphore");
    //NAME(m_writtenSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "m_writtenSemaphore");
    //NAME(m_queue, VK_OBJECT_TYPE_QUEUE, "m_queue");
        
    m_windowSize = swapchainExtent;
    // To destroy:  Complete and call function destroySwapchain (DONE)
}

/*********************************************************************
 *
 * 
 * brief:  Destroys the swap chain, including the image views and 
 *         synchronization items
 **********************************************************************/
void VkApp::destroySwapchain()
{
    vkDeviceWaitIdle(m_device);

    // @@
    // Destroy all (3)  m_imageViews with vkDestroyImageView(m_device, imageView, nullptr) (DONE)
    for (VkImageView& imageView : m_imageViews)
    {
      vkDestroyImageView(m_device, imageView, nullptr);
    }

    // Destroy the synchronization items: 
    vkDestroyFence(m_device, m_waitFence, nullptr);
    vkDestroySemaphore(m_device, m_readSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_writtenSemaphore, nullptr);

    // Destroy the actual swapchain with: vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    m_swapchain = VK_NULL_HANDLE;
    m_imageViews.clear();
    m_barriers.clear();
}

/*********************************************************************
 *
 * 
 * brief:  Creates depth image 
 **********************************************************************/
void VkApp::createDepthResource()
{
  uint mipLevels = 1;

  // Note m_depthImage is type ImageWrap; a tiny wrapper around
  // several related Vulkan objects.
  m_depthImage = createImageWrap(m_windowSize.width, m_windowSize.height,
    VK_FORMAT_X8_D24_UNORM_PACK32, 
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    mipLevels);

  m_depthImage.imageView = createImageView(m_depthImage.image,
    VK_FORMAT_X8_D24_UNORM_PACK32,
    VK_IMAGE_ASPECT_DEPTH_BIT);

  // To destroy: m_depthImage.destroy(m_device); (DONE)
}

/*********************************************************************
 * param:  typeFilter, unsigned int representing type of memory
 * param:  properties, requested flags for memory to contain
 *
 * return: spot in memory containing specific properties
 * 
 * brief:  Gets a list of memory types supported by the GPU, and search 
 *         through that list for one that matches the requested properties 
 *         flag. The (only?) two types requested here are: 
 * 
 *         (1) VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT: For the bulk of the memory used 
 *             by the GPU to store things internally. 
 *          
 *         (2) VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: 
 *             for memory visible to the CPU  for CPU to GPU copy operations.
 **********************************************************************/
uint32_t VkApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i))
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; } }

    throw std::runtime_error("failed to find suitable memory type!");
}

/*********************************************************************
 * 
 * 
 * brief:  A factory function for an ImageWrap, this creates a VkImage and 
 *         creates and binds an associated VkDeviceMemory object. The VkImageView 
 *         and VkSampler are left empty to be created elsewhere as needed.
 **********************************************************************/
ImageWrap VkApp::createImageWrap(uint32_t width, uint32_t height,
                                 VkFormat format,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties, uint mipLevels)
{
    ImageWrap myImage;
    
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result =  vkCreateImage(m_device, &imageInfo, nullptr, &myImage.image);

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, myImage.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &myImage.memory);

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate memory!");
    }
    
    result = vkBindImageMemory(m_device, myImage.image, myImage.memory, 0);

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to bind image memory!");
    }

    myImage.imageView = VK_NULL_HANDLE;
    myImage.sampler = VK_NULL_HANDLE;

    return myImage;
    // @@ Verify success for vkCreateImage, and vkAllocateMemory (DONE)
}

/*********************************************************************
 *
 * 
 * brief:  Wrapper to create image view
 **********************************************************************/
VkImageView VkApp::createImageView(VkImage image, VkFormat format,
                                         VkImageAspectFlagBits aspect)
{
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView;
    VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    // @@ Verify success for vkCreateImageView (DONE)

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create image view!");
    }
    
    return imageView;
}

/*********************************************************************
 *
 * 
 * brief:  Need to tell Vulkan about the framebuffer attachments that will 
 *         be used while rendering. We need to specify how many color and 
 *         depth buffers there will be, how many samples to use for each of 
 *         them and how their contents should be handled throughout the 
 *         rendering operations.
 **********************************************************************/
void VkApp::createPostRenderPass()
{  
    std::array<VkAttachmentDescription, 2> attachments{};
    // Color attachment
    attachments[0].format      = VK_FORMAT_B8G8R8A8_UNORM;
    attachments[0].loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].samples     = VK_SAMPLE_COUNT_1_BIT;

    // Depth attachment
    attachments[1].format        = VK_FORMAT_X8_D24_UNORM_PACK32; 
    attachments[1].loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].samples       = VK_SAMPLE_COUNT_1_BIT;

    const VkAttachmentReference colorReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference depthReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};


    std::array<VkSubpassDependency, 1> subpassDependencies{};
    // Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commands executed outside of the actual renderpass)
    subpassDependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass      = 0;
    subpassDependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpassDependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    subpassDependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount    = 1;
    subpassDescription.pColorAttachments       = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;

    VkRenderPassCreateInfo renderPassInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    renderPassInfo.pDependencies   = subpassDependencies.data();

    vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_postRenderPass);
    // To destroy: vkDestroyRenderPass(m_device, m_postRenderPass, nullptr); (DONE)
}

/*********************************************************************
 *
 * 
 * brief:  A VkFrameBuffer wraps several images into a render target 
 *         -- usually a color buffer and a depth buffer.
 **********************************************************************/
void VkApp::createPostFrameBuffers()
{
  std::array<VkImageView, 2> fbattachments{};

  // Create frame buffers for every swap chain image
  VkFramebufferCreateInfo _ci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
  _ci.renderPass = m_postRenderPass;
  _ci.width = m_windowSize.width;
  _ci.height = m_windowSize.height;
  _ci.layers = 1;
  _ci.attachmentCount = 2;
  _ci.pAttachments = fbattachments.data();

  // Each of the three swapchain images gets an associated frame
  // buffer, all sharing one depth buffer.
  m_framebuffers.resize(m_imageCount);
  for (uint32_t i = 0; i < m_imageCount; i++)
  {
    fbattachments[0] = m_imageViews[i];         // A color attachment from the swap chain
    fbattachments[1] = m_depthImage.imageView;  // A depth attachment
    VkResult result = vkCreateFramebuffer(m_device, &_ci, nullptr, &m_framebuffers[i]);
    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create frame buffer!");
    }
  }

  // To destroy: In a loop, call: vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr); (DONE)
  // Verify success (DONE)
}

/*********************************************************************
 *
 * 
 * brief:  Create graphics pipelines
 **********************************************************************/
void VkApp::createPostPipeline()
{
    // Creating the pipeline layout
    VkPipelineLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};


    // @@ What we eventually want:
    createInfo.setLayoutCount         = 1;
    createInfo.pSetLayouts            = &m_postDesc.descSetLayout;
    
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges    = nullptr;
    
    vkCreatePipelineLayout(m_device, &createInfo, nullptr, &m_postPipelineLayout);

    ////////////////////////////////////////////
    // Create the shaders
    ////////////////////////////////////////////
    VkShaderModule vertShaderModule = createShaderModule(loadFile("spv/post.vert.spv"));
    VkShaderModule fragShaderModule = createShaderModule(loadFile("spv/post.frag.spv"));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //auto bindingDescription = Vertex::getBindingDescription();
    //auto attributeDescriptions = Vertex::getAttributeDescriptions();

    // No geometry in this pipeline's draw.
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
        
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_windowSize.width;
    viewport.height = (float) m_windowSize.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = VkExtent2D{m_windowSize.width, m_windowSize.height};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; //??
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;// BEWARE!!  NECESSARY!!
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = m_postPipelineLayout;
    pipelineInfo.renderPass = m_postRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_postPipeline);

    if (result != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create graphics pipeline!");
    }

    // The pipeline has fully compiled copies of the shaders, so these
    // intermediate (SPV) versions can be destroyed.
    // @@
    // For the two modules fragShaderModule and vertShaderModule
    // destroy right *here* via vkDestroyShaderModule(m_device, ..., nullptr); (DONE)
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    
    // To destroy:  vkDestroyPipelineLayout(m_device, m_postPipelineLayout, nullptr);
    //  and:        vkDestroyPipeline(m_device, m_postPipeline, nullptr); (DONE)
    // Document the vkCreateGraphicsPipelines call with an api_dump.  (DONE)

}

std::string VkApp::loadFile(const std::string& filename)
{
    std::string   result;
    std::ifstream stream(filename, std::ios::ate | std::ios::binary);  //ate: Open at file end

    if(!stream.is_open())
        return result;

    result.reserve(stream.tellg()); // tellg() is last char position in file (i.e.,  length)
    stream.seekg(0, std::ios::beg);

    result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return result;
}

//-------------------------------------------------------------------------------------------------
// Post processing pass: tone mapper, UI
void VkApp::postProcess()
{
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color        = {{1,1,1,1}};
    clearValues[1].depthStencil = {1.0f, 0};
            
    VkRenderPassBeginInfo _i{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    _i.clearValueCount = 2;
    _i.pClearValues    = clearValues.data();
    _i.renderPass      = m_postRenderPass;
    _i.framebuffer     = m_framebuffers[m_swapchainIndex];
    _i.renderArea      = {{0, 0}, m_windowSize};
    
    vkCmdBeginRenderPass(m_commandBuffer, &_i, VK_SUBPASS_CONTENTS_INLINE);
    {   // extra indent for renderpass commands
        
        auto aspectRatio = static_cast<float>(m_windowSize.width)
            / static_cast<float>(m_windowSize.height);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postPipeline);
        // Eventually uncomment this
        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_postPipelineLayout, 0, 1, &m_postDesc.descSet, 0, nullptr);

        // Weird! This draws 3 vertices but with no vertices/triangles buffers bound in.
        // Hint: The vertex shader fabricates vertices from gl_VertexIndex
        vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

        #ifdef GUI
        ImGui::Render();  // Rendering UI
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_commandBuffer);
        #endif
    }
    vkCmdEndRenderPass(m_commandBuffer);
}
