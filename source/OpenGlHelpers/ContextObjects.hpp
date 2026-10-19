#pragma once
#include "IncludeHelper.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief The GL objects every context keeps its own of. Buffers, textures and shaders are shared between all windows,
    ///        VAOs and framebuffers are not, so an object drawn in several windows' contexts - a window drawn inside another
    ///        window's tab, an icon shared between windows - needs one of them in each. One is made the first time a context
    ///        binds it, and deleted only while its own context is current.
    class ContextObject
    {
    public:
        ContextObject(const ContextObject&) = delete;
        ContextObject(ContextObject&&) = delete;
        auto operator =(const ContextObject&) = delete;
        auto operator =(ContextObject&&) = delete;

        /// @brief Deletes what objects destroyed while another context was current left behind for the given context. Does
        ///        nothing unless that context is the current one. Window::Draw calls it right after making its context current.
        static void DeletePending(GLFWwindow* context);

        /// @brief Drops every name of a context that is gone, since they died with it. ~Window calls it once its native window
        ///        is destroyed, so a later window that GLFW gives the same address never finds them.
        static void ForgetContext(GLFWwindow* context);

    protected:
        enum class Kind : u32
        {
            VertexArray,
            Framebuffer
        };

        /// @param describe sets a new name up while it is bound: a VAO's buffers and attribute pointers, a framebuffer's attachments
        ContextObject(const Kind kind, std::function<void()> describe);

        /// @brief Deletes the current context's name now. The other contexts' names wait until their context is current again.
        ~ContextObject();

        /// @brief Binds the current context's name, making and setting it up there first if the context has none yet.
        void bindInCurrentContext();

    private:
        /// @brief A name of a destroyed object, waiting for its context.
        struct PendingName
        {
            Kind _Kind { Kind::VertexArray };
            u32 _Name { 0 };
        };

        Kind _kind { Kind::VertexArray };
        std::function<void()> _describe {};

        /// @brief The names each context holds, by object. Never freed, so an object destroyed late in shutdown can still take
        ///        itself off.
        static std::unordered_map<GLFWwindow*, std::unordered_map<const ContextObject*, u32>>& names();

        /// @brief Names of destroyed objects, by the context that has to be current to delete them. Never freed, like names().
        static std::unordered_map<GLFWwindow*, std::vector<PendingName>>& pendingNames();

        static u32 create(const Kind kind);
        static void destroy(const Kind kind, const u32 name);
        static void bind(const Kind kind, const u32 name);
    };

    /// @brief A vertex array object for every GL context that draws its owner.
    class VertexArray final : public ContextObject
    {
    public:
        /// @param describeLayout binds the owner's buffers and sets its attribute pointers. Runs once in each context.
        explicit VertexArray(std::function<void()> describeLayout);

        /// @brief Binds the current context's VAO, making it there first if needed.
        void Bind();

        /// @brief Binds no VAO.
        static void Unbind();
    };

    /// @brief A framebuffer object for every GL context that renders into its owner's textures.
    class Framebuffer final : public ContextObject
    {
    public:
        /// @param attach attaches the owner's textures. Runs once in each context.
        explicit Framebuffer(std::function<void()> attach);

        /// @brief Binds the current context's framebuffer, making it there first if needed.
        void Bind();

        /// @brief Binds the window's own framebuffer again.
        static void Unbind();
    };
}