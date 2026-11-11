#pragma once
#include "IWidget.hpp"
#include "Environment.hpp"
#include "Plane.hpp"
#include "ContextObjects.hpp"
#include "WidgetReference.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief How an EnvironmentView's camera projects the world
    enum class CameraMode : u32
    {
        Perspective,
        Orthographic
    };

    /// @brief Shows an Environment, usually one in another window, through a camera of its own: one world, owned once by its
    ///        Environment, seen from several windows at once. A model moved or changed in it shows in every view. The pointer
    ///        over a view reaches the world's widgets through the camera, so they hover, take presses, drags, the wheel and
    ///        typing there as in their own window. The world is drawn into a picture the view shows like any widget: tilted,
    ///        faded and cut with what it sits in
    class EnvironmentView final : public IWidget
    {
    public:
        EnvironmentView(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        EnvironmentView() = delete;
        EnvironmentView(const EnvironmentView&) = delete;
        EnvironmentView(EnvironmentView&&) = delete;
        auto operator =(const EnvironmentView&) = delete;
        auto operator =(EnvironmentView&&) = delete;
        ~EnvironmentView() override;

        /// @brief Draws the world through the camera into the view's picture, and shows it
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

        /// @brief Whether a view is drawing its world right now
        static bool IsDrawingWorld();

    protected:
        void applySize(const glm::vec3& size) override;

        void applyPosition(const glm::vec3& position) override;

    private:
        friend class Environment; //passes its widgets the frame's input through the view the pointer is over
        friend class IWidget; //the world's widgets read the pointer through the view
        friend class IDropDown; //an open list of the world's is placed and hit-tested as the view sees it
        friend class Window; //the cursor icon and the wheel of the world's widgets go to the view's window
        friend class TextEditor; //a text box clicked through a view takes the view's window's keys
        friend class Tooltip; //opens in the view's window, at the cursor
        friend class ContextMenu; //opens in the view's window, at the cursor

        /// @brief The pointer a view passes on to its world's widgets.
        struct ViewPointer
        {
            const Environment* _World { nullptr };
            glm::vec2 _WindowPoint { 0.0f }; //a point of the world's window that its projection, made the camera, sends where the view sees the pointer
            GLFWwindow* _Window { nullptr }; //the view's
            glm::vec2 _ViewWindowPoint { 0.0f }; //where the cursor really is, in the view's window
        };

        /// @brief Passes the world the frame's input when the pointer is over the view, or anywhere while a press that began on
        ///        it is held. Returns whether it did.
        bool interactWorld(const bool isPressHeld);

        /// @brief Where the pointer is on the view, from -1 to 1 across it each way. Nothing when it isn't over the view, unless
        ///        anywhere counts, for a press held since it began on it.
        std::optional<glm::vec2> getPointerOnView(const bool anywhere, glm::vec2& outWindowPoint) const;

        /// @brief Draws the world into the picture, sized to the view in pixels.
        void renderWorld(Environment& world, const glm::i32vec2& pixelSize);

        /// @brief The picture's size in pixels. As many along the view's width and height as it spans in the window, through its
        ///        tilt and the window's perspective, so a view brought closer stays sharp and a far one takes fewer
        glm::i32vec2 getPictureSize() const;

        /// @brief The largest side a picture may have. What the GPU allows, at most 4096
        static i32 getMaxPictureSide();

        /// @brief Gives the picture and its depth buffer that size, the first time or when the view's size changed.
        void sizePicture(const glm::i32vec2& pixelSize);

        /// @brief The camera's projection times its view, for a view aspect times as wide as high, reaching past a world worldRadius units around.
        glm::mat4 getCamera(const f32 aspect, const f32 worldRadius) const;

        /// @brief The pointer a view passes on to its world's widgets now, or nothing.
        static const std::optional<ViewPointer>& getViewPointer();

        /// @brief The view's window while a view passes the world's widgets the pointer and window is the world's, else window.
        static GLFWwindow* redirectWindow(GLFWwindow* window);

    //Elements
        std::unique_ptr<Plane> _plane { nullptr }; //shows the picture
        std::vector<glm::vec4> _planeColors { glm::vec4(0.0f) }; //transparent, so the picture shows as it is. The plane keeps a span over it
        std::unique_ptr<Framebuffer> _framebuffer { nullptr };
        u32
            _pictureTexture { 0 },
            _depthBuffer { 0 }; //depth and stencil, which the world's lines use
        glm::i32vec2 _pictureSize { 0 };

    //Logic
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
        static inline std::optional<ViewPointer> _viewPointer {};
    };
}