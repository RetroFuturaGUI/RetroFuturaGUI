#pragma once
#include "SvgTexture.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;

namespace RetroFuturaGUI
{
    /// @brief Manages, shares and copies resources loaded from disk
    class ResourceManager
    {
    public:
        /// @brief The texture loaded from the image file at path (UTF-8). It is kept until Free(path), or until the last scene counting it closes, so every widget showing the file borrows the same one
        static Texture* GetTexture(std::string_view path);

        /// @brief A new SvgTexture made from the SVG file at path (UTF-8). Its colors and size can be changed without touching anyone else's. After Free(path) no new copies can be acquired.
        static std::unique_ptr<SvgTexture> CreateSvgTexture(std::string_view path, Projection* projection = nullptr, const glm::i32vec2 initialResolution = glm::i32vec2(0));

        /// @brief The meshes of the 3D model file at path (UTF-8), imported once with their material textures and kept like a
        ///        texture, so every Model widget showing the file borrows the same ones. nullptr when the file didn't import
        static std::vector<Mesh>* GetModel(std::string_view path);

        /// @brief A new checkmark icon of its own; see CreateSvgTexture.
        static std::unique_ptr<SvgTexture> CreateCheckmarkIcon(Projection* projection);

        /// @brief A new drop-down arrow icon of its own; see CreateSvgTexture.
        static std::unique_ptr<SvgTexture> CreateDropDownArrowIcon(Projection* projection);

        /// @brief Whether a texture, SVG text or model from path is kept
        static bool IsLoaded(std::string_view path);

        /// @brief Frees the texture, SVG text or model loaded from path. Whoever frees one must make sure no widget still uses it. SvgTextures already made stay with their owners.
        static void Free(std::string_view path);

        /// @brief Frees every texture, SVG image and model, so they go while GL still runs
        static void Release();
        static constexpr const glm::vec4 _Eigengrau { 0.086f, 0.086f, 0.114f, 1.0f };

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;
        auto operator=(const ResourceManager&) = delete;
        auto operator=(ResourceManager&&) = delete;

        friend class SceneLoader; //loads a scene's files ahead, on its own threads, and hands them over here

        /// @brief A model's meshes and the files of the textures they show, which count with the model wherever it is used
        struct LoadedModel
        {
            std::vector<Mesh> _Meshes {};
            std::vector<std::string> _TexturePaths {};
        };

        /// @brief One mesh read from a model file, not uploaded yet.
        struct PreparedMesh
        {
            std::vector<Vertex> _Vertices {};
            std::vector<u32> _Indices {};
            std::vector<std::string> _TexturePaths {}; //its diffuse textures
        };

        /// @brief A model read from its file without GL, so any thread can read it: its meshes and, when asked for, its
        ///        textures decoded too.
        struct PreparedModel
        {
            std::vector<PreparedMesh> _Meshes {};
            std::vector<std::string> _TexturePaths {}; //every texture once
            std::unordered_map<std::string, DecodedImage> _Images {}; //decoded ahead, by path
        };

        /// @brief The key a texture, an SVG text or a model is kept under.
        static uSize hashPath(std::string_view path);

        /// @brief The UTF-8 path of an icon in the Resources/img folder next to the executable.
        static std::string getIconPath(std::string_view fileName);

        /// @brief The text of the SVG file at path, read once. nullptr when it can't be read.
        static const std::string* findSvgSource(std::string_view path);

        /// @brief The texture from path, loaded now if it isn't kept yet, without counting it for a scene.
        static Texture* findOrLoadTexture(std::string_view path);

        /// @brief The model from path, imported now if it isn't kept yet, without counting it for a scene.
        static LoadedModel* findOrLoadModel(std::string_view path);

        /// @brief Counts a model and the textures it shows for the scene loading it.
        static void recordModel(std::string_view path, const LoadedModel& model);

        /// @brief Keeps an image decoded on another thread as the texture of path, unless one is kept already.
        static void adoptTexture(std::string_view path, DecodedImage&& image);

        /// @brief Keeps an SVG file's text read on another thread, unless it is kept already.
        static void adoptSvgSource(std::string_view path, std::string&& text);

        /// @brief Uploads a model read on another thread and keeps it, unless it is kept already. nullptr if it is empty.
        static LoadedModel* adoptModel(std::string_view path, PreparedModel&& prepared);

        /// @brief Reads a model file without GL, so it may run on any thread. Nothing when Assimp can't read it.
        /// @param decodeTextures Whether its textures are decoded too, for a loading thread.
        static std::optional<PreparedModel> prepareModel(std::string_view path, const bool decodeTextures);

        /// @brief Adds the meshes of a node and its children.
        static void prepareNode(const aiNode* node, const aiScene* scene, std::string_view directory, PreparedModel& model);

        /// @brief One mesh, its vertices, indices and the paths of its diffuse textures.
        static PreparedMesh prepareMesh(const aiMesh* unprocessedMesh, const aiScene* scene, std::string_view directory, PreparedModel& model);

        /// @brief Uploads the meshes, with their textures from the decoded images or, for those not decoded, from their files.
        static std::unique_ptr<LoadedModel> uploadModel(PreparedModel&& prepared);

        static inline std::unordered_map<uSize, std::unique_ptr<Texture>> _textures {}; //by their path's hash
        static inline std::unordered_map<uSize, std::string> _svgSources {}; //each SVG file's text, by its path's hash
        static inline std::unordered_map<uSize, std::unique_ptr<LoadedModel>> _models {}; //by their path's hash
    };
}
