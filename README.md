# RetroFuturaGUI

RetroFuturaGUI is a signal-slot-based GUI framework written in C++. It is designed to maximize flexibility across multiple platforms. By compiling the framework's logic to a dynamic library, it is possible to use the framework with other languages as well. Last but not least, RetroFuturaGUI brings back many aesthetics of the past, as well as the present and future; hence its name.

## Roadmap

| # | Name | Depends on | Current State |
|------|-----|------------| ---- |
| 1 | Button, Label, Window, MainWindow, Image, Grid2D, WindowBar with Buttons |  | WIP |
| 2 | linux support | 1 | | 
| 3 | dll/so/dylib compilation for C# and Python support | 1 | | 
| 4 | More Widgets (TextBox, Table, VideoPlayer, AudioPlayer, 3D Model, Slider, CheckBox, DropdownBox, RadioButton, Tabs, 3D Scene, Lights, Grid3D, Carousel, Color Pickers) | 1 | | 
| 5 | .bechaml 🥣 (**B**ased and **E**xtra **C**reamy **H**ollandaise **A**pplication **M**arkup **L**anguage) | 4 | | 
| 6 | VS Code extension with project generator/manager | 5 | | 
| 7 | Aesthetics: Flat Metro, Frutiger Aero (Win Vista/7, Wii, Wii U, Pikmin 3), Frutiger Metro, Y2K (F-Zero GX), Vaporwave, Outrun/Synthwave, Neumorphism, Glassmorphism/Liquid Glass, Comic/Anime/LoFi, ... | 6 | | 
| 8 | Android (w/o Android Studio), MacOS, iOS support | 6 | | 
| 9 | dll/so/dylib compilation for C, Rust, Zig, Java, JavaScript, TypeScript, Go, Ruby, Lua, Swift, Carbon, Gleem (if it has evolved enough) | 6 | | 
| 10 | gcn and wii support (devkitpro/libogc) | 6 | | 
| 11 | XBox series Support (all XBox consoles, not just the Series series, du'h) (DirectX), Direct X for Windows | 6 | | 
| 12 | Dreamcast support | 6 | | 
| 13 | Wii U support  | 6 | | 
| 14 | 3DS support | 6 | | 
| 15 | Switch support | 6 | | 
| 16 | Playstation series support | 6 | | 


### WIP in detail
- Text
  - Vertical text flow
  - UTF-8 support
  - Emoji support
- WindowBar
  - Positioning to bottom, left, right
- Better Viewports mayhaps

### Known Bugs
- Window.hpp/.cpp
  - glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE()) causes objects with transparency to show what's behind the window even if the background is completely opaque