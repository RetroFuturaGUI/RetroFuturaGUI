#include "IWidgetBase.hpp"

IWidgetBase::IWidgetBase(const std::string& name, void* parent)
{
    _name = name;
    //_typeId = WidgetTypeID::None;
    _parent = parent;
}

std::string IWidgetBase::GetName() const
{
    return _name;
}

void IWidgetBase::SetName(const std::string& name)
{
    _name = name;
}