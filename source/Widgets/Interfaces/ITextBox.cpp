#include "ITextBox.hpp"
#include <array>

#if defined(TARGET_PLATFORM_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(TARGET_PLATFORM_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>

RetroFuturaGUI::ITextBox::ITextBox(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow, const TextLines lines)
   : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow),
     _placeholder(projection),
     _text(projection, [this](const WidgetTextChange change) { textChanged(change); }, &_placeholder),
     _editor
     {
         projection,
         parentWindow,
         lines,
         TextEditorHooks
         {
             ._GetActiveText = [this]() { return &_text.GetElement(); },
             ._GetTextArea = [this]() { return getTextArea(); },
             ._IsReadOnly = [this]() { return _readOnly; },
             ._OnChange = [this]() { textEdited(); }
         }
     }
{
    _placeholder.SetColors(std::array { glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) }, ColorState::Enabled);
    _text.SetTransitionSettings(&_transitionSettings);
    _placeholder.SetTransitionSettings(&_transitionSettings);
    _editor.SetTransitionSettings(&_transitionSettings);
    _background.SetTransitionSettings(&_transitionSettings);
    _border.SetTransitionSettings(&_transitionSettings);
    useBorderGap(&_background, &_border);
}

void RetroFuturaGUI::ITextBox::textChanged(const WidgetTextChange change)
{
    if(WidgetTextChange::String != change)
        return;

    //Text set by code rather than typed. No keystroke placed the caret and its old place may lie past the new text's end
    _editor.PlaceCaretAfterText();
}

void RetroFuturaGUI::ITextBox::Draw()
{
    _background.Draw();
    _border.Draw();
    _editor.DrawSelectedArea();

    if(!_text.GetElement().GetTextUTF32().empty())
    {
        _text.Draw();
        _editor.DrawCaret();
        return;
    }

    _placeholder.Draw();

    _editor.DrawCaret();
}

void RetroFuturaGUI::ITextBox::applySize(const glm::vec3& size)
{
    IWidget::applySize(size);

    _background.SetSize(size);
    _border.SetSize(size);
    _text.SetSize(glm::vec2(size.x, size.y));
    _placeholder.SetSize(glm::vec2(size.x, size.y));
}

void RetroFuturaGUI::ITextBox::applyPosition(const glm::vec3& position)
{
    IWidget::applyPosition(position);

    _background.SetPosition(position);
    _editor.GetSelectedArea().SetPosition(position + glm::vec3(0.0f, 0.0f, 0.1f));
    _border.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.2f));
    _text.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));
    _placeholder.SetPosition(position + glm::vec3(0.0f, 0.0f, 0.3f));
    _editor.GetCaret().SetPosition(position + glm::vec3(0.0f, 0.0f, 0.5f));
}

void RetroFuturaGUI::ITextBox::interact()
{
    const PointerState pointer { readPointer() };
    glm::vec2 mousePos { pointer._Position };
    bool
        isMouseTextBoxPressed { pointer._LeftButtonPressed },
        isMouseInside { pointer._HasPosition && isPointInside(mousePos) };

    if(_editor.IsEditing() && !_mouseEnteredFlag && !_editor.IsMarking() && isMouseTextBoxPressed)
        _editor.EndEditing();

    _editor.Interact();

    if(pointer._IsCaptured)
        return; //ScrollView above has taken the pointer. Typing still works, the keyboard was handled above

    if(_editor.IsMarking())
    {
        if(isMouseTextBoxPressed && pointer._HasPosition)
        {
            requestCursorIcon(PlatformBridge::CursorIcon::Text); //also once the drag leaves the box
            //clamp before the hit-test, so the selection doesn't extend outside the visible text area
            //also consider scrolling when marking goes out of bounds
            _editor.MarkTo(_text.GetElement().GetBoundaryAtPosition(mousePos));
        }
        else
        {
            _editor.StopMarking();
        }
    }

    if(!_isEnabledFlag || !isMouseInside) //no action and mouse leave
    {
        if(_mouseEnteredFlag)
        {
            _mouseEnteredFlag = false;
            _onMouseLeaveAsync.EmitAsync();
            _onMouseLeave.Emit();
            setColors(ColorState::Enabled);
        }

        if(!_editor.IsEditing())
            _editor.ReleaseKeyboardFocus();

        return;
    }

    bool isHovering = _isEnabledFlag && isMouseInside;
    if(isHovering) // hover
    {
        requestCursorIcon(PlatformBridge::CursorIcon::Text);
        _whileHoverAsync.EmitAsync();
        _whileHover.Emit();
    }

    if (isHovering && !_mouseLeftFlag && !_mouseEnteredFlag) //enter
    {
        _mouseEnteredFlag = true;
        _onMouseEnterAsync.EmitAsync();
        _onMouseEnter.Emit();
        setColors(ColorState::Hover);
    }

    if (isMouseTextBoxPressed && !_wasClicked) //click
    {
        _onClickAsync.EmitAsync();
        _onClick.Emit();
        setColors(ColorState::Clicked);
        _editor.BeginEditing();
        _editor.StartMarking(_text.GetElement().GetBoundaryAtPosition(glm::vec2(_editor.ClampToTextBounds(mousePos.x), mousePos.y)));
    }
    else if(!isMouseTextBoxPressed && _wasClicked) //release
    {
        _onReleaseAsync.EmitAsync();
        _onRelease.Emit();

        if(isHovering)
            setColors(ColorState::Hover);
        else
            setColors(ColorState::Enabled);
    }

    _wasClicked = isMouseTextBoxPressed;
}

void RetroFuturaGUI::ITextBox::TakeKeyboardFocus()
{
    _editor.BeginEditing();
    _editor.SelectFromStart(_text.GetElement().GetGlyphCount());
}

void RetroFuturaGUI::ITextBox::setColors(const ColorState state)
{
    _background.SetColorState(state);
    _border.SetColorState(state);
    _text.SetColorState(state);
}

RetroFuturaGUI::Background& RetroFuturaGUI::ITextBox::GetBackground()
{
    return _background;
}

const RetroFuturaGUI::Background& RetroFuturaGUI::ITextBox::GetBackground() const
{
    return _background;
}

RetroFuturaGUI::Border& RetroFuturaGUI::ITextBox::GetBorder()
{
    return _border;
}

const RetroFuturaGUI::Border& RetroFuturaGUI::ITextBox::GetBorder() const
{
    return _border;
}

RetroFuturaGUI::TextEditArea RetroFuturaGUI::ITextBox::getTextArea() const
{
    return { ._Left = _position.x - _size.x * 0.5f, ._Bottom = _position.y - _size.y * 0.5f, ._Right = _position.x + _size.x * 0.5f, ._Top = _position.y + _size.y * 0.5f };
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextBox::GetText()
{
    return _text;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextBox::GetText() const
{
    return _text;
}

RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextBox::GetPlaceholder()
{
    return _placeholder;
}

const RetroFuturaGUI::WidgetText& RetroFuturaGUI::ITextBox::GetPlaceholder() const
{
    return _placeholder;
}

void RetroFuturaGUI::ITextBox::EnableReadOnly(const bool enable)
{
    _readOnly = enable;
}

bool RetroFuturaGUI::ITextBox::IsReadOnly() const
{
    return _readOnly;
}

RetroFuturaGUI::Caret& RetroFuturaGUI::ITextBox::GetCaret()
{
    return _editor.GetCaret();
}

const RetroFuturaGUI::Caret& RetroFuturaGUI::ITextBox::GetCaret() const
{
    return _editor.GetCaret();
}

RetroFuturaGUI::SelectedArea& RetroFuturaGUI::ITextBox::GetSelectedArea()
{
    return _editor.GetSelectedArea();
}

const RetroFuturaGUI::SelectedArea& RetroFuturaGUI::ITextBox::GetSelectedArea() const
{
    return _editor.GetSelectedArea();
}

const std::string& RetroFuturaGUI::ITextBox::GetCopiedText() const
{
    return _editor.GetCopiedText();
}

const char* RetroFuturaGUI::ITextBox::GetSelection() const
{
    return _editor.GetSelection();
}

std::string RetroFuturaGUI::ITextBox::GetSelectedString() const
{
    return _editor.GetSelectedString();
}

void RetroFuturaGUI::ITextBox::CopySelectedText()
{
    _editor.CopySelectedText();
}

void RetroFuturaGUI::ITextBox::CutSelectedText()
{
    _editor.CutSelectedText();
}

void RetroFuturaGUI::ITextBox::PasteSelectedText()
{
    _editor.PasteSelectedText();
}

void RetroFuturaGUI::ITextBox::Connect_OnEnterPressed(const typename Signal<>::Slot& slot, const bool async)
{
    _editor.Connect_OnEnterPressed(slot, async);
}

void RetroFuturaGUI::ITextBox::Connect_OnEnterReleased(const typename Signal<>::Slot& slot, const bool async)
{
    _editor.Connect_OnEnterReleased(slot, async);
}

void RetroFuturaGUI::ITextBox::Connect_OnCopy(const typename Signal<>::Slot& slot, const bool async)
{
    _editor.Connect_OnCopy(slot, async);
}

void RetroFuturaGUI::ITextBox::Connect_OnPaste(const typename Signal<>::Slot& slot, const bool async)
{
    _editor.Connect_OnPaste(slot, async);
}

void RetroFuturaGUI::ITextBox::Disconnect_OnEnterPressed(const typename Signal<>::Slot& slot)
{
    _editor.Disconnect_OnEnterPressed(slot);
}

void RetroFuturaGUI::ITextBox::Disconnect_OnEnterReleased(const typename Signal<>::Slot& slot)
{
    _editor.Disconnect_OnEnterReleased(slot);
}

void RetroFuturaGUI::ITextBox::Disconnect_OnCopy(const typename Signal<>::Slot& slot)
{
    _editor.Disconnect_OnCopy(slot);
}

void RetroFuturaGUI::ITextBox::Disconnect_OnPaste(const typename Signal<>::Slot& slot)
{
    _editor.Disconnect_OnPaste(slot);
}

void RetroFuturaGUI::ITextBox::textEdited()
{
    if(_editor.WasEnterPressed())
        return;

    _text.EmitChange();
}