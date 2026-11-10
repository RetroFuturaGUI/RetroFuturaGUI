#pragma once
#include "Background.hpp"
#include "IncludeHelper.hpp"
#include "IWidget.hpp"
#include "IContainer.hpp"
#include "Border.hpp"
#include "Box.hpp"
#include "CloseButton.hpp"
#include "Frame.hpp"
#include "Plane.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "Text.hpp"
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
    class Tab final : public IWidget, public IContainer
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
            addField(std::move(page), title);
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

            return dynamic_cast<T*>(_fields[index]._Page.get());
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

    //Strip
        /// @brief Sets the height of the strip the fields sit in
        void SetStripHeight(const f32 height);

        /// @brief Sets how narrow and how wide a field may get. Fields share the strip evenly within that range, and the ones
        ///        that no longer fit at the minimum width are left out
        void SetFieldWidthRange(const f32 minimumWidth, const f32 maximumWidth);

        /// @brief Sets the gap between fields, and above them
        void SetFieldSpacing(const f32 spacing);

    //Titles
        /// @brief Sets the font of the titles. Without one they don't show
        void SetTitleFontFamily(std::string_view fontFamily, const f32 fontSize, const FontSizeUnit sizeUnit, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

        /// @brief Returns the title font size in the given unit
        f32 GetTitleFontSize(const FontSizeUnit sizeUnit) const;

        /// @brief Sets a title's colors for Enabled, Disabled, Hover or Clicked. The selected page's title shows the same colors as
        ///        the others, so Selected is ignored
        void SetTitleColors(std::span<const glm::vec4> colors, const ColorState state);

        /// @brief Sets the space between a field's left edge and its title
        void SetTitlePadding(const f32 padding);

    //Close buttons
        /// @brief Shows or hides the cross on every field (shown by default). Without it, pages are only removed by RemovePage
        void ShowCloseButtons(const bool show);

        /// @brief Returns whether the fields show their cross
        bool AreCloseButtonsShown() const;

        /// @brief Sets the width and height of the close buttons
        void SetCloseButtonSize(const f32 size);

        /// @brief Returns the close button every field draws: its background, transparent until hovered by default, and its
        ///        cross. Each field's button shows that field's state. Its corners round with a quarter of SetCloseButtonSize
        CloseButton& GetCloseButton();

        /// @brief Returns the close button every field draws (see the other overload)
        const CloseButton& GetCloseButton() const;

        /// @brief Returns the look of the fields: Enabled, Hover, Clicked and Disabled, and Selected for the selected page's field,
        ///        which usually matches the content so the two join. Rounded at the top, without a border by default
        Box& GetFieldStyle();

        /// @brief Returns the look of the fields; see the other overload
        const Box& GetFieldStyle() const;

    //Content
        /// @brief Returns the Tab's background
        Background& GetBackground();

        /// @brief Returns the Tab's background
        const Background& GetBackground() const;

        /// @brief Returns the Tab's border
        Border& GetBorder();

        /// @brief Returns the Tab's border
        const Border& GetBorder() const;

    protected:
        /// @brief Sets the center of the whole Tab, strip and content area together
        void applyPosition(const glm::vec3& position) override;

        /// @brief Sets the size of the whole Tab, strip and content area together
        void applySize(const glm::vec3& size) override;

    private:
        /// @brief One page and the title its field shows
        struct Field
        {
            std::unique_ptr<IWidget> _Page { nullptr }; //a RootContainer.
            std::string _Title {};
            std::unique_ptr<Text> _TitleText { nullptr };
            TransitionValue<glm::vec2> _Shown {}; //the field's left edge and width as drawn, on their way to its slot
            bool _HasShown { false }; //a field not shown yet stands in its slot at once
        };

        /// @brief The colors for every state a field and its parts can be in
        struct FieldColors
        {
            std::vector<glm::vec4>
                _Enabled { glm::vec4(0.0f) },
                _Hover { glm::vec4(0.0f) },
                _Clicked { glm::vec4(0.0f) },
                _Disabled { glm::vec4(0.0f) },
                _Selected { glm::vec4(0.0f) };
        };

    //Pages
        /// @brief Puts a built page on the strip, lays it out, selects it when it is the first, and tells the slots
        void addField(std::unique_ptr<IWidget> page, std::string_view title);

        /// @brief Selects the field's page, telling the slots when it changed and emitSignal is set
        void selectField(const uSize index, const bool emitSignal);

        /// @brief Lays every page out over the content area. Layout moves at once
        void layoutPages();

        /// @brief Lays one page out over the content area
        void layoutPage(IWidget& page) const;

    //Interaction
        /// @brief Frees the removed pages, follows the pointer on the strip, then passes the input on to the selected page
        void interact() override;

        /// @brief Updates every page, the hidden ones too
        void update() override;

        /// @brief Selects or removes on a press, and drags the pressed field along the strip until the button is let go
        void followPointer();

        /// @brief Selects the field and remembers where it was grabbed, in case the press turns into a drag
        void pressField(const uSize index, const glm::vec2& point);

        /// @brief Moves the pressed field with the cursor along the strip, reordering the fields
        void dragField(const glm::vec2& point);

        /// @brief The field under the point, or _kNoField
        uSize fieldAt(const glm::vec2& point) const;

        /// @brief The state a field draws in, given what the pointer is doing
        ColorState fieldState(const uSize index) const;

    //Layout, in window coordinates with y running upward. A rectangle is (left, bottom, width, height)
        glm::vec4 getStripRect() const;
        glm::vec4 getContentRect() const;
        f32 getFieldWidth() const;

        /// @brief Where the field at the given place sits, ignoring a drag
        glm::vec4 slotRect(const uSize slot) const;

        /// @brief Where the field is drawn: on its way to its slot, or under the cursor while it is dragged
        glm::vec4 fieldRect(const uSize index) const;

        /// @brief Returns the left edge and width a field is moving toward: its slot's, or where the cursor drags it
        glm::vec2 getFieldTarget(const uSize index) const;

        /// @brief Moves every field toward its slot over the Position transition. The dragged field follows the cursor at once
        void syncFieldRects();

        /// @brief The place along the strip whose slot the given x lies over
        uSize slotAt(const f32 x) const;

        /// @brief Whether the field fits in the strip. The ones that don't are left out
        bool fitsInStrip(const glm::vec4& field) const;

        glm::vec4 closeButtonRect(const glm::vec4& field) const;
        glm::vec4 titleRect(const glm::vec4& field) const;

    //Drawing
        void drawFields();
        void drawField(const uSize index, const f32 z);
        void drawFieldShape(const glm::vec4& rect, const f32 z, Box& style, const ColorState state);

        /// @brief The state a field's look shows: Disabled, Selected for the selected page's field, or the field's own
        static ColorState fieldStyleState(const ColorState state, const bool selected);

        /// @brief Gives the field look its default colors, corners and border width
        void setFieldStyleDefaults();
        void drawCloseButton(const glm::vec4& field, const f32 z, const ColorState state);
        void drawTitle(Text* title, const glm::vec4& field, const f32 z, const std::vector<glm::vec4>& colors);
        void drawSelectedPage();

        void applyTitleFont(Text* title) const;

    //Bookkeeping
        uSize indexOf(const IWidget* page) const;
        uSize indexOf(std::string_view name) const;

        /// @brief Takes the field off the strip and keeps its page until the next Interact. When it was selected, the field now in
        ///        its place is, or the one before it
        void removeField(const uSize index);

        /// @brief The colors to draw with. The selected ones for the selected field unless it is disabled, else the state's
        static const std::vector<glm::vec4>& colorFor(const FieldColors& colors, const ColorState state, const bool selected);

        /// @brief The colors stored for the state
        static std::vector<glm::vec4>& colorOf(FieldColors& colors, const ColorState state);

        /// @brief Whether any of the colors can be seen, so a fully transparent part isn't drawn at all
        static bool isVisible(std::span<const glm::vec4> colors);

        static bool contains(const glm::vec4& rect, const glm::vec2& point);
        /// @brief The UTF-8 path of the close icon, in the Resources/img folder next to the executable
        static std::string getCloseIconPath();

    //Elements
        std::vector<Field> _fields {};
        std::vector<std::unique_ptr<IWidget>> _removedPages {}; //destroyed at the start of the next Interact
        Background _background { &_projection };
        Border _border { &_projection };
        Box _fieldStyle { &_projection }; //drawn once per field, in its colors, so it takes no transitions
        CloseButton _closeButton { &_projection, ResourceManager::CreateSvgTexture(getCloseIconPath(), &_projection, glm::i32vec2(32)) }; //its own, so its colors and size are this tab's alone.

    //Design
        f32
            _stripHeight { 36.0f },
            _minimumFieldWidth { 80.0f },
            _maximumFieldWidth { 240.0f },
            _fieldSpacing { 4.0f },
            _titlePadding { 12.0f },
            _closeButtonSize { 24.0f };
        FieldColors
            _titleColors
            {
                ._Enabled = { glm::vec4(0.7f, 0.7f, 0.75f, 1.0f) },
                ._Hover = { glm::vec4(0.9f, 0.9f, 0.95f, 1.0f) },
                ._Clicked = { glm::vec4(1.0f) },
                ._Disabled = { glm::vec4(0.4f, 0.4f, 0.45f, 1.0f) },
                ._Selected = { glm::vec4(1.0f) }
            };
        std::string _titleFontFamily {};
        f32 _titleFontSize { FontManager::ToPixels(18.0f, FontSizeUnit::Points) }; //in pixels, as every stored font size
        PlatformBridge::Fonts::Slant _titleFontSlant { PlatformBridge::Fonts::Slant::Roman };
        PlatformBridge::Fonts::Weight _titleFontWeight { PlatformBridge::Fonts::Weight::Normal };
        bool _areCloseButtonsShown { true };

    //Logic
        static constexpr uSize _kNoField { ~uSize(0) };
        static constexpr f32
            _zStep { 0.01f },
            _closeIconScale { 0.4f },
            _dragThreshold { 5.0f }; //how far a press may move before it drags the field

        IWidget
            *_selectedPage { nullptr },
            *_pressedPage { nullptr }; //the page whose field the press went to, followed through reordering
        uSize
            _hoveredField { _kNoField },
            _hoveredCloseButton { _kNoField },
            _pressedCloseButton { _kNoField };
        glm::vec2 _pressPoint { 0.0f }; //where the press began
        f32
            _grabOffset { 0.0f }, //where along the field the press holds it, from its left edge
            _dragLeft { 0.0f }; //the dragged field's left edge, which follows the cursor
        bool
            _isDraggingField { false },
            _wasPressed { false };

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