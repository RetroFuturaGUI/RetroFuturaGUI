#include "Model.hpp"
#include "Texture.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <print>

RetroFuturaGUI::Model::Model(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Model;
}

void RetroFuturaGUI::Model::LoadModel(std::string_view path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::println("ERROR::ASSIMP::{}", importer.GetErrorString());
        return;
    }

    _directory = std::string(path.substr(0, path.find_last_of('/')));
    processNode(scene->mRootNode, scene);
}

void RetroFuturaGUI::Model::Draw()
{
    for(Mesh& mesh : _meshes)
        mesh.Draw(_projection.GetProjectionMatrix(), _translationMatrix, _rotationMatrix, _scalingMatrix);
}

void RetroFuturaGUI::Model::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    _scalingMatrix = glm::scale(glm::mat4(1.0f), size);
}

void RetroFuturaGUI::Model::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);
    _translationMatrix = glm::translate(glm::mat4(1.0f), position);
}

void RetroFuturaGUI::Model::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);
    glm::vec3 radians = glm::radians(rotation);
    _rotationMatrix =
        glm::rotate(glm::mat4(1.0f), radians.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), radians.x, glm::vec3(1.0f, 0.0f, 0.0f));
}

RetroFuturaGUI::Mesh RetroFuturaGUI::Model::processMesh(aiMesh* unprocessedMesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<MeshTexture> textures;

    vertices.reserve(unprocessedMesh->mNumVertices);

    for(u32 i = 0; i < unprocessedMesh->mNumVertices; ++i)
    {
        Vertex vertex;
        Vec3 vector;

        // positions
        vector._X = unprocessedMesh->mVertices[i].x;
        vector._Y = unprocessedMesh->mVertices[i].y;
        vector._Z = unprocessedMesh->mVertices[i].z;
        vertex._Position = vector;

        // normals
        if (unprocessedMesh->HasNormals())
        {
            vector._X = unprocessedMesh->mNormals[i].x;
            vector._Y = unprocessedMesh->mNormals[i].y;
            vector._Z = unprocessedMesh->mNormals[i].z;
            vertex._Normal = vector;
        }

        // texture coordinates
        if(unprocessedMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            Vec2 vec;
            vec._X = unprocessedMesh->mTextureCoords[0][i].x;
            vec._Y = unprocessedMesh->mTextureCoords[0][i].y;
            vertex._TextureCoordinate = vec;
        }
        else
            vertex._TextureCoordinate = Vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for(u32 i = 0; i < unprocessedMesh->mNumFaces; ++i)
    {
        aiFace face = unprocessedMesh->mFaces[i];

        for(u32 j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    
    aiMaterial* material = scene->mMaterials[unprocessedMesh->mMaterialIndex];
    std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

void RetroFuturaGUI::Model::processNode(aiNode *node, const aiScene *scene)
{
    for(u32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh *mesh { scene->mMeshes[node->mMeshes[i]] };
        _meshes.push_back(processMesh(mesh, scene));
    }
    
    for(u32 i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene);
}

std::vector<RetroFuturaGUI::MeshTexture> RetroFuturaGUI::Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName)
{
    std::vector<MeshTexture> textures;

    for(u32 i = 0; i < mat->GetTextureCount(type); ++i)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string fullPath { _directory + "/" + str.C_Str() };
        Texture* texture { nullptr };

        for(const std::unique_ptr<Texture>& loaded : _loadedTextures)
        {
            if(loaded->GetPath() == fullPath)
            {
                texture = loaded.get();
                break;
            }
        }

        if(!texture)
        {
            _loadedTextures.push_back(std::make_unique<Texture>(fullPath, true));
            texture = _loadedTextures.back().get();
        }

        textures.push_back(MeshTexture{ texture, std::string(typeName), fullPath });
    }

    return textures;
}
