# RetroFuturaGUI

<img width="1280" height="722" alt="demo" src="https://github.com/user-attachments/assets/5d6a6bc7-6c49-4432-8ad6-d2fa5d9db9fd" />


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

### Implemented Features
<details><summary>CLICK TO EXPAND</summary>
  <ul>
<li>Widgets</li>
  <ul>
  <li>Label</li>
    <ul>
    <li>Render text</li>
    <li>Align Text</li>
      <ul>
        <li>Left</li>
        <li>Right</li>
        <li>Center</li>
    </ul>
  <li>Button</li>
      <ul>
        <li>Slots</li>
        <ul>
          <li>OnClick</li>
          <li>OnRelease</li>
          <li>OnMouseEnter</li>
          <li>OnMouseLeave</li>
          <li>WhileHover</li>
          <li>SetEnabled</li>
        </ul>
        <li>Background</li>
        <ul>
          <li>Solid Fill</li>
          <li>Linear Gradient</li>
          <li>Radial Gradient</li>
        </ul>
        <li>Borders</li>
        <ul>
          <li>Solid Fill</li>
          <li>Linear Gradient</li>
          <li>Radial Gradient</li>
          <li>Set Corner Radii</li>
          <li>Set Corner Width</li>
        </ul>
      </ul>
  </ul>
    <li>Shadered background and borders (solid, animated gradients)</li>
    <li>Corner Radii</li>
    <li>Signals</li>
      <ul>
      <li>onClick</li>
      <li>whileHover</li>
      <li>onMouseEnter</li>
      <li>onMouseLeave</li>
      <li>onRelease</li>
      <li>setEnable</li>
      </ul>
    <li>SetPosition, SetSize, SetRotation</li>
    <li>Image2D</li>
      <ul>
      <li>SetPosition, SetSize, SetRotation</li>
      <li>Supports most image formats</li>
      </ul>
    <li>Grid2D</li>
      <ul>
      <li>Align Widgets</li>
      <li>Widget Size Policies (const size, expand X, Y, X/Y)</li>
      <li>Debug Lines</li>
      <li>Row Definitions</li>
      <li>Column Defitions</li>
      </ul>
  <li>WindowBar</li>
    <ul>
    <li>Top, Bottom Position</li>
    <li>Close, Minimize, Maximize (all shadered)</li>
    <li>Window Title</li>
    <li>Shadered background</li>
    </ul>
  <li>Window</li>
    <ul></ul>
    <li>Background image</li>
    <ul>
      <li>Background image ID can be used to create glass effects on widgets</li>
      <li>Resizeable</li>
      <li>Movable</li>
  </ul>
</ul>
<li>Shaders</li>
  <ul>
  <li>Solid Fill</li>
    <ul>
    <li>Glass Effect</li>
    <li>Rounded corners</li>
    </ul>
  <li>Linear Gradient</li>
  <ul>
    <li>Animated (rotation, motion)</li>
    <li>up to 256 colors (RGBA)</li>
    <li>Glass Effect</li>
    <li>Rounded corners</li>
  </ul>
  <li>Radiant Gradient</li>
  <ul>
    <li>Animated (rotation, motion)</li>
    <li>up to 256 colors (RGBA)</li>
    <li>Glass Effect</li>
    <li>Rounded corners</li>
  </ul>
  <li>Texture</li>
  <li>Shader Manager</li>
  </ul>
<li>Input Manager</li>
<li>Cross Language Capabilities</li>
  <ul>
  <li>Widget ID Manager</li>
  <li>C#</li>
  <li>Python</li>
  </ul>
<li>Text Renderer</li>
<ul>
  <li>Basic Text Renderer (ASCII)</li>
  <li>Solid color fill</li>
</ul>
</ul>
</details>
  


### ToDos before milestone 5
<details><summary>CLICK TO EXPAND</summary>
  <ul>
<li>Update and FixedUpdate loops</li>
<li>Text</li>
  <ul>
  <li>UTF-8 support</li>
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
<li>Rename Grid2d to Grid and give it a 3rd dimension</li>
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
