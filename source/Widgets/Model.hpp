#pragma once
#include "IWidget.hpp"
#include "Mesh.hpp"
#include <vector>

namespace RetroFuturaGUI
{
    class Model final : public IWidget
    {
        public:
            /// @brief Constructs an empty Model widget under the given parent widget/window. Call LoadModel to populate it.
            Model(std::string_view name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
            Model(const Model&) = delete;
            Model(Model&&) = delete;
            auto operator =(const Model&) = delete;
            auto operator =(Model&&) = delete;
            ~Model() override = default;

            /// @brief Shows the 3D model file at path, replacing any shown before. ResourceManager imports each file once, and
            ///        every Model showing it borrows the same meshes, each with its own place, size and turn
            void LoadModel(std::string_view path);

            /// @brief Draws all meshes of the loaded model.
            void Draw() override;

        protected:
            /// @brief Sets the size (scale) of the model.
            void applySize(const glm::vec3& size) override;

            /// @brief Sets the world position of the model.
            void applyPosition(const glm::vec3& position) override;

        private:
            std::vector<Mesh>* _meshes { nullptr };
            glm::mat4
                _translationMatrix { 1.0f },
                _scalingMatrix { 1.0f };
    };

}