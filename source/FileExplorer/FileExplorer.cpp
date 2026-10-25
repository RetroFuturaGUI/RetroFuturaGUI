#include "FileExplorer.hpp"
#include "FileExplorerPrompts.hpp"
#include "FilePreview.hpp"
#include "ThumbnailCache.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "ComboBox.hpp"
#include "FontManager.hpp"
#include "Label.hpp"
#include "MenuBar.hpp"
#include "PlatformBridge.hpp"
#include "SceneLoader.hpp"
#include "ScrollView.hpp"
#include "Slider.hpp"
#include "TextBox.hpp"
#include "Window.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <format>
#include <numeric>
#include <stdexcept>
#include <tuple>

RetroFuturaGUI::FileExplorer::FileExplorer(std::string_view title, const i32 width, const i32 height)
    : Popup(std::format("FileExplorer{}", _createdCount), width, height),
      _explorerNumber(_createdCount++),
      _width(width),
      _height(height)
{
    Window* window { GetWindow() };

    if(!window)
        return;

    styleWindow(*window, title);
}

void RetroFuturaGUI::FileExplorer::styleWindow(Window& window, std::string_view title)
{
    window.SetWindowTitle(title);
    window.SetCornerRadii(glm::vec4(12.0f));
    window.SetBorderWidth(1.0f);
    window.SetBorderColor(glm::vec4(0.35f, 0.35f, 0.45f, 1.0f), ColorState::Enabled);
}

RetroFuturaGUI::FileExplorer::~FileExplorer() = default;

bool RetroFuturaGUI::FileExplorer::Open(std::string_view startPath, const FileExplorerFlags flags, std::string_view extensions)
{
    return openWithFilters(startPath, flags, filtersOf(extensions));
}

bool RetroFuturaGUI::FileExplorer::Open(std::string_view startPath, const FileExplorerFlags flags, const std::vector<FileFilter>& filters)
{
    return openWithFilters(startPath, flags, filtersOf(filters));
}

bool RetroFuturaGUI::FileExplorer::openWithFilters(std::string_view startPath, const FileExplorerFlags flags, std::vector<Filter>&& filters)
{
    if(_viewHost)
        return false;

    const FileExplorerFlags modeFlags { flags & (FileExplorerFlags::MultiFile | FileExplorerFlags::Folder | FileExplorerFlags::WriteFile) };

    if(1 < std::popcount(static_cast<u32>(modeFlags)))
        return false;

    Window* window { GetWindow() };

    if(!window)
        return false;

    GLFWwindow* glfwWindow { window->GetGlfwWindow() };

    if(!glfwWindow)
        return false;

    _flags = flags;
    _mode = modeOf(flags);
    _showHidden = HasFlag(flags, FileExplorerFlags::ShowHiddenFiles);
    _showExtensions = HasFlag(flags, FileExplorerFlags::ShowFileExtensions);
    _showThumbnails = HasFlag(flags, FileExplorerFlags::Thumbnails);

    //The widgets' GL objects belong to the explorer's context, whichever was made current since the constructor
    glfwMakeContextCurrent(glfwWindow);

    //Numbered, because the loader's keys span every window and several explorers can be open
    std::unique_ptr<ViewHost> host { std::make_unique<ViewHost>(std::format("FileExplorerScene{}", _explorerNumber), window, _viewAxis) };
    ensureFontBlock(Latin1SupplementFirst, Latin1SupplementLast);
    ensureFontBlock(GeometricShapesFirst, GeometricShapesLast);

    if(!buildView(*host, window->GetWindowTitle()))
        return false;

    Scene* scene { host->GetScene() };

    if(!scene)
        return false;

    scene->Connect_OnUpdate([this]() { update(); }, false); //before the widgets draw, so a key the list also takes is seen in the same frame
    _filters = std::move(filters);
    applyMode();
    addFilters();
    updateHeaderTexts();
    updateMenus();
    PlatformBridge::FileSystem::Refresh(); //refresh if drives have ben attached or detached
    addPlaces();
    _viewHost = host.get();
    AddScene(std::move(host));
    updateNavigationButtons(); //no history yet
    navigateToStart(startPath);

    if(_widgets._Files)
        _widgets._Files->TakeKeyboardFocus(); //the keys work from the start, before anything is clicked

    return true;
}

void RetroFuturaGUI::FileExplorer::SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight)
{
    _font = ViewFont { ._Family = std::string(fontFamily), ._Size = fontSize, ._Slant = slant, ._Weight = weight };
}

RetroFuturaGUI::FileExplorer::ViewHost::ViewHost(std::string_view sceneName, Window* window, const AxisDefinition& lasagnaAxis)
{
    if(!window)
        return;

    _projection = window->GetProjection();

    if(!_projection)
        return;

    _glfwWindow = window->GetGlfwWindow();
    _scene = SceneLoader::CreateScene(sceneName, window);

    if(!_scene)
        return;

    _scene->SetLasagnaAxis(lasagnaAxis, _projection);
}

RetroFuturaGUI::FileExplorer::ViewHost::~ViewHost()
{
    if(!_scene)
        return;

    SceneLoader::UnregisterScene(SceneLoader::GetSceneID(_scene->GetName()));
}

RetroFuturaGUI::Scene* RetroFuturaGUI::FileExplorer::ViewHost::GetScene() const
{
    return _scene.get();
}

RetroFuturaGUI::Lasagna* RetroFuturaGUI::FileExplorer::ViewHost::GetLasagna() const
{
    if(!_scene)
        return nullptr;

    return _scene->GetRootContainer<Lasagna>();
}

bool RetroFuturaGUI::FileExplorer::buildView(ViewHost& host, std::string_view title)
{
    Lasagna* lasagna { host.GetLasagna() };

    if(!lasagna)
        return false;

    Window* window { GetWindow() };

    if(!window)
        return false;

    Projection* projection { window->GetProjection() };

    if(!projection)
        return false;

    GLFWwindow* glfwWindow { window->GetGlfwWindow() };

    if(!glfwWindow)
        return false;

    const glm::vec2 windowSize { static_cast<f32>(_width), static_cast<f32>(_height) };
    ViewWidgets built {};

    built._Bar = attachBar(host, *lasagna, *window, title, _viewColumnCount, _viewAxis._RowDefinition[0] * windowSize.y, windowSize, _font);

    if(!built._Bar)
        return false;

    built._Close = built._Bar->GetWidget<Button>(_barCloseIndex);

    if(!built._Close)
        return false;

    built._Close->Connect_OnClick([this]() { on_cancelButton_click(); }, false);

    const ViewCell //Navigation
        backCell { ._Row = 1, ._Column = 0, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        forwardCell { ._Row = 1, ._Column = 3, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        upCell { ._Row = 1, ._Column = 6, ._ColumnSpan = 3, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        addressCell { ._Row = 1, ._Column = 9, ._ColumnSpan = 31, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    built._Back = host.Attach<Button>("Back", *lasagna, backCell);

    if(!built._Back)
        return false;

    styleButton(*built._Back, "<", cellSize(_viewAxis, windowSize, backCell), _font);
    built._Back->Connect_OnClick([this]() { on_backButton_click(); }, false);
    built._Forward = host.Attach<Button>("Forward", *lasagna, forwardCell);

    if(!built._Forward)
        return false;

    styleButton(*built._Forward, ">", cellSize(_viewAxis, windowSize, forwardCell), _font);
    built._Forward->Connect_OnClick([this]() { on_forwardButton_click(); }, false);
    built._Up = host.Attach<Button>("Up", *lasagna, upCell);

    if(!built._Up)
        return false;

    styleButton(*built._Up, "Up", cellSize(_viewAxis, windowSize, upCell), _font);
    built._Up->Connect_OnClick([this]() { on_upButton_click(); }, false);
    built._Address = host.Attach<TextBox>("Address", *lasagna, addressCell);

    if(!built._Address)
        return false;

    styleTextBox(*built._Address, "Folder", cellSize(_viewAxis, windowSize, addressCell), _font);
    built._Address->Connect_OnEnterPressed([this]() { on_addressTextBox_enterPressed(); }, false);

//Menu strip: the File and Edit menus with the operations that are on, the Rename button, then the three toggles
    const ViewFont smallFont { toolbarFont(_font) };
    _fileMenuActions = menuActionsOf(_fileMenuChoices);
    _editMenuActions = menuActionsOf(_editMenuChoices);

    const ViewCell
        fileMenuCell { ._Row = 2, ._Column = 0, ._ColumnSpan = _menuColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        editMenuCell { ._Row = 2, ._Column = _menuColumnCount, ._ColumnSpan = _menuColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        renameCell { ._Row = 2, ._Column = 2 * _menuColumnCount, ._ColumnSpan = 4, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
    const f32 menuListWidth { windowSize.x * static_cast<f32>(_menuListColumnCount) / static_cast<f32>(_viewColumnCount) };

    built._FileMenu = host.Attach<ComboBox>("FileMenu", *lasagna, fileMenuCell);

    if(!built._FileMenu)
        return false;

    styleMenu(*built._FileMenu, "File", _fileMenuActions, cellSize(_viewAxis, windowSize, fileMenuCell), menuListWidth, smallFont);
    built._FileMenu->Connect_OnItemClicked([this]() { on_fileMenu_itemClicked(); }, false);

    //Edit has nothing in it when the operations are off and only one file can be picked
    if(!_editMenuActions.empty())
    {
        built._EditMenu = host.Attach<ComboBox>("EditMenu", *lasagna, editMenuCell);

        if(!built._EditMenu)
            return false;

        styleMenu(*built._EditMenu, "Edit", _editMenuActions, cellSize(_viewAxis, windowSize, editMenuCell), menuListWidth, smallFont);
        built._EditMenu->Connect_OnItemClicked([this]() { on_editMenu_itemClicked(); }, false);
    }

    if(HasFlag(_flags, FileExplorerFlags::Rename))
    {
        built._Rename = host.Attach<Button>("Rename", *lasagna, renameCell);

        if(!built._Rename)
            return false;

        styleButton(*built._Rename, "Rename", cellSize(_viewAxis, windowSize, renameCell), smallFont);
        built._Rename->Connect_OnClick([this]() { on_renameButton_click(); }, false); //enabled by updateMenus while one row is selected
    }

    const std::array<std::tuple<std::string_view, uSize, bool, Button**>, 3> toggles
    {{
        { "Thumbnails", 25, _showThumbnails, &built._Thumbnails },
        { "Extensions", 30, _showExtensions, &built._Extensions },
        { "Hidden files", 35, _showHidden, &built._Hidden }
    }};

    for(const auto& [text, column, isOn, slot] : toggles)
    {
        const ViewCell cell { ._Row = 2, ._Column = column, ._ColumnSpan = 5, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };
        Button* toggle { host.Attach<Button>(text, *lasagna, cell) };

        if(!toggle)
            return false;

        styleButton(*toggle, text, cellSize(_viewAxis, windowSize, cell), smallFont);
        styleToggle(*toggle, isOn);
        *slot = toggle;
    }

    built._Thumbnails->Connect_OnClick([this]() { on_thumbnailsButton_click(); }, false);
    built._Extensions->Connect_OnClick([this]() { on_extensionsButton_click(); }, false);
    built._Hidden->Connect_OnClick([this]() { on_hiddenButton_click(); }, false);

    //Places, files, the scrollbar, since the table has no wheel or drag scrolling of its own, and the preview pane with ImagePreview
    //or MediaPreview; without it the files take its columns.
    const uSize filesColumnCount { hasPreview() ? _filesColumnCount : _filesColumnCount + _previewColumnCount };
    const ViewCell
        placesCell { ._Row = 3, ._Column = 0, ._ColumnSpan = _placesColumnCount, ._SizingMode = SizingMode::FILL, ._Layer = 0 },
        filesCell { ._Row = 3, ._Column = _placesColumnCount, ._ColumnSpan = filesColumnCount, ._SizingMode = SizingMode::FILL, ._Layer = 0 },
        scrollCell { ._Row = 3, ._Column = _placesColumnCount + filesColumnCount, ._ColumnSpan = 1, ._SizingMode = SizingMode::FILL, ._Layer = 0 };

    built._Places = host.Attach<ScrollView>("Places", *lasagna, placesCell);

    if(!built._Places)
        return false;

    built._Places->EnableScrolling(ScrollView::ScrollEnable::Y); //the places stack vertically, the default
    built._Files = host.Attach<Table>("Files", *lasagna, filesCell);

    if(!built._Files)
        return false;

    built._Files->SetBorderWidth(1.0f);
    built._Files->SetBorderColor(glm::vec4(0.35f, 0.35f, 0.45f, 1.0f), ColorState::Enabled);

    //Before the first cell is made: new cells take the font and the first track's colors
    built._Files->SetTrackAlternatingColorCount(2);
    std::vector<glm::vec4>
        evenRowColors {{ 0.1f, 0.1f, 0.13f, 1.0f }},
        oddRowColors {{ 0.13f, 0.13f, 0.17f, 1.0f }},
        innerBorderColors {{ 0.18f, 0.18f, 0.22f, 1.0f }},
        headerColors {{ 0.16f, 0.16f, 0.22f, 1.0f }};
    built._Files->SetTrackBackgroundColors(evenRowColors, ColorState::Enabled, 0);
    built._Files->SetTrackBackgroundColors(oddRowColors, ColorState::Enabled, 1);
    built._Files->SetTrackBorderColors(innerBorderColors, ColorState::Enabled, 0);
    built._Files->SetTrackBorderColors(innerBorderColors, ColorState::Enabled, 1);
    built._Files->SetInnerBorderWidth(1.0f);
    built._Files->SetFontFamily(_font._Family, _font._Size, _font._Slant, _font._Weight);
    built._Files->SetTextAlignment(TextAlignment::Left);
    built._Files->SetTextPadding(6.0f);

    //A name being renamed sits on a selected row, whose blue hides the default blue of selected text, so it is lightened instead
    std::vector<glm::vec4> renameSelectionColors {{ 0.95f, 0.95f, 1.0f, 0.35f }};
    built._Files->SetSelectedAreaColors(renameSelectionColors);

    //The header cells come with the columns, so they are named once these exist
    built._Files->SetTrackDefinitions(std::vector<Table::TrackDefinition> {}, columnDefinition(_font, _showThumbnails));
    built._Files->SetHeaderFontFamily(_font._Family, _font._Size, _font._Slant, _font._Weight);
    built._Files->SetHeaderTextAlignment(TextAlignment::Left);
    built._Files->SetHeaderTextPadding(6.0f);
    built._Files->SetHeaderBackgroundColors(headerColors, ColorState::Enabled);
    built._Files->SetHeaderOuterBorderColors(innerBorderColors, ColorState::Enabled);
    built._Files->SetHorizontalHeaderSize(rowHeight(_font) + 4.0f);
    built._Files->ShowHorizontalHeader(true); //named by updateHeaderTexts, which marks the sort column; fillTable makes the columns read-only

    built._Files->Connect_OnSelectionChanged([this]() { on_filesTable_selectionChanged(); }, false);
    built._Files->Connect_OnRowDoubleClicked([this](const uSize row) { on_filesTable_rowDoubleClicked(row); }, false);
    built._Files->Connect_OnHorizontalHeaderClicked([this](const uSize column) { on_filesTable_headerClicked(column); }, false);
    built._Files->Connect_OnScrolled([this]() { syncScrollSlider(); }, false);

    if(HasFlag(_flags, FileExplorerFlags::DragAndDrop))
    {
        built._Files->Connect_OnSelectionDrag([this](const Table::SelectionDragPhase phase, const glm::vec2 point) { on_filesTable_selectionDrag(phase, point); }, false);
        window->Connect_OnFilesDropped([this](const std::vector<std::string> paths, const glm::vec2 point) { on_window_filesDropped(paths, point); }, false);
    }

    if(HasFlag(_flags, FileExplorerFlags::Rename))
    {
        built._Files->Connect_OnRowSlowDoubleClicked([this](const uSize row) { on_filesTable_rowSlowDoubleClicked(row); }, false);
        built._Files->Connect_OnCellEditEnded([this](const TrackIndex& index, const Table::EditEnd end, const std::string& text) { on_filesTable_cellEditEnded(index, end, text); }, false);
    }

    built._Scroll = host.Attach<Slider>("Scroll", *lasagna, scrollCell);

    if(!built._Scroll)
        return false;

    std::vector<glm::vec4>
        indicatorColors {{ 0.45f, 0.45f, 0.6f, 1.0f }},
        indicatorBorderColors {{ 0.6f, 0.6f, 0.8f, 1.0f }};
    built._Scroll->SetOrientation(IRangedValue::Orientation::Vertical);
    built._Scroll->SetTrackDirection(IRangedValue::TrackDirection::Inverted); //unscrolled at the top
    built._Scroll->SetBackgroundColor(glm::vec4(0.12f, 0.12f, 0.15f, 1.0f), ColorState::Enabled);
    built._Scroll->SetBackgroundFillType(FillType::SOLID);
    built._Scroll->SetBorderColor(glm::vec4(0.35f, 0.35f, 0.45f, 1.0f), ColorState::Enabled);
    built._Scroll->SetBorderWidth(1.0f);
    built._Scroll->SetIndicatorBackgroundColors(indicatorColors, ColorState::Enabled);
    built._Scroll->SetIndicatorBorderColors(indicatorBorderColors, ColorState::Enabled);
    built._Scroll->SetIndicatorSize(glm::vec2(15.0f, 100.0f), IRangedValue::ElementSizing::Percent);
    built._Scroll->SetMinValue<f32>(0.0f);
    built._Scroll->SetMaxValue<f32>(1.0f);
    built._Scroll->SetStepSize<f32>(rowHeight(_font));
    built._Scroll->SetValue<f32>(0.0f, false); //sets the value type too, so it comes after the range
    built._Scroll->Connect_OnValueChanged([this]() { on_scrollSlider_valueChanged(); }, false);

    if(hasPreview())
    {
        const ViewCell previewCell { ._Row = 3, ._Column = _placesColumnCount + filesColumnCount + 1, ._ColumnSpan = _previewColumnCount, ._SizingMode = SizingMode::FILL, ._Layer = 0 };
        _preview = std::make_unique<FilePreview>(host, *lasagna, previewCell, windowSize, _font, _flags);

        if(!_preview->IsBuilt())
            return false;
    }

//Options: the name box and the filter box
    const ViewCell
        nameCell { ._Row = 4, ._Column = 0, ._ColumnSpan = 24, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        filterCell { ._Row = 4, ._Column = 24, ._ColumnSpan = 16, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    if(Mode::WriteFile == _mode)
    {
        built._Name = host.Attach<TextBox>("Name", *lasagna, nameCell);

        if(!built._Name)
            return false;

        styleTextBox(*built._Name, "File name", cellSize(_viewAxis, windowSize, nameCell), _font);
        built._Name->Connect_OnTextChange([this]() { on_nameTextBox_textChanged(); }, false);
        built._Name->Connect_OnEnterPressed([this]() { on_okButton_click(); }, false);
    }

    built._Filter = host.Attach<ComboBox>("Filter", *lasagna, filterCell);

    if(!built._Filter)
        return false;

    built._Filter->SetFontFamily(_font._Family, _font._Size, _font._Slant, _font._Weight);
    built._Filter->SetSize(cellSize(_viewAxis, windowSize, filterCell));
    built._Filter->SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
    built._Filter->SetDropDownPosition(IDropDown::DropDownPosition::Top); //down would run out of the window
    built._Filter->Connect_OnIndexChanged([this]() { on_filterComboBox_indexChanged(); }, false);

//Status, the Stop button of a file operation, and the buttons it closes with
    const bool hasTransfers { HasFlag(_flags, FileExplorerFlags::Delete) || HasFlag(_flags, FileExplorerFlags::DeletePermanently) || HasFlag(_flags, FileExplorerFlags::CopyAndMove) || HasFlag(_flags, FileExplorerFlags::DragAndDrop) };
    const ViewCell
        statusCell { ._Row = 5, ._Column = 0, ._ColumnSpan = hasTransfers ? _statusColumnCount : _statusColumnCount + _stopColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        stopCell { ._Row = 5, ._Column = _statusColumnCount, ._ColumnSpan = _stopColumnCount, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        okCell { ._Row = 5, ._Column = 24, ._ColumnSpan = 8, ._SizingMode = SizingMode::FIXED, ._Layer = 0 },
        cancelCell { ._Row = 5, ._Column = 32, ._ColumnSpan = 8, ._SizingMode = SizingMode::FIXED, ._Layer = 0 };

    if(hasTransfers)
    {
        built._Stop = host.Attach<Button>("Stop", *lasagna, stopCell);

        if(!built._Stop)
            return false;

        styleButton(*built._Stop, "Stop", cellSize(_viewAxis, windowSize, stopCell), _font);
        built._Stop->Connect_OnClick([this]() { on_stopButton_click(); }, false);

        LasagnaCell* stopLasagnaCell { lasagna->GetCell(TrackIndex { ._Row = stopCell._Row, ._Column = stopCell._Column, ._Layer = 0 }) };

        if(!stopLasagnaCell)
            return false;

        stopLasagnaCell->_Show = false; //until an operation runs
    }

    built._Status = host.Attach<Label>("Status", *lasagna, statusCell);

    if(!built._Status)
        return false;

    built._Status->SetFontFamily(_font._Family, _font._Size, _font._Slant, _font._Weight);
    built._Status->SetTextAlignment(TextAlignment::Left);
    built._Status->SetTextColor(glm::vec4(0.9f, 0.8f, 0.55f, 1.0f), ColorState::Enabled);
    built._Status->SetTextPadding(8.0f);
    built._Status->SetSize(cellSize(_viewAxis, windowSize, statusCell));
    built._Ok = host.Attach<Button>("Ok", *lasagna, okCell);

    if(!built._Ok)
        return false;

    styleButton(*built._Ok, "OK", cellSize(_viewAxis, windowSize, okCell), _font);
    built._Ok->Connect_OnClick([this]() { on_okButton_click(); }, false);
    built._Cancel = host.Attach<Button>("Cancel", *lasagna, cancelCell);

    if(!built._Cancel)
        return false;

    styleButton(*built._Cancel, "Cancel", cellSize(_viewAxis, windowSize, cancelCell), _font);
    built._Cancel->Connect_OnClick([this]() { on_cancelButton_click(); }, false);

    _widgets = built;
    return true;
}

RetroFuturaGUI::MenuBar* RetroFuturaGUI::FileExplorer::attachBar(ViewHost& host, Lasagna& lasagna, Window& window, std::string_view title, const uSize columnCount, const f32 barHeight, const glm::vec2& windowSize, const ViewFont& font)
{
    Projection* projection { window.GetProjection() };

    if(!projection)
        return nullptr;

    GLFWwindow* glfwWindow { window.GetGlfwWindow() };

    if(!glfwWindow)
        return nullptr;

    //The title drags the window, the spacer takes what is left, and the close button is a square of the bar's height
    MenuBar* bar { host.Attach<MenuBar>("Bar", lasagna, ViewCell { ._Row = 0, ._Column = 0, ._ColumnSpan = columnCount, ._SizingMode = SizingMode::FILL, ._Layer = 0 }) };

    if(!bar)
        return nullptr;

    bar->SetDockingEdge(MenuBar::DockingEdge::Top);
    bar->SetBackgroundColor(glm::vec4(0.08f, 0.08f, 0.12f, 0.9f), ColorState::Enabled);
    bar->SetBackgroundFillType(FillType::SOLID);
    bar->SetBorderColor(glm::vec4(0.35f, 0.35f, 0.45f, 1.0f), ColorState::Enabled);
    bar->SetBorderWidth(1.0f);
    bar->SetCornerRadii(glm::vec4(12.0f, 12.0f, 0.0f, 0.0f)); //the window's top corners, which a popup never squares
    bar->SetItemBackgroundColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), ColorState::Enabled);
    bar->SetItemBackgroundColor(glm::vec4(0.25f, 0.25f, 0.35f, 0.9f), ColorState::Hover);
    bar->SetItemBackgroundColor(glm::vec4(0.35f, 0.35f, 0.5f, 0.95f), ColorState::Clicked);
    bar->SetItemBorderColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), ColorState::Enabled);
    bar->SetItemBorderWidth(0.0f);

    std::vector<MenuBar::FlexDefinition> flexDefinition
    {
        { ._FlexSizing = MenuBar::FlexSizing::Auto,   ._Width = 0.0f, ._IsWindowDrag = true,  ._IsWindowUndock = false },
        { ._FlexSizing = MenuBar::FlexSizing::Star,   ._Width = 1.0f, ._IsWindowDrag = false, ._IsWindowUndock = false },
        { ._FlexSizing = MenuBar::FlexSizing::Square, ._Width = 0.0f, ._IsWindowDrag = false, ._IsWindowUndock = false }
    };
    bar->SetFlexDefinition(flexDefinition);

    std::unique_ptr<Label> barTitle { std::make_unique<Label>("BarTitle", projection, bar, WidgetTypeID::MenuBar, glfwWindow) };
    barTitle->SetFontFamily(font._Family, font._Size, font._Slant, font._Weight);
    barTitle->SetText(title, false);
    barTitle->SetTextAlignment(TextAlignment::Left);
    barTitle->SetTextColor(glm::vec4(1.0f), ColorState::Enabled);
    barTitle->SetTextPadding(8.0f);
    barTitle->SetSize(glm::vec3(0.5f * windowSize.x, barHeight, 0.01f)); //as tall as the bar, since text is clipped to its label
    bar->AddWidget(std::move(barTitle), 0);

    std::unique_ptr<Button> closeButton { std::make_unique<Button>("BarClose", projection, bar, WidgetTypeID::MenuBar, glfwWindow) };
    closeButton->SetSize(glm::vec3(barHeight - 2.0f * _margin, barHeight - 2.0f * _margin, 0.01f));
    closeButton->SetBackgroundImage(toUtf8(PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() / "Resources" / "img" / "CloseIcon.svg")); //UTF-8, as Texture reads paths
    closeButton->SetBackgroundImagePadding(4.0f);
    closeButton->SetBackgroundColor(glm::vec4(1.0f, 0.1f, 0.1f, 0.0f), ColorState::Enabled);
    closeButton->SetBackgroundColor(glm::vec4(1.0f, 0.2f, 0.2f, 0.45f), ColorState::Hover);
    closeButton->SetBackgroundColor(glm::vec4(1.0f, 0.3f, 0.3f, 0.6f), ColorState::Clicked);
    closeButton->SetBorderWidth(0.0f);
    bar->AddWidget(std::move(closeButton), _barCloseIndex);
    return bar;
}

void RetroFuturaGUI::FileExplorer::styleButton(Button& button, std::string_view text, const glm::vec3& size, const ViewFont& font)
{
    button.SetSize(size);
    button.SetCornerRadii(glm::vec4(8.0f));
    button.SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
    button.SetBackgroundColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), ColorState::Hover);
    button.SetBackgroundColor(glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), ColorState::Clicked);
    button.SetBackgroundColor(glm::vec4(0.12f, 0.12f, 0.12f, 1.0f), ColorState::Disabled);
    button.SetBackgroundFillType(FillType::SOLID);
    button.SetBorderColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), ColorState::Enabled);
    button.SetBorderWidth(1.0f);
    button.SetFontFamily(font._Family, font._Size, font._Slant, font._Weight);
    button.SetText(text, false);
    button.SetTextAlignment(TextAlignment::Center);
}

void RetroFuturaGUI::FileExplorer::styleMenu(ComboBox& menu, std::string_view caption, std::span<const MenuAction> actions, const glm::vec3& size, const f32 listWidth, const ViewFont& font)
{
    menu.SetFontFamily(font._Family, font._Size, font._Slant, font._Weight);
    menu.SetSize(size);
    menu.SetBackgroundColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), ColorState::Enabled); //flat until hovered, as a menu title is
    menu.SetBackgroundColor(glm::vec4(0.25f, 0.25f, 0.35f, 0.9f), ColorState::Hover);
    menu.SetBackgroundColor(glm::vec4(0.35f, 0.35f, 0.5f, 0.95f), ColorState::Clicked);
    menu.SetBorderWidth(0.0f);
    menu.SetTextAlignment(TextAlignment::Left);
    menu.SetDropDownWidth(listWidth);
    menu.SetCaption(caption);

    for(const MenuAction action : actions)
        menu.AddItem(menuText(action));
}

void RetroFuturaGUI::FileExplorer::styleCheckBox(CheckBox& checkBox, const f32 side)
{
    std::vector<glm::vec4>
        backgroundColors {{ 0.2f, 0.2f, 0.2f, 1.0f }},
        backgroundHoverColors {{ 0.3f, 0.3f, 0.3f, 1.0f }},
        checkedColors {{ _toggleOnColor }},
        checkedHoverColors {{ _toggleOnHoverColor }},
        checkedClickedColors {{ _toggleOnClickedColor }},
        checkmarkColors {{ 1.0f, 1.0f, 1.0f, 1.0f }},
        borderColors {{ 0.5f, 0.5f, 0.5f, 1.0f }};

    checkBox.SetSize(glm::vec3(side, side, 0.01f));
    checkBox.SetBackgroundColors(backgroundColors, ColorState::Enabled);
    checkBox.SetBackgroundColors(backgroundHoverColors, ColorState::Hover);
    checkBox.SetBackgroundColors(backgroundColors, ColorState::Clicked);
    checkBox.SetInheritColors(checkedColors, ColorState::Enabled);
    checkBox.SetInheritColors(checkedHoverColors, ColorState::Hover);
    checkBox.SetInheritColors(checkedClickedColors, ColorState::Clicked);
    checkBox.SetCheckmarkColors(checkmarkColors, ColorState::Enabled);
    checkBox.SetCheckmarkColors(checkmarkColors, ColorState::Hover);
    checkBox.SetCheckmarkColors(checkmarkColors, ColorState::Clicked);
    checkBox.SetBorderColors(borderColors, ColorState::Enabled);
    checkBox.SetBorderColors(borderColors, ColorState::Hover);
    checkBox.SetBorderColors(borderColors, ColorState::Clicked);
    checkBox.SetBorderWidth(1.0f);
    checkBox.SetCornerRadii(glm::vec4(6.0f));
    checkBox.SetInnerPadding(4.0f);
}

void RetroFuturaGUI::FileExplorer::styleLabel(Label& label, std::string_view text, const glm::vec3& size, const ViewFont& font)
{
    label.SetFontFamily(font._Family, font._Size, font._Slant, font._Weight);
    label.SetText(text, false);
    label.SetTextAlignment(TextAlignment::Left);
    label.SetTextColor(glm::vec4(1.0f), ColorState::Enabled);
    label.SetTextPadding(8.0f);
    label.SetSize(size);
}

void RetroFuturaGUI::FileExplorer::styleToggle(Button& toggle, const bool isOn)
{
    toggle.SetBackgroundColor(isOn ? _toggleOnColor : glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
    toggle.SetBackgroundColor(isOn ? _toggleOnHoverColor : glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), ColorState::Hover);
    toggle.SetBackgroundColor(isOn ? _toggleOnClickedColor : glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), ColorState::Clicked);
}

void RetroFuturaGUI::FileExplorer::styleTextBox(TextBox& textBox, std::string_view placeholder, const glm::vec3& size, const ViewFont& font)
{
    textBox.SetSize(size);
    textBox.SetCornerRadii(glm::vec4(8.0f));
    textBox.SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Enabled);
    textBox.SetBackgroundColor(glm::vec4(0.3f, 0.3f, 0.3f, 1.0f), ColorState::Hover);
    textBox.SetBackgroundColor(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), ColorState::Clicked);
    textBox.SetBackgroundFillType(FillType::SOLID);
    textBox.SetBorderColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), ColorState::Enabled);
    textBox.SetBorderWidth(1.0f);
    textBox.SetFontFamily(font._Family, font._Size, font._Slant, font._Weight);
    textBox.SetTextAlignment(TextAlignment::Left);
    textBox.SetTextPadding(6.0f);
    textBox.SetPlaceholderText(placeholder);
    textBox.SetPlaceholderTextColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
}

glm::vec3 RetroFuturaGUI::FileExplorer::placeButtonSize(const glm::vec2& windowSize, const ViewFont& font)
{
    return glm::vec3(static_cast<f32>(_placesColumnCount) * windowSize.x / static_cast<f32>(_viewColumnCount) - 2.0f * _margin, rowHeight(font) + 4.0f, 0.01f);
}

f32 RetroFuturaGUI::FileExplorer::rowHeight(const ViewFont& font)
{
    return std::ceil(font._Size * 1.55f);
}

RetroFuturaGUI::FileExplorer::ViewFont RetroFuturaGUI::FileExplorer::toolbarFont(const ViewFont& font)
{
    return ViewFont { ._Family = font._Family, ._Size = std::round(font._Size * 0.8f), ._Slant = font._Slant, ._Weight = font._Weight };
}

std::vector<RetroFuturaGUI::Table::TrackDefinition> RetroFuturaGUI::FileExplorer::columnDefinition(const ViewFont& font, const bool hasThumbnails)
{
    std::vector<Table::TrackDefinition> columns
    {
        { ._Sizing = Table::TrackSizing::Star, ._Value = 1.0f },                               //name
        { ._Sizing = Table::TrackSizing::Fixed, ._Value = std::ceil(6.0f * font._Size) },  //size
        { ._Sizing = Table::TrackSizing::Fixed, ._Value = std::ceil(11.0f * font._Size) }, //modified
        { ._Sizing = Table::TrackSizing::Fixed, ._Value = std::ceil(5.5f * font._Size) }   //type, and room for the sort arrow in its header
    };

    if(hasThumbnails)
        columns.insert(columns.begin(), Table::TrackDefinition { ._Sizing = Table::TrackSizing::Fixed, ._Value = _thumbnailColumnWidth });

    return columns;
}

glm::vec3 RetroFuturaGUI::FileExplorer::cellSize(const AxisDefinition& axis, const glm::vec2& windowSize, const ViewCell& cell)
{
    if(axis._RowDefinition.size() <= cell._Row)
        return glm::vec3(0.0f);

    if(axis._ColumnDefinition.size() < cell._Column + cell._ColumnSpan)
        return glm::vec3(0.0f);

    f32 columnShare { 0.0f };

    for(uSize column = cell._Column; column < cell._Column + cell._ColumnSpan; ++column)
        columnShare += axis._ColumnDefinition[column];

    return glm::vec3(columnShare * windowSize.x - 2.0f * _margin, axis._RowDefinition[cell._Row] * windowSize.y - 2.0f * _margin, 0.01f);
}

bool RetroFuturaGUI::FileExplorer::hasPreview() const
{
    return HasFlag(_flags, FileExplorerFlags::ImagePreview) || HasFlag(_flags, FileExplorerFlags::MediaPreview);
}

RetroFuturaGUI::FileExplorer::Mode RetroFuturaGUI::FileExplorer::modeOf(const FileExplorerFlags flags)
{
    if(HasFlag(flags, FileExplorerFlags::MultiFile))
        return Mode::MultiFile;

    if(HasFlag(flags, FileExplorerFlags::Folder))
        return Mode::Folder;

    if(HasFlag(flags, FileExplorerFlags::WriteFile))
        return Mode::WriteFile;

    return Mode::SingleFile;
}

std::vector<RetroFuturaGUI::FileExplorer::Filter> RetroFuturaGUI::FileExplorer::filtersOf(std::string_view extensions)
{
    std::vector<Filter> filters {};
    const std::vector<std::string> split { splitExtensions(extensions) };
    Filter together { ._Name = {}, ._Extensions = {} };
    bool listsEveryFile { split.empty() }; //nothing usable lists every file, as ".*" does

    for(const std::string& extension : split)
    {
        if(".*" == extension)
        {
            listsEveryFile = true;
            continue;
        }

        together._Extensions.push_back(fromUtf8(extension));
    }

    if(1 < together._Extensions.size())
    {
        together._Name = joinedExtensions(together._Extensions);
        filters.push_back(together);
    }

    for(const std::filesystem::path& extension : together._Extensions)
        filters.push_back(Filter { ._Name = toUtf8(extension), ._Extensions = { extension } });

    if(listsEveryFile)
        filters.push_back(Filter { ._Name = "All files", ._Extensions = {} });

    return filters;
}

std::vector<RetroFuturaGUI::FileExplorer::Filter> RetroFuturaGUI::FileExplorer::filtersOf(const std::vector<FileFilter>& fileFilters)
{
    std::vector<Filter> filters {};

    for(const FileFilter& fileFilter : fileFilters)
    {
        Filter filter { ._Name = fileFilter._Name, ._Extensions = {} };
        bool listsEveryFile { false };

        for(const std::string& extension : splitExtensions(fileFilter._Extensions))
        {
            if(".*" == extension)
            {
                listsEveryFile = true;
                continue;
            }

            filter._Extensions.push_back(fromUtf8(extension));
        }

        if(listsEveryFile)
            filter._Extensions.clear(); //every file is listed anyway

        if(filter._Name.empty())
            filter._Name = filter._Extensions.empty() ? "All files" : joinedExtensions(filter._Extensions);

        filters.push_back(std::move(filter));
    }

    if(filters.empty())
        filters.push_back(Filter { ._Name = "All files", ._Extensions = {} });

    return filters;
}

std::string RetroFuturaGUI::FileExplorer::joinedExtensions(const std::vector<std::filesystem::path>& extensions)
{
    std::string joined {};

    for(const std::filesystem::path& extension : extensions)
        joined += joined.empty() ? toUtf8(extension) : ", " + toUtf8(extension);

    return joined;
}

std::vector<std::string> RetroFuturaGUI::FileExplorer::splitExtensions(std::string_view extensions)
{
    std::vector<std::string> split {};

    for(uSize start { 0 }; start <= extensions.size();)
    {
        const uSize comma { extensions.find(',', start) };
        const uSize end { std::string_view::npos == comma ? extensions.size() : comma };
        std::string_view token { extensions.substr(start, end - start) };
        start = end + 1;

        while(!token.empty() && (' ' == token.front() || '\t' == token.front()))
            token.remove_prefix(1);

        while(!token.empty() && (' ' == token.back() || '\t' == token.back()))
            token.remove_suffix(1);

        if(token.empty())
            continue;

        if(token.starts_with("*."))
            token.remove_prefix(1); //"*.mp3", the way file dialogs write it

        std::string extension { token.starts_with('.') ? std::string(token) : "." + std::string(token) };

        for(char& character : extension)
        {
            if('A' <= character && 'Z' >= character)
                character = static_cast<char>(character - 'A' + 'a'); //".MP3" and ".mp3" are one entry
        }

        if(std::find(split.begin(), split.end(), extension) != split.end())
            continue;

        split.push_back(std::move(extension));
    }

    return split;
}

void RetroFuturaGUI::FileExplorer::ensureFontBlock(const u32 first, const u32 last) const
{
    const FontInfo* fontInfo { FontManager::GetFontInfo(_font._Family, _font._Size, _font._Slant, _font._Weight) };

    if(!fontInfo)
        return;

    const auto atlas { fontInfo->_Atlasses.find(FontManager::FontSizeToIntegral(_font._Size)) };

    if(fontInfo->_Atlasses.end() == atlas)
        return;

    if(atlas->second._GlyphBlocks.contains(first))
        return;

    FontManager::ExtendFontset(_font._Family, _font._Family, _font._Size, _font._Slant, _font._Weight, first, last);
}

void RetroFuturaGUI::FileExplorer::applyMode()
{
    if(!_widgets._Files)
        return;

    if(!_widgets._Ok)
        return;

    _widgets._Files->SetRowSelection(Mode::MultiFile == _mode ? Table::RowSelection::Multiple : Table::RowSelection::Single);
    _widgets._Ok->SetText(getOkButtonText(), false);
}

void RetroFuturaGUI::FileExplorer::addPlaces()
{
    using PlatformBridge::FileSystem;

    const std::array<std::pair<FileSystem::KnownFolder, std::string_view>, 7> knownFolders
    {{
        { FileSystem::KnownFolder::Home, "Home" },
        { FileSystem::KnownFolder::Desktop, "Desktop" },
        { FileSystem::KnownFolder::Documents, "Documents" },
        { FileSystem::KnownFolder::Downloads, "Downloads" },
        { FileSystem::KnownFolder::Music, "Music" },
        { FileSystem::KnownFolder::Pictures, "Pictures" },
        { FileSystem::KnownFolder::Videos, "Videos" }
    }};
    uSize placeCount { 0 };

    for(const auto& [folder, name] : knownFolders)
    {
        const std::filesystem::path& place { FileSystem::GetKnownFolder(folder) };

        if(place.empty())
            continue;

        addPlaceButton(name, place, placeCount);
        ++placeCount;
    }

    for(const FileSystem::Root& root : FileSystem::GetRoots())
    {
        //"Windows OS (C:)", as Windows names drives; a Linux volume goes by its mount point
        const std::filesystem::path drive { root._Path.has_root_name() ? root._Path.root_name() : root._Path };
        const std::string text { root._Label.empty() ? toUtf8(root._Path) : std::format("{} ({})", root._Label, toUtf8(drive)) };
        addPlaceButton(text, root._Path, placeCount);
        ++placeCount;
    }
}

void RetroFuturaGUI::FileExplorer::addPlaceButton(std::string_view text, const std::filesystem::path& place, const uSize index)
{
    if(!_widgets._Places)
        return;

    Button* button { _widgets._Places->AddWidget<Button>(std::format("Place{}", index)) };

    if(!button)
        return;

    styleButton(*button, text, placeButtonSize(glm::vec2(static_cast<f32>(_width), static_cast<f32>(_height)), _font), _font);
    button->SetTextAlignment(TextAlignment::Left);
    button->SetTextPadding(8.0f);
    button->Connect_OnClick([this, place]() { on_placeButton_click(place); }, false);
    _placeButtons.emplace_back(button, place); //where a drag can drop
}

void RetroFuturaGUI::FileExplorer::addFilters()
{
    if(!_widgets._Filter)
        return;

    for(const Filter& filter : _filters)
        _widgets._Filter->AddItem(filter._Name);
}

void RetroFuturaGUI::FileExplorer::navigateToStart(std::string_view startPath)
{
    std::error_code error {};
    std::filesystem::path start { fromUtf8(startPath) };

    if(!start.empty() && start.is_relative())
        start = std::filesystem::absolute(start, error);

    //A file opens its folder, and in WriteFile mode names the file to write
    if(!start.empty() && std::filesystem::is_regular_file(start, error))
    {
        if(Mode::WriteFile == _mode && _widgets._Name)
            _widgets._Name->SetValue<std::string_view>(toUtf8(start.filename()), false);

        start = start.parent_path();
    }

    std::vector<std::filesystem::path> candidates { start, std::filesystem::current_path(error), PlatformBridge::FileSystem::GetKnownFolder(PlatformBridge::FileSystem::KnownFolder::Home) };

    for(const PlatformBridge::FileSystem::Root& root : PlatformBridge::FileSystem::GetRoots())
        candidates.push_back(root._Path);

    std::erase_if(candidates, [](const std::filesystem::path& candidate) { return candidate.empty(); });

    if(candidates.empty())
        return;

    //The first that can be listed: each one that can't hands on to the next once its listing comes back
    const std::filesystem::path first { candidates.front() };
    candidates.erase(candidates.begin());
    requestListing(first, ListingIntent { ._Fallbacks = std::move(candidates), ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = true, ._KeepsView = false });
}

void RetroFuturaGUI::FileExplorer::navigateTo(const std::filesystem::path& directory, const bool addToHistory)
{
    requestListing(directory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = addToHistory, ._KeepsView = false });
}

void RetroFuturaGUI::FileExplorer::relist()
{
    if(_pending)
        return;

    requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = false, ._KeepsView = true });
}

void RetroFuturaGUI::FileExplorer::showEntries()
{
    //The listing on its way has to be shown with the new settings, and it is newer than what the list shows
    if(_pending)
    {
        PendingListing pending { *_pending };
        requestListing(pending._Directory, std::move(pending._Intent));
        return;
    }

    ListingRequest request { listingRequest(_currentDirectory) };
    request._Entries = _listedEntries;
    _pending = PendingListing { ._Directory = _currentDirectory, ._Intent = ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = false, ._KeepsView = true }, ._Since = std::chrono::steady_clock::now(), ._IsShowingProgress = false };
    postListing(std::move(request));
}

void RetroFuturaGUI::FileExplorer::requestListing(const std::filesystem::path& directory, ListingIntent&& intent)
{
    //A copy first: the caller may pass an entry or a history slot that the listing replaces
    const std::filesystem::path target { normalizedDirectory(directory) };

    //A rename being typed is kept, as a press elsewhere keeps it, so the listing finds the new name
    if(_renamingPath && _widgets._Files)
        _widgets._Files->EndEdit();

    if(target != _currentDirectory)
        _statusMessage.clear(); //it was about the folder being left

    _pending = PendingListing { ._Directory = target, ._Intent = std::move(intent), ._Since = std::chrono::steady_clock::now(), ._IsShowingProgress = false };
    postListing(listingRequest(target));
}

void RetroFuturaGUI::FileExplorer::postListing(ListingRequest&& request)
{
    //A listing still running, maybe stuck on a slow drive, would hold the new one up behind it. Its worker is let go to
    //finish on its own and drops what it made, and a new one takes this listing.
    if(!_listingWork || _listingWork->IsBusy())
        _listingWork = std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish);

    if(!_listingWork)
        return;

    request._Generation = ++_listingGeneration;
    _listingWork->Post([request = std::move(request)](std::stop_token stop) mutable { return makeListing(std::move(request), stop); },
                       [this](Listing&& listing) { showListing(std::move(listing)); });
}

RetroFuturaGUI::FileExplorer::ListingRequest RetroFuturaGUI::FileExplorer::listingRequest(const std::filesystem::path& directory) const
{
    return ListingRequest
    {
        ._Generation = 0, //postListing numbers it
        ._Directory = directory,
        ._Entries = {},
        ._Filter = getCurrentFilter(),
        ._Mode = _mode,
        ._SortColumn = _sortColumn,
        ._ShowsHidden = _showHidden,
        ._IsSortDescending = _isSortDescending
    };
}

RetroFuturaGUI::FileExplorer::Listing RetroFuturaGUI::FileExplorer::makeListing(ListingRequest&& request, std::stop_token stop)
{
    Listing listing { ._Generation = request._Generation, ._Directory = request._Directory, ._Error = {}, ._IsListed = false, ._Listed = {}, ._Shown = {}, ._Rows = {} };

    if(request._Entries)
    {
        listing._Listed = std::move(*request._Entries);
        listing._IsListed = true;
    }
    else
    {
        listing._IsListed = PlatformBridge::FileSystem::ListDirectory(request._Directory, listing._Listed, listing._Error);
    }

    if(!listing._IsListed || stop.stop_requested())
        return listing;

    for(const Entry& entry : listing._Listed)
    {
        if(entry._IsHidden && !request._ShowsHidden)
            continue;

        const bool isDirectory { PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind };

        if(!isDirectory && (Mode::Folder == request._Mode || !matchesFilter(entry._Path, request._Filter)))
            continue;

        listing._Shown.push_back(entry);
    }

    sortEntries(listing._Shown, request._SortColumn, request._IsSortDescending);

    //Every cell's text, which is most of a big folder's fill: both names, so the extensions toggle only picks one
    LocalClock clock { ._Zone = getLocalZone(), ._Info = {}, ._HasInfo = false };
    listing._Rows.reserve(listing._Shown.size());

    for(const Entry& entry : listing._Shown)
    {
        const bool isDirectory { PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind };
        std::string name { toUtf8(entry._Path.filename()) };
        std::string shortName { isDirectory ? name : toUtf8(entry._Path.stem()) }; //".bashrc" keeps its name: a leading dot is no extension

        listing._Rows.push_back(RowTexts
        {
            ._Name = std::move(name),
            ._ShortName = std::move(shortName),
            ._Size = isDirectory ? std::string {} : formatSize(entry._Size),
            ._Modified = formatTime(entry._LastWriteTime, clock),
            ._Type = typeName(entry)
        });
    }

    return listing;
}

void RetroFuturaGUI::FileExplorer::showListing(Listing&& listing)
{
    if(listing._Generation != _listingGeneration)
        return; //a newer listing was asked for since

    if(!_pending)
        return;

    PendingListing pending { std::move(*_pending) };
    _pending.reset();

    if(!listing._IsListed)
    {
        if(!pending._Intent._Fallbacks.empty())
        {
            const std::filesystem::path next { pending._Intent._Fallbacks.front() };
            pending._Intent._Fallbacks.erase(pending._Intent._Fallbacks.begin());
            requestListing(next, std::move(pending._Intent));
            return;
        }

        //The folder shown was deleted or moved away: the list goes to the one above it, the first that still exists
        const bool isShownFolderGone { pending._Intent._KeepsView && listing._Directory == _currentDirectory && listing._Error == std::errc::no_such_file_or_directory };

        if(isShownFolderGone && _currentDirectory.has_relative_path())
        {
            std::vector<std::filesystem::path> ancestors {};

            for(std::filesystem::path ancestor { _currentDirectory.parent_path() }; ; ancestor = ancestor.parent_path())
            {
                ancestors.push_back(ancestor);

                if(!ancestor.has_relative_path())
                    break;
            }

            const std::filesystem::path parent { ancestors.front() };
            ancestors.erase(ancestors.begin());
            requestListing(parent, ListingIntent { ._Fallbacks = std::move(ancestors), ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = true, ._KeepsView = false });
            return;
        }

        setStatus(std::format("Can't open {}: {}", toUtf8(listing._Directory), describeError(listing._Error)));
        updateStatusNote(); //no longer "Opening"
        return;
    }

    const bool
        isSameFolder { listing._Directory == _currentDirectory },
        keepsView { pending._Intent._KeepsView && isSameFolder };
    const std::filesystem::path previousDirectory { _currentDirectory };
    const std::vector<std::filesystem::path> selected { keepsView ? getSelectedPaths() : std::vector<std::filesystem::path> {} };
    const f32 scroll { keepsView && _widgets._Files ? _widgets._Files->GetVerticalScrollPosition() : 0.0f };

    if(pending._Intent._HistoryIndex && *pending._Intent._HistoryIndex < _history.size())
    {
        _historyIndex = *pending._Intent._HistoryIndex;
    }
    else if(pending._Intent._AddsToHistory && !isSameFolder)
    {
        //Opening a folder drops the ones Forward would have gone back to
        if(!_history.empty())
            _history.resize(_historyIndex + 1);

        _history.push_back(listing._Directory);
        _historyIndex = _history.size() - 1;
    }

    _currentDirectory = listing._Directory;
    _listedEntries = std::move(listing._Listed);
    _shownEntries = std::move(listing._Shown);
    _shownRows = std::move(listing._Rows);

    if(!isSameFolder)
        disarmReplace();

    //The folder shown is the one watched. A watch lost with its folder is taken up again once the folder lists.
    if(!isSameFolder || !_watcher.IsWatching())
    {
        std::error_code watchError {};
        _watcher.Watch(_currentDirectory, watchError); //unwatched, the list only changes with F5
        _hasUnlistedChange = false;
    }

    if(_widgets._Address)
        _widgets._Address->SetValue<std::string_view>(toUtf8(_currentDirectory), false);

    fillTable();
    updateNavigationButtons();

    if(keepsView && _widgets._Files)
    {
        _widgets._Files->SetVerticalScrollPosition(scroll); //before the selection, which scrolls only when its row is out of view
        syncScrollSlider();
    }

    if(!pending._Intent._Select.empty())
    {
        selectPaths(pending._Intent._Select, true); //a folder just made or a file just renamed, wherever it sorted to
    }
    else if(keepsView)
    {
        selectPaths(selected, false); //the same rows as before, so a slot needn't hear of them again
    }
    else if(previousDirectory.parent_path() == _currentDirectory && previousDirectory != _currentDirectory)
    {
        selectPaths({ previousDirectory }, true); //Up or Back out of a folder selects it, so the keys go on from where the list was
    }

    updateStatusNote(); //a message, like an operation's outcome or the question whether to replace a file, stays over it
    updateMenus(); //the rows changed, and with them what the menus can do
    showPreview(); //a file changed on disk shows again; the same one unchanged stays

    if(pending._Intent._WriteTarget.empty())
        return;

    if(_widgets._Name)
        _widgets._Name->SetValue<std::string_view>(toUtf8(pending._Intent._WriteTarget.filename()), false);

    confirmWrite(pending._Intent._WriteTarget);
}

void RetroFuturaGUI::FileExplorer::update()
{
    if(_isDragOutPending)
        dragOut();

    drainListings();
    followTransfer();
    followClipboard();
    showThumbnails(); //after the listing, so a list filled anew gets its thumbnails before it draws

    if(_preview)
        _preview->Update();
    watchChanges();
    showProgress();
    handleKeys();
}

void RetroFuturaGUI::FileExplorer::watchChanges()
{
    const std::chrono::steady_clock::time_point now { std::chrono::steady_clock::now() };

    //A watch that ended with its folder still has that last change to list, which finds the folder gone
    if(_watcher.HasChanged())
    {
        if(!_hasUnlistedChange)
            _firstUnlistedChange = now;

        _hasUnlistedChange = true;
        _lastChange = now;
    }

    if(!_hasUnlistedChange)
        return;

    if(_relistQuietTime > now - _lastChange && _relistLongestWait > now - _firstUnlistedChange)
        return;

    if(_pending)
        return; //it may have been asked for before the change, so the change waits until it is shown

    if(_renamingPath)
        return; //listing again would end the rename being typed

    _hasUnlistedChange = false;
    relist();
}

void RetroFuturaGUI::FileExplorer::drainListings()
{
    if(!_listingWork)
        return;

    _listingWork->Drain();
}

void RetroFuturaGUI::FileExplorer::showProgress()
{
    if(!_pending)
        return;

    if(_pending->_IsShowingProgress)
        return;

    if(std::chrono::steady_clock::now() - _pending->_Since < _progressDelay)
        return; //a quick listing would only flash the text

    _pending->_IsShowingProgress = true;
    updateStatusNote();
}

const std::filesystem::path& RetroFuturaGUI::FileExplorer::getTargetDirectory() const
{
    return _pending ? _pending->_Directory : _currentDirectory;
}

void RetroFuturaGUI::FileExplorer::updateHeaderTexts()
{
    if(!_widgets._Files)
        return;

    constexpr std::array<std::string_view, _fileColumnCount> names { "Name", "Size", "Modified", "Type" };
    constexpr std::array<SortColumn, _fileColumnCount> columns { SortColumn::Name, SortColumn::Size, SortColumn::Modified, SortColumn::Type };
    const uSize firstColumn { getNameColumn() };

    if(_showThumbnails)
        _widgets._Files->SetHorizontalHeaderText("", 0); //the thumbnails' column has no name, and may hold the one Name had

    for(uSize column = 0; column < _fileColumnCount; ++column)
    {
        std::string text { names[column] };

        if(columns[column] == _sortColumn)
            text += _isSortDescending ? " \xE2\x96\xBC" : " \xE2\x96\xB2"; //U+25BC and U+25B2 in UTF-8, from the Geometric Shapes block Open adds

        _widgets._Files->SetHorizontalHeaderText(text, firstColumn + column);
    }
}

std::vector<std::filesystem::path> RetroFuturaGUI::FileExplorer::getSelectedPaths() const
{
    std::vector<std::filesystem::path> paths {};

    if(!_widgets._Files)
        return paths;

    for(const uSize row : _widgets._Files->GetSelectedRows())
    {
        if(_shownEntries.size() <= row)
            continue;

        paths.push_back(_shownEntries[row]._Path);
    }

    return paths;
}

void RetroFuturaGUI::FileExplorer::selectPaths(const std::vector<std::filesystem::path>& paths, const bool emitSignal)
{
    if(!_widgets._Files)
        return;

    //Sorted, so a folder with thousands of rows, all selected, isn't compared row by row against every path
    std::vector<std::filesystem::path::string_type> wanted {};
    wanted.reserve(paths.size());

    for(const std::filesystem::path& path : paths)
        wanted.push_back(path.native());

    std::sort(wanted.begin(), wanted.end());
    std::vector<uSize> rows {};

    for(uSize row = 0; row < _shownEntries.size(); ++row)
    {
        if(!std::binary_search(wanted.begin(), wanted.end(), _shownEntries[row]._Path.native()))
            continue;

        rows.push_back(row);
    }

    if(rows.empty())
        return;

    _isSelectingByCode = true;
    _widgets._Files->SelectRows(rows, emitSignal);
    _isSelectingByCode = false;
    _widgets._Files->ScrollRowIntoView(rows.front(), false);
    syncScrollSlider();
}

void RetroFuturaGUI::FileExplorer::fillTable()
{
    if(!_widgets._Files)
        return;

    const std::vector<Table::TrackDefinition> rowDefinition(_shownRows.size(), Table::TrackDefinition { ._Sizing = Table::TrackSizing::Fixed, ._Value = getListRowHeight() });
    _widgets._Files->SetTrackDefinitions(rowDefinition, columnDefinition(_font, _showThumbnails));

    //The read-only flags follow the columns, which the thumbnails' adds to
    for(uSize column = 0; column < getNameColumn() + _fileColumnCount; ++column)
        _widgets._Files->SetTrackReadOnly(column, true);

    const uSize firstColumn { getNameColumn() };

    for(uSize row = 0; row < _shownRows.size(); ++row)
    {
        const RowTexts& texts { _shownRows[row] };
        _widgets._Files->SetValue(displayName(texts), TrackIndex { ._Row = row, ._Column = firstColumn, ._Layer = 0 }, false);
        _widgets._Files->SetValue(texts._Size, TrackIndex { ._Row = row, ._Column = firstColumn + 1, ._Layer = 0 }, false);
        _widgets._Files->SetValue(texts._Modified, TrackIndex { ._Row = row, ._Column = firstColumn + 2, ._Layer = 0 }, false);
        _widgets._Files->SetValue(texts._Type, TrackIndex { ._Row = row, ._Column = firstColumn + 3, ._Layer = 0 }, false);
    }

    if(_showThumbnails)
    {
        if(!_thumbnails)
            _thumbnails = std::make_unique<ThumbnailCache>();

        _thumbnails->ShowIcons(*_widgets._Files, _shownEntries, 0);
    }

    _widgets._Files->SetScrollPosition(glm::vec2(0.0f));
    updateHeaderTexts(); //the header cells came anew with the columns
    updateScrollSlider();
}

const std::string& RetroFuturaGUI::FileExplorer::displayName(const RowTexts& texts) const
{
    return _showExtensions ? texts._Name : texts._ShortName;
}

void RetroFuturaGUI::FileExplorer::updateNameColumn()
{
    if(!_widgets._Files)
        return;

    for(uSize row = 0; row < _shownRows.size(); ++row)
        _widgets._Files->SetValue(displayName(_shownRows[row]), TrackIndex { ._Row = row, ._Column = getNameColumn(), ._Layer = 0 }, false);
}

uSize RetroFuturaGUI::FileExplorer::getNameColumn() const
{
    return _showThumbnails ? 1 : 0;
}

f32 RetroFuturaGUI::FileExplorer::getListRowHeight() const
{
    return _showThumbnails ? std::max(_thumbnailRowHeight, rowHeight(_font)) : rowHeight(_font);
}

void RetroFuturaGUI::FileExplorer::showThumbnails()
{
    if(!_showThumbnails)
        return;

    if(!_thumbnails)
        return;

    if(!_widgets._Files)
        return;

    _thumbnails->Update(*_widgets._Files, _shownEntries, 0);
}

void RetroFuturaGUI::FileExplorer::updateScrollSlider()
{
    if(!_widgets._Scroll)
        return;

    if(!_widgets._Files)
        return;

    const f32 maxScroll { _widgets._Files->GetMaxScroll().y };

    //Back to the top first: a smaller range would clamp the value, and that clamp would scroll the table again.
    //A range of zero would leave the indicator nowhere to go, so a list that fits keeps one and disables the slider.
    _widgets._Scroll->SetValue<f32>(0.0f, false);
    _widgets._Scroll->SetMaxValue<f32>(0.0f < maxScroll ? maxScroll : 1.0f);
    _widgets._Scroll->SetEnabled(0.0f < maxScroll, false);
    _widgets._Scroll->SetStepSize<f32>(getListRowHeight()); //a row, which thumbnails make taller
}

void RetroFuturaGUI::FileExplorer::syncScrollSlider()
{
    if(!_widgets._Scroll)
        return;

    if(!_widgets._Files)
        return;

    _widgets._Scroll->SetValue<f32>(_widgets._Files->GetVerticalScrollPosition(), false);
}

void RetroFuturaGUI::FileExplorer::updateNavigationButtons()
{
    if(!_widgets._Back)
        return;

    if(!_widgets._Forward)
        return;

    if(!_widgets._Up)
        return;

    _widgets._Back->SetEnabled(0 < _historyIndex, false);
    _widgets._Forward->SetEnabled(_historyIndex + 1 < _history.size(), false);
    _widgets._Up->SetEnabled(_currentDirectory.has_relative_path(), false); //a root has nothing above it
}

void RetroFuturaGUI::FileExplorer::setStatus(std::string_view text)
{
    _statusMessage = text;
    showStatus();
}

void RetroFuturaGUI::FileExplorer::updateStatusNote()
{
    const uSize selectedCount { _widgets._Files ? _widgets._Files->GetSelectedRows().size() : 0 };

    if(_pending && _pending->_IsShowingProgress)
    {
        const std::filesystem::path name { _pending->_Directory.has_filename() ? _pending->_Directory.filename() : _pending->_Directory };
        _statusNote = std::format("Opening {}...", toUtf8(name));
    }
    else if(_shownEntries.empty())
    {
        _statusNote = _listedEntries.empty() ? "This folder is empty" : "Nothing here matches the filter";
    }
    else
    {
        _statusNote = Mode::MultiFile == _mode && 1 < selectedCount ? std::format("{} selected", selectedCount) : std::string {};
    }

    showStatus();
}

void RetroFuturaGUI::FileExplorer::showPreview()
{
    if(!_preview)
        return;

    if(!_widgets._Files)
        return;

    std::vector<Entry> selected {};

    for(const uSize row : _widgets._Files->GetSelectedRows())
    {
        if(_shownEntries.size() <= row)
            continue;

        selected.push_back(_shownEntries[row]);
    }

    _preview->Show(selected);
}

void RetroFuturaGUI::FileExplorer::showStatus()
{
    if(!_widgets._Status)
        return;

    _widgets._Status->SetText(_statusMessage.empty() ? _statusNote : _statusMessage, false);
}

const RetroFuturaGUI::FileExplorer::Filter& RetroFuturaGUI::FileExplorer::getCurrentFilter() const
{
    //both filtersOf leave at least one
    const uSize index { _widgets._Filter ? _widgets._Filter->GetSelectedIndex() : 0 };
    return _filters[index < _filters.size() ? index : 0];
}

void RetroFuturaGUI::FileExplorer::openSelectedFiles()
{
    if(!_widgets._Files)
        return;

    std::vector<std::filesystem::path> files {};
    std::filesystem::path folder {};

    for(const uSize row : _widgets._Files->GetSelectedRows())
    {
        if(_shownEntries.size() <= row)
            continue;

        const Entry& entry { _shownEntries[row] };

        if(PlatformBridge::FileSystem::EntryKind::Directory != entry._Kind)
            files.push_back(entry._Path);
        else if(folder.empty())
            folder = entry._Path;
    }

    if(!files.empty())
    {
        closeWith(files);
        return;
    }

    //Only folders selected: OK opens the first, as a double-click would
    if(!folder.empty())
    {
        navigateTo(folder, true);
        return;
    }

    setStatus(Mode::MultiFile == _mode ? "Select one or more files first" : "Select a file first");
}

void RetroFuturaGUI::FileExplorer::openSelectedFolder()
{
    if(!_widgets._Files)
        return;

    for(const uSize row : _widgets._Files->GetSelectedRows())
    {
        if(_shownEntries.size() <= row)
            continue;

        if(PlatformBridge::FileSystem::EntryKind::Directory != _shownEntries[row]._Kind)
            continue;

        closeWith({ _shownEntries[row]._Path });
        return;
    }

    //Nothing selected: the folder that is open
    closeWith({ _currentDirectory });
}

std::filesystem::path RetroFuturaGUI::FileExplorer::getNameBoxTarget() const
{
    if(!_widgets._Name)
        return {};

    std::filesystem::path name { fromUtf8(_widgets._Name->GetText()) };

    if(name.empty())
        return {};

    const Filter& filter { getCurrentFilter() };

    if(!name.has_extension() && !filter._Extensions.empty())
        name += filter._Extensions.front();

    //A typed absolute path replaces the folder
    return (_currentDirectory / name).lexically_normal();
}

void RetroFuturaGUI::FileExplorer::confirmWrite(const std::filesystem::path& target)
{
    if(!_widgets._Ok)
        return;

    if(!target.has_filename())
    {
        setStatus("Type a file name first");
        return;
    }

    std::error_code error {};
    const std::filesystem::file_status status { std::filesystem::status(target, error) };

    //A folder's name opens it
    if(std::filesystem::is_directory(status))
    {
        navigateTo(target, true);
        return;
    }

    if(!std::filesystem::is_directory(target.parent_path(), error))
    {
        setStatus(std::format("The folder {} doesn't exist", toUtf8(target.parent_path())));
        return;
    }

    const bool isConfirmed { _isReplaceArmed && _replaceTarget == target };

    if(std::filesystem::exists(status) && !isConfirmed)
    {
        _isReplaceArmed = true;
        _replaceTarget = target;
        _widgets._Ok->SetText("Replace", false);
        setStatus(std::format("{} already exists. Press Replace to overwrite it.", toUtf8(target.filename())));
        return;
    }

    closeWith({ target });
}

void RetroFuturaGUI::FileExplorer::disarmReplace()
{
    if(!_widgets._Ok)
        return;

    if(!_isReplaceArmed)
        return;

    _isReplaceArmed = false;
    _replaceTarget.clear();
    _widgets._Ok->SetText(getOkButtonText(), false);
    setStatus("");
}

void RetroFuturaGUI::FileExplorer::closeWith(const std::vector<std::filesystem::path>& paths)
{
    std::vector<std::string> results {};
    results.reserve(paths.size());

    for(const std::filesystem::path& path : paths)
        results.push_back(toUtf8(path));

    closeAfterAsking([this, results = std::move(results)]() mutable { Close(std::move(results)); });
}

std::string_view RetroFuturaGUI::FileExplorer::getOkButtonText() const
{
    switch(_mode)
    {
        case Mode::WriteFile:
            return "Save";
        case Mode::Folder:
            return "Select";
        default:
            return "Open";
    }
}

void RetroFuturaGUI::FileExplorer::on_backButton_click()
{
    if(0 == _historyIndex)
        return;

    //The index moves once the folder is shown, so a folder that can't be listed leaves it where it was
    requestListing(_history[_historyIndex - 1], ListingIntent { ._Fallbacks = {}, ._HistoryIndex = _historyIndex - 1, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = false, ._KeepsView = false });
}

void RetroFuturaGUI::FileExplorer::on_forwardButton_click()
{
    if(_history.size() <= _historyIndex + 1)
        return;

    requestListing(_history[_historyIndex + 1], ListingIntent { ._Fallbacks = {}, ._HistoryIndex = _historyIndex + 1, ._WriteTarget = {}, ._Select = {}, ._AddsToHistory = false, ._KeepsView = false });
}

void RetroFuturaGUI::FileExplorer::on_upButton_click()
{
    //Above the folder on its way, so pressing Up twice quickly goes up twice
    const std::filesystem::path directory { getTargetDirectory() };

    if(!directory.has_relative_path())
        return;

    navigateTo(directory.parent_path(), true);
}

void RetroFuturaGUI::FileExplorer::on_addressTextBox_enterPressed()
{
    if(!_widgets._Address)
        return;

    const std::filesystem::path typed { fromUtf8(_widgets._Address->GetText()) };

    if(typed.empty())
        return;

    //Relative to the open folder; an absolute path replaces it. A path that leads nowhere does nothing.
    const std::filesystem::path path { (_currentDirectory / typed).lexically_normal() };
    std::error_code error {};
    const std::filesystem::file_status status { std::filesystem::status(path, error) };

    if(std::filesystem::is_directory(status))
    {
        navigateTo(path, true);
        return;
    }

    switch(_mode)
    {
        case Mode::SingleFile:
        case Mode::MultiFile:
            if(!std::filesystem::exists(status))
                return;

            closeWith({ path });
            return;
        case Mode::WriteFile:
            //A file to write, new or existing, in a folder that exists: its folder opens, then the name goes in the name box
            if(!std::filesystem::is_directory(path.parent_path(), error))
                return;

            requestListing(path.parent_path(), ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = path, ._Select = {}, ._AddsToHistory = true, ._KeepsView = false });
            return;
        case Mode::Folder:
            if(!std::filesystem::exists(status))
                return;

            navigateTo(path.parent_path(), true); //a file opens the folder holding it
            return;
    }
}

void RetroFuturaGUI::FileExplorer::on_placeButton_click(const std::filesystem::path& place)
{
    navigateTo(place, true);
}

void RetroFuturaGUI::FileExplorer::on_filesTable_selectionChanged()
{
    if(!_widgets._Files)
        return;

    updateMenus();

    //A message stays until the user picks something else; a selection made by the explorer leaves it
    if(!_isSelectingByCode)
        _statusMessage.clear();

    updateStatusNote();
    showPreview();
    const std::vector<uSize> rows { _widgets._Files->GetSelectedRows() };

    if(Mode::WriteFile == _mode)
    {
        if(!_widgets._Name)
            return;

        if(1 != rows.size() || _shownEntries.size() <= rows.front())
            return;

        const Entry& entry { _shownEntries[rows.front()] };

        if(PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind)
            return;

        _widgets._Name->SetValue<std::string_view>(toUtf8(entry._Path.filename()), false);
        disarmReplace();
    }
}

void RetroFuturaGUI::FileExplorer::on_filesTable_rowDoubleClicked(const uSize row)
{
    if(_shownEntries.size() <= row)
        return;

    //A copy: opening a folder replaces the entries
    const Entry entry { _shownEntries[row] };

    if(PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind)
    {
        navigateTo(entry._Path, true);
        return;
    }

    switch(_mode)
    {
        case Mode::WriteFile:
            confirmWrite(entry._Path); //asks first, since the file exists
            return;
        case Mode::Folder:
            return;
        default:
            closeWith({ entry._Path });
    }
}

void RetroFuturaGUI::FileExplorer::on_filesTable_headerClicked(const uSize column)
{
    constexpr std::array<SortColumn, _fileColumnCount> columns { SortColumn::Name, SortColumn::Size, SortColumn::Modified, SortColumn::Type };

    if(getNameColumn() > column)
        return; //the thumbnails don't sort

    const uSize fileColumn { column - getNameColumn() };

    if(columns.size() <= fileColumn)
        return;

    if(columns[fileColumn] == _sortColumn)
    {
        _isSortDescending = !_isSortDescending;
    }
    else
    {
        _sortColumn = columns[fileColumn];
        _isSortDescending = false;
    }

    updateHeaderTexts();
    showEntries(); //keeps the selection, its rows only move
}

template<typename Prompt>
Prompt* RetroFuturaGUI::FileExplorer::pushPrompt(std::unique_ptr<Prompt> prompt)
{
    if(!prompt)
        return nullptr;

    Window* window { GetWindow() };

    if(!window)
        return nullptr;

    //Through this explorer's window, which hands it on to the stack the explorer is on. The keys come back to the list once
    //it's dismissed; a result's slot gives them back itself, since it may open the next prompt.
    Prompt* pushed { prompt.get() };
    prompt->Connect_OnDismiss([this]() { giveKeysToList(); }, false);

    if(!window->PushPopup(std::move(prompt)))
        return nullptr;

    pushed->TakeKeys();
    return pushed;
}

void RetroFuturaGUI::FileExplorer::giveKeysToList()
{
    if(!_widgets._Files)
        return;

    Window* window { GetWindow() };

    if(!window)
        return;

    window->TakeKeyboardInput(); //the prompt's window had them
    _widgets._Files->TakeKeyboardFocus();
}

void RetroFuturaGUI::FileExplorer::askNewFolderName()
{
    if(!canDo(MenuAction::NewFolder))
        return;

    const std::filesystem::path suggestion { PlatformBridge::FileSystem::FreeName(_currentDirectory / "New folder", true) };
    std::unique_ptr<NamePrompt> prompt { std::make_unique<NamePrompt>("New folder", "The new folder's name:", toUtf8(suggestion.filename()), _font) };

    if(!prompt)
        return;

    prompt->Connect_OnResult([this](std::string name) { giveKeysToList(); createFolder(name); }, false);
    pushPrompt(std::move(prompt));
}

void RetroFuturaGUI::FileExplorer::createFolder(std::string_view name)
{
    if(!isValidName(name))
        return;

    const std::filesystem::path target { _currentDirectory / fromUtf8(name) };
    std::error_code error {};

    if(std::filesystem::exists(target, error))
    {
        setStatus(std::format("{} already exists", name));
        return;
    }

    if(!std::filesystem::create_directory(target, error))
    {
        setStatus(std::format("Can't make {}: {}", name, describeError(error)));
        return;
    }

    recordUndoStep(FileUndoHistory::Kind::NewFolder, { FileUndoHistory::Item { ._From = {}, ._To = target } });
    requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = { target }, ._AddsToHistory = false, ._KeepsView = true });
}

void RetroFuturaGUI::FileExplorer::startRename()
{
    if(!canDo(MenuAction::Rename))
        return;

    if(!_widgets._Files)
        return;

    const std::vector<uSize> rows { _widgets._Files->GetSelectedRows() };

    if(1 != rows.size() || _shownEntries.size() <= rows.front())
        return;

    const uSize row { rows.front() };
    const Entry& entry { _shownEntries[row] };

    //With extensions shown, only the name before the extension is selected, so typing keeps the extension
    const bool hasShownExtension { _showExtensions && PlatformBridge::FileSystem::EntryKind::Directory != entry._Kind && entry._Path.has_extension() };
    const uSize selectedLength { hasShownExtension ? codePointCount(toUtf8(entry._Path.stem())) : std::numeric_limits<uSize>::max() };

    _renamingPath = entry._Path;
    _widgets._Files->EditCell(TrackIndex { ._Row = row, ._Column = getNameColumn(), ._Layer = 0 }, selectedLength);
}

void RetroFuturaGUI::FileExplorer::finishRename(std::string_view text)
{
    if(!_renamingPath)
        return;

    const std::filesystem::path from { *_renamingPath };
    _renamingPath.reset();

    //A rename that doesn't happen writes the names again, which puts the old one back in the cell and leaves the status alone
    if(!isValidName(text))
    {
        updateNameColumn();
        return;
    }

    //With extensions hidden the list shows the name without its extension, so the edit left the extension alone
    std::error_code error {};
    const bool isDirectory { std::filesystem::is_directory(from, error) };
    const std::string name { !_showExtensions && !isDirectory ? std::string(text) + toUtf8(from.extension()) : std::string(text) };
    const std::filesystem::path to { from.parent_path() / fromUtf8(name) };

    if(to == from)
        return;

    //A change of case alone names the same file on Windows, so that one is no clash
    if(std::filesystem::exists(to, error) && !std::filesystem::equivalent(from, to, error))
    {
        setStatus(std::format("{} already exists", name));
        updateNameColumn();
        return;
    }

    std::filesystem::rename(from, to, error);

    if(error)
    {
        setStatus(std::format("Can't rename {}: {}", toUtf8(from.filename()), describeError(error)));
        updateNameColumn();
        return;
    }

    recordUndoStep(FileUndoHistory::Kind::Rename, { FileUndoHistory::Item { ._From = from, ._To = to } });
    requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = { to }, ._AddsToHistory = false, ._KeepsView = true });
}

bool RetroFuturaGUI::FileExplorer::isValidName(std::string_view name)
{
    if(name.empty())
    {
        setStatus("A name can't be empty");
        return false;
    }

    if(std::string_view::npos != name.find_first_of("\\/"))
    {
        setStatus("A name can't hold \\ or /");
        return false;
    }

    if("." == name || ".." == name)
    {
        setStatus("A name can't be . or ..");
        return false;
    }

    return true;
}

uSize RetroFuturaGUI::FileExplorer::codePointCount(std::string_view text)
{
    //Every byte but UTF-8's continuation bytes starts a code point
    return static_cast<uSize>(std::count_if(text.begin(), text.end(), [](const char byte) { return 0x80 != (static_cast<unsigned char>(byte) & 0xC0); }));
}

bool RetroFuturaGUI::FileExplorer::canDo(const MenuAction action) const
{
    return isMenuActionOffered(action) && isMenuActionAvailable(action);
}

void RetroFuturaGUI::FileExplorer::on_renameButton_click()
{
    startRename();
}

void RetroFuturaGUI::FileExplorer::on_filesTable_rowSlowDoubleClicked(const uSize)
{
    startRename(); //the table reports it only on the row that is the one selected
}

void RetroFuturaGUI::FileExplorer::on_filesTable_cellEditEnded(const TrackIndex&, const Table::EditEnd end, const std::string& text)
{
    if(!_renamingPath)
        return;

    if(Table::EditEnd::Cancelled == end)
    {
        _renamingPath.reset();
        return;
    }

    finishRename(text);
}

void RetroFuturaGUI::FileExplorer::deleteSelected()
{
    if(!canDo(MenuAction::Delete))
        return;

    startTransfer(FileTransfer::Request { ._Operation = FileTransfer::Operation::MoveToTrash, ._Sources = getSelectedPaths(), ._Targets = {}, ._Destination = {}, ._ClashRule = FileTransfer::ClashRule::KeepBoth });
}

void RetroFuturaGUI::FileExplorer::askDeletePermanently(std::vector<std::filesystem::path>&& paths)
{
    if(paths.empty())
        return;

    if(HasFlag(_flags, FileExplorerFlags::SkipDeleteConfirmation))
    {
        startTransfer(FileTransfer::Request { ._Operation = FileTransfer::Operation::DeletePermanently, ._Sources = std::move(paths), ._Targets = {}, ._Destination = {}, ._ClashRule = FileTransfer::ClashRule::KeepBoth });
        return;
    }

    const std::string message { std::format("Delete {} permanently?", itemsText(paths)) };
    std::unique_ptr<ConfirmPrompt> prompt { std::make_unique<ConfirmPrompt>("Delete permanently", message, "This can't be undone.", "Delete", _font) };

    if(!prompt)
        return;

    prompt->Connect_OnResult([this, paths = std::move(paths)]() mutable
        {
            giveKeysToList();
            startTransfer(FileTransfer::Request { ._Operation = FileTransfer::Operation::DeletePermanently, ._Sources = std::move(paths), ._Targets = {}, ._Destination = {}, ._ClashRule = FileTransfer::ClashRule::KeepBoth });
        }, false);
    pushPrompt(std::move(prompt));
}

void RetroFuturaGUI::FileExplorer::putSelectedOnClipboard(const PlatformBridge::Clipboard::FileOperation operation)
{
    if(!canDo(PlatformBridge::Clipboard::FileOperation::Cut == operation ? MenuAction::Cut : MenuAction::Copy))
        return;

    const std::vector<std::filesystem::path> paths { getSelectedPaths() };

    if(PlatformBridge::Clipboard::ClipboardErrorCode::Success != PlatformBridge::Clipboard::CopyFilesToClipboard(paths, operation))
    {
        setStatus("Can't use the clipboard right now");
        return;
    }

    setStatus(std::format("Ready to paste: {}", itemsText(paths)));
    followClipboard(); //Paste can be done now
}

void RetroFuturaGUI::FileExplorer::pasteFromClipboard()
{
    if(!canDo(MenuAction::Paste))
        return;

    std::vector<std::filesystem::path> paths {};
    PlatformBridge::Clipboard::FileOperation operation { PlatformBridge::Clipboard::FileOperation::Copy };

    if(PlatformBridge::Clipboard::ClipboardErrorCode::Success != PlatformBridge::Clipboard::PasteFilesFromClipboard(paths, operation))
    {
        setStatus("The clipboard holds no files");
        return;
    }

    //A folder pasted into itself would copy itself without end
    for(const std::filesystem::path& path : paths)
    {
        if(!FileTransfer::IsInside(_currentDirectory, path))
            continue;

        setStatus(std::format("Can't paste {} into itself", toUtf8(path.filename())));
        return;
    }

    const bool isCut { PlatformBridge::Clipboard::FileOperation::Cut == operation };
    _pastedCut = isCut ? paths : std::vector<std::filesystem::path> {};
    startTransfer(FileTransfer::Request
    {
        ._Operation = isCut ? FileTransfer::Operation::Move : FileTransfer::Operation::Copy,
        ._Sources = std::move(paths),
        ._Targets = {},
        ._Destination = _currentDirectory,
        ._ClashRule = HasFlag(_flags, FileExplorerFlags::AskOnNameClash) ? FileTransfer::ClashRule::Ask : FileTransfer::ClashRule::KeepBoth
    });
}

void RetroFuturaGUI::FileExplorer::followClipboard()
{
    if(!isMenuActionOffered(MenuAction::Paste))
        return;

    const bool hasFiles { PlatformBridge::Clipboard::HasFilesOnClipboard() };

    if(hasFiles == _hadFilesOnClipboard)
        return;

    _hadFilesOnClipboard = hasFiles;
    updateMenus();
}

void RetroFuturaGUI::FileExplorer::on_filesTable_selectionDrag(const Table::SelectionDragPhase phase, const glm::vec2& point)
{
    if(Table::SelectionDragPhase::Started == phase)
    {
        endDrag();
        _draggedPaths = getSelectedPaths();
        _isDraggingRows = !_draggedPaths.empty();
        return;
    }

    if(Table::SelectionDragPhase::Cancelled == phase)
    {
        endDrag();
        setStatus("");
        return;
    }

    if(!_isDraggingRows)
        return;

    //Once the system has the drag, or is about to take it, the table's press only winds down
    if(_hasDraggedOut || _isDragOutPending)
    {
        if(Table::SelectionDragPhase::Released == phase)
            endDrag();

        return;
    }

    const bool
        isShiftHeld { PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_R) },
        isControlHeld { PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R) };
    std::optional<uSize> row {};
    Button* place { nullptr };
    const std::filesystem::path folder { dropFolderAt(point, _draggedPaths, false, row, place) };

    if(Table::SelectionDragPhase::Released == phase)
    {
        std::vector<std::filesystem::path> items { std::move(_draggedPaths) };
        endDrag();
        setStatus("");

        if(folder.empty())
            return;

        const bool isMove { isDropMove(items, folder, isShiftHeld, isControlHeld) };
        dropInto(std::move(items), folder, isMove);
        return;
    }

    //Past the window's edge the system takes the drag over, so the files can land in other programs
    const glm::vec2 size { getWindowSize() };
    const bool isOutside { 0.0f > point.x || 0.0f > point.y || size.x < point.x || size.y < point.y };

    if(isOutside && PlatformBridge::DragDrop::CanDragFilesOut())
    {
        showDropTarget(std::nullopt, nullptr);
        _isDragOutPending = true;
        return;
    }

    showDropTarget(row, place);

    if(folder.empty())
    {
        if(!_statusMessage.empty())
            setStatus("");

        return;
    }

    //Set only when it changes, since the pointer moves every frame
    const std::string
        folderName { toUtf8(folder.has_filename() ? folder.filename() : folder) },
        dropText { std::format("{} to {}", isDropMove(_draggedPaths, folder, isShiftHeld, isControlHeld) ? "Move" : "Copy", folderName) };

    if(dropText != _statusMessage)
        setStatus(dropText);
}

void RetroFuturaGUI::FileExplorer::on_window_filesDropped(const std::vector<std::string>& texts, const glm::vec2& point)
{
    std::vector<std::filesystem::path> items {};
    items.reserve(texts.size());

    for(const std::string& text : texts)
        items.push_back(fromUtf8(text));

    if(items.empty())
        return;

    //Rows dragged out and let go over the explorer again drop as they would have inside: into a folder row or a place
    std::vector<std::filesystem::path>
        sortedItems { items },
        sortedDraggedOut { std::move(_draggedOutPaths) };
    std::sort(sortedItems.begin(), sortedItems.end());
    std::sort(sortedDraggedOut.begin(), sortedDraggedOut.end());
    _draggedOutPaths.clear();
    const bool isOwnDrag { sortedItems == sortedDraggedOut && _ownDropWindow > std::chrono::steady_clock::now() - _draggedOutAt };

    std::optional<uSize> row {};
    Button* place { nullptr };
    const std::filesystem::path folder { dropFolderAt(point, isOwnDrag ? items : std::vector<std::filesystem::path> {}, !isOwnDrag, row, place) };

    if(folder.empty())
        return;

    //The keys went to the program dragged from, so the system is asked which are held
    const bool
        isShiftHeld { PlatformBridge::Input::IsKeyPhysicallyDown(PB_KEY_SHIFT_L) || PlatformBridge::Input::IsKeyPhysicallyDown(PB_KEY_SHIFT_R) },
        isControlHeld { PlatformBridge::Input::IsKeyPhysicallyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyPhysicallyDown(PB_KEY_CONTROL_R) },
        isMove { isOwnDrag ? isDropMove(items, folder, isShiftHeld, isControlHeld) : isShiftHeld }; //from another program it copies unless Shift moves
    dropInto(std::move(items), folder, isMove);
}

std::filesystem::path RetroFuturaGUI::FileExplorer::dropFolderAt(const glm::vec2& point, const std::vector<std::filesystem::path>& dragged, const bool isOpenFolderTaken, std::optional<uSize>& rowOut, Button*& placeOut) const
{
    rowOut.reset();
    placeOut = nullptr;
    const auto isDragged { [&dragged](const std::filesystem::path& folder) { return dragged.end() != std::find(dragged.begin(), dragged.end(), folder); } };

    //A folder row under the pointer
    uSize row { 0 };
    const bool isOverRow { _widgets._Files && _widgets._Files->GetRowAt(point, row) && row < _shownEntries.size() };

    if(isOverRow && PlatformBridge::FileSystem::EntryKind::Directory == _shownEntries[row]._Kind && !isDragged(_shownEntries[row]._Path))
    {
        rowOut = row;
        return _shownEntries[row]._Path;
    }

    //A place under the pointer, unless it is where the dragged rows are already or one of them
    for(const auto& [button, place] : _placeButtons)
    {
        if(!button)
            continue;

        const glm::vec3
            position { button->GetPosition() },
            size { button->GetSize() };

        if(size.x * 0.5f < std::abs(point.x - position.x) || size.y * 0.5f < std::abs(point.y - position.y))
            continue;

        std::error_code error {};

        if(isDragged(place) || (!dragged.empty() && std::filesystem::equivalent(place, _currentDirectory, error)))
            return {};

        placeOut = button;
        return place;
    }

    return isOpenFolderTaken ? _currentDirectory : std::filesystem::path {};
}

void RetroFuturaGUI::FileExplorer::showDropTarget(const std::optional<uSize> row, Button* place)
{
    if(!_widgets._Files)
        return;

    if(row)
        _widgets._Files->SetDropTargetRow(*row);
    else
        _widgets._Files->ClearDropTargetRow();

    if(place == _dropTargetPlace)
        return;

    markPlace(_dropTargetPlace, false);
    _dropTargetPlace = place;
    markPlace(_dropTargetPlace, true);
}

glm::vec2 RetroFuturaGUI::FileExplorer::getWindowSize() const
{
    const glm::vec2 openedSize { static_cast<f32>(_width), static_cast<f32>(_height) };
    Window* window { GetWindow() };

    if(!window)
        return openedSize;

    Projection* projection { window->GetProjection() };

    if(!projection)
        return openedSize;

    return projection->GetResolution();
}

void RetroFuturaGUI::FileExplorer::markPlace(Button* place, const bool isMarked)
{
    if(!place)
        return;

    styleToggle(*place, isMarked);
}

bool RetroFuturaGUI::FileExplorer::isDropMove(const std::vector<std::filesystem::path>& items, const std::filesystem::path& folder, const bool isShiftHeld, const bool isControlHeld)
{
    if(isShiftHeld)
        return true;

    if(isControlHeld)
        return false;

    if(items.empty())
        return false;

    return PlatformBridge::FileSystem::IsSameVolume(items.front(), folder);
}

void RetroFuturaGUI::FileExplorer::dragOut()
{
    _isDragOutPending = false;

    if(!_isDraggingRows)
        return;

    //The system's drag loop runs until the drop; the program dropped on copies or moves the files itself, and the watcher
    //shows what left the open folder
    _hasDraggedOut = true;
    setStatus("");
    _draggedOutPaths = _draggedPaths;
    PlatformBridge::DragDrop::DragFilesOut(_draggedPaths, true);
    _draggedOutAt = std::chrono::steady_clock::now(); //a drop on the explorer itself arrives after this
}

void RetroFuturaGUI::FileExplorer::dropInto(std::vector<std::filesystem::path>&& items, const std::filesystem::path& folder, const bool isMove)
{
    //A folder put into itself would copy itself without end
    for(const std::filesystem::path& item : items)
    {
        if(!FileTransfer::IsInside(folder, item))
            continue;

        setStatus(std::format("Can't put {} inside itself", toUtf8(item.filename())));
        return;
    }

    startTransfer(FileTransfer::Request
    {
        ._Operation = isMove ? FileTransfer::Operation::Move : FileTransfer::Operation::Copy,
        ._Sources = std::move(items),
        ._Targets = {},
        ._Destination = folder,
        ._ClashRule = HasFlag(_flags, FileExplorerFlags::AskOnNameClash) ? FileTransfer::ClashRule::Ask : FileTransfer::ClashRule::KeepBoth
    });
}

void RetroFuturaGUI::FileExplorer::endDrag()
{
    showDropTarget(std::nullopt, nullptr);
    _draggedPaths.clear();
    _isDraggingRows = false;
    _isDragOutPending = false;
    _hasDraggedOut = false;
}

void RetroFuturaGUI::FileExplorer::undoLast()
{
    if(!canDo(MenuAction::Undo))
        return;

    std::optional<FileUndoHistory::Step> step { FileUndoHistory::TakeLast() };

    if(!step || step->_Items.empty())
        return;

    const FileUndoHistory::Item& first { step->_Items.front() };
    std::error_code error {};

    switch(step->_Kind)
    {
        case FileUndoHistory::Kind::NewFolder:
        {
            //Only while it is empty: what was put in it since stays
            const std::string name { toUtf8(first._To.filename()) };

            if(!std::filesystem::is_empty(first._To, error) || error)
            {
                setStatus(error ? std::format("Can't undo making {}: {}", name, describeError(error)) : std::format("Can't undo making {}: it isn't empty", name));
                return;
            }

            std::filesystem::remove(first._To, error);
            setStatus(error ? std::format("Can't undo making {}: {}", name, describeError(error)) : std::format("Removed the new folder {}", name));
            requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = rowAfterRemoved({ first._To }), ._AddsToHistory = false, ._KeepsView = true });
        }
        break;
        case FileUndoHistory::Kind::Rename:
        {
            const std::string name { toUtf8(first._From.filename()) };

            //A change of case alone names the same file, so it may be undone
            if(std::filesystem::exists(std::filesystem::symlink_status(first._From, error)) && !std::filesystem::equivalent(first._From, first._To, error))
            {
                setStatus(std::format("Can't undo the rename: {} is taken", name));
                return;
            }

            std::filesystem::rename(first._To, first._From, error);
            setStatus(error ? std::format("Can't undo the rename: {}", describeError(error)) : std::format("Renamed {} back to {}", toUtf8(first._To.filename()), name));
            requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = { first._From }, ._AddsToHistory = false, ._KeepsView = true });
        }
        break;
        default:
        {
            //The rest are transfers: the copies go to the bin, moved items go back, recycled ones come out of it
            std::vector<std::filesystem::path>
                sources {},
                targets {};
            FileTransfer::Operation operation { FileTransfer::Operation::MoveToTrash };

            for(const FileUndoHistory::Item& item : step->_Items)
            {
                if(FileUndoHistory::Kind::Copy == step->_Kind)
                {
                    sources.push_back(item._To);
                }
                else if(FileUndoHistory::Kind::Move == step->_Kind)
                {
                    sources.push_back(item._To);
                    targets.push_back(item._From);
                    operation = FileTransfer::Operation::Move;
                }
                else
                {
                    sources.push_back(item._From);
                    operation = FileTransfer::Operation::RestoreFromTrash;
                }
            }

            _isTransferUndoing = true;
            startTransfer(FileTransfer::Request { ._Operation = operation, ._Sources = std::move(sources), ._Targets = std::move(targets), ._Destination = {}, ._ClashRule = FileTransfer::ClashRule::KeepBoth });
            _isTransferUndoing = _transfer.IsBusy(); //it didn't start
        }
        break;
    }

    updateMenus();
}

void RetroFuturaGUI::FileExplorer::recordUndoStep(const FileUndoHistory::Kind kind, std::vector<FileUndoHistory::Item>&& items)
{
    if(!HasFlag(_flags, FileExplorerFlags::Undo))
        return;

    FileUndoHistory::Record(FileUndoHistory::Step { ._Kind = kind, ._Items = std::move(items) });
    updateMenus();
}

void RetroFuturaGUI::FileExplorer::startTransfer(FileTransfer::Request&& request)
{
    if(request._Sources.empty())
        return;

    //A delete's rows may be listed away before it ends, so the row taking their place is picked now
    const bool isDelete { FileTransfer::Operation::MoveToTrash == request._Operation || FileTransfer::Operation::DeletePermanently == request._Operation };
    std::vector<std::filesystem::path> selectAfter { isDelete ? rowAfterRemoved(request._Sources) : std::vector<std::filesystem::path> {} };

    if(!_transfer.Start(std::move(request)))
    {
        setStatus("Another file operation is still running");
        return;
    }

    _selectAfterTransfer = std::move(selectAfter);
    showStopButton(true);
    updateMenus(); //nothing else starts until it ends
}

void RetroFuturaGUI::FileExplorer::followTransfer()
{
    if(!_transfer.IsBusy())
        return;

    if(std::optional<FileTransfer::Outcome> outcome { _transfer.TakeOutcome() })
    {
        showTransferOutcome(std::move(*outcome));
        return;
    }

    //Over every other message while it runs, a selection's included
    const std::string progress { progressText(_transfer.GetProgress()) };

    if(progress != _statusMessage)
        setStatus(progress);

    if(_isAskingAboutClash)
        return;

    if(const std::optional<FileTransfer::Clash> clash { _transfer.GetClash() })
        askAboutClash(*clash);
}

void RetroFuturaGUI::FileExplorer::showTransferOutcome(FileTransfer::Outcome&& outcome)
{
    showStopButton(false);

    //An undo's move puts things back where they were
    const bool isMovedBack { _isTransferUndoing && FileTransfer::Operation::Move == outcome._Operation && 0 == outcome._FailedCount && !outcome._IsStopped && !outcome._Done.empty() };
    setStatus(isMovedBack ? outcomeText(outcome) + " back" : outcomeText(outcome));

    //Moved, the cut files are no longer where the clipboard says; stopped, the rest can still be pasted
    if(FileTransfer::Operation::Move == outcome._Operation && !outcome._IsStopped && !_pastedCut.empty())
        PlatformBridge::Clipboard::ClearCutFiles(_pastedCut);

    _pastedCut.clear();

    //What was done goes into the history, even when stopped partway; an undo's own work doesn't
    std::optional<FileUndoHistory::Kind> undoKind {};

    if(FileTransfer::Operation::Copy == outcome._Operation)
        undoKind = FileUndoHistory::Kind::Copy;
    else if(FileTransfer::Operation::Move == outcome._Operation)
        undoKind = FileUndoHistory::Kind::Move;
    else if(FileTransfer::Operation::MoveToTrash == outcome._Operation)
        undoKind = FileUndoHistory::Kind::MoveToTrash;

    if(undoKind && !_isTransferUndoing)
    {
        std::vector<FileUndoHistory::Item> items {};
        items.reserve(outcome._Done.size());

        for(const FileTransfer::DoneItem& done : outcome._Done)
            items.push_back(FileUndoHistory::Item { ._From = done._From, ._To = done._To });

        recordUndoStep(*undoKind, std::move(items));
    }

    _isTransferUndoing = false;

    //A close confirmed while it ran comes first; nothing else is shown then
    if(_closeAfterTransfer)
    {
        std::function<void()> close { std::move(_closeAfterTransfer) };
        _closeAfterTransfer = {};
        close();
        return;
    }

    //What arrived in the open folder is selected once it shows; after a delete, the row that takes the deleted one's place
    std::vector<std::filesystem::path> select {};

    for(const FileTransfer::DoneItem& done : outcome._Done)
    {
        if(done._To.empty() || done._To.parent_path() != _currentDirectory)
            continue;

        select.push_back(done._To);
    }

    if(select.empty())
        select = std::move(_selectAfterTransfer);

    _selectAfterTransfer.clear();
    requestListing(_currentDirectory, ListingIntent { ._Fallbacks = {}, ._HistoryIndex = {}, ._WriteTarget = {}, ._Select = std::move(select), ._AddsToHistory = false, ._KeepsView = true });
    updateMenus();

    if(outcome._NotTrashed.empty())
        return;

    //The recycle bin can't take these, so they are deleted for good only once the user agrees
    const std::string message { std::format("{} can't go to the recycle bin.", itemsText(outcome._NotTrashed)) };
    const std::string_view detail { 1 == outcome._NotTrashed.size() ? "Delete it permanently? This can't be undone." : "Delete them permanently? This can't be undone." };
    std::unique_ptr<ConfirmPrompt> prompt { std::make_unique<ConfirmPrompt>("Delete permanently", message, detail, "Delete", _font) };

    if(!prompt)
        return;

    prompt->Connect_OnResult([this, paths = std::move(outcome._NotTrashed)]() mutable
        {
            giveKeysToList();
            startTransfer(FileTransfer::Request { ._Operation = FileTransfer::Operation::DeletePermanently, ._Sources = std::move(paths), ._Targets = {}, ._Destination = {}, ._ClashRule = FileTransfer::ClashRule::KeepBoth });
        }, false);
    pushPrompt(std::move(prompt));
}

void RetroFuturaGUI::FileExplorer::askAboutClash(const FileTransfer::Clash& clash)
{
    const std::filesystem::path folder { clash._Target.parent_path() };
    std::unique_ptr<ClashPrompt> prompt { std::make_unique<ClashPrompt>(toUtf8(clash._Target.filename()), toUtf8(folder.has_filename() ? folder.filename() : folder), _font) };

    if(!prompt)
        return;

    //Closing the prompt without an answer stops the operation
    prompt->Connect_OnResult([this](const ClashAnswer answer, const ClashScope scope)
        {
            _isAskingAboutClash = false;
            giveKeysToList();
            _transfer.AnswerClash(answer, scope);
        }, false);
    prompt->Connect_OnDismiss([this]()
        {
            _isAskingAboutClash = false;
            _transfer.Stop();
        }, false);

    if(!pushPrompt(std::move(prompt)))
        return;

    _isAskingAboutClash = true;
}

void RetroFuturaGUI::FileExplorer::showStopButton(const bool isShown)
{
    if(!_widgets._Stop)
        return;

    if(!_viewHost)
        return;

    Lasagna* lasagna { _viewHost->GetLasagna() };

    if(!lasagna)
        return;

    LasagnaCell* cell { lasagna->GetCell(TrackIndex { ._Row = 5, ._Column = _statusColumnCount, ._Layer = 0 }) };

    if(!cell)
        return;

    cell->_Show = isShown;
}

void RetroFuturaGUI::FileExplorer::closeAfterAsking(std::function<void()>&& close)
{
    if(!_transfer.IsBusy())
    {
        close();
        return;
    }

    if(_closeAfterTransfer)
        return; //asked and confirmed already; it closes once the operation has stopped

    const std::string message { progressText(_transfer.GetProgress()) };
    std::unique_ptr<ConfirmPrompt> prompt { std::make_unique<ConfirmPrompt>("File operation running", message, "Closing stops it after the current file.", "Stop and close", _font) };

    if(!prompt)
        return;

    prompt->Connect_OnResult([this, close = std::move(close)]() mutable
        {
            if(!_transfer.IsBusy())
            {
                close(); //it ended while the prompt was up
                return;
            }

            _closeAfterTransfer = std::move(close);
            _transfer.Stop();
        }, false);
    pushPrompt(std::move(prompt));
}

bool RetroFuturaGUI::FileExplorer::allowsSystemClose()
{
    if(!_transfer.IsBusy())
        return true;

    closeAfterAsking([this]() { Dismiss(); });
    return false;
}

std::string RetroFuturaGUI::FileExplorer::progressText(const FileTransfer::Progress& progress)
{
    const std::string
        name { toUtf8(progress._Current.filename()) },
        count { 1 < progress._TotalCount ? std::format(" ({} of {})", progress._DoneCount + 1, progress._TotalCount) : std::string {} };

    switch(progress._Operation)
    {
        case FileTransfer::Operation::Copy:
            return std::format("Copying {}{}", name, count);
        case FileTransfer::Operation::Move:
            return std::format("Moving {}{}", name, count);
        case FileTransfer::Operation::MoveToTrash:
            return std::format("Moving {} to the recycle bin{}", name, count);
        case FileTransfer::Operation::RestoreFromTrash:
            return std::format("Restoring {}{}", name, count);
        default:
            return std::format("Deleting {}{}", name, count);
    }
}

std::string RetroFuturaGUI::FileExplorer::outcomeText(const FileTransfer::Outcome& outcome)
{
    if(0 < outcome._FailedCount)
    {
        const std::string_view verb
        {
            FileTransfer::Operation::Copy == outcome._Operation ? "copy" :
            FileTransfer::Operation::Move == outcome._Operation ? "move" :
            FileTransfer::Operation::MoveToTrash == outcome._Operation ? "move to the recycle bin" :
            FileTransfer::Operation::RestoreFromTrash == outcome._Operation ? "restore" : "delete"
        };
        const std::string
            reason { std::errc::invalid_argument == outcome._Error ? std::string("a folder can't go inside itself") : describeError(outcome._Error) },
            others { 1 < outcome._FailedCount ? std::format(", and {} more", outcome._FailedCount - 1) : std::string {} };
        return std::format("Can't {} {}: {}{}", verb, toUtf8(outcome._FailedPath.filename()), reason, others);
    }

    if(outcome._IsStopped)
        return std::format("Stopped after {} of {}", outcome._Done.size(), outcome._TotalCount);

    if(outcome._Done.empty())
        return outcome._Skipped.empty() ? std::string {} : std::format("Skipped {}", itemsText(outcome._Skipped));

    std::vector<std::filesystem::path> done {};
    done.reserve(outcome._Done.size());

    for(const FileTransfer::DoneItem& item : outcome._Done)
        done.push_back(item._From);

    const std::string skipped { outcome._Skipped.empty() ? std::string {} : std::format(", skipped {}", itemsText(outcome._Skipped)) };

    switch(outcome._Operation)
    {
        case FileTransfer::Operation::Copy:
            return std::format("Copied {}{}", itemsText(done), skipped);
        case FileTransfer::Operation::Move:
            return std::format("Moved {}{}", itemsText(done), skipped);
        case FileTransfer::Operation::MoveToTrash:
            return std::format("Moved {} to the recycle bin", itemsText(done));
        case FileTransfer::Operation::RestoreFromTrash:
            return std::format("Restored {}", itemsText(done));
        default:
            return std::format("Deleted {} permanently", itemsText(done));
    }
}

std::vector<std::filesystem::path> RetroFuturaGUI::FileExplorer::rowAfterRemoved(const std::vector<std::filesystem::path>& removed) const
{
    std::vector<std::filesystem::path::string_type> gone {};
    gone.reserve(removed.size());

    for(const std::filesystem::path& path : removed)
        gone.push_back(path.native());

    std::sort(gone.begin(), gone.end());
    const auto isGone { [&gone](const Entry& entry) { return std::binary_search(gone.begin(), gone.end(), entry._Path.native()); } };
    const auto firstGone { std::find_if(_shownEntries.begin(), _shownEntries.end(), isGone) };

    if(_shownEntries.end() == firstGone)
        return {};

    //The first row after it that is still there, or else the last one before it
    const auto after { std::find_if_not(firstGone, _shownEntries.end(), isGone) };

    if(_shownEntries.end() != after)
        return { after->_Path };

    const auto before { std::find_if_not(std::make_reverse_iterator(firstGone), _shownEntries.rend(), isGone) };

    if(_shownEntries.rend() != before)
        return { before->_Path };

    return {};
}

std::string RetroFuturaGUI::FileExplorer::itemsText(const std::vector<std::filesystem::path>& paths)
{
    if(1 == paths.size())
        return toUtf8(paths.front().filename());

    return std::format("{} items", paths.size());
}

std::vector<RetroFuturaGUI::FileExplorer::MenuAction> RetroFuturaGUI::FileExplorer::menuActionsOf(std::span<const MenuAction> choices) const
{
    std::vector<MenuAction> actions {};

    for(const MenuAction action : choices)
    {
        if(!isMenuActionOffered(action))
            continue;

        actions.push_back(action);
    }

    return actions;
}

bool RetroFuturaGUI::FileExplorer::isMenuActionOffered(const MenuAction action) const
{
    switch(action)
    {
        case MenuAction::NewFolder:
            return HasFlag(_flags, FileExplorerFlags::NewFolder);
        case MenuAction::Rename:
            return HasFlag(_flags, FileExplorerFlags::Rename);
        case MenuAction::Delete:
            return HasFlag(_flags, FileExplorerFlags::Delete);
        case MenuAction::DeletePermanently:
            return HasFlag(_flags, FileExplorerFlags::DeletePermanently);
        case MenuAction::Undo:
            return HasFlag(_flags, FileExplorerFlags::Undo);
        case MenuAction::Cut:
        case MenuAction::Copy:
        case MenuAction::Paste:
            return HasFlag(_flags, FileExplorerFlags::CopyAndMove);
        case MenuAction::SelectAll:
            return Mode::MultiFile == _mode;
        default: //Close
            return true;
    }
}

bool RetroFuturaGUI::FileExplorer::isMenuActionAvailable(const MenuAction action) const
{
    switch(action)
    {
        case MenuAction::Close:
            return true;
        case MenuAction::NewFolder:
            return !_currentDirectory.empty();
        case MenuAction::Rename:
            return _widgets._Files && 1 == _widgets._Files->GetSelectedRows().size();
        case MenuAction::Delete:
        case MenuAction::DeletePermanently:
            return _widgets._Files && !_widgets._Files->GetSelectedRows().empty() && !_transfer.IsBusy(); //one operation at a time
        case MenuAction::Cut:
        case MenuAction::Copy:
            return _widgets._Files && !_widgets._Files->GetSelectedRows().empty();
        case MenuAction::Paste:
            return _hadFilesOnClipboard && !_currentDirectory.empty() && !_transfer.IsBusy();
        case MenuAction::Undo:
            return FileUndoHistory::HasSteps() && !_transfer.IsBusy();
        case MenuAction::SelectAll:
            return !_shownEntries.empty();
        default:
            return false; //the file operations aren't built yet
    }
}

void RetroFuturaGUI::FileExplorer::updateMenus()
{
    for(const auto& [menu, actions] : { std::pair { _widgets._FileMenu, &_fileMenuActions }, std::pair { _widgets._EditMenu, &_editMenuActions } })
    {
        if(!menu)
            continue;

        for(uSize index = 0; index < actions->size(); ++index)
            menu->SetItemEnabled(index, isMenuActionAvailable((*actions)[index]));
    }

    if(_widgets._Rename)
        _widgets._Rename->SetEnabled(isMenuActionAvailable(MenuAction::Rename), false);
}

void RetroFuturaGUI::FileExplorer::runMenuAction(const MenuAction action)
{
    if(!isMenuActionAvailable(action))
        return;

    switch(action)
    {
        case MenuAction::Close:
            on_cancelButton_click();
        break;
        case MenuAction::NewFolder:
            askNewFolderName();
        break;
        case MenuAction::Rename:
            startRename();
        break;
        case MenuAction::Delete:
            deleteSelected();
        break;
        case MenuAction::DeletePermanently:
            askDeletePermanently(getSelectedPaths());
        break;
        case MenuAction::Cut:
            putSelectedOnClipboard(PlatformBridge::Clipboard::FileOperation::Cut);
        break;
        case MenuAction::Copy:
            putSelectedOnClipboard(PlatformBridge::Clipboard::FileOperation::Copy);
        break;
        case MenuAction::Paste:
            pasteFromClipboard();
        break;
        case MenuAction::Undo:
            undoLast();
        break;
        case MenuAction::SelectAll:
            selectAllRows();
        break;
        default:
        break;
    }
}

void RetroFuturaGUI::FileExplorer::selectAllRows()
{
    if(!_widgets._Files)
        return;

    std::vector<uSize> rows(_shownEntries.size());
    std::iota(rows.begin(), rows.end(), uSize { 0 });
    _widgets._Files->SelectRows(rows, true);
}

std::string_view RetroFuturaGUI::FileExplorer::menuText(const MenuAction action)
{
    switch(action)
    {
        case MenuAction::NewFolder:
            return "New folder";
        case MenuAction::Rename:
            return "Rename";
        case MenuAction::Delete:
            return "Delete";
        case MenuAction::DeletePermanently:
            return "Delete permanently";
        case MenuAction::Undo:
            return "Undo";
        case MenuAction::Cut:
            return "Cut";
        case MenuAction::Copy:
            return "Copy";
        case MenuAction::Paste:
            return "Paste";
        case MenuAction::SelectAll:
            return "Select all";
        default:
            return "Close";
    }
}

void RetroFuturaGUI::FileExplorer::on_fileMenu_itemClicked()
{
    if(!_widgets._FileMenu)
        return;

    const uSize index { _widgets._FileMenu->GetSelectedIndex() };

    if(_fileMenuActions.size() <= index)
        return;

    runMenuAction(_fileMenuActions[index]);
}

void RetroFuturaGUI::FileExplorer::on_editMenu_itemClicked()
{
    if(!_widgets._EditMenu)
        return;

    const uSize index { _widgets._EditMenu->GetSelectedIndex() };

    if(_editMenuActions.size() <= index)
        return;

    runMenuAction(_editMenuActions[index]);
}

void RetroFuturaGUI::FileExplorer::handleKeys()
{
    if(!_widgets._Files)
        return;

    //While a name is edited in the list the keys are the edit's; the Enter or Escape that ends it is spent with them
    uSize
        editRow { 0 },
        editColumn { 0 };

    if(!_widgets._Files->HasKeyboardFocus() || _widgets._Files->GetEditedCell(editRow, editColumn))
    {
        _hadListFocus = false;
        return;
    }

    if(!_hadListFocus)
    {
        syncKeys(); //keys typed while a box had the keyboard are spent
        _hadListFocus = true;
        return;
    }

    //Every key is taken each frame, so none fires late
    const bool
        isEnterPressed { wasPressed(_enterKey) },
        isBackspacePressed { wasPressed(_backspaceKey) },
        isEscapePressed { wasPressed(_escapeKey) },
        isUpPressed { wasPressed(_upKey) },
        isLeftPressed { wasPressed(_leftKey) },
        isRightPressed { wasPressed(_rightKey) },
        isRefreshPressed { wasPressed(_refreshKey) },
        isRenamePressed { wasPressed(_renameKey) },
        isNewFolderPressed { wasPressed(_newFolderKey) },
        isDeletePressed { wasPressed(_deleteKey) },
        isCopyPressed { wasPressed(_copyKey) },
        isCutPressed { wasPressed(_cutKey) },
        isPastePressed { wasPressed(_pasteKey) },
        isUndoPressed { wasPressed(_undoKey) },
        isControlHeld { PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_CONTROL_R) },
        isShiftHeld { PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_L) || PlatformBridge::Input::IsKeyDown(PB_KEY_SHIFT_R) };

    if(isRefreshPressed)
        relist();
    else if(isRenamePressed)
        startRename();
    else if(isControlHeld && isShiftHeld && isNewFolderPressed)
        askNewFolderName();
    else if(isDeletePressed && isShiftHeld && canDo(MenuAction::DeletePermanently))
        askDeletePermanently(getSelectedPaths());
    else if(isDeletePressed && !isShiftHeld)
        deleteSelected();
    else if(isControlHeld && isCopyPressed)
        putSelectedOnClipboard(PlatformBridge::Clipboard::FileOperation::Copy);
    else if(isControlHeld && isCutPressed)
        putSelectedOnClipboard(PlatformBridge::Clipboard::FileOperation::Cut);
    else if(isControlHeld && isPastePressed)
        pasteFromClipboard();
    else if(isControlHeld && isUndoPressed)
        undoLast();
    else if(isBackspacePressed || (isControlHeld && isUpPressed))
        on_upButton_click();
    else if(isControlHeld && isLeftPressed)
        on_backButton_click();
    else if(isControlHeld && isRightPressed)
        on_forwardButton_click();
    else if(isEscapePressed && !PlatformBridge::Input::IsMouseButtonDown(PlatformBridge::MouseButton::Left)) //held, Escape stops the list's band or drag instead
        on_cancelButton_click();
    else if(isEnterPressed)
        openFromKeyboard();
}

void RetroFuturaGUI::FileExplorer::openFromKeyboard()
{
    if(!_widgets._Files)
        return;

    const std::vector<uSize> rows { _widgets._Files->GetSelectedRows() };

    if(1 == rows.size() && _shownEntries.size() > rows.front() && PlatformBridge::FileSystem::EntryKind::Directory == _shownEntries[rows.front()]._Kind)
    {
        navigateTo(_shownEntries[rows.front()]._Path, true); //a copy is made before the entries are replaced
        return;
    }

    on_okButton_click();
}

bool RetroFuturaGUI::FileExplorer::wasPressed(KeyPress& key)
{
    const u32 pressCount { PlatformBridge::Input::GetKeyPressCount(key._Key) };

    if(pressCount == key._PressCountSeen)
        return false;

    key._PressCountSeen = pressCount;
    return true;
}

void RetroFuturaGUI::FileExplorer::syncKeys()
{
    for(KeyPress* key : { &_enterKey, &_backspaceKey, &_escapeKey, &_upKey, &_leftKey, &_rightKey, &_refreshKey, &_renameKey, &_newFolderKey, &_deleteKey, &_copyKey, &_cutKey, &_pasteKey, &_undoKey })
        key->_PressCountSeen = PlatformBridge::Input::GetKeyPressCount(key->_Key);
}

void RetroFuturaGUI::FileExplorer::on_scrollSlider_valueChanged()
{
    if(!_widgets._Scroll)
        return;

    if(!_widgets._Files)
        return;

    _widgets._Files->SetVerticalScrollPosition(_widgets._Scroll->GetValue<f32>());
}

void RetroFuturaGUI::FileExplorer::on_filterComboBox_indexChanged()
{
    disarmReplace();
    showEntries();
}

void RetroFuturaGUI::FileExplorer::on_hiddenButton_click()
{
    if(!_widgets._Hidden)
        return;

    _showHidden = !_showHidden;
    styleToggle(*_widgets._Hidden, _showHidden);
    showEntries();
}

void RetroFuturaGUI::FileExplorer::on_extensionsButton_click()
{
    if(!_widgets._Extensions)
        return;

    _showExtensions = !_showExtensions;
    styleToggle(*_widgets._Extensions, _showExtensions);
    updateNameColumn(); //only the names change, so the selection and the scroll position stay
}

void RetroFuturaGUI::FileExplorer::on_thumbnailsButton_click()
{
    if(!_widgets._Thumbnails)
        return;

    if(!_widgets._Files)
        return;

    //A rename being typed sits in a column that moves
    if(_renamingPath)
        _widgets._Files->EndEdit();

    //The rows change height, so the list keeps the selection in view, or else the row that was at the top
    const std::vector<std::filesystem::path> selected { getSelectedPaths() };
    uSize
        firstRow { 0 },
        endRow { 0 };
    _widgets._Files->GetDisplayedRows(firstRow, endRow);

    _showThumbnails = !_showThumbnails;
    styleToggle(*_widgets._Thumbnails, _showThumbnails);
    fillTable();
    _widgets._Files->SetVerticalScrollPosition(static_cast<f32>(firstRow) * getListRowHeight());
    syncScrollSlider();
    selectPaths(selected, false); //the same rows, so a slot needn't hear of them again
    showThumbnails();
}

void RetroFuturaGUI::FileExplorer::on_nameTextBox_textChanged()
{
    disarmReplace();
}

void RetroFuturaGUI::FileExplorer::on_okButton_click()
{
    switch(_mode)
    {
        case Mode::WriteFile:
            confirmWrite(getNameBoxTarget());
            return;
        case Mode::Folder:
            openSelectedFolder();
            return;
        default:
            openSelectedFiles();
    }
}

void RetroFuturaGUI::FileExplorer::on_cancelButton_click()
{
    closeAfterAsking([this]() { Dismiss(); });
}

void RetroFuturaGUI::FileExplorer::on_stopButton_click()
{
    _transfer.Stop();
}

void RetroFuturaGUI::FileExplorer::sortEntries(std::vector<Entry>& entries, const SortColumn column, const bool isDescending)
{
    std::vector<SortKey> keys {};
    keys.reserve(entries.size());

    for(uSize index = 0; index < entries.size(); ++index)
        keys.push_back(SortKey { ._Name = entries[index]._Path.filename().native(), ._Type = lowerCase(typeName(entries[index])), ._Index = index }); //"PNG" sorts with "png"

    std::sort(keys.begin(), keys.end(), [&entries, column, isDescending](const SortKey& left, const SortKey& right) { return isSortedBefore(left, right, entries, column, isDescending); });

    std::vector<Entry> sorted {};
    sorted.reserve(entries.size());

    for(const SortKey& key : keys)
        sorted.push_back(std::move(entries[key._Index]));

    entries = std::move(sorted);
}

bool RetroFuturaGUI::FileExplorer::isSortedBefore(const SortKey& left, const SortKey& right, const std::vector<Entry>& entries, const SortColumn column, const bool isDescending)
{
    const Entry
        & leftEntry { entries[left._Index] },
        & rightEntry { entries[right._Index] };
    const bool
        isLeftDirectory { PlatformBridge::FileSystem::EntryKind::Directory == leftEntry._Kind },
        isRightDirectory { PlatformBridge::FileSystem::EntryKind::Directory == rightEntry._Kind };

    if(isLeftDirectory != isRightDirectory)
        return isLeftDirectory; //folders first, whichever way the list runs

    //Descending compares the other way round, ties included, so the order stays strict
    const SortKey
        & first { isDescending ? right : left },
        & second { isDescending ? left : right };
    const Entry
        & firstEntry { entries[first._Index] },
        & secondEntry { entries[second._Index] };

    switch(column)
    {
        case SortColumn::Size:
            if(firstEntry._Size != secondEntry._Size)
                return firstEntry._Size < secondEntry._Size;
        break;
        case SortColumn::Modified:
            if(firstEntry._LastWriteTime != secondEntry._LastWriteTime)
                return firstEntry._LastWriteTime < secondEntry._LastWriteTime;
        break;
        case SortColumn::Type:
            if(first._Type != second._Type)
                return first._Type < second._Type;
        break;
        default: //Name
        break;
    }

    return isNameBefore(first._Name, second._Name);
}

bool RetroFuturaGUI::FileExplorer::isNameBefore(const std::filesystem::path::string_type& left, const std::filesystem::path::string_type& right)
{
    const auto isDigit = [](const std::filesystem::path::value_type character) { return '0' <= character && '9' >= character; };
    uSize
        leftIndex { 0 },
        rightIndex { 0 };

    while(leftIndex < left.size() && rightIndex < right.size())
    {
        if(isDigit(left[leftIndex]) && isDigit(right[rightIndex]))
        {
            //Compare the numbers: without leading zeros the longer one is larger, and equal lengths compare digit by digit
            uSize
                leftStart { leftIndex },
                rightStart { rightIndex };

            while(leftStart < left.size() && '0' == left[leftStart])
                ++leftStart;

            while(rightStart < right.size() && '0' == right[rightStart])
                ++rightStart;

            uSize
                leftEnd { leftStart },
                rightEnd { rightStart };

            while(leftEnd < left.size() && isDigit(left[leftEnd]))
                ++leftEnd;

            while(rightEnd < right.size() && isDigit(right[rightEnd]))
                ++rightEnd;

            const uSize
                leftDigitCount { leftEnd - leftStart },
                rightDigitCount { rightEnd - rightStart };

            if(leftDigitCount != rightDigitCount)
                return leftDigitCount < rightDigitCount;

            for(uSize digit = 0; digit < leftDigitCount; ++digit)
            {
                if(left[leftStart + digit] != right[rightStart + digit])
                    return left[leftStart + digit] < right[rightStart + digit];
            }

            leftIndex = leftEnd;
            rightIndex = rightEnd;
            continue;
        }

        const std::filesystem::path::value_type
            leftFolded { foldCase(left[leftIndex]) },
            rightFolded { foldCase(right[rightIndex]) };

        if(leftFolded != rightFolded)
            return leftFolded < rightFolded;

        ++leftIndex;
        ++rightIndex;
    }

    const uSize
        leftRestCount { left.size() - leftIndex },
        rightRestCount { right.size() - rightIndex };

    if(leftRestCount != rightRestCount)
        return leftRestCount < rightRestCount;

    //Equal but for case or leading zeros: the names themselves decide, so the order stays strict
    return left < right;
}

std::filesystem::path::value_type RetroFuturaGUI::FileExplorer::foldCase(const std::filesystem::path::value_type character)
{
    if('A' <= character && 'Z' >= character)
        return static_cast<std::filesystem::path::value_type>(character - 'A' + 'a');

    return character;
}

bool RetroFuturaGUI::FileExplorer::matchesFilter(const std::filesystem::path& path, const Filter& filter)
{
    if(filter._Extensions.empty())
        return true;

    const std::filesystem::path::string_type extension { path.extension().native() };

    for(const std::filesystem::path& wanted : filter._Extensions)
    {
        const std::filesystem::path::string_type& wantedExtension { wanted.native() };

        if(wantedExtension.size() != extension.size())
            continue;

        const bool isMatch
        {
            std::equal(extension.begin(), extension.end(), wantedExtension.begin(),
                [](const std::filesystem::path::value_type left, const std::filesystem::path::value_type right) { return foldCase(left) == foldCase(right); })
        };

        if(isMatch)
            return true;
    }

    return false;
}

std::filesystem::path RetroFuturaGUI::FileExplorer::normalizedDirectory(const std::filesystem::path& directory)
{
    const std::filesystem::path normalized { directory.lexically_normal() };

    if(!normalized.has_filename() && normalized.has_relative_path())
        return normalized.parent_path();

    return normalized;
}

std::string RetroFuturaGUI::FileExplorer::toUtf8(const std::filesystem::path& path)
{
    //Not path.string(): on Windows that goes through the ANSI code page and turns names outside it into '?'
    try
    {
        const std::u8string text { path.u8string() };
        return std::string(text.begin(), text.end());
    }
    catch(const std::system_error&)
    {
        return "\xEF\xBF\xBD"; //a name that isn't valid UTF-16, shown as the replacement character
    }
}

std::filesystem::path RetroFuturaGUI::FileExplorer::fromUtf8(std::string_view text)
{
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}

std::string RetroFuturaGUI::FileExplorer::formatSize(const u64 size)
{
    if(1024 > size)
        return std::format("{} B", size);

    constexpr std::array<std::string_view, 4> units { "KB", "MB", "GB", "TB" };
    f64 value { static_cast<f64>(size) / 1024.0 };
    uSize unit { 0 };

    while(1024.0 <= value && unit + 1 < units.size())
    {
        value /= 1024.0;
        ++unit;
    }

    return std::format("{:.1f} {}", value, units[unit]);
}

std::string RetroFuturaGUI::FileExplorer::formatTime(const std::filesystem::file_time_type time, LocalClock& clock)
{
    if(time == std::filesystem::file_time_type {})
        return {};

    const std::chrono::sys_time<std::chrono::minutes> systemTime { std::chrono::floor<std::chrono::minutes>(std::chrono::clock_cast<std::chrono::system_clock>(time)) };

    if(clock._Zone && (!clock._HasInfo || clock._Info.begin > systemTime || clock._Info.end <= systemTime))
    {
        clock._Info = clock._Zone->get_info(systemTime);
        clock._HasInfo = true;
    }

    const std::chrono::minutes localTime { systemTime.time_since_epoch() + std::chrono::floor<std::chrono::minutes>(clock._Zone ? clock._Info.offset : std::chrono::seconds { 0 }) };
    const std::chrono::days day { std::chrono::floor<std::chrono::days>(localTime) };
    const std::chrono::year_month_day date { std::chrono::sys_days { day } };
    const std::chrono::hh_mm_ss<std::chrono::minutes> timeOfDay { localTime - day };

    //From the fields, since the chrono format specifiers go through the locale and take most of a large folder's fill
    return std::format("{:04}-{:02}-{:02} {:02}:{:02}", static_cast<i32>(date.year()), static_cast<u32>(date.month()), static_cast<u32>(date.day()), timeOfDay.hours().count(), timeOfDay.minutes().count());
}

const std::chrono::time_zone* RetroFuturaGUI::FileExplorer::getLocalZone()
{
    try
    {
        return std::chrono::current_zone();
    }
    catch(const std::runtime_error&)
    {
        return nullptr;
    }
}

std::string RetroFuturaGUI::FileExplorer::typeName(const Entry& entry)
{
    if(PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind)
        return "Folder";

    std::string extension { toUtf8(entry._Path.extension()) };

    if(1 >= extension.size())
        return "File";

    extension.erase(0, 1); //the dot; the rest keeps the file's casing
    return extension;
}

std::string RetroFuturaGUI::FileExplorer::lowerCase(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](const char character) { return 'A' <= character && 'Z' >= character ? static_cast<char>(character - 'A' + 'a') : character; });
    return text;
}

RetroFuturaGUI::FileExplorer::FileKind RetroFuturaGUI::FileExplorer::fileKindOf(const Entry& entry)
{
    if(PlatformBridge::FileSystem::EntryKind::Directory == entry._Kind)
        return FileKind::Folder;

    //Through UTF-8, since a native string in the system's code page throws on characters it doesn't have
    const std::string extension { lowerCase(toUtf8(entry._Path.extension())) };

    constexpr std::array<std::string_view, 7> imageExtensions { ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".svg" };
    constexpr std::array<std::string_view, 9>
        audioExtensions { ".mp3", ".flac", ".wav", ".ogg", ".oga", ".opus", ".m4a", ".adx", ".ahx" },
        videoExtensions { ".webm", ".mkv", ".mp4", ".m4v", ".mov", ".ogv", ".mpg", ".mpeg", ".avi" };
    const auto isAmong { [&extension](const auto& extensions) { return extensions.end() != std::find(extensions.begin(), extensions.end(), extension); } };

    if(isAmong(imageExtensions))
        return FileKind::Image;

    if(isAmong(audioExtensions))
        return FileKind::Audio;

    if(isAmong(videoExtensions))
        return FileKind::Video;

    return FileKind::Other;
}

std::string RetroFuturaGUI::FileExplorer::describeError(const std::error_code& error)
{
    //In English like the rest of the explorer, not in the system's language
    if(error == std::errc::permission_denied)
        return "access denied";

    if(error == std::errc::no_such_file_or_directory)
        return "not found";

    if(error == std::errc::not_a_directory)
        return "not a folder";

    if(error == std::errc::resource_unavailable_try_again || error == std::errc::device_or_resource_busy)
        return "the drive isn't ready";

    if(error == std::errc::filename_too_long)
        return "the path is too long";

    return std::format("error {}", error.value());
}