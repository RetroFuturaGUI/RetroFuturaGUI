# RetroFuturaGUI

<img width="1280" height="722" alt="demo" src="https://github.com/user-attachments/assets/5d6a6bc7-6c49-4432-8ad6-d2fa5d9db9fd" />


RetroFuturaGUI is a signal-slot-based GUI framework written in C++. It merges retro-inspired and modern aesthetics with backward/forward compatibility - hence its name.
The framework is designed for cross-platform use, and its logic can be compiled into a dynamic library, making it accessible from other languages while ensuring compatibility with past, current, and future standards.


## Roadmap
| # | Name | Depends on | Current State |
|------|-----|------------| ---- |
| 1 | Button, Label, Window, MainWindow, Image, Grid2D, WindowBar with Buttons |  | ✅ |
| 2 | dll/so/dylib compilation for C# and Python support, Widget ID manager | 1 | ✅ | 
| 3 | Linux Support, Font Manager | 2 | ✅ | 
| 4 | More Widgets (TextBox, Table, VideoPlayer, AudioPlayer, 3D Model, Slider, CheckBox, DropdownBox, RadioButton, Tabs, 3D Scene, Lights, change Grid2d to "Lasagna" and add a 3rd dimension, Carousel, Color Pickers) | 1 | WIP | 
| 5 | .bechaml markup language for GUI design 🥣 (**B**eautifully **E**xtended **C**ascading but **H**airbally **A**pplication **M**arkup **L**anguage) | 4 | | 
| 6 | VS Code extension with project generator/manager | 5 | | 
| 7 | Aesthetics: Flat Metro, Frutiger Aero (Win Vista/7, Wii, Wii U, Pikmin 3), Frutiger Metro, Y2K Futurism (F-Zero GX), Vaporwave, Outrun/Synthwave, Neumorphism, Glassmorphism/Liquid Glass, Comic/Anime/LoFi, ... | 6 | | 
| 8 | Android (without Android Studio), MacOS, iOS support | 6 | | 
| 9 | dll/so/dylib compilation for C, Rust, Zig, Java, JavaScript, TypeScript, Go, Ruby, Lua, Swift, Carbon, Gleem (if it has evolved enough) | 6 | | 
| 10 | gcn and wii support (libogc (devkitPro)) | 6 | | 
| 11 | XBox series Support (all XBox consoles, not just the Series series, du'h) (DirectX), Direct X for Windows | 6 | | 
| 12 | Dreamcast support (OpenGL 1.5 + PVR APi) | 6 | | 
| 13 | Wii U support (GX2 (devkitPro)) | 6 | | 
| 14 | 3DS support (citro3d/citro2d (devkitPro)) | 6 | | 
| 15 | Switch support (deko3d (devkitPro)) | 6 | | 
| 16 | PS2 (ps2gl), PS3 (RSXGL), PS4 (OpenOrbis/GNMX/GNM) support | 6 | | 

### WIP in detail
- Font Manager
  - Update Windows font support to the new system
- Update all Input to use PlatformBridge's implementation
- Test ITextEditable.cpp#L119 on Linux
- Text
  - Optimizations
- TextBox
  - suppress text overflow
- Button
  - suppress text overflow
- WindowBar
  - Window Icon

### Implemented Features
<details><summary>CLICK TO EXPAND</summary>

- Widgets
  - Label
    - Render text
    - Text Alignment (Left, Right, Center)
    - Text Padding
    - Per-state text color (Enabled, Disabled, Clicked, Hover)
    - SetEnabled, SetPosition, SetSize, SetRotation
  - Button
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - SetEnabled
    - Background: Solid Fill, Linear Gradient, Radial Gradient, HueStar Gradient
    - Border: Solid Fill, Linear Gradient, Radial Gradient, HueStar Gradient, Corner Radii, Border Width
    - Label text (shares Label's text properties)
    - SetPosition, SetSize, SetRotation
  - TextBox
    - Text input & editing (insert, backspace, key repeat)
    - Read-only mode
    - Caret: blinking with configurable blink time, colors, Solid/Linear/Radial/HueStar Gradient fill, gradient animation speed
    - Text selection: mouse-drag selection, colors, Solid/Linear/Radial/HueStar Gradient fill, gradient offset/degree/rotation speed, corner radii
    - Placerholder Text
    - Copy / Cut / Paste (via PlatformBridge's Clipboard)
    - Signals: OnTextChange, OnEnterPressed, OnEnterReleased, OnCopy, OnPaste
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation
  - Image
    - Supports most image formats
    - Fit widget geometry to the texture's native aspect ratio
    - SetPosition, SetSize, SetRotation
  - Model
    - Load 3D models and their material textures (Assimp)
    - Multiple meshes per model
    - SetPosition, SetSize, SetRotation
  - Lasagna (three-dimensional successor of Grid)
    - Align widgets in a three-dimensional pattern
    - Widget sizing policies (fixed size, expand X, Y, Z, XY)
    - Row, Column and Layer definitions
    - Debug lines
  - WindowBar
    - Top, Bottom Position
    - Close, Minimize, Maximize buttons (all shadered)
    - Toggleable elements (buttons, title, background, no-docking-drag zone)
    - Window Title
    - Shadered background
    - Per-button styling (background/border colors, gradients, corner radii, border width)
    - Maximize callback
  - Window
    - Background color or image
      - Background image ID can be used to create glass effects on widgets
    - Resizeable
    - Movable
    - Toggleable WindowBar
- Shaders
  - Solid Fill
    - Glass Effect
    - Rounded corners
  - Linear Gradient
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
  - Radial Gradient
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
  - HueStar Gradient
    - Animated (rotation, motion)
    - Glass Effect
    - Rounded corners
  - Dedicated Border variants of the above (Solid, Linear, Radial, HueStar)
  - Line Fill
  - Font Atlas Fill (used by the Text Renderer)
  - Texture
  - Shader Manager (compiles and caches all shaders once)
- Text Renderer
  - Full Unicode codepoint range support via configurable glyph atlasses (FreeType)
  - Multiple font families, weights and slants, resolved from installed system fonts
  - Fallback/extended fontsets for filling in missing glyph ranges
  - Dual UTF-8 / UTF-32 string encoding (DoubleEncodedString), always kept in sync
  - Text Alignment (Left, Right, Center)
  - Text Padding & horizontal scroll offset for overflowing text
  - Glyph boundary lookup (used for caret positioning & selection)
- Signals
  - Thread-safe Connect/Disconnect
  - Synchronous (Emit) and asynchronous, threaded (EmitAsync) dispatch
- Input Manager
  - Per-window mouse position & button state
  - Focused/hovered window tracking
- PlatformBridge (cross-platform system layer)
  - Windows & Linux support
  - Clipboard (copy/paste of text and bitmap data)
  - System font enumeration (family, weight, slant, Unicode ranges)
  - Keyboard input, held-key tracking, per-window key capture
  - Mouse button state & window-relative cursor position
  - Monitor enumeration (Windows)
  - Executable path & working directory utilities
- Cross Language Capabilities
  - dll/so/dylib compilation
  - Widget ID Manager (DynamicLibWidgetManager) addressing widgets by string ID
  - Signal connection via C callbacks (OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover, OnTextChange, OnEnterPressed, OnEnterReleased)
  - C#
  - Python

</details>
  


### ToDos before milestone 5
<details><summary>CLICK TO EXPAND</summary>
  <ul>
<li>Update and FixedUpdate loops</li>
<li>Text</li>
  <ul>
  <li>Emoji support</li>
  <li>Vertical text flow</li>
  <li>Text Shaders with all shader options</li>
  <li>Text Border</li>
  </ul>
<li>Shaders</li>
  <ul>
  <li>shadow</li>
  <li>glow fog</li>
  <li>cut corners</li>
  <li>rename gradient fill to linear shift</li>
  <li>radial shift</li>
  <li>conic radiant shift</li>
  <li>noise options</li>
  <li>optimize shader handling</li>
    <ul>
    <li>different IDs but only once compiled code</li>
    <li>only pass parameters on initialzation and on parameter value change</li>
    </ul>
  </ul>
<li>WindowBar</li>
  <ul>
  <li>Positioning, left, right</li>
  <li>Docking emblem</li>
  <li>Icon</li>
  </ul>
<li>Window
  <ul>
  <li>Docking</li>
  </ul>
<li>Font Manager</li>
<li>Cursor Icon Manager</li>
<li>Animation Engine</li>
<li>Test Carbon compatibility</li>
<li>Add more widget interactions to the DynamicLibWidgetManager</li>
  <ul>
  <li>SetRotation, SetPosition, SetSize</li>
  <li>Set Color/Decoration parameters</li>
  </ul>
<li>C++ modules once ALL common compilers (GCC, CLang, MSVC) support them</li>
<li>Wiki/Documentation</li>
<li>More text color properties</li>
<li>Rectangle, Text, Texture, IWidget: 3rd dimension</li>
<li>Complete Python binding</li>
<li>Refactor Binding</li>
</ul>
</details>

#### Nice-to-have's
- Better (relative) Viewports
- Custom text renderer with many IANA character sets

### Why?
There are many C++ GUI frameworks available, but most come with one or more drawbacks: restrictive licensing, enforced use of proprietary containers, inflexible window layouts, limited design capabilities, or poor cross-platform support, often confined to C++ only.
RetroFuturaGUI aims to break these barriers!

### Known Bugs
- Window.hpp/.cpp
  - glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE()) causes objects with transparency to show what's behind the window even if the background is completely opaque
