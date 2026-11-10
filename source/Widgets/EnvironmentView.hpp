#pragma once
#include "IWidget.hpp"
#include "Environment.hpp"
#include "WidgetReference.hpp"

namespace RetroFuturaGUI
{
    /// @brief How an EnvironmentView's camera projects the world
    enum class CameraMode : u32
    {
        Perspective,
        Orthographic
    };

    /// @brief Shows an Environment, usually one in another window, through a camera of its own: one world, owned once by its
    ///        Environment, seen from several windows at once. A model moved or changed in it shows in every view. The view only
    ///        shows the world: hover and clicks stay with the Environment's own window, and a drop-down list opened there stays
    ///        there. It draws into its own rectangle, untilted
    class EnvironmentView final : public IWidget
    {
    public:
        EnvironmentView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        EnvironmentView() = delete;
        EnvironmentView(const EnvironmentView&) = delete;
        EnvironmentView(EnvironmentView&&) = delete;
        auto operator =(const EnvironmentView&) = delete;
        auto operator =(EnvironmentView&&) = delete;
        ~EnvironmentView() override = default;

        /// @brief Draws the world through the camera into the view's rectangle
        void Draw() override;

        /// @brief Shows the given Environment, nullptr nothing. The view borrows it, and shows nothing once it is freed
        void SetWorld(Environment* world);

        Environment* GetWorld() const;

        /// @brief Places the camera: where the eye is and the point it looks at, in the world's own units, and which way is up
        void SetCamera(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

        const glm::vec3& GetEye() const;
        const glm::vec3& GetTarget() const;
        const glm::vec3& GetUp() const;

        /// @brief Shows the world in perspective, fieldOfView degrees from the bottom of the view to its top (60 by default)
        void SetPerspective(const f32 fieldOfView);

        /// @brief Shows the world without perspective, height world units from the bottom of the view to its top
        void SetOrthographic(const f32 height);

        CameraMode GetCameraMode() const;

        /// @brief Whether a view is drawing its world right now. A drop-down in the world keeps its list to its own window then
        static bool IsDrawingWorld();

    private:
        /// @brief The camera's projection times its view, for a view aspect times as wide as high, reaching past a world worldRadius units around.
        glm::mat4 getCamera(const f32 aspect, const f32 worldRadius) const;

        WidgetReference<Environment> _world {};
        glm::vec3
            _eye { 0.0f, 0.0f, 10.0f },
            _target { 0.0f },
            _up { 0.0f, 1.0f, 0.0f };
        f32
            _fieldOfView { 60.0f },
            _orthographicHeight { 10.0f };
        CameraMode _cameraMode { CameraMode::Perspective };
        static inline bool _isDrawingWorld { false };
    };
}