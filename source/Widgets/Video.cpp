#include "Video.hpp"
#include "AudioPlayback.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include <memory>

RetroFuturaGUI::Video::Video(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Video;
    _plane = std::make_unique<Plane>(projection);
    _audioPlayback = std::make_unique<AudioPlayback>();

    if(!_plane)
        return;

    _plane->SetColors(_planeColors);

    if(!_audioPlayback)
        return;

    _audioPlayback->InitDevice();

    _videoPlayback = std::make_unique<VideoPlayback>(*_audioPlayback);

    if(!_videoPlayback)
        return;

    ITexture* texture { _videoPlayback->GetTexture() };

    if(!texture)
        return;

    //The video's texture keeps its ID for its whole life, so the plane takes it once
    _plane->SetBackgroundImageTextureID(texture->GetID());
}

RetroFuturaGUI::Video::~Video()
{
    Stop();

    if(!_videoPlayback)
        return;

    _videoPlayback->Close();
}

void RetroFuturaGUI::Video::Draw()
{
    if(!_plane)
        return;

    if(!_videoPlayback)
        return;

    if(!_audioPlayback)
        return;

    _audioPlayback->UpdateFrequencyBands();

    ITexture* texture { _videoPlayback->GetTexture() };

    if(!texture)
        return;

    _videoPlayback->Update();

    if(0.0f >= texture->GetAspectRatio())
        return;

    if(texture->GetAspectRatio() != _textureAspectRatio)
    {
        _textureAspectRatio = texture->GetAspectRatio();
        fitGeometryToTexture();
    }

    _plane->Draw();
}

void RetroFuturaGUI::Video::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    fitGeometryToTexture();
}

void RetroFuturaGUI::Video::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(!_plane)
        return;

    _plane->SetPosition(_position);
}

void RetroFuturaGUI::Video::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_plane)
        return;

    _plane->SetRotation(_rotation);
}

bool RetroFuturaGUI::Video::OpenVideoFile(std::string_view file)
{
    if(!_videoPlayback)
        return false;

    return _videoPlayback->Open(file);
}

void RetroFuturaGUI::Video::fitGeometryToTexture()
{
    if(!_plane)
        return;

    if(0.0f >= _textureAspectRatio)
    {
        _plane->SetSize(glm::vec2(_size));
        return;
    }

    const f32 width { std::min(_size.x, _size.y * _textureAspectRatio) };
    _plane->SetSize(glm::vec2(width, width / _textureAspectRatio));
}

void RetroFuturaGUI::Video::Play()
{
    if(!_videoPlayback)
        return;

    _videoPlayback->Play();
}

void RetroFuturaGUI::Video::Pause()
{
    if(!_videoPlayback)
        return;

    _videoPlayback->Pause();
}

void RetroFuturaGUI::Video::Stop()
{
    if(!_videoPlayback)
        return;

    _videoPlayback->Pause();
    _videoPlayback->Seek(0);
    
}

f32 RetroFuturaGUI::Video::GetChannelVolume(const u32 channel) const
{
    if(!_audioPlayback)
        return 0.0f;

    return _audioPlayback->GetChannelVolume(channel);
}

std::span<const f32> RetroFuturaGUI::Video::GetFrequencyBands() const
{
    if(!_audioPlayback)
        return {};

    return _audioPlayback->GetFrequencyBands();
}

void RetroFuturaGUI::Video::SetFrequencyBandCount(const uSize bandCount)
{
    if(!_audioPlayback)
        return;

    _audioPlayback->SetFrequencyBandCount(bandCount);
}

bool RetroFuturaGUI::Video::Seek(const i64 milliseconds)
{
    if(!_videoPlayback)
        return false;

    return _videoPlayback->Seek(milliseconds);
}

void RetroFuturaGUI::Video::Close()
{
    if(!_videoPlayback)
        return;

    _videoPlayback->Close();
}

bool RetroFuturaGUI::Video::IsOpen() const
{
    if(!_videoPlayback)
        return false;

    return _videoPlayback->IsOpen();
}

i64 RetroFuturaGUI::Video::GetDuration() const
{
    if(!_videoPlayback)
        return 0;

    return _videoPlayback->GetDuration();
}