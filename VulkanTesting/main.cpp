//
//  main.cpp
//  VulkanTesting
//
//  Created by Apple on 17/05/21.
//
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// This header defines a set of standard exceptions that both the library and programs can use to report common errors
#include <stdexcept>
#include <vector>

#include <iostream>

#include "VulkanRenderer.hpp"

#include <unistd.h>

GLFWwindow *window;
VulkanRenderer vulkanRenderer;

// initializes window for rendering
int initWindow(std::string wName="Vulkan", const int width=800, const int height=600){
    // initialize glfw
    glfwInit();
    
    // set GLFW not to work with OpenGL which is default for GLFW
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(width, height, wName.c_str(), nullptr, nullptr);
    
    // create vulkan renderer instance
    if(vulkanRenderer.init(window) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int main() {
    // create window
    initWindow("Vulkan", 1366, 768);

    float angle = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    char* dir = getcwd(NULL, 0);
    printf("Current directory path - %s\n", dir);
    int helicopter = vulkanRenderer.createMeshModel("FA18f/FA-18F.obj");
    
    // game loop
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        
        float now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;
        
        angle += 10.0f * deltaTime;
        if(angle > 360.0f){
            angle -= 360.0f;
        }
        
        glm::mat4 testMat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        //testMat = glm::rotate(testMat, glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        vulkanRenderer.updateModel(helicopter, testMat);
        
        vulkanRenderer.draw();
    }
    
    // perform clean up activities here
    vulkanRenderer.cleanUp();
    // destroy GLFW window and stop GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
