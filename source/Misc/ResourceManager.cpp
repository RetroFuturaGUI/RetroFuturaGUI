#include "ResourceManager.hpp"
#include "PlatformBridge.hpp"
#include "SceneLoader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <print>

RetroFuturaGUI::Texture* RetroFuturaGUI::ResourceManager::GetTexture(std::string_view path)
{
    SceneLoader::recordAsset(path); //a texture handed out again counts for the scene asking too
    return findOrLoadTexture(path);
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateSvgTexture(std::string_view path, Projection* projection, const glm::i32vec2 initialResolution)
{
    const std::string* source { findSvgSource(path) };

    if(!source)
        return nullptr;

    return std::make_unique<SvgTexture>(*source, path, true, projection, initialResolution);
}

std::vector<RetroFuturaGUI::Mesh>* RetroFuturaGUI::ResourceManager::GetModel(std::string_view path)
{
    LoadedModel* model { findOrLoadModel(path) };

    if(!model)
        return nullptr;

    recordModel(path, *model);
    return &model->_Meshes;
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateCheckmarkIcon(Projection* projection)
{
    return CreateSvgTexture(getIconPath("checkmark.svg"), projection);
}

std::unique_ptr<RetroFuturaGUI::SvgTexture> RetroFuturaGUI::ResourceManager::CreateDropDownArrowIcon(Projection* projection)
{
    return CreateSvgTexture(getIconPath("DropDownArrow.svg"), projection);
}

bool RetroFuturaGUI::ResourceManager::IsLoaded(std::string_view path)
{
    const uSize hash { hashPath(path) };
    return _textures.contains(hash) || _svgSources.contains(hash) || _models.contains(hash);
}

void RetroFuturaGUI::ResourceManager::Free(std::string_view path)
{
    const uSize hash { hashPath(path) };
    _textures.erase(hash);
    _svgSources.erase(hash);
    _models.erase(hash);
}

void RetroFuturaGUI::ResourceManager::Release()
{
    _models.clear();
    _textures.clear();
    _svgSources.clear();
}

uSize RetroFuturaGUI::ResourceManager::hashPath(std::string_view path)
{
    return std::hash<std::string_view>{}(path);
}

std::string RetroFuturaGUI::ResourceManager::getIconPath(std::string_view fileName)
{
    const std::filesystem::path iconPath { PlatformBridge::Paths::GetModuleFolder() / "Resources" / "img" / fileName };
    const std::u8string text { iconPath.u8string() };
    return std::string(text.begin(), text.end());
}

const std::string* RetroFuturaGUI::ResourceManager::findSvgSource(std::string_view path)
{
    SceneLoader::recordAsset(path);
    const uSize hash { hashPath(path) };
    auto found { _svgSources.find(hash) };

    if(_svgSources.end() != found)
        return &found->second;

    SceneLoader::finishPreparing(path); //read on a loading thread already, perhaps

    found = _svgSources.find(hash);

    if(_svgSources.end() != found)
        return &found->second;

    //UTF-8 into a native path, so names outside the system's code page open too!
    std::ifstream file { std::filesystem::path(std::u8string(path.begin(), path.end())), std::ios::binary };

    if(!file)
    {
        std::println("Error reading SVG: {}", path);
        return nullptr;
    }

    found = _svgSources.emplace(hash, std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>())).first;
    return &found->second;
}

RetroFuturaGUI::Texture* RetroFuturaGUI::ResourceManager::findOrLoadTexture(std::string_view path)
{
    const uSize hash { hashPath(path) };
    auto found { _textures.find(hash) };

    if(_textures.end() != found)
        return found->second.get();

    SceneLoader::finishPreparing(path); //decoded on a loading thread already, perhaps
    found = _textures.find(hash);

    if(_textures.end() != found)
        return found->second.get();

    return _textures.emplace(hash, std::make_unique<Texture>(path, true)).first->second.get();
}

RetroFuturaGUI::ResourceManager::LoadedModel* RetroFuturaGUI::ResourceManager::findOrLoadModel(std::string_view path)
{
    const uSize hash { hashPath(path) };
    auto found { _models.find(hash) };

    if(_models.end() != found)
        return found->second.get();

    SceneLoader::finishPreparing(path); //read on a loading thread already, perhaps
    found = _models.find(hash);

    if(_models.end() != found)
        return found->second.get();

    std::optional<PreparedModel> prepared { prepareModel(path, false) };

    if(!prepared)
        return nullptr;

    return adoptModel(path, std::move(*prepared));
}

void RetroFuturaGUI::ResourceManager::recordModel(std::string_view path, const LoadedModel& model)
{
    SceneLoader::recordAsset(path);

    //Its textures count for the scene asking too, or a scene closing would free them under this one
    for(const std::string& texturePath : model._TexturePaths)
        SceneLoader::recordAsset(texturePath);
}

void RetroFuturaGUI::ResourceManager::adoptTexture(std::string_view path, DecodedImage&& image)
{
    const uSize hash { hashPath(path) };

    if(_textures.contains(hash))
        return;

    _textures.emplace(hash, std::make_unique<Texture>(std::move(image)));
}

void RetroFuturaGUI::ResourceManager::adoptSvgSource(std::string_view path, std::string&& text)
{
    _svgSources.try_emplace(hashPath(path), std::move(text));
}

RetroFuturaGUI::ResourceManager::LoadedModel* RetroFuturaGUI::ResourceManager::adoptModel(std::string_view path, PreparedModel&& prepared)
{
    const uSize hash { hashPath(path) };
    const auto found { _models.find(hash) };

    if(_models.end() != found)
        return found->second.get();

    return _models.emplace(hash, uploadModel(std::move(prepared))).first->second.get();
}

std::optional<RetroFuturaGUI::ResourceManager::PreparedModel> RetroFuturaGUI::ResourceManager::prepareModel(std::string_view path, const bool decodeTextures)
{
    Assimp::Importer importer;
    const aiScene* scene { importer.ReadFile(std::string(path), aiProcess_Triangulate | aiProcess_FlipUVs) };

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::println("ERROR::ASSIMP::{}", importer.GetErrorString());
        return std::nullopt;
    }

    PreparedModel model {};
    prepareNode(scene->mRootNode, scene, path.substr(0, path.find_last_of('/')), model);

    if(!decodeTextures)
        return model;

    for(const std::string& texturePath : model._TexturePaths)
    {
        std::optional<DecodedImage> image { Texture::DecodeFile(std::filesystem::path(std::u8string(texturePath.begin(), texturePath.end())), true) };

        if(image)
            model._Images.emplace(texturePath, std::move(*image));
    }

    return model;
}

void RetroFuturaGUI::ResourceManager::prepareNode(const aiNode* node, const aiScene* scene, std::string_view directory, PreparedModel& model)
{
    for(u32 i = 0; i < node->mNumMeshes; ++i)
        model._Meshes.push_back(prepareMesh(scene->mMeshes[node->mMeshes[i]], scene, directory, model));

    for(u32 i = 0; i < node->mNumChildren; ++i)
        prepareNode(node->mChildren[i], scene, directory, model);
}

RetroFuturaGUI::ResourceManager::PreparedMesh RetroFuturaGUI::ResourceManager::prepareMesh(const aiMesh* unprocessedMesh, const aiScene* scene, std::string_view directory, PreparedModel& model)
{
    PreparedMesh mesh {};
    mesh._Vertices.reserve(unprocessedMesh->mNumVertices);

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

        mesh._Vertices.push_back(vertex);
    }

    for(u32 i = 0; i < unprocessedMesh->mNumFaces; ++i)
    {
        const aiFace& face { unprocessedMesh->mFaces[i] };

        for(u32 j = 0; j < face.mNumIndices; ++j)
            mesh._Indices.push_back(face.mIndices[j]);
    }

    const aiMaterial* material { scene->mMaterials[unprocessedMesh->mMaterialIndex] };

    for(u32 i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i)
    {
        aiString file;
        material->GetTexture(aiTextureType_DIFFUSE, i, &file);
        std::string fullPath { std::string(directory) + "/" + file.C_Str() };

        if(std::find(model._TexturePaths.begin(), model._TexturePaths.end(), fullPath) == model._TexturePaths.end())
            model._TexturePaths.push_back(fullPath);

        mesh._TexturePaths.push_back(std::move(fullPath));
    }

    return mesh;
}

std::unique_ptr<RetroFuturaGUI::ResourceManager::LoadedModel> RetroFuturaGUI::ResourceManager::uploadModel(PreparedModel&& prepared)
{
    for(auto& [texturePath, image] : prepared._Images)
        adoptTexture(texturePath, std::move(image));

    std::unique_ptr<LoadedModel> model { std::make_unique<LoadedModel>() };
    model->_TexturePaths = std::move(prepared._TexturePaths);
    model->_Meshes.reserve(prepared._Meshes.size());

    for(PreparedMesh& mesh : prepared._Meshes)
    {
        std::vector<MeshTexture> textures;

        for(std::string& texturePath : mesh._TexturePaths)
        {
            Texture* texture { findOrLoadTexture(texturePath) }; //loaded once, however many meshes and models use it
            textures.push_back(MeshTexture { ._Texture = texture, ._Type = "texture_diffuse", ._Path = std::move(texturePath) });
        }

        model->_Meshes.emplace_back(std::move(mesh._Vertices), std::move(mesh._Indices), std::move(textures));
    }

    return model;
}
