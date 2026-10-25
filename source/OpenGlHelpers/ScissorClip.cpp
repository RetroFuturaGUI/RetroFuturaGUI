#include "ScissorClip.hpp"
#include <algorithm>

RetroFuturaGUI::ScissorClip::ScissorClip(const glm::ivec4& box)
{
    _wasEnabled = GL_TRUE == glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_SCISSOR_BOX, _previousBox);

    i32
        clipLeft { box.x },
        clipBottom { box.y },
        clipRight { box.x + box.z },
        clipTop { box.y + box.w };

    if(_wasEnabled) //the clip already active stays stays, this one only narrows it
    {
        clipLeft = std::max(clipLeft, _previousBox[0]);
        clipBottom = std::max(clipBottom, _previousBox[1]);
        clipRight = std::min(clipRight, _previousBox[0] + _previousBox[2]);
        clipTop = std::min(clipTop, _previousBox[1] + _previousBox[3]);
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(clipLeft, clipBottom, std::max(clipRight - clipLeft, 0), std::max(clipTop - clipBottom, 0));
}

RetroFuturaGUI::ScissorClip::~ScissorClip()
{
    //The scissor state is global, so it goes back exactly as it was found
    glScissor(_previousBox[0], _previousBox[1], _previousBox[2], _previousBox[3]);

    if(!_wasEnabled)
        glDisable(GL_SCISSOR_TEST);
}