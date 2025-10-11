#pragma once
#include <config.hpp>
#include <string>

class IWidgetBase
{
public:
    IWidgetBase(const std::string& name, void* parent = nullptr);
    std::string GetName() const;
    void SetName(const std::string& name);

    template<typename T> T* GetParent() const
    {
        return static_cast<T*>(_parent);
    }

    template<typename T> T* GetChild() const
    {
        return static_cast<T*>(_child);
    }

    template<typename T> void SetChild(T* child)
    {
        _child = static_cast<void*>(child);
    }

protected:
    std::string _name;
    //WidgetTypeID _typeId;
    void* _parent;
    void* _child;
    
private:

};