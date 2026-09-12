#pragma once
#include "IncludeHelper.hpp"
#include "IWindowAccessor.hpp"
#include <chrono>

namespace RetroFuturaGUI
{
    //parts of text interaction that don't care where the text lives
    class ITextInteraction : virtual public IWindowAccessor
    {
    protected:
        /// @brief Returns whether the widget's window is the one currently taking keyboard input.
        bool hasInputFocus() const;

        /// @brief Flips the caret's visibility once a full blink interval has elapsed.
        void updateCaretBlink();

        /// @brief Shows the caret and restarts its blink interval, so it stays solid while being moved.
        void resetCaretBlink();

        /// @brief Drops any selection and stops mouse marking.
        void deselect();

        /// @brief Returns the lower end of the marked range, whichever direction it was dragged in.
        uSize markedStart() const;

        /// @brief Returns the upper end of the marked range, whichever direction it was dragged in.
        uSize markedEnd() const;

        /* A held key fires once, pauses, then repeats at a steady rate - the same cadence the caret,
           the character input and backspace all follow, each counting its own frames. */
        static bool shouldRepeat(const u32 holdFrames);

    //Caret blink
        bool
            _showCaret { false },
            _caretBlinkState { true },
            _caretNeverBlinks { false };
        f64 _blinkForMilliseconds { 650.0 };
        std::chrono::high_resolution_clock::time_point _millisecondsPassed { std::chrono::high_resolution_clock::now() };

    //Selection
        uSize
            _selectedPositionFirst { 0 },
            _selectedPositionLast { 0 };
        bool
            _isMarking { false },
            _isSelected { false };

    //Key repeat cadence
        static constexpr u32
            _keyRepeatInitialDelay { 60 },
            _keyRepeatInterval { 5 };
    };
}
