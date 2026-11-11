#pragma once
#include "IncludeHelper.hpp"
#include "ITab.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "Scene.hpp"
#include "Signal.hpp"
#include <memory>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RetroFuturaGUI
{
    /// @brief Carries a defined set of views that can be selected by clicking their corresponding names in the tab bar. Each
    ///        view is a page
    class Tab final : public ITab
    {
    public:
        /// @brief Constructs an empty Tab
        Tab(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Tab() = delete;
        Tab(const Tab&) = delete;
        Tab(Tab&&) = delete;
        auto operator =(const Tab&) = delete;
        auto operator =(Tab&&) = delete;
        ~Tab() override = default;

        void Draw() override;

    //Pages
        /// @brief Builds an page of type T at the end of the strip and owns it. The Tab is its parent and lays it out over the area
        ///        below the strip. The first page is selected
        /// @param title What the page's field shows
        /// @param args Constructor arguments after the parent window, such as a Lasagna's AxisDefinition
        /// @return The new page, or nullptr, building nothing, when the name is taken. It lives until it is removed or the Tab
        ///         is destroyed
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread
        template<RootContainer T, typename... Args>
        T* AddPage(std::string_view name, std::string_view title, Args&&... args)
        {
            if(_kNoField != indexOf(name))
            {
                std::println("ERROR::TAB::ITEM_NAME_TAKEN::{}::{}", _name, name);
                return nullptr;
            }

            std::unique_ptr<T> page { std::make_unique<T>(name, &_projection, this, _widgetTypeID, _parentWindow, std::forward<Args>(args)...) };
            T* addedPage { page.get() };
            addPage(std::move(page), title);
            return addedPage;
        }

        /// @brief Removes the page with the given name and its field. It is destroyed at the start of the next Interact, so its own
        ///        widgets' slots may remove it. When it was selected, the page now in its place is, or the one before it
        /// @return false when no page has that name
        bool RemovePage(std::string_view name);

        /// @brief Returns the page with the given name, or nullptr when there is none or it isn't a T
        template<RootContainer T> T* GetPage(std::string_view name) const
        {
            const uSize index { indexOf(name) };

            if(_kNoField == index)
                return nullptr;

            return dynamic_cast<T*>(pageOf(index));
        }

        /// @brief Returns how many pages the Tab carries
        uSize GetPageCount() const;

        /// @brief Selects the page with the given name. It draws and takes the input
        /// @return false when no page has that name
        bool SelectPage(std::string_view name, const bool emitSignal);

        /// @brief Returns the selected page, or nullptr while the Tab carries none
        IWidget* GetSelectedPage() const;

        /// @brief Sets what the page's field shows
        /// @return false when no page has that name
        bool SetPageTitle(std::string_view name, std::string_view title);

        /// @brief Returns what the page's field shows, empty when no page has that name
        const std::string& GetPageTitle(std::string_view name) const;

    //Signals
        /// @brief Connects a slot called when another page is selected, with the page, or with nullptr once the last page is gone
        void Connect_OnSelectionChanged(const typename Signal<IWidget*>::Slot& slot, const bool async);
        void Disconnect_OnSelectionChanged(const typename Signal<IWidget*>::Slot& slot);

        /// @brief Connects a slot called with each page added.
        void Connect_OnPageAdded(const typename Signal<IWidget*>::Slot& slot, const bool async);
        void Disconnect_OnPageAdded(const typename Signal<IWidget*>::Slot& slot);

        /// @brief Connects a slot called with each page removed, by its cross or by RemovePage. The page lives until the start of
        ///        the next Interact
        void Connect_OnPageRemoved(const typename Signal<IWidget*>::Slot& slot, const bool async);
        void Disconnect_OnPageRemoved(const typename Signal<IWidget*>::Slot& slot);

    //Close buttons
        /// @brief Shows or hides the cross on every field (shown by default). Without it, pages are only removed by RemovePage
        void ShowCloseButtons(const bool show);

        /// @brief Returns whether the fields show their cross
        bool AreCloseButtonsShown() const;

    private:
    //Pages
        /// @brief Puts a built page on the strip, lays it out, selects it when it is the first, and tells the slots
        void addPage(std::unique_ptr<IWidget> page, std::string_view title);

        /// @brief Takes the page's field off the strip and keeps the page until the next Interact. When it was selected, the
        ///        field now in its place is, or the one before it
        void removePage(const uSize index);

        /// @brief Lays every page out over the content area. Layout moves at once
        void layoutPages();

        /// @brief Lays one page out over the content area
        void layoutPage(IWidget& page) const;

        /// @brief The page of the field, or nullptr
        IWidget* pageOf(const uSize index) const;

        uSize indexOf(std::string_view name) const;
        using ITab::indexOf;

    //Interaction
        /// @brief Frees the removed pages, follows the pointer on the strip, then passes the input on to the selected page
        void interact() override;

        /// @brief Updates every page, the hidden ones too
        void update() override;

        void drawSelectedPage();

    //ITab
        void fieldSelected(IHierarchyNode* item, const bool emitSignal) override;
        void selectionCleared() override;

        /// @brief The cross removes the page
        void closeField(const uSize index) override;
        bool showsCloseButtons() const override;
        void contentAreaChanged() override;

        /// @brief The background and the border take the content radii at their bottom corners, as their built-in look.
        void contentCornerRadiiChanged() override;

    //Elements
        std::vector<std::unique_ptr<IWidget>>
            _pages {}, //the pages on the strip, in the order they were added; _fields keeps their order on it.
            _removedPages {}; //destroyed at the start of the next Interact
        bool _areCloseButtonsShown { true };

    //Signals
        Signal<IWidget*>
            _onSelectionChanged,
            _onSelectionChangedAsync,
            _onPageAdded,
            _onPageAddedAsync,
            _onPageRemoved,
            _onPageRemovedAsync;
    };
}