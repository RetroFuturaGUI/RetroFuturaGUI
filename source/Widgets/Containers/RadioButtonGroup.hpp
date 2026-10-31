#pragma once
#include "ITextProperties.hpp"
#include "Lasagna.hpp"
#include "RadioButton.hpp"
#include "Label.hpp"
#include "config.hpp"
#include <glm/ext/vector_int2_sized.hpp>
#include <memory>
#include <string_view>
#include <vector>

namespace RetroFuturaGUI
{
    class RadioButton;

    class RadioButtonGroup final : public IWidget, public IBorder, public ITextProperties
    {
    public:
        /// @brief Constructs a RadioButtonGroup under the given parent widget/window.
        RadioButtonGroup(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        RadioButtonGroup(const RadioButtonGroup&) = delete;
        RadioButtonGroup(RadioButton&&) = delete;
        ~RadioButtonGroup() = default;
        auto operator =(const RadioButtonGroup&) = delete;
        auto operator =(RadioButtonGroup&&) = delete;

        void Draw() override;

        /// @brief Sets the size of the radio button group.
        void SetSize(const glm::vec3& size) override;

        /// @brief Sets the world position of the radio button group.
        void SetPosition(const glm::vec3& position) override;

        /// @brief Sets the rotation of the radio button group.
        void SetRotation(const glm::vec3& rotation) override;

        /// @brief deselcts all registered RadioButtons except the passed one
        /// @param a pointer of the RadioButton to stay checked
        void DeselectOthers(RadioButton* activeRadioButton);

        /// @brief Builds a RadioButton and its Label, named by its name by default. It is placed in the given cell.
        /// @param index Row and column of the cell. SetAxisDefinitions has to have made the cell first.
        /// @return The button, or nullptr, building nothing, when the cell is outside the grid or taken, or the name is taken.
        /// @warning Call from the GUI thread only: a widget creates OpenGL objects, and those must stay on the context's thread.
        RadioButton* AddRadioButton(std::string_view name, const glm::i64vec2& index);

        /// @brief Sets the label based on a cell's name.
        void SetLabel(std::string_view name, std::string_view label, const bool emitSignal);

        /// @brief Removes the RadioButton with the given name and its Label, and frees their cell at once
        /// @return false when no button has that name.
        bool RemoveRadioButton(std::string_view name);

        /// @brief Returns the RadioButton with the given name, or nullptr when there is none.
        RadioButton* GetRadioButton(std::string_view name) const;

        /// @brief Returns the Label of the RadioButton with the given name, or nullptr when there is none.
        Label* GetRadioButtonLabel(std::string_view radioButtonName) const;

        /// @brief Sets the text content, in UTF-8.
        void SetText(std::string_view text, const bool emitSignal) override;

        /// @brief Sets sections of the border to skip drawing; see Frame::SetBorderGaps.
        void SetBorderGaps(std::span<BorderGap> gaps) override;

        /// @brief Sets the Grid according to the axis definitions. Radio buttons in cells the new grid still has keep them; the
        ///        others are removed with their labels.
        void SetAxisDefinitions(std::span<f32> rowDefinitions, std::span<f32> columnDefinitions);

        /// @brief Sets how each cell's radio button + label pair is aligned within the cell's width.
        void SetGridContentAlignment(const TextAlignment alignment);

        /// @brief Sets how much each cell's RadioButton + Label should be away from the border's inner edge
        /// @param pixels (const f32): Distance in pixels
        void SetGridContentPadding(const f32 pixels);

    private:
        /// @brief Sweeps the removed buttons, then passes the frame's input on to every button and its label.
        void interact() override;

        struct GridCell
        {
            glm::vec2 _Position {};
            glm::vec2 _Size {};
            std::unique_ptr<RadioButton> _RadioButton { nullptr };
            std::unique_ptr<Label> _Label { nullptr };
        };

        /// @brief The cell holding the RadioButton with the given name, or nullptr.
        GridCell* findCell(std::string_view name);
        const GridCell* findCell(std::string_view name) const;

        /// @brief Keeps the cell's button and label until the next Interact, and empties the cell.
        void releaseCell(GridCell& cell);

        void updateLabelLayout();
        void updateGridLayout();

        f32 
            _textLeftPadding { 0.0f },
            _borderPadding { 0.0f };
        TextAlignment _gridContentAlignment { TextAlignment::Center };
        std::vector<f32>
            _rowDefinitions {},
            _columnDefinitions {};
        std::vector<std::vector<GridCell>> _grid {};
        std::vector<std::unique_ptr<IWidget>> _removedWidgets {}; //destroyed at the start of the next Interact
    };
}