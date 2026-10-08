#pragma once
#include "IncludeHelper.hpp"
#include "config.hpp"
#include "IRangedMultiValue.hpp"

namespace RetroFuturaGUI
{
    class Histogram final : public IRangedMultiValue
    {
    public:
        /// @brief Constructs a Histogram widget with non-owning data. Data must be updated externally, and must remain valid for the lifetime of the Histogram or be set to nullptr.
        Histogram(std::string_view name, Projection* projection, IHierarchyNode* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        Histogram() = delete;
        Histogram(const Histogram&) = delete;
        Histogram(Histogram&&) = delete;
        ~Histogram() = default;
        auto operator =(const Histogram&) = delete;
        auto operator =(Histogram&&) = delete;

        
        void Draw() override;
        void SetBarInterstice(const f32 interstice);

    private:
        void calculateBarWidth();
        f32 calculateBarHeight(const uSize index) const;
        void alignElementsToTrack() override;
        void drawGraph() override;
        glm::vec3 calculateBarPosition(const uSize index) const;
        glm::vec3 calculateBarRotation() const;
        void interact();

        f32
            _barWidth { 20.0f },
            _barInterstice { 3.0f };
    };
}