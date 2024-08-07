/*********************************************************************
 * file:   app.cpp
 * author: lawrence.winters (lawrence.winters@digipen.edu)
 * date:   July 2, 2024
 * Copyright � 2024 DigiPen (USA) Corporation.
 *
 * brief: Main and GUI functions here
 *********************************************************************/
#include <iostream>
#include <array>

#include <cstdio>
#include <chrono>
#include <thread>

#include "vkapp.h"
#include "app.h"
#include "extensions_vk.hpp"

// GLFW Callback functions
static void onErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


void drawGUI(App* app, VkApp& VK)
{
    
    // @@ Once GUI is defined, you can put some gui elements on the screen
    
    // ImGui::ShowDemoWindow();  // Turn on ImGui's demonstration of all widgets.

    // Display the frame rate:
    ImGui::Text("Rate %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // An example check box:
    ImGui::Checkbox("Ray Trace", &VK.useRaytracer);

    // Use of full BRDF
    ImGui::Checkbox("Full BRDF", &VK.m_pcRay.BRDF);

    // Accumulate paths
    ImGui::Checkbox("Accumulate", &VK.m_pcRay.accumulate);

    // History Tracking
    ImGui::Checkbox("History", &VK.m_pcRay.history);

    // Denoising
    ImGui::Checkbox("Denoise", &VK.denoiser);

    // Clear and repath
    ImGui::Checkbox("Clear", &(app->myCamera.modified));

    // An example slider:
    if (ImGui::SliderFloat("Exposure", &VK.m_pcRay.exposure, 0.5f, 8.0f, "%.5f"))
        VK.m_pcRay.clear = true;

    if (ImGui::SliderFloat("Depth Threshold", &VK.m_pcRay.exposure, 0.0f, 1.0f, "%.2f"))
      VK.m_pcRay.clear = true;

    if (ImGui::SliderFloat("Normal Threshold", &VK.m_pcRay.exposure, 0.0f, 1.0f, "%.2f"))
      VK.m_pcRay.clear = true;


    ImGui::Text("Frame Count: %i", VK.frameCount);
}

//////////////////////////////////////////////////////////////////////////
static int const WIDTH  = 10*128;
static int const HEIGHT = 6*128;
static std::string PROJECT = "rtrt";

App* app;  // The app, declared here so static callback functions can find it.

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
double limit = 16.667;

//---------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  app = new App(argc, argv); // Constructs the glfw window and sets UI callbacks


  VkApp VK(app); // Creates and manages all things Vulkan.

  // The draw loop
  printf("looping =======================================\n");
  while (!glfwWindowShouldClose(app->GLFW_window)) {

    // Maintain designated frequency of 5 Hz (200 ms per frame)
    /*a = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> work_time = a - b;

    if (work_time.count() < limit)
    {
      std::chrono::duration<double, std::milli> delta_ms(limit - work_time.count());
      auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
      std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }

    b = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> sleep_time = b - a;*/

    // Your code here

    glfwPollEvents();
    app->updateCamera();

#ifdef GUI
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (app->m_show_gui)
      drawGUI(app, VK);
#endif

    VK.drawFrame();
  }

  // Cleanup

  VK.destroyAllVulkanResources();

  glfwDestroyWindow(app->GLFW_window);
  glfwTerminate();
}

void framebuffersize_cb(GLFWwindow* window, int w, int h)
{
    assert(false && "Not ready for window resize events.");
}

void scroll_cb(GLFWwindow* window, double x, double y)
{
    #ifdef GUI
    if(ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;
    #endif

    int delta = y;
    if(delta != 0)
        app->myCamera.wheel(delta > 0 ? 1 : -1);
}

void mousebutton_cb(GLFWwindow* window, int button, int action, int mods)
{
    #ifdef GUI
    if(ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;
    #endif

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    app->myCamera.setMousePosition(x, y);
}

void cursorpos_cb(GLFWwindow* window, double x, double y)
{
    #ifdef GUI
    if(ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse)
        return;
    #endif

    if (app->myCamera.lmb || app->myCamera.rmb || app->myCamera.mmb)
        app->myCamera.mouseMove(x, y);
}

void char_cb(GLFWwindow* window, unsigned int key)
{
    #ifdef GUI
    if(ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard)
        return;
    #endif
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const bool pressed = action != GLFW_RELEASE;

    if (pressed && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, 1);
}

static float lastTime = 0;

void App::updateCamera()
{
    float now = glfwGetTime();
    float deltaTime = now-lastTime;
    lastTime = now;

    float dist = myCamera.rate*deltaTime; // distance = rate * time

    float rad = 3.14159/180;
  
    myCamera.lmb   = glfwGetMouseButton(GLFW_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        
    if (glfwGetKey(GLFW_window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 direction = glm::vec3(sin(myCamera.spin*rad), 0.0, -cos(myCamera.spin*rad));
        myCamera.eyeMoveBy(dist*direction); }
    
    if (glfwGetKey(GLFW_window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 direction = glm::vec3(sin(myCamera.spin*rad), 0.0, -cos(myCamera.spin*rad));
        myCamera.eyeMoveBy(-dist*direction); }
    
    if (glfwGetKey(GLFW_window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 direction = glm::vec3(cos(myCamera.spin*rad), 0.0, sin(myCamera.spin*rad));
        myCamera.eyeMoveBy(-dist*direction); }
    
    if (glfwGetKey(GLFW_window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 direction = glm::vec3(cos(myCamera.spin*rad), 0.0, sin(myCamera.spin*rad));
        myCamera.eyeMoveBy(dist*direction); }
    
    if (glfwGetKey(GLFW_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        myCamera.eyeMoveBy(dist*glm::vec3(0,1,0)); }
        
    if (glfwGetKey(GLFW_window, GLFW_KEY_C) == GLFW_PRESS) {
        myCamera.eyeMoveBy(dist*glm::vec3(0,-1,0)); }
}

App::App(int argc, char** argv)
{
    doApiDump = false;

    int argi = 1;
    while (argi<argc) {
        std::string arg = argv[argi++];
        if (arg == "-d")
            doApiDump = true;
        else {
            printf("Unknown argument: %s\n", arg.c_str());
            exit(-1); } }

    glfwSetErrorCallback(onErrorCallback);

    if(!glfwInit()) {
        printf("Could not initialize GLFW.");
        exit(1); }
  
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFW_window = glfwCreateWindow(WIDTH, HEIGHT, PROJECT.c_str(), nullptr, nullptr);

    if(!glfwVulkanSupported()) {
        printf("GLFW: Vulkan Not Supported\n");
        exit(1); }

    glfwSetWindowUserPointer(GLFW_window, this);
    glfwSetKeyCallback(GLFW_window, &key_cb);
    glfwSetCharCallback(GLFW_window, &char_cb);
    glfwSetCursorPosCallback(GLFW_window, &cursorpos_cb);
    glfwSetMouseButtonCallback(GLFW_window, &mousebutton_cb);
    glfwSetScrollCallback(GLFW_window, &scroll_cb);
    glfwSetFramebufferSizeCallback(GLFW_window, &framebuffersize_cb);
}
