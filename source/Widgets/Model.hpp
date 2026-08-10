#pragma once
#include "IWidget.hpp"
#include "Mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace RetroFuturaGUI
{
    class Model : public IWidget
    {
        public:
            /// @brief Constructs an empty Model widget under the given parent widget/window. Call LoadModel to populate it.
            Model(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
            Model(const Model&) = delete;
            Model(Model&&) = delete;
            auto operator =(const Model&) = delete;
            auto operator =(Model&&) = delete;
            ~Model() override = default;

            /// @brief Loads a 3D model file via Assimp, replacing any previously loaded meshes.
            void LoadModel(std::string_view path);

            /// @brief Draws all meshes of the loaded model.
            void Draw() override;

            /// @brief Sets the size (scale) of the model.
            void SetSize(const glm::vec3& size) override;

            /// @brief Sets the world position of the model.
            void SetPosition(const glm::vec3& position) override;

            /// @brief Sets the rotation of the model.
            void SetRotation(const glm::vec3& rotation) override;

        private:
            void processNode(aiNode* node, const aiScene* scene);
            Mesh processMesh(aiMesh* unprocessedMesh, const aiScene* scene);
            std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string_view typeName);

            std::vector<std::unique_ptr<Texture>> _loadedTextures;
            std::vector<Mesh> _meshes;
            std::string _directory;
            glm::mat4
                _translationMatrix { 1.0f },
                _rotationMatrix { 1.0f },
                _scalingMatrix { 1.0f };
    };

}
