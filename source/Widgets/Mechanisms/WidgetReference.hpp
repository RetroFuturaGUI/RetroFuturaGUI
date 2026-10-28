#pragma once
#include "IWidget.hpp"
#include <concepts>
#include <cstddef>
#include <memory>

namespace RetroFuturaGUI
{
    /// @brief A smart pointer-like class that borrows a widget and tells whether the widget has been freed. A guard by negation can be used too: if(!_members->_okButton) return;
    /// @warning -> and * on an empty reference reach nullptr, so guard it before using it.
    template<typename T> class WidgetReference
    {
        static_assert(std::derived_from<T, IWidget>, "a WidgetReference only refers to widgets");

    public:
        WidgetReference() = default;
        WidgetReference(std::nullptr_t) {}

        /// @brief Refers to widget or to nothing when it is nullptr.
        WidgetReference(T* widget)
            : _widget { widget }
        {
            if(!widget)
                return;

            _lifetime = static_cast<const IWidget*>(widget)->_lifetimeToken;
        }

        /// @brief Refers to the widget another WidgetReference refers to
        template<typename U> requires std::convertible_to<U*, T*> WidgetReference(const WidgetReference<U>& other)
            : _widget { other._widget }, _lifetime { other._lifetime }
        { }

        /// @brief Returns the widget, or nullptr once it has been freed.
        T* Get() const 
        { 
            return _lifetime.expired() ? nullptr : _widget; 
        }

        T* operator->() const 
        { 
            return Get(); 
        }

        T& operator*() const 
        { 
            return *Get(); 
        }

        /// @brief Whether the widget still exists.
        explicit operator bool() const 
        { 
            return !_lifetime.expired(); 
        }

        /// @brief Refers to nothing.
        void Reset()
        {
            _widget = nullptr;
            _lifetime.reset();
        }

    private:
        template<typename> friend class WidgetReference;

        T* _widget { nullptr };
        std::weak_ptr<const bool> _lifetime {};
    };
}