#include "Video.hpp"
#include "AudioPlayback.hpp"
#include "IWidget.hpp"
#include "IncludeHelper.hpp"
#include <memory>

RetroFuturaGUI::Video::Video(const std::string& name, Projection* projection, IWidget* parentWidget, const WidgetTypeID parentWidgetTypeID, GLFWwindow* parentWindow)
    : IWidget(name, projection, parentWidget, parentWidgetTypeID, parentWindow)
{
    _widgetTypeID = WidgetTypeID::Video;
    _audioPlayback = std::make_unique<AudioPlayback>();

    if(_audioPlayback)
        _audioPlayback->InitDevice();

    _videoPlayback = std::make_unique<VideoPlayback>(*_audioPlayback, projection); 
}

RetroFuturaGUI::Video::~Video()
{
    Stop();

    if(_videoPlayback)
        _videoPlayback->Close();
}

void RetroFuturaGUI::Video::Draw()
{
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

    texture->Draw();
}

void RetroFuturaGUI::Video::SetSize(const glm::vec3& size)
{
    IWidget::SetSize(size);
    fitGeometryToTexture();
}

void RetroFuturaGUI::Video::SetPosition(const glm::vec3& position)
{
    IWidget::SetPosition(position);

    if(!_videoPlayback)
        return;

    ITexture* texture { _videoPlayback->GetTexture() };

    if(!texture)
        return;

    _videoPlayback->GetTexture()->SetPosition(_position);
}

void RetroFuturaGUI::Video::SetRotation(const glm::vec3& rotation)
{
    IWidget::SetRotation(rotation);

    if(!_videoPlayback)
        return;

    ITexture* texture { _videoPlayback->GetTexture() };

    if(!texture)
        return;

    _videoPlayback->GetTexture()->SetRotation(_rotation);
}

bool RetroFuturaGUI::Video::OpenVideoFile(std::string_view file)
{
    if(!_videoPlayback)
        return false;

    return _videoPlayback->Open(file);
}

void RetroFuturaGUI::Video::fitGeometryToTexture()
{
    if(!_videoPlayback)
        return;

    ITexture* texture { _videoPlayback->GetTexture() };

    if(!texture)
        return;

    if(0.0f >= _textureAspectRatio)
    {
        texture->SetSize(glm::vec2(_size));
        return;
    }

    const f32 width { std::min(_size.x, _size.y * _textureAspectRatio) };
    texture->SetSize(glm::vec2(width, width / _textureAspectRatio));
}

void RetroFuturaGUI::Video::Play()
{
    if(_videoPlayback)
        _videoPlayback->Play();
}

void RetroFuturaGUI::Video::Pause()
{
    if(_videoPlayback)
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
    if(_videoPlayback)
        return _videoPlayback->Seek(milliseconds);

    return false;
}