#include "DialogKit.hpp"
#include "Button.hpp"
#include "Label.hpp"
#include "MenuBar.hpp"
#include "PlatformBridge.hpp"
#include "SceneLoader.hpp"
#include "Window.hpp"
#include <array>
#include <cmath>
#include <system_error>
#include <vector>

RetroFuturaGUI::DialogKit::ViewHost::ViewHost(std::string_view sceneName, Window* window, const AxisDefinition& lasagnaAxis)
{
    if(!window)
        return;

    Projection* projection { window->GetProjection() };

    if(!projection)
        return;

    _scene = SceneLoader::CreateScene(sceneName, window);

    if(!_scene)
        return;

    const SceneLoader::AssetScope assets { *_scene }; //what the widgets below load counts as the explorer's

    _scene->SetLasagnaAxis(lasagnaAxis, projection);
}

RetroFuturaGUI::DialogKit::ViewHost::~ViewHost()
{
    if(!_scene)
        return;

    SceneLoader::UnregisterScene(SceneLoader::GetSceneID(_scene->GetName()));
}

void RetroFuturaGUI::DialogKit::styleWindow(Window& window, std::string_view title)
{
    window.SetWindowTitle(title);
    window.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(12.0f));
    window.GetBorder().SetBuiltInCornerRadii(styleKey(), glm::vec4(12.0f));
    window.GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
    window.GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Enabled);
}

RetroFuturaGUI::Scene* RetroFuturaGUI::DialogKit::ViewHost::GetScene() const
{
    return _scene.get();
}

RetroFuturaGUI::Lasagna* RetroFuturaGUI::DialogKit::ViewHost::GetLasagna() const
{
    if(!_scene)
        return nullptr;

    return _scene->GetRootContainer<Lasagna>();
}

RetroFuturaGUI::MenuBar* RetroFuturaGUI::DialogKit::attachBar(ViewHost& host, Lasagna& lasagna, std::string_view title, const uSize columnCount, const f32 barHeight, const glm::vec2& windowSize, const ViewFont& font)
{
    //The title drags the window, the spacer takes what is left, and the close button is a square of the bar's height
    MenuBar* bar { host.Attach<MenuBar>("Bar", lasagna, ViewCell { ._Row = 0, ._Column = 0, ._ColumnSpan = columnCount, ._SizingMode = SizingMode::FILL, ._Layer = 0 }) };

    if(!bar)
        return nullptr;

    bar->SetDockingEdge(MenuBar::DockingEdge::Top);
    bar->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.08f, 0.08f, 0.12f, 0.9f) }, ColorState::Enabled);
    bar->GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.45f, 1.0f) }, ColorState::Enabled);
    bar->GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
    bar->GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(12.0f, 12.0f, 0.0f, 0.0f)); //the window's top corners, which a popup never squares
    bar->GetBorder().SetBuiltInCornerRadii(styleKey(), glm::vec4(12.0f, 12.0f, 0.0f, 0.0f));
    bar->GetItemStyle().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) }, ColorState::Enabled);
    bar->GetItemStyle().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.25f, 0.25f, 0.35f, 0.9f) }, ColorState::Hover);
    bar->GetItemStyle().GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.35f, 0.35f, 0.5f, 0.95f) }, ColorState::Clicked);
    bar->GetItemStyle().GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.0f, 0.0f, 0.0f, 0.0f) }, ColorState::Enabled);
    bar->GetItemStyle().GetBorder().SetBuiltInWidth(styleKey(), 0.0f);

    std::vector<MenuBar::FlexDefinition> flexDefinition
    {
        { ._FlexSizing = MenuBar::FlexSizing::Auto,   ._Width = 0.0f, ._IsWindowDrag = true,  ._IsWindowUndock = false },
        { ._FlexSizing = MenuBar::FlexSizing::Star,   ._Width = 1.0f, ._IsWindowDrag = false, ._IsWindowUndock = false },
        { ._FlexSizing = MenuBar::FlexSizing::Square, ._Width = 0.0f, ._IsWindowDrag = false, ._IsWindowUndock = false }
    };
    bar->SetFlexDefinition(flexDefinition);

    Label* barTitle { bar->AddWidget<Label>("BarTitle", 0) };

    if(!barTitle)
        return bar;

    barTitle->GetText().SetFontFamily(font._Family, font._Size, FontSizeUnit::Pixels, font._Slant, font._Weight);
    barTitle->GetText().SetString(title, false);
    barTitle->GetText().SetAlignment(TextAlignment::Left);
    barTitle->GetText().SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    barTitle->GetText().SetPadding(8.0f);
    barTitle->SetSize(glm::vec3(0.5f * windowSize.x, barHeight, 0.01f)); //as tall as the bar, since text is clipped to its label

    Button* closeButton { bar->AddWidget<Button>("BarClose", _barCloseIndex) };

    if(!closeButton)
        return bar;

    closeButton->SetSize(glm::vec3(barHeight - 2.0f * _margin, barHeight - 2.0f * _margin, 0.01f));
    closeButton->GetBackground().SetImage(toUtf8(PlatformBridge::Paths::GetExecutablePathFSPath().parent_path() / "Resources" / "img" / "CloseIcon.svg")); //UTF-8, as Texture reads paths
    closeButton->GetBackground().SetImagePadding(4.0f);
    closeButton->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f, 0.1f, 0.1f, 0.0f) }, ColorState::Enabled);
    closeButton->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f, 0.2f, 0.2f, 0.45f) }, ColorState::Hover);
    closeButton->GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f, 0.3f, 0.3f, 0.6f) }, ColorState::Clicked);
    closeButton->GetBorder().SetBuiltInWidth(styleKey(), 0.0f);
    return bar;
}

void RetroFuturaGUI::DialogKit::styleButton(Button& button, std::string_view text, const glm::vec3& size, const ViewFont& font)
{
    button.SetSize(size);
    button.GetBackground().SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f));
    button.GetBorder().SetBuiltInCornerRadii(styleKey(), glm::vec4(8.0f));
    button.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.2f, 0.2f, 0.2f, 1.0f) }, ColorState::Enabled);
    button.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.3f, 0.3f, 0.3f, 1.0f) }, ColorState::Hover);
    button.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.4f, 0.4f, 0.4f, 1.0f) }, ColorState::Clicked);
    button.GetBackground().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.12f, 0.12f, 0.12f, 1.0f) }, ColorState::Disabled);
    button.GetBorder().SetBuiltInColors(styleKey(), std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Enabled);
    button.GetBorder().SetBuiltInWidth(styleKey(), 1.0f);
    button.GetText().SetFontFamily(font._Family, font._Size, FontSizeUnit::Pixels, font._Slant, font._Weight);
    button.GetText().SetString(text, false);
    button.GetText().SetAlignment(TextAlignment::Center);
}

void RetroFuturaGUI::DialogKit::styleLabel(Label& label, std::string_view text, const glm::vec3& size, const ViewFont& font)
{
    label.GetText().SetFontFamily(font._Family, font._Size, FontSizeUnit::Pixels, font._Slant, font._Weight);
    label.GetText().SetString(text, false);
    label.GetText().SetAlignment(TextAlignment::Left);
    label.GetText().SetBuiltInColors(styleKey(), std::array { glm::vec4(1.0f) }, ColorState::Enabled);
    label.GetText().SetPadding(8.0f);
    label.SetSize(size);
}

f32 RetroFuturaGUI::DialogKit::rowHeight(const ViewFont& font)
{
    return std::ceil(font._Size * 1.55f);
}

glm::vec3 RetroFuturaGUI::DialogKit::cellSize(const AxisDefinition& axis, const glm::vec2& windowSize, const ViewCell& cell)
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

std::string RetroFuturaGUI::DialogKit::toUtf8(const std::filesystem::path& path)
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

RetroFuturaGUI::StyleKey RetroFuturaGUI::DialogKit::styleKey()
{
    return StyleKey {};
}
