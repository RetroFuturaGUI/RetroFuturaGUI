#pragma once
#include "AudioPlayback.hpp"
#include "IWidget.hpp"
#include "VideoPlayback.hpp"
#include <memory>
#include <string_view>

namespace RetroFuturaGUI
{
    class Video final : public IWidget
    {
    public:
        Video(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow);
        ~Video();
        Video(const Video&) = delete;
        Video(Video&&) = delete;
        auto operator =(const Video&) = delete;
        auto operator =(Video&&) = delete;

        /// @brief Draws the Video
        void Draw() override;
        void SetSize(const glm::vec3& size) override;
        void SetPosition(const glm::vec3& position) override;
        void SetRotation(const glm::vec3& rotation) override;
        bool OpenVideoFile(std::string_view file);
        void Play();
        void Pause();
        void Stop();
        bool Seek(const i64 milliseconds);

        /// @brief The video sound's level per channel, linear RMS 0.0 to ~1.0 - see AudioPlayback::GetChannelVolume.
        f32 GetChannelVolume(const u32 channel) const;

    private:
        void fitGeometryToTexture();

        std::unique_ptr<AudioPlayback> _audioPlayback {};
        std::unique_ptr<VideoPlayback> _videoPlayback {};
        f32 _textureAspectRatio { 1.777f };
    };
}