#include "ContextObjects.hpp"
#include <utility>

RetroFuturaGUI::ContextObject::ContextObject(const Kind kind, std::function<void()> describe)
    : _kind(kind), _describe(std::move(describe))
{
}

RetroFuturaGUI::ContextObject::~ContextObject()
{
    //Null when no context is current (during shutdown, ...); every name waits
    GLFWwindow* current { glfwGetCurrentContext() };

    for(auto& [context, contextNames] : getNames())
    {
        const auto found { contextNames.find(this) };

        if(found == contextNames.end())
            continue;

        if(context == current)
            destroy(_kind, found->second);
        else
            getPendingNames()[context].push_back(PendingName { ._Kind = _kind, ._Name = found->second });

        contextNames.erase(found);
    }
}

void RetroFuturaGUI::ContextObject::DeletePending(GLFWwindow* context)
{
    if(!context)
        return;

    if(glfwGetCurrentContext() != context)
        return;

    const auto found { getPendingNames().find(context) };

    if(found == getPendingNames().end())
        return;

    for(const PendingName& pending : found->second)
        destroy(pending._Kind, pending._Name);

    getPendingNames().erase(found);
}

void RetroFuturaGUI::ContextObject::ForgetContext(GLFWwindow* context)
{
    if(!context)
        return;

    getNames().erase(context);
    getPendingNames().erase(context);
}

void RetroFuturaGUI::ContextObject::bindInCurrentContext()
{
    GLFWwindow* context { glfwGetCurrentContext() };

    if(!context)
        return;

    std::unordered_map<const ContextObject*, u32>& contextNames { getNames()[context] };
    const auto found { contextNames.find(this) };

    if(found != contextNames.end())
    {
        bind(_kind, found->second);
        return;
    }

    //First use in this context: the new name is set up while it is bound, and stays bound for the caller
    const u32 name { create(_kind) };
    contextNames.emplace(this, name);
    bind(_kind, name);

    if(_describe)
        _describe();
}

std::unordered_map<GLFWwindow*, std::unordered_map<const RetroFuturaGUI::ContextObject*, u32>>& RetroFuturaGUI::ContextObject::getNames()
{
    static std::unordered_map<GLFWwindow*, std::unordered_map<const ContextObject*, u32>>* table { new std::unordered_map<GLFWwindow*, std::unordered_map<const ContextObject*, u32>>() };
    return *table;
}

std::unordered_map<GLFWwindow*, std::vector<RetroFuturaGUI::ContextObject::PendingName>>& RetroFuturaGUI::ContextObject::getPendingNames()
{
    static std::unordered_map<GLFWwindow*, std::vector<PendingName>>* table { new std::unordered_map<GLFWwindow*, std::vector<PendingName>>() };
    return *table;
}

u32 RetroFuturaGUI::ContextObject::create(const Kind kind)
{
    u32 name { 0 };

    if(Kind::Framebuffer == kind)
        glGenFramebuffers(1, &name);
    else
        glGenVertexArrays(1, &name);

    return name;
}

void RetroFuturaGUI::ContextObject::destroy(const Kind kind, const u32 name)
{
    if(0 == name)
        return;

    if(Kind::Framebuffer == kind)
        glDeleteFramebuffers(1, &name);
    else
        glDeleteVertexArrays(1, &name);
}

void RetroFuturaGUI::ContextObject::bind(const Kind kind, const u32 name)
{
    if(Kind::Framebuffer == kind)
        glBindFramebuffer(GL_FRAMEBUFFER, name);
    else
        glBindVertexArray(name);
}

RetroFuturaGUI::VertexArray::VertexArray(std::function<void()> describeLayout)
    : ContextObject(Kind::VertexArray, std::move(describeLayout))
{
}

void RetroFuturaGUI::VertexArray::Bind()
{
    bindInCurrentContext();
}

void RetroFuturaGUI::VertexArray::Unbind()
{
    glBindVertexArray(0);
}

RetroFuturaGUI::Framebuffer::Framebuffer(std::function<void()> attach)
    : ContextObject(Kind::Framebuffer, std::move(attach))
{
}

void RetroFuturaGUI::Framebuffer::Bind()
{
    bindInCurrentContext();
}

void RetroFuturaGUI::Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}