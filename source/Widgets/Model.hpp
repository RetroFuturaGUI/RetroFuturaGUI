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
            Model(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
            Model(const Model&) = delete;
            Model(Model&&) = delete;
            auto operator =(const Model&) = delete;
            auto operator =(Model&&) = delete;
            ~Model() override = default;

            void LoadModel(std::string_view path);
            void Draw() override;
            void SetSize(const glm::vec3& size) override;
            void SetPosition(const glm::vec3& position) override;
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
