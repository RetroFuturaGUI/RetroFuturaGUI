#pragma once
#include "ITableWidget.hpp"
#include "IncludeHelper.hpp"
#include "Rectangle.hpp"
#include "SvgTexture.hpp"
#include <glm/ext/vector_float4.hpp>

namespace RetroFuturaGUI
{
    class TableCheckBox final : public ITableWidget
    {
        public:
            TableCheckBox(Table* parentTable, Projection* projection, Rectangle* background, Rectangle* border, SvgTexture* checkmark);
            TableCheckBox() = delete;

            void Draw() override;

            /// @brief Takes the cell's rect and keeps the largest centered square that fits inside the cell margin.
            void SetSize(const glm::vec3& size) override;
            void SetPosition(const glm::vec3& position) override;
            void SetRotation(const glm::vec3& rotation) override;

            /// @brief Sets the gap/padding between the cell's edge and the box, in pixels. Takes effect on the parent table's next layout pass.
            void SetPadding(const f32 padding);

            /// @brief Sets the gap/padding between the box's border and the checkmark in pixels.
            void SetInnerPadding(const f32 padding);

            /// @brief Sets the checked state.
            void SetValue(const bool value);

            /// @brief Returns the checked state.
            bool GetValue() const;

        private:
            Rectangle
                *_background { nullptr },
                *_border { nullptr };
            SvgTexture* _checkmark { nullptr };
            bool _isChecked { false };
            f32
                _padding { 4.0f },
                _innerPadding { 5.0f };
    };
}