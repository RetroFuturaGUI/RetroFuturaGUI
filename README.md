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
| 4 | More Widgets (TextBox ✅, Table ✅, VideoPlayer, AudioPlayer, 3D Model ✅, Slider ✅, CheckBox ✅, ComboBox ✅, ExtendedComoBox ✅, RadioButton ✅, RadioButtonGroup ✅, SeparatorLine ✅, Tabs, Lights, change Grid2d to "Lasagna" and add a 3rd dimension ✅, Color Pickers, MenuBar, Environment), Scene, HUD  | 1 | WIP | 
| 5 | .bechaml markup language for GUI design 🥣 (**B**eautifully **E**xtended **C**ascading but **H**airbally **A**pplication **M**arkup **L**anguage) | 4 | | 
| 6 | VS Code extension with project generator/manager | 5 | | 
| 7 | Pre-built Prefabs (StepperSlider, SpinBox, Table with Sliders, Carousel, Extended Color Pickers) | 6 |
| 8 | Interoperability API Exporter | 7 |  |
| 9 | Aesthetics: Flat Metro, Frutiger Aero (Win Vista/7, Wii, Wii U, Pikmin 3), Frutiger Metro, Y2K Futurism (F-Zero GX), Vaporwave, Outrun/Synthwave, Neumorphism, Glassmorphism/Liquid Glass, Constructivism, Comic/Anime/LoFi, Bauhaus, ... | 8 | | 
| 10 | Android (without Android Studio), MacOS, iOS support | 8 | | 
| 11 | dll/so/dylib compilation for C, Rust, Zig, Java, JavaScript, TypeScript, Go, Ruby, Lua, Swift, Carbon, Gleam | 8 | | 
| 12 | gcn and wii support (libogc (devkitPro)) | 8 | | 
| 13 | XBox series Support (all XBox consoles, not just the Series series, du'h) (DirectX), Direct X for Windows | 8 | | 
| 14 | Dreamcast support (OpenGL 1.5 + PVR APi) | 8 | | 
| 15 | Wii U support (GX2 (devkitPro)) | 8 | | 
| 16 | 3DS support (citro3d/citro2d (devkitPro)) | 8 | | 
| 17 | Switch support (deko3d (devkitPro)) | 8 | | 
| 18 | PS2 (ps2gl), PS3 (RSXGL), PS4 (OpenOrbis/GNMX/GNM) support | 8 | | 

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
- Table
  - More cell types (TableImage, TableProgressBar); TableText, TableColor and TableCheckBox exist so far
  - Connect_/Disconnect_OnColorChange and OnCheckBoxChange: both signals are emitted when a cell's value changes but have no public connector yet
  - Checkbox inner padding (the gap between the box' border and its checkmark) is stored but not applied yet, so the checkmark fills the whole box
  - Auto track sizing (measure a track against its content); currently resolves like Fixed
  - Row/column spanning (the cells carry the spans already, the layout doesn't apply them yet)
  - Draw the header band's outer border (the width and colors are configurable but unused)
  - Keyboard traversal between cells (Tab); the arrow keys move the caret within a cell
  - Clipping is an axis-aligned scissor, so it stops being exact once the table carries a rotation
- Slider / ProgressBar
  - The graph doesn't account for the track's border width, so an enabled graph paints over the frame; the indicator does account for it
  - A Circle indicator takes its corner radius from the indicator's x size alone, so it draws as a rounded rectangle whenever the two axes differ
- IDropDown (shared base of ComboBox and ExtendedComboBox)
  - _borderColorsEnabled/Disabled/Hover/Clicked and _backgroundColorsEnabled/Disabled/Hover/Clicked are never read or written. The colors that actually reach the closed box come from IBorder's _borderColorEnabled and IBackground's _backgroundColorEnabled - one letter apart - so assigning the IDropDown ones looks right and changes nothing
  - The per-row border (_dropDownItemBorder) is declared but never built or drawn
  - Most of the protected surface is only touched inside IDropDown.cpp and could be private: setDropDownArrowColors, setDropDownBackgroundColors, setDropDownBorderColors, dropDownCenter, rotateLocalOffset, the three per-state drop-down color sets, _highLightColor, _dropDownArrowPathFill, the three ColorState fields, _zOffset, _arrowPadding and _maxItemsToDraw - subclasses reach all of them through the public setters and setColors
  - _dropDownBackgroundColorSate and _dropDownBorderColorSate are missing a t
- ComboBox
  - Scrolling past the visible row cap; the list shows the first rows only, up to the cap
  - SetIndex neither clamps to the item count nor emits OnIndexChanged - only picking a row does
  - No SetEnabled override, so the disabled colors it carries never reach the elements
  - RemoveItem doesn't range-check its index and leaves the selected index pointing at whatever moved into that slot
  - Only the drop-down's corner radii are exposed (SetDropDownCornerRadii); the closed box has no public setter, and neither the panel's nor the arrow's fill type can be chosen, so their gradient fills are out of reach
  - The visible row cap is fixed at 8 with no setter
  - In the closed box the selected item's text can run under the arrow icon; only the border width is kept clear of it
- Prefab
  - Children aren't registered with the DynamicLibWidgetManager, so a binding can't address them by string ID yet. That needs a deregistration path as well, or destroying a prefab would leave the manager holding freed pointers
  - The Lasagna is fixed at construction; a prefab keeps whatever AxisDefinition it was built with
- Scene
  - A Scene doesn't own its widgets yet. Lasagna::AttachWidget only arranges what it is handed, and the generated _p struct still holds the unique_ptrs, so destroying a Scene would leave its widgets alive with their layout gone from under them. Dynamic loading needs that ownership moved into the Scene first - along with Prefab's name-to-child lookup, which would then exist in two places and is worth factoring out instead of copying
  - No SceneManager: nothing loads, unloads or holds scenes, and nothing owns the assets they share. Once it exists it has to be the single asset cache rather than a second one beside ResourceManager, and widgets need a way to reach it - they currently load their own textures, SVGs and models straight from a path in their constructors
  - No lifecycle signals (OnActivate, OnDeactivate, OnLoad, OnUnload), so application code has nothing to hook a scene transition to
  - Overlay scenes don't block the scene underneath them. Widgets hit-test inside their own Draw, so there is no central pass that could stop at the topmost hit the way a raycast would. Docking avoids it geometrically - a content scene is never given the strip, so it has nothing there to click - but a modal overlay needs the scene beneath it disabled, passing emitSignal = false to SetEnabled so the sweep doesn't fire every connected slot
  - Only a Lasagna can be the root container. That generalizes to an IContainer interface once ScrollView and Environment exist
  - SetLasagnaAxis takes its AxisDefinition by value, copying three vectors per call
- SeparatorLine
  - The caption's left padding isn't clamped to the line's width, so a long caption or a large padding runs the gap and its text off the right end
  - The gap always starts from the left edge, so centering or right-aligning a caption means working out the padding by hand. SetTextAlignment doesn't do it either: the caption is always centered inside its own gap, and the alignment passed in is overwritten the next time the layout runs
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
  - SeparatorLine
    - A rule for dividing content, with an optional caption set into it
    - Caption: font family, size, slant and weight, text padding, and distance from the line's left edge (SetTextLeftPadding), shown or hidden with ShowText
    - While the caption is shown the line carries a background gap sized to hold it, so the rule breaks around the text rather than running underneath it. Hiding the caption or clearing its text closes the gap again
    - The caption is clipped to whichever is taller, the line or the text itself, so a rule only a few pixels thick still draws its glyphs in full
    - Background (same options as Button)
    - SetPosition, SetSize, SetRotation
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
  - ComboBox
    - Picks one entry out of a list of text items: AddItem, RemoveItem, GetItemCount, SetIndex, GetSelectedIndex, GetSelectedText
    - Drop-down panel hanging off the bottom edge of the closed box, as tall as the item count up to a cap of visible rows, with its own per-state background and border colors and corner radii
    - Clicking the box opens and closes the list, clicking a row selects it and closes it, a click anywhere outside puts it away
    - The row under the cursor is highlighted, and the hit test walks the same row rectangles the renderer places, so what lights up is what gets picked
    - Arrow icon (shared SVG) with per-state colors, fitted to the icon's own aspect ratio inside the right edge of the box
    - Item text: font family, size, slant and weight, alignment and padding, inset by the border width so the frame never crosses a glyph
    - Signals: OnItemClicked, OnIndexChanged, OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation: every part sits around the widget's center and its offset is rotated with the widget, so the open list stays attached to a rotated box
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
    - Horizontal and Vertical orientation: the value always runs along the track's local x-axis, so a vertical slider is a horizontal one turned a quarter turn, while its own rotation stays at whatever the caller set. SetSize takes the on-screen footprint either way, so a layout container can size a vertical slider without knowing about the turn
    - Track direction (Normal/Inverted) chooses which end of the track holds the minimum, for values that count the opposite way to the track - a scrollbar's offset, for instance. Affects where the indicator and graph are drawn, not the value
    - Drag the indicator or click anywhere on the track to set the value
    - Step the value by a configurable step size, in whatever type the value currently holds
    - Indicator: Stroke or Circle type, sized per axis in pixels or percent of the track, per-state colors (Enabled, Disabled, Clicked, Hover), Solid/Linear/Radial/HueStar Gradient fill, Dotted Pattern, Fog Effect, corner radii, border width and border gaps. Sized and positioned inside the track's border rather than over it
    - Graph (the filled part of the track): Bar or Wave mode, per-state colors, width, all fill types, corner radii
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - ProgressBar
    - Shares the Slider's value, indicator, graph and stepping machinery (IRangedValue) without the dragging
    - Horizontal and Vertical orientation, Normal or Inverted track direction
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - Table
    - Grid of typed cells, each created on demand the first time a value is assigned to it. The cell type follows from what is assigned: a string or a number makes a TableText, a color makes a TableColor, a bool makes a TableCheckBox
    - TableText cells: editable text, one Text mesh per cell (see the text interaction entry below)
    - TableColor cells: a solid color swatch. The cell stores only its color value and draws through a single rectangle shared by the whole table, so a color column costs one set of GL buffers no matter how many rows it has
    - TableCheckBox cells: a checkbox sized as the largest centered square that fits the cell minus a configurable margin, so it keeps its shape as the tracks resize. The cell stores only its checked state and draws through one background rectangle, one border rectangle and one SVG checkmark shared by the whole table, so a checkbox column costs one set of GL buffers regardless of row count
    - Checkbox interaction: the click target is the box itself rather than the surrounding cell, with hover and pressed states tracked per cell. Per-state colors (Enabled, Disabled, Hover, Clicked) for background, border and checkmark, plus corner radii and border width
    - Per-track sizing policies: Star (divides whatever viewport space the fixed tracks left over) and Fixed (absolute pixels, the mode that lets content outgrow the table and become scrollable)
    - Scrolling on both axes, with content extent, maximum scroll and scroll position exposed so an external Slider can drive it through the signal/slot mechanism
    - Only the tracks overlapping the viewport are drawn; partially visible ones are clipped by a scissor that intersects with whatever clip is already active, so a half-scrolled row can't spill past the border
    - Alternating track coloring with any number of color variants, banding by row or by column (TableOrientation)
    - Per-variant background, inner border and text colors
    - Column and row headers, placeable Top/Bottom and Left/Right, with their own font, alignment, padding, colors and border widths. The band is taken out of the content viewport and stays pinned while the content scrolls underneath it
    - Text interaction per cell: click to place the caret, drag to mark, arrow keys with key repeat, select all, copy / cut / paste, backspace. One caret and one selection highlight are shared by every cell, reused by whichever one currently has focus
    - Read-only tracks: marking and copying still work, only the edits are rejected. A checkbox in a read-only track still shows its value but won't toggle
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
    - Row, Column and Layer definitions, checkable before a Lasagna is built from them (non-empty, positive track sizes, within the per-axis track limit) - which matters for definitions arriving from another language
    - Row, Column and Layer spanning (a widget can occupy multiple cells along any axis)
    - AttachWidget reports whether the placement succeeded, so an out-of-range or already-occupied cell fails visibly instead of silently
    - Cell lookup by TrackIndex, and the current track count per axis
    - Debug lines
  - Prefab
    - A group of widgets laid out in its own Lasagna and treated as a single widget by whatever contains it
    - Composed, not subclassed: children are added with AttachWidget&lt;T&gt;(name, placement), so a prefab is a tree of widgets rather than a new C++ type per kind. A subclass per kind would have to expose its children through the C ABI, which addresses widgets by string and cannot hand back a pointer, so every prefab would grow the binding surface
    - Children are reached by name afterwards: GetChildWidget&lt;T&gt;, ShowChildWidget, IsChildWidgetShown
    - Owns its children; refuses a name that is already taken, and a cell that is out of range or occupied, without keeping the widget it was asked to build
    - SetPosition, SetSize, SetRotation, forwarded to its Lasagna
  - WindowBar
    - Top, Bottom Position
    - Close, Minimize, Maximize buttons (all shadered)
    - Toggleable elements (buttons, title, background, no-docking-drag zone)
    - Window Title
    - Shadered background
    - Per-button styling (background/border colors, gradients, corner radii, border width)
    - Maximize callback
    - Reserves space across the edge it sits on so content is laid out beside it rather than underneath it, or overlays the content instead (Window::SetWindowBarOverlaps) for designs where content is meant to run under the bar
  - Window
    - Background color or image
      - Background image ID can be used to create glass effects on widgets
    - Resizeable
    - Movable
    - Toggleable WindowBar
    - Holds an ordered list of Scenes it draws over its root Lasagna and under its WindowBar (AddScene, RemoveScene), borrowed rather than owned
    - Divides its surface into a client area: the WindowBar and then each docked Scene take their strip, and whatever is left goes to the overlay Scenes and the root Lasagna. Recomputed on resize, on adding or removing a Scene, and whenever a Scene changes what it reserves or whether it is active
- Scenes
  - Scene
    - A named group of widgets with its own root Lasagna, hanging under a Window: Window / Scene / Container / Widgets. Meant for scene loading and for HUDs
    - Deliberately not an IWidget, only an IHierarchyNode. A scene has no geometry, no hit-testing and no color state of its own, and staying off IWidget keeps it out of Lasagna::AttachWidget - so a Scene can never end up nested inside a cell, and the hierarchy stays at a fixed depth
    - Contributes its name to its widgets' paths, so GetPath composes MainWindow/MyScene/RootLasagna/TestButton for the interoperability API and .bechaml to address
    - Active or Inactive (SetActive), which is a different axis from a widget's Enabled or Disabled: Active decides whether the scene draws at all, Enabled decides whether a widget reacts and which ColorState it renders in. Since widgets hit-test inside their own Draw, an inactive scene stops responding as well as drawing. A disabled widget still draws - so an active scene full of disabled widgets is exactly the dimmed-behind-a-modal case, and a Scene needs no separate visibility flag
    - Docks to an edge of the client area (SetReservedEdge with a DockEdge), reserving a strip in pixels that everything laid out after it is fitted around, so a menu bar or status bar can't hide or swallow clicks meant for the content behind it. ClearReservedEdge turns it back into an overlay
    - Overlay is the default: the scene covers the whole client area and reserves nothing, for floating panels and modals
    - Docked scenes reserve in the order they were added, so the first one to claim an edge also owns the corner where two claims meet
    - An inactive scene reserves nothing, and is laid out again when it is reactivated, so activating one never shows a layout computed for a different window size
    - SetPosition, SetSize, SetRotation, forwarded to its root Lasagna
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
  - Background Gaps (combinable, works on Solid Fill and Linear/Radial/HueStar Gradient; the Border variants carry their own Border Gaps instead)
    - Skips sections of the fill in a repeating solid/gap pattern: no gap, a single one, a set number of them, or tiled across the whole element
    - Offset and length are absolute pixels measured from the edge the pattern starts at, so each segment keeps its size and its distance from that edge as the element resizes, rather than stretching with it
    - The pattern runs along X and can be rotated to any angle, so the same definition gives vertical stripes, horizontal bands or anything between
    - A background is one element rather than four edges, so a single definition covers it - unlike Border Gaps, which describe one edge at a time
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
- SvgTexture.cpp
  - Draw() switches the depth test off for its whole draw so that its coplanar mask passes can blend over the base layer. That also lets any SVG drawn later in the frame paint over whatever is already there, whatever the z - an open ComboBox drop-down is covered by an SvgImage sitting in a later Lasagna cell, for instance. Keeping the test on and using GL_LEQUAL for the mask passes would keep the blending and respect depth
- TextBox.cpp
  - A TextBox reports WidgetTypeID::Button, so DynamicLibWidgetManager::SetText and ConnectSlot take the Button branch, dynamic_cast to Button* yields null and is then dereferenced. The WidgetTypeID::TextBox branches are unreachable as a result

### Is AI used in this project?
AI is often used for repititive tasks like adding triple-slash comments to functions, classes, and structs and updating the readme. Shaders are mostly written by AI and AI is sometimes used for finding bugs and to assist with complicated calculations.