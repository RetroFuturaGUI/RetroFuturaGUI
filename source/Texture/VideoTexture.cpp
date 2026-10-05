#include "VideoTexture.hpp"
#include "ShaderManager.hpp"

extern "C"
{
    #include <libavutil/frame.h>
    #include <libavutil/pixdesc.h>
    #include <libswscale/swscale.h>
}

// What the YuvToRgb shader needs for one picture
struct ColorConversion
{
    glm::vec3 _Offset { 0.0f };
    glm::vec3 _Scale { 1.0f };
    glm::mat3 _Matrix { 1.0f };
};

// Kr and Kb are the red and blue weights of the colour space's luma; everything else follows from them
static ColorConversion makeConversion(const f32 kr, const f32 kb, const bool fullRange)
{
    const f32 kg { 1.0f - kr - kb };
    ColorConversion conversion;

    // Limited range keeps Y within 16..235 and Cb/Cr within 16..240 - full range uses all of 0..255.
    // The plane textures hand the codes over as code / 255, so offsets and scales are in that unit.
    if(fullRange)
    {
        conversion._Offset = glm::vec3(0.0f, 128.0f / 255.0f, 128.0f / 255.0f);
        conversion._Scale = glm::vec3(1.0f);
    }
    else
    {
        conversion._Offset = glm::vec3(16.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f);
        conversion._Scale = glm::vec3(255.0f / 219.0f, 255.0f / 224.0f, 255.0f / 224.0f);
    }

    // one column each for what Y, Cb and Cr add to R, G and B
    conversion._Matrix = glm::mat3(
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(0.0f, -2.0f * kb * (1.0f - kb) / kg, 2.0f * (1.0f - kb)),
        glm::vec3(2.0f * (1.0f - kr), -2.0f * kr * (1.0f - kr) / kg, 0.0f));

    return conversion;
}

static ColorConversion colorConversionFor(const AVFrame* frame)
{
    if(!frame)
        return makeConversion(0.2126f, 0.0722f, false);

    const bool fullRange { frame->color_range == AVCOL_RANGE_JPEG };

    switch(frame->colorspace)
    {
        case AVCOL_SPC_BT709:
            return makeConversion(0.2126f, 0.0722f, fullRange);

        case AVCOL_SPC_BT470BG:
        case AVCOL_SPC_SMPTE170M:
            return makeConversion(0.299f, 0.114f, fullRange);

        case AVCOL_SPC_BT2020_NCL:
            return makeConversion(0.2627f, 0.0593f, fullRange);

        default:
            // untagged: by convention HD and up is BT.709, anything smaller BT.601
            return 720 <= frame->height ? makeConversion(0.2126f, 0.0722f, fullRange) : makeConversion(0.299f, 0.114f, fullRange);
    }
}

// The GL state the conversion pass changes. Update runs between the draws of other widgets, which
// expect their state exactly as they left it - so it's saved before the pass and restored after.
struct SavedGlState
{
    i32
        _DrawFramebuffer { 0 },
        _ReadFramebuffer { 0 },
        _Program { 0 },
        _VertexArray { 0 },
        _ActiveTexture { 0 },
        _UnpackRowLength { 0 },
        _UnpackAlignment { 4 };
    i32 _Viewport[4] {};
    i32 _Textures[3] {};
    GLboolean
        _Blend { GL_FALSE },
        _DepthTest { GL_FALSE },
        _ScissorTest { GL_FALSE },
        _CullFace { GL_FALSE };
};

static SavedGlState saveGlState()
{
    SavedGlState state;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &state._DrawFramebuffer);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &state._ReadFramebuffer);
    glGetIntegerv(GL_CURRENT_PROGRAM, &state._Program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &state._VertexArray);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &state._ActiveTexture);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &state._UnpackRowLength);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &state._UnpackAlignment);
    glGetIntegerv(GL_VIEWPORT, state._Viewport);

    for(u32 unit = 0; 3 > unit; ++unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &state._Textures[unit]);
    }

    state._Blend = glIsEnabled(GL_BLEND);
    state._DepthTest = glIsEnabled(GL_DEPTH_TEST);
    state._ScissorTest = glIsEnabled(GL_SCISSOR_TEST);
    state._CullFace = glIsEnabled(GL_CULL_FACE);
    return state;
}

static void setEnabled(const GLenum capability, const GLboolean enabled)
{
    if(enabled)
        glEnable(capability);
    else
        glDisable(capability);
}

static void restoreGlState(const SavedGlState& state)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<u32>(state._DrawFramebuffer));
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<u32>(state._ReadFramebuffer));
    glUseProgram(static_cast<u32>(state._Program));
    glBindVertexArray(static_cast<u32>(state._VertexArray));
    glPixelStorei(GL_UNPACK_ROW_LENGTH, state._UnpackRowLength);
    glPixelStorei(GL_UNPACK_ALIGNMENT, state._UnpackAlignment);
    glViewport(state._Viewport[0], state._Viewport[1], state._Viewport[2], state._Viewport[3]);

    for(u32 unit = 0; 3 > unit; ++unit)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, static_cast<u32>(state._Textures[unit]));
    }

    glActiveTexture(static_cast<GLenum>(state._ActiveTexture));
    setEnabled(GL_BLEND, state._Blend);
    setEnabled(GL_DEPTH_TEST, state._DepthTest);
    setEnabled(GL_SCISSOR_TEST, state._ScissorTest);
    setEnabled(GL_CULL_FACE, state._CullFace);
}

// Width or height of a chroma plane in 4:2:0 - half, rounded up for odd sizes
static i32 chromaSize(const i32 size)
{
    return (size + 1) / 2;
}

RetroFuturaGUI::VideoTexture::VideoTexture(Projection* projection)
    : ITexture(projection, true) // stored bottom row first, like a Texture loaded with flipVertically
{
    // Every GL object is created once and only resized later: the texture keeps its ID for the whole
    // life of the VideoTexture, so whoever took GetID() once still has the right one after a size change.
    glGenTextures(1, &_id);
    glGenTextures(static_cast<i32>(_planeTextureIds.size()), _planeTextureIds.data());
    glGenFramebuffers(1, &_framebuffer);
    glGenVertexArrays(1, &_emptyVao);
    setupQuad(); // only builds the quad with a Projection
}

RetroFuturaGUI::VideoTexture::~VideoTexture()
{
    releaseVideoResources(); // ITexture deletes the RGBA texture and the quad
}

void RetroFuturaGUI::VideoTexture::releaseVideoResources()
{
    glDeleteTextures(static_cast<i32>(_planeTextureIds.size()), _planeTextureIds.data());
    glDeleteFramebuffers(1, &_framebuffer);
    glDeleteVertexArrays(1, &_emptyVao);
    sws_freeContext(_converter); // accepts null
    av_frame_free(&_convertedFrame); // accepts null and nulls the pointer
    _converter = nullptr;
}

bool RetroFuturaGUI::VideoTexture::Update(const AVFrame* frame)
{
    if(!frame)
        return false;

    const AVFrame* picture { toYuv420p(frame) };

    if(!picture)
        return false;

    if(0 >= picture->width || 0 >= picture->height)
        return false;

    const SavedGlState saved { saveGlState() };

    if(picture->width != _resolution.x || picture->height != _resolution.y)
    {
        if(!allocate(picture->width, picture->height))
        {
            restoreGlState(saved);
            return false;
        }
    }

    uploadPlanes(picture);
    convert(picture);
    restoreGlState(saved);

    // non-square pixels (anamorphic video) are stretched to the shape they're meant to be shown at
    const AVRational pixelShape { frame->sample_aspect_ratio };
    const f64 pixelAspect { 0 < pixelShape.num && 0 < pixelShape.den ? av_q2d(pixelShape) : 1.0 };
    _aspectRatio = static_cast<f32>(pixelAspect * picture->width / picture->height);
    _hasPicture = true;
    return true;
}

bool RetroFuturaGUI::VideoTexture::HasPicture() const
{
    return _hasPicture;
}

bool RetroFuturaGUI::VideoTexture::allocate(const i32 width, const i32 height)
{
    // the RGBA texture everything else binds: repeats like any texture, so tiled UVs on a model work,
    // and has mipmaps, so it doesn't shimmer on a distant surface - they're rebuilt after each conversion
    glBindTexture(GL_TEXTURE_2D, _id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // the planes: one channel each, U and V at half size. Linear filtering upsamples the chroma smoothly.
    for(uSize plane = 0; _planeTextureIds.size() > plane; ++plane)
    {
        glBindTexture(GL_TEXTURE_2D, _planeTextureIds[plane]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        const i32 planeWidth { 0 == plane ? width : chromaSize(width) };
        const i32 planeHeight { 0 == plane ? height : chromaSize(height) };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, planeWidth, planeHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _id, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        _resolution = glm::i32vec2(0);
        return false;
    }

    _resolution = glm::i32vec2(width, height);
    return true;
}

const AVFrame* RetroFuturaGUI::VideoTexture::toYuv420p(const AVFrame* frame)
{
    if(!frame)
        return nullptr;

    // 8-bit 4:2:0 goes to the GPU as it is - unless its rows run bottom-up, which GL can't upload
    const bool yuv420p { frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P };
    const bool topDown { 0 < frame->linesize[0] && 0 < frame->linesize[1] && 0 < frame->linesize[2] };

    if(yuv420p && topDown)
        return frame;

    if(!_convertedFrame)
        _convertedFrame = av_frame_alloc();

    if(!_convertedFrame)
        return nullptr;

    // reused while the input stays the same, rebuilt when its format or size changes
    _converter = sws_getCachedContext(_converter, frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
        frame->width, frame->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);

    if(!_converter)
        return nullptr;

    const AVPixFmtDescriptor* format { av_pix_fmt_desc_get(static_cast<AVPixelFormat>(frame->format)) };

    if(!format)
        return nullptr;

    // swscale only changes the layout and keeps the range - the shader applies range and colour matrix.
    // An RGB source has neither, so swscale's own BT.601 limited-range output is what the shader gets told.
    const bool rgbSource { 0 != (format->flags & AV_PIX_FMT_FLAG_RGB) };
    const bool fullRange { !rgbSource && frame->color_range == AVCOL_RANGE_JPEG };
    const i32* coefficients { sws_getCoefficients(SWS_CS_DEFAULT) };
    sws_setColorspaceDetails(_converter, coefficients, fullRange, coefficients, fullRange, 0, 1 << 16, 1 << 16);

    if(_convertedFrame->width != frame->width || _convertedFrame->height != frame->height)
    {
        av_frame_unref(_convertedFrame);
        _convertedFrame->format = AV_PIX_FMT_YUV420P;
        _convertedFrame->width = frame->width;
        _convertedFrame->height = frame->height;

        if(0 > av_frame_get_buffer(_convertedFrame, 0))
            return nullptr;
    }

    if(0 >= sws_scale(_converter, frame->data, frame->linesize, 0, frame->height, _convertedFrame->data, _convertedFrame->linesize))
        return nullptr;

    _convertedFrame->colorspace = rgbSource ? AVCOL_SPC_SMPTE170M : frame->colorspace;
    _convertedFrame->color_range = fullRange ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;
    return _convertedFrame;
}

void RetroFuturaGUI::VideoTexture::uploadPlanes(const AVFrame* frame)
{
    if(!frame)
        return;

    // Each row sits linesize bytes after the previous one, padding included - telling GL so uploads the
    // planes straight from the decoder's memory, without copying them tight first
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for(uSize plane = 0; _planeTextureIds.size() > plane; ++plane)
    {
        const i32 planeWidth { 0 == plane ? frame->width : chromaSize(frame->width) };
        const i32 planeHeight { 0 == plane ? frame->height : chromaSize(frame->height) };

        glBindTexture(GL_TEXTURE_2D, _planeTextureIds[plane]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[plane]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, planeWidth, planeHeight, GL_RED, GL_UNSIGNED_BYTE, frame->data[plane]);
    }
}

void RetroFuturaGUI::VideoTexture::convert(const AVFrame* frame)
{
    const ColorConversion conversion { colorConversionFor(frame) };

    Shader& shader = ShaderManager::GetYuvToRgbShader();
    shader.UseProgram();
    shader.SetUniformInt("uY", 0);
    shader.SetUniformInt("uU", 1);
    shader.SetUniformInt("uV", 2);
    shader.SetUniformVec3("uOffset", conversion._Offset);
    shader.SetUniformVec3("uScale", conversion._Scale);
    shader.SetUniformMat3("uYuvToRgb", conversion._Matrix);

    for(u32 plane = 0; 3 > plane; ++plane)
    {
        glActiveTexture(GL_TEXTURE0 + plane);
        glBindTexture(GL_TEXTURE_2D, _planeTextureIds[plane]);
    }

    // every pixel of the target is written once, untouched by whatever the widgets drawn before set up
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glViewport(0, 0, _resolution.x, _resolution.y);
    glBindVertexArray(_emptyVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // the smaller versions for distant and minified use - built from the finished picture, so after the
    // pass and with the framebuffer released
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _id);
    glGenerateMipmap(GL_TEXTURE_2D);
}
