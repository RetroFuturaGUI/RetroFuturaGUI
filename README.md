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
  - SetParentSize doesn't rebuild the mesh, so glyph clipping goes stale when a widget is resized after its text was set
- TextBox
  - suppress text overflow
- Button
  - suppress text overflow
- Table
  - More cell types (TableImage, TableCheckBox, TableProgressBar); TableText and TableColor exist so far
  - Connect_/Disconnect_OnColorChange: the signal is emitted when a cell's color changes but has no public connector yet
  - Auto track sizing (measure a track against its content); currently resolves like Fixed
  - Row/column spanning (the cells carry the spans already, the layout doesn't apply them yet)
  - Draw the header band's outer border (the width and colors are configurable but unused)
  - Keyboard traversal between cells (Tab); the arrow keys move the caret within a cell
  - Clipping is an axis-aligned scissor, so it stops being exact once the table carries a rotation
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
  - CheckBox
    - Represents a boolean value
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - SetEnabled, SetValue, GetValue
    - Checkmark (SVG) with per-state colors (Enabled, Disabled, Hover, Clicked), Solid/Linear/Radial/HueStar Gradient fill
    - Inner fill field with per-state colors, Solid/Linear/Radial/HueStar Gradient fill, gradient offset/degree/animation & rotation speed
    - Optional binding to an external bool (UseInherietedValue + SetInheritValueReference) so the CheckBox mirrors a value owned elsewhere
    - Inner padding (distance between border and checkmark)
    - Background & Border (same options as Button)
    - Corner radii
    - SetPosition, SetSize, SetRotation
  - RadioButton
    - A single option belonging to a RadioButtonGroup
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - SetEnabled, SetValue, GetValue
    - Indicator field with per-state colors (Enabled, Disabled, Hover, Clicked), Solid/Linear/Radial/HueStar Gradient fill, gradient offset/degree/animation & rotation speed
    - Indicator padding (distance between border and indicator)
    - Background & Border (same options as Button)
    - Corner radii
    - SetPosition, SetSize, SetRotation
  - RadioButtonGroup
    - Owns a set of RadioButtons and enforces mutual exclusivity, deselecting all others when one is selected
    - Register/Unregister RadioButtons dynamically, each paired with a Label
    - Grid layout: row/column axis definitions, with each cell holding a RadioButton + Label pair
    - Per-cell content alignment (Left, Right, Center) and content padding
    - Group text label (shares Label's text properties via ITextProperties)
    - Border (same options as Button), with configurable border gaps
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
  - Slider
    - Represents a numeric value of any type (Bool, Int8-64, UInt8-64, Float32/64) with configurable min/max
    - Horizontal and Vertical orientation: the value always runs along the track's local x-axis, so a vertical slider is a horizontal one turned a quarter turn, while its own rotation stays at whatever the caller set
    - Drag the indicator or click anywhere on the track to set the value
    - Indicator: Stroke or Circle type, sized in pixels or percent of the track, per-state colors (Enabled, Disabled, Clicked, Hover), Solid/Linear/Radial/HueStar Gradient fill, Dotted Pattern, Fog Effect, corner radii, border width and border gaps
    - Graph (the filled part of the track): Bar or Wave mode, per-state colors, width, all fill types, corner radii
    - Optional increment/decrement buttons with a configurable step size
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - ProgressBar
    - Shares the Slider's value, indicator and graph machinery (IRangedValue) without the dragging and buttons
    - Horizontal and Vertical orientation
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - Table
    - Grid of typed cells, each created on demand the first time a value is assigned to it. The cell type follows from which SetTableWidget overload is called: a string makes a TableText, a color makes a TableColor
    - TableText cells: editable text, one Text mesh per cell (see the text interaction entry below)
    - TableColor cells: a solid color swatch. The cell stores only its color value and draws through a single rectangle shared by the whole table, so a color column costs one set of GL buffers no matter how many rows it has
    - Per-track sizing policies: Star (divides whatever viewport space the fixed tracks left over) and Fixed (absolute pixels, the mode that lets content outgrow the table and become scrollable)
    - Scrolling on both axes, with content extent, maximum scroll and scroll position exposed so an external Slider can drive it through the signal/slot mechanism
    - Only the tracks overlapping the viewport are drawn; partially visible ones are clipped by a scissor that intersects with whatever clip is already active, so a half-scrolled row can't spill past the border
    - Alternating track coloring with any number of color variants, banding by row or by column (TableOrientation)
    - Per-variant background, inner border and text colors
    - Column and row headers, placeable Top/Bottom and Left/Right, with their own font, alignment, padding, colors and border widths. The band is taken out of the content viewport and stays pinned while the content scrolls underneath it
    - Text interaction per cell: click to place the caret, drag to mark, arrow keys with key repeat, select all, copy / cut / paste, backspace. One caret and one selection highlight are shared by every cell, reused by whichever one currently has focus
    - Read-only tracks: marking and copying still work, only the edits are rejected
    - Signals: OnTextChange, OnEnterPressed, OnEnterReleased, OnCopy, OnPaste
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation
  - Image
    - Supports most image formats
    - Fit widget geometry to the texture's native aspect ratio
    - SetPosition, SetSize, SetRotation
  - SvgImage
    - Loads and rasterizes SVG vector graphics (lunasvg)
    - Fit widget geometry to the SVG's intrinsic aspect ratio
    - Per-path runtime recoloring of named SVG paths (Solid, Linear, Radial, HueStar Gradient) without re-rasterizing
    - SetPosition, SetSize, SetRotation
  - Model
    - Load 3D models and their material textures (Assimp)
    - Multiple meshes per model
    - SetPosition, SetSize, SetRotation
  - Lasagna (three-dimensional successor of Grid)
    - Align widgets in a three-dimensional pattern
    - Widget sizing policies (fixed size, expand X, Y, Z, XY)
    - Row, Column and Layer definitions
    - Row, Column and Layer spanning (a widget can occupy multiple cells along any axis)
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
    - Dotted Pattern
    - Fog Effect
  - Linear Gradient
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
    - Dotted Pattern
    - Fog Effect
  - Radial Gradient
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
    - Dotted Pattern
    - Fog Effect
  - HueStar Gradient
    - Animated (rotation, motion)
    - Glass Effect
    - Rounded corners
    - Dotted Pattern
    - Fog Effect
  - Dotted Pattern (combinable overlay, works on every fill and border shader above)
    - Per-dot radius sampled from a growable/shrinkable curve along a configurable direction
    - Seamless, endless scrolling animation along that direction
    - Configurable dot color, spacing, and opacity falloff (hard edge to soft center-only glow)
    - Alpha-blends over whatever the fill/border/glass effect already rendered; clipped by rounded corners
  - Fog Effect (combinable overlay, works on Solid Fill and Linear/Radial/HueStar Gradient; not the Border variants)
    - Dynamic, irregular cloud-like density from a multi-octave fractal noise field
    - Configurable overall opacity, drift speed, per-octave density/weight curve, and clearing threshold (how much clear/fog-free area shows through)
    - Alpha-blends over whatever the fill/dotted pattern already rendered; clipped by rounded corners
  - Dedicated Border variants of the above (Solid, Linear, Radial, HueStar), including Dotted Pattern
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
