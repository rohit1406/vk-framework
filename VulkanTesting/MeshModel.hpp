//
//  MeshModel.hpp
//  VulkanTesting
//
//  Created by Apple on 27/05/21.
//

#ifndef MeshModel_hpp
#define MeshModel_hpp

#include <vector>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Mesh.hpp"
#include <stdio.h>

class MeshModel{
public:
    MeshModel();
    MeshModel(std::vector<Mesh> newMeshList);
    ~MeshModel();
    
    size_t getMeshCount();
    Mesh* getMesh(size_t index);
    
    glm::mat4 getModel();
    void setModel(glm::mat4 newModel);
    
    void destroyMeshModel();
    
    static std::vector<std::string> LoadMaterials(const aiScene *scene);
    static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiNode* node, const aiScene* scene, std::vector<int> matToTex);
    static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh* mesh, const aiScene* scene, std::vector<int> matToTex);
    
private:
    std::vector<Mesh> meshList;
    glm::mat4 model;
};
#endif /* MeshModel_hpp */
