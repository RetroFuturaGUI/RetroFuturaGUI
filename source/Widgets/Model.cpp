#include "Model.hpp"
#include "Texture.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <print>

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

void RetroFuturaGUI::Model::SetRotation(const f32 rotation)
{
    IWidget::SetRotation(rotation);
    _rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}

RetroFuturaGUI::Mesh RetroFuturaGUI::Model::processMesh(aiMesh* unprocessedMesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<MeshTexture> textures;

    vertices.reserve(unprocessedMesh->mNumVertices);

    // walk through each of the mesh's vertices
    for(u32 i = 0; i < unprocessedMesh->mNumVertices; i++)
    {
        Vertex vertex;
        Vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to our Vec3 class so we transfer the data to this placeholder first.
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
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec._X = unprocessedMesh->mTextureCoords[0][i].x;
            vec._Y = unprocessedMesh->mTextureCoords[0][i].y;
            vertex._TextureCoordinate = vec;
        }
        else
            vertex._TextureCoordinate = Vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(u32 i = 0; i < unprocessedMesh->mNumFaces; ++i)
    {
        aiFace face = unprocessedMesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(u32 j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials. The current shader pipeline only supports a single unlit diffuse
    // texture per draw call, so that's the only material slot we load.
    aiMaterial* material = scene->mMaterials[unprocessedMesh->mMaterialIndex];
    std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // return a mesh object created from the extracted mesh data
    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

void RetroFuturaGUI::Model::processNode(aiNode *node, const aiScene *scene)
{
    // process all the node's meshes (if any)
    for(u32 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for(u32 i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

std::vector<RetroFuturaGUI::MeshTexture> RetroFuturaGUI::Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName)
{
    std::vector<MeshTexture> textures;

    for(u32 i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        // check if texture was loaded before for this model and if so, reuse it: skip loading a new texture
        Texture* texture = nullptr;
        for(const std::unique_ptr<Texture>& loaded : _loadedTextures)
        {
            if(loaded->GetPath() == str.C_Str())
            {
                texture = loaded.get();
                break;
            }
        }

        if(!texture)
        {
            // aiProcess_FlipUVs already flips texture coordinates at the vertex level, so the
            // pixel data itself must not also be flipped here.
            _loadedTextures.push_back(std::make_unique<Texture>(_directory + "/" + str.C_Str(), false));
            texture = _loadedTextures.back().get();
        }

        textures.push_back(MeshTexture{ texture, std::string(typeName), str.C_Str() });
    }
    return textures;
}
