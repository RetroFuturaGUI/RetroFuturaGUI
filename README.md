# RetroFuturaGUI

RetroFuturaGUI is a signal-slot-based GUI framework written in C++. It merges retro-inspired and modern aesthetics with backward/forward compatibility - hence its name.
The framework is designed for cross-platform use, and its logic can be compiled into a dynamic library, making it accessible from other languages while ensuring compatibility with past, current, and future standards.


## Roadmap
| # | Name | Depends on | Current State |
|------|-----|------------| ---- |
| 1 | Button, Label, Window, MainWindow, Image, Grid2D, WindowBar with Buttons |  | ✅ |
| 2 | dll/so/dylib compilation for C# and Python support, Widget ID manager | 1 | ✅ | 
| 3 | linux support | 2 | WIP | 
| 4 | More Widgets (TextBox, Table, VideoPlayer, AudioPlayer, 3D Model, Slider, CheckBox, DropdownBox, RadioButton, Tabs, 3D Scene, Lights, change Grid2d to "Grid" and add a 3rd dimension, Carousel, Color Pickers) | 1 | | 
| 5 | .bechaml markup language for GUI design 🥣 <!-- (**B**ased and **E**xtra **C**reamy **H**ollandaise **A**pplication **M**arkup **L**anguage) -->| 4 | | 
| 6 | VS Code extension with project generator/manager | 5 | | 
| 7 | Aesthetics: Flat Metro, Frutiger Aero (Win Vista/7, Wii, Wii U, Pikmin 3), Frutiger Metro, Y2K (F-Zero GX), Vaporwave, Outrun/Synthwave, Neumorphism, Glassmorphism/Liquid Glass, Comic/Anime/LoFi, ... | 6 | | 
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
- Linux Support
  - Add  Linux API to PlatformBridge

### ToDos before milestone 5
- Update and FixedUpdate loops
- Text
  - UTF-8 support
  - Emoji support
  - Vertical text flow
  - Text Shaders with all shader options
  - Text Border
- Shaders
  - shadow
  - glow fog
  - cut corners
  - rename gradient fill to linear shift
  - radial shift
  - conic radiant shift
  - noise options
  - optimize shader handling
    - different IDs but only once compiled code
    - only pass parameters on initialzation and on parameter value change
- WindowBar
  - Positioning, left, right
  - Docking emblem
  - Icon
- Window
  - Docking
- Font Manager
- Cursor Icon Manager
- Animation Engine
- Test Carbon compatibility
- Add more widget interactions to the DynamicLibWidgetManager
  - SetRotation, SetPosition, SetSize
  - Set Color/Decoration parameters
- C++ modules once ALL common compilers (GCC, CLang, MSVC) support them
- Wiki/Documentation
- More text color properties
- Rename Grid2d to Grid and give it a 3rd dimension
- Rectangle, Text, Texture, IWidget: 3rd dimension
- Complete Python binding
- Refactor Binding

#### Nice-to-have's
- Better (relative) Viewports
- Custom text renderer with many IANA character sets

### Why?
There are many C++ GUI frameworks available, but most come with one or more limitations: restrictive licensing, enforced use of proprietary containers, inflexible window layouts, limited design capabilities, or poor cross-platform support—often confined to C++ only.
RetroFuturaGUI aims to break these barriers!

### Implemented Features
- Widgets
  - Label
    - Render text
    - Align Text
  - Button
    - Shadered background and borders (solid, animated gradients)
    - Corner Radii
    - Signals
      - onClick
      - whileHover
      - onMouseEnter
      - onMouseLeave
      - onRelease
    - SetPosition, SetSize, SetRotation
    - Image2D
      - SetPosition, SetSize, SetRotation
      - Supports most image formats
    - Grid2D
      - Align Widgets
      - Widget Size Policies (const size, expand X, Y, X/Y)
      - Debug Lines
      - Row Definitions
      - Column Defitions
  - WindowBar
    - Top, Bottom Position
    - Close, Minimize, Maximize (all shadered)
    - Window Title
    - Shadered background
  - Window
    - Background image
      - Background image ID can be used to create glass effects on widgets
      - Resizeable
      - Movable
- Shaders
  - Solid Fill
    - Glass Effect
    - Rounded corners
  - Gradient Fill
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
  - Texture
  - Shader Manager
- Input Manager
- Cross Language Capabilities
  - Widget ID Manager
  - C#
- Text Renderer
  - Basic Text Renderer (ASCII)
  - Solid color fill

### Known Bugs
- Window.hpp/.cpp
  - glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE()) causes objects with transparency to show what's behind the window even if the background is completely opaque