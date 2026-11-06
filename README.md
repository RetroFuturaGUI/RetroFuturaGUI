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
| 4 | More Widgets (TextBox ✅, Table ✅, 3D Model ✅, Slider ✅, CheckBox ✅, ComboBox ✅, ExtendedComoBox ✅, RadioButton ✅, RadioButtonGroup ✅, SeparatorLine ✅, ColorPreview ✅, change Grid2d to "Lasagna" and add a 3rd dimension ✅, MenuBar ✅, MultilineTextBox ✅, Histogram ✅, VideoPlayer ✅, Environment ✅, LineDiagram ✅, ScrollView ✅, Color Pickers ✅, WindowTab ✅, window docking ✅, ContextMenu), Scene ✅, SceneLoader ✅, AudioPlayback ✅, Popups ✅, Tooltip ✅, FileExplorer ✅, Big Refactor, Context Menu, TabView | 1 | WIP | 
| 5 | .bechaml markup language for GUI design 🥣 (**B**eautifully **E**xtended **C**ascading but **H**airbally **A**pplication **M**arkup **L**anguage), ColorManager, CornerRadiiManager, DropShadow and InnerShade (Text, Border, Background) | 4 | | 
| 6 | VS Code extension with project generator/manager | 5 | | 
| 7 | Pre-built Prefabs (StepperSlider, SpinBox, Table with Sliders, Carousel, Extended Color Pickers, MediaPlayer, LabeledCheckBox) | 6 |
| 8 | Interoperability API Exporter | 7 |  |
| 9 | Aesthetics: Flat Metro, Frutiger Aero (Win Vista/7, Wii, Wii U, Pikmin 3), Frutiger Metro, Y2K Futurism (F-Zero GX), Vaporwave, Outrun/Synthwave, Neumorphism, Glassmorphism/Liquid Glass, Constructivism, Comic/Anime/LoFi, Bauhaus, ... | 8 | | 
| 10 | Android (without Android Studio), MacOS, iOS support | 8 | | 
| 11 | dll/so/dylib compilation for C, Rust, Zig, Java, JavaScript, TypeScript, Go, Ruby, Lua, Swift, Carbon, Gleam, Goose | 8 | | 
| 12 | gcn and wii support (libogc (devkitPro)) | 8 | | 
| 13 | XBox series Support (all XBox consoles, not just the Series series, du'h) (DirectX), DirectX for Windows | 8 | | 
| 14 | Dreamcast support (OpenGL 1.5 + PVR APi) | 8 | | 
| 15 | Wii U support (GX2 (devkitPro)) | 8 | | 
| 16 | 3DS support (citro3d/citro2d (devkitPro)) | 8 | | 
| 17 | Switch support (deko3d (devkitPro)) | 8 | | 
| 18 | PS2 (ps2gl), PS3 (RSXGL), PS4 (OpenOrbis/GNMX/GNM) support | 8 | | 
|    | Environment Lights, Node | | |

### WIP in detail
- Font Manager
  - Improve font loading times on Windows (PlatformBridge)
- Update all Input to use PlatformBridge's implementation
- Table
  - More cell types (TableProgressBar)
  - Connect_/Disconnect_OnColorChange and OnCheckBoxChange: both signals are emitted when a cell's value changes but have no public connector yet
  - Changing column widths and row heights by hand: dragging the line between two header cells resizes that track, down to a minimum, and a double-click on it fits the track to its content once Auto sizing exists.
  - Draw the header band's outer border (the width and colors are configurable but unused)
  - Keyboard traversal between cells (Tab); the arrow keys move the caret within a cell
  - A drag of the selected rows reports the pointer (OnSelectionDrag), but the table can't mark the row a drop would land on yet
- Slider / ProgressBar
  - The graph doesn't account for the track's border width, so an enabled graph paints over the frame; the indicator does account for it
  - A Circle indicator takes its corner radius from the indicator's x size alone, so it draws as a rounded rectangle whenever the two axes differ
- LineDiagram
  - The lines are only spaced by index across the longest set. Sets that cover the same span at different resolutions (32 and 64 frequency bands, say) want a mode that stretches every line across the full width
  - Labels and the hover text take the track's rotation, so with Orientation::Vertical they turn with the lines and read sideways
  - Where points sit closer together than the line is wide, the inner side of a mitered corner can fold back over the previous segment
  - A translucent line blends twice where a bevel overlaps its segments at a sharp turn - Polyline draws with GL_LEQUAL so the overlap isn't rejected by the depth test
  - The pointer is the inherited indicator, which IRangedValue switches on without ever applying its colors, so it stays invisible until ShowIndicator(true) or SetIndicatorBackgroundColors is called
  - No signal tells the application which line or data point is hovered
  - Labels always take their line's color; there is no label color of their own
- IDropDown (shared base of ComboBox and ExtendedComboBox)
  - _borderColorsEnabled/Disabled/Hover/Clicked and _backgroundColorsEnabled/Disabled/Hover/Clicked are never read or written. The colors that actually reach the closed box come from IBorder's _borderColorEnabled and Background's _colorsEnabled - one letter apart - so assigning the IDropDown ones looks right and changes nothing
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
  - Shared, cross-window scenes. SceneLoader::CreateScene hands back a std::unique_ptr, so a scene is one instance belonging to one window, and "the same scene in two windows" means instantiating the same description twice. Returning a shared scene is planned and needs three things: windows created with a shared GL context (done: every window shares textures, buffers and shaders with the first one created), per-context VAOs over shared buffers (done: the spec never shares VAOs or framebuffers, so VertexArray and Framebuffer keep one for each context that draws the object), and splitting shared state from per-window view. Consequence until then: a 3D Model changed in one window does not propagate to another - they are separate objects with separate transform matrices. For 2D widgets sharing is incoherent anyway, because Window::UpdateLayout writes screen-space positions into the widgets and two differently sized windows need different values in the same fields; for 3D content it is coherent, and the shape it wants is one world with a camera per window, which belongs to the planned Environment container rather than to Scene
  - Widgets are owned by the generated _p struct, not by the Scene, and the Scene owns only its root container. Closing a scene therefore has to tear down its owner's entry too - SceneLoader registers scenes and borrows a pointer, but whoever holds the unique_ptr owns the lifetime, so CloseScene needs a release hook to make closing actually complete rather than just unregistering
  - SceneLoader owns no assets yet. ResourceManager holds the textures and SVG texts loaded from files, so assets a SceneLoader prepares have to go through it rather than into a second cache beside it. Models still load straight from a path in their constructors
  - Loading is synchronous. SceneLoadingProgress always reports 1.0 for a scene that exists, and becomes meaningful only once asset loading is split into an off-thread prepare step and a main-thread realize step - every gl* call has to stay on the context thread, and destruction can never leave it at all
  - No lifecycle signals (OnActivate, OnDeactivate, OnLoad, OnUnload), so application code has nothing to hook a scene transition to
  - Overlay scenes don't block the scene underneath them. Widgets hit-test inside their own Draw, so there is no central pass that could stop at the topmost hit the way a raycast would. Docking avoids it geometrically - a content scene is never given the strip, so it has nothing there to click - but a modal overlay needs the scene beneath it disabled, passing emitSignal = false to SetEnabled so the sweep doesn't fire every connected slot
  - The root container is one of a fixed set - Lasagna, Environment, ScrollView or WindowTab, through the RootContainer concept. An IContainer interface would open it to any container
  - SetLasagnaAxis takes its AxisDefinition by value, copying three vectors per call
- SeparatorLine
  - The caption's left padding isn't clamped to the line's width, so a long caption or a large padding runs the gap and its text off the right end
  - The gap always starts from the left edge, so centering or right-aligning a caption means working out the padding by hand. SetTextAlignment doesn't do it either: the caption is always centered inside its own gap, and the alignment passed in is overwritten the next time the layout runs
- ColorPreview
  - The checkerboard's two colors and its square size are fixed internals, so neither the board's contrast nor its scale can be set from outside
  - The dual preview is on by default, so a preview that was never asked to be split is
  - Against the light checkerboard the white half reads almost the same as the checkered one - compositing over white and over a white/light-grey board differ only inside the grey squares. Drawing the color at full opacity there instead would carry information the checkered half can't
  - DualPrevieAlignment is missing a w
- Window
  - Check if Draw()'s updateProjection() call can be made conditional again and also called at programmatic size change instead
  - Rounded window corners are cut by the fill shaders discarding fragments, so their edges aren't anti-aliased
  - The docking preview is a window of its own, so it gets a taskbar button while it shows. Hiding it needs WS_EX_TOOLWINDOW on Windows (_NET_WM_STATE_SKIP_TASKBAR on X11), which belongs in PlatformBridge
  - A minimized or maximized window stays out of the docking layout until it is restored, and minimizing a group's root leaves the rest of the group on screen
- WindowTab
  - The strip runs along the top only. Its fields have no icons, and there is no "+" field, no closing by middle-click and no switching tabs by keyboard
  - A dropped window goes to the end of the strip, the way ImGui docks one, not where the cursor is; reorder it by dragging afterwards
  - A dragged window stays a window until it is let go, instead of joining the strip as a tab right away the way browsers do
  - Fields that don't fit at their minimum width are left out; the strip doesn't scroll
  - Tabs don't nest: a window holding windows in tabs doesn't go into a tab, and a window in a tab takes none
  - The WindowTab's rotation is ignored, since the content is drawn through a viewport, which can't turn
  - Not addressable through the C binding (DynamicLibWidgetManager) yet
  - Linux is untested: a window in a tab keeps its native window, hidden, under the content area for its input, and whether X11 moves and reports an unmapped window the same way is unverified
- Popups
  - Widgets under a popup keep whatever hover color they had when it opened, since they stop reacting before they would reset it
  - A popup opens centered over the one below it, so a nested popup of the same size covers it completely
  - No key closes the top popup yet (Escape, Enter)
  - No styling API: a popup's window background, border, corner radii and bar are whatever the code building it sets, so the library's own popups (the FileExplorer and its prompts) can't be made to match an app's look. A style set on a popup, or once for every popup of a window, would do it, and later the aesthetics of roadmap 9
  - Linux is untested: the owner is set with XSetTransientForHint, and whether the popup stays off the taskbar is up to the window manager
- Third dimension
  - Rectangles, text and textures keep a two-dimensional size: they are flat, and the frame tilts them, so a third size component would go unused
  - Tooltips and context menus stay flat over a tilted widget, by design, and so does a tabbed window's content (see WindowTab)
  - Nested clips in one frame and depth cut to their overlap, but only two differently tilted ones count at once: a third drops the outermost
  - Under perspective, layouts stack within 1 px (Projection::GetDepth), so a Lasagna with many layers gives each a slice close to the widgets' own stacking steps
  - A tilted widget is depth-tested against everything drawn, so a part tilting toward the eye draws over what lies in front of the widget's plane, the window bar included
  - A widget seen exactly edge-on takes no pointer, and under perspective a part projected past the window's edge can't be reached there
- IWidget
  - SetEnabled ignores its emitSignal argument, so OnEnable and OnDisable never fire for widgets that don't override it - SvgImage among them, which is why a window bar polls its undock handle's state instead of listening
- SvgImage
  - No disabled look of its own. Graying it out means recoloring its named paths, which would overwrite path colors set by the application, so whoever owns the image does it
- FileExplorer
  - No styling API: only the font can be set (SetFontFamily). Every color is fixed in FileExplorer.cpp - the bar, the buttons and menus, the list's row, header, selection and drop target colors, the toggles' accent, the scrollbar, the boxes - and the prompts (NamePrompt, ConfirmPrompt, ClashPrompt) copy them. A style set before Open, per widget kind and color state, which the prompts take over too, would let an app match its own look
  - With thumbnails on, the Name column gets what the thumbnails (72 px), Size, Modified and Type leave: about 70 px in the default 1000 px window with the preview pane, a few characters
  - The Thumbnails toggle was tried only by opening with the Thumbnails flag; clicking it, which keeps the selection and the top row in view, hasn't been done yet
  - The preview shows as many detail rows as fit, the most telling first; the rest can't be scrolled to yet. The row for playback controls stays empty until playing is built
  - A cloud file that isn't on this computer is downloaded to be previewed, which the pane says meanwhile; that hasn't been tried yet, since only personal files were at hand to download
  - The file operations were tried through their keys and prompts only, by posting key messages to the windows: the menu items, the Rename and Stop buttons, a slow double-click to rename and the clash prompt's Replace and Skip buttons haven't been clicked yet. Of drag and drop only a drop from another program was tried, called the way the system's drop calls it; dragging rows inside, the drop target marks and dragging out haven't been done with the mouse yet
  - Files dragged in from another program show where they would go only once dropped: GLFW takes drops as WM_DROPFILES, which tells nothing while they hover. While rows are dragged out, the system runs the drag in a loop of its own, so the app's windows don't draw until the drop
  - Linux can't drag files out yet: a drag past the window's edge stays the explorer's own
  - A Replace can't be undone in full: Undo takes the copy away, but the item it replaced was deleted. Undo covers what explorers with the Undo flag did, and only while the app runs
  - Redo (Ctrl+Y, and a Redo item in the Edit menu) is still to come: an undone step should go onto a redo list that the next new operation clears
  - The Hidden files and Extensions toggles are buttons drawn in an accent color while on; they become labeled checkboxes once a LabeledCheckBox prefab exists (roadmap 7)
  - Renaming has its own button in the menu strip; once a ContextMenu widget exists (roadmap 4), Rename moves to a right-click menu on the rows and the button goes. Delete, Cut, Copy and Paste join that menu too
  - Typing a name doesn't jump to it
  - File icons show only with thumbnails on; the plain list has none
  - The menu strip's titles and buttons use the font at 0.8 of its size so they fit; long drive labels still run past their place buttons (see Button)
  - Only filling the table is left on the main thread: storing the cells' strings and resolving the track sizes. In a Debug build the frame that shows WinSxS (35951 entries) takes about 140 ms, System32 (4781) under 40 ms
  - A folder still being listed when the app exits is cut off wherever its thread is, since a stuck listing is let go rather than waited for
  - Names in other scripts need a font holding their glyphs, set with SetFontFamily; the explorer only adds Latin-1 to its font itself
  - Its widgets are sized from the window size given to the constructor, so resizing the explorer's window doesn't refit them
  - From another language it opens over the main window only, and with an extensions string: the named filters stay C++-only. The Python binding's OpenFileExplorer hasn't been run, since no Python is installed here
  - Linux is untested: PlatformBridge::FileSystem's Linux side (listing, watching, the trash) and the in-app clipboard are written but have never been compiled
- Raster settings (Background, IBorder, IRangedValue, MenuBar)
  - The raster wrappers expose the Dotted settings only - no pattern selection and no secondary color - so a widget's background or border can't draw the Checkered pattern. Only a Plane or Frame owned directly can, which is why ColorPreview builds its own
- Background
  - Padding that automatically applies to the background image as well
- AudioPlayback / MediaSource / Decoder
  - With the royalty-free checks on (the default), Decoder::Open refuses anything outside the codec allowlist in DecodingPolicy.hpp, so a build can't quietly start distributing a decoder it isn't licensed for. NonCommercialUseCodecs names the encumbered ones (H.264, HEVC, VVC, VC1, AAC) so a refusal can say which codec it refused and why. The FFmpeg build itself is LGPL 2.1 with GPL and nonfree off, and has to stay dynamically linked
  - ADX, HCA and Nintendo DSP ADPCM aren't on the royalty-free list, so they decode only with the checks off. Each addition to the list is a deliberate licensing decision rather than a code change
  - Packet owns an AVPacket's lifetime, but MediaSource::ReadPacket still takes a raw AVPacket* and leaves the unref to the caller. A Frame wrapper and a reader that runs the send / receive / drain / flush-after-seek protocol behind one call aren't built yet, so every caller currently repeats it by hand
  - A reader built on FindBestStream drops the packets of every other stream. That's right for audio alone; audio and video from one file will need a packet queue per stream
  - A WAV, FLAC or MP3 with loop points plays through FFmpeg instead of miniaudio: miniaudio's streamed sounds loop without a gap only over the whole file (a seek inside a stream is asynchronous, and its cursor wraps at the file's end, not the loop's). Loop points set on a file miniaudio plays reopen it through FFmpeg, which restarts its audio briefly
  - AudioStream decrypts HCA blocks itself (HcaCipher) before they reach FFmpeg: FFmpeg 9 decrypts type 56 only when handed the key through its demuxer's options, and type 1 not at all (7.1.1 misread the header and never decrypted). FFmpeg is never handed the key, so nothing is decrypted twice. Cipher type 1 (the fixed table) is built but untested, for want of a sample file
  - FFmpeg doesn't trim HCA's encoder delay and padding, so an HCA file decodes to its block count times 1024 samples; the sample count and loop points in AudioMetadata count the same way, so they match what plays
  - Tracker modules seek to the nearest row, not the exact sample (libopenmpt seeks by pattern position), and their length is libopenmpt's estimate: a song played once runs a few hundredths of a second past it, to the end of its last row. SetLoopPoints refuses a module, which loops by its own patterns
  - AudioMetadata doesn't read the Opus vendor string (it sits in OpusTags, which FFmpeg skips) or APE tags; WAV loops come from the first smpl loop only, and RF64/BW64/RIFX files aren't scanned for smpl at all
  - DecodeCoverArt decodes PNG only with an FFmpeg built with zlib (vcpkg's ffmpeg[zlib] feature), since FFmpeg's PNG decoder needs it; without, a PNG cover decodes to nothing
  - The decoder helper restricts itself on Windows only (low integrity, no programs of its own, 1 GB at most, ending with the program); on Linux it runs as a separate process with the program's rights. A video in the preview pane still opens in the program for its paused frame - only after the helper decoded a frame of it without crashing, so a file that crashes decoders never reaches the program, but one that breaks later in playback still could
  - StreamProbing::HeadersOnly reads what the container's headers say; the bit rate a full probe would estimate from the packets stays 0 for files whose headers don't carry one
  - AHX and ADX encoding types other than 3 read but don't decode - FFmpeg has no AHX support at all, and its ADX decoder takes standard ADX only. Encrypted ADX decodes to noise without FFmpeg noticing, so AdxMetadata::_Decodable has to gate playback
  - One sound at a time. AudioPlayback owns a single ma_sound so it can be sought and queried, which means opening a second file replaces the first. Layered UI sounds want a second, fire-and-forget path alongside it
  - GetDuration costs differ by format: an MP3 (decoded by miniaudio) has to be decoded whole to answer, so cache it rather than poll it
  - Playback speed shifts pitch with it, because it is resampling rather than time-stretching. miniaudio has no time-stretcher at all; pitch-preserving speed would mean routing through FFmpeg's `atempo` filter, which is already compiled into the build
  - Negative speeds do nothing - miniaudio drops any pitch <= 0, silently, through a void function. Reverse playback isn't a rate at all: it needs the decoded audio reversed by frame (not by sample, or the channels swap) behind a custom data source, which also gives up streaming
  - Levels are linear RMS with no dB helper, so a meter bound straight to them sits low and barely moves - music averages well under 0.3. A 20*log10 mapping belongs either in the meter widget or beside GetChannelVolume
  - The meter's decay is a fixed constant per processed block, so its feel shifts with the device's buffer size instead of with wall-clock time
  - No signals yet (OnPlaybackFinished, OnPositionChanged), so application code has nothing to hook a track ending to

### Implemented Features
<details><summary>CLICK TO EXPAND</summary>

- Widgets
  - Label
    - Render text
    - Text Alignment (Left, Right, Center)
    - Text Padding
    - Per-state text colors 
    - Text Color animations with gradients
    - Textured Texts, the image stretched or keeping its proportions, and grayscale masks (metal text)
    - Text border, vertical flow, color emoji and shaped text (see Text Renderer)
  - SeparatorLine
    - A rule for visually dividing content, with an optional caption set into it
  - Button
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - Background: Solid Fill, Linear Shift, Radial Gradient, HueStar Gradient
    - Background image (SetBackgroundImage, SetBackgroundImagePadding): a picture sits behind the fill, shared through ResourceManager; an SVG sits over the fill as the button's own SvgTexture, so the hover and click tints stay behind it and SetBackgroundImagePathFill recolors its named paths
    - Border: Solid Fill, Linear Shift, Radial Gradient, HueStar Gradient, Corner Radii, Border Width
    - Label text (shares Label's text properties)
    - SetPosition, SetSize, SetRotation
  - CheckBox
    - Represents a boolean value
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - SetEnabled, SetValue, GetValue
    - Checkmark (SVG) with per-state colors (Enabled, Disabled, Hover, Clicked), Solid/Linear Shift/Radial/HueStar Gradient fill
    - Inner fill field with per-state colors, Solid/Linear Shift/Radial/HueStar Gradient fill, gradient offset/degree/animation & rotation speed
    - Optional binding to an external bool (EnableInheritedValue + SetInheritValueReference) so the CheckBox mirrors a value owned elsewhere
    - Inner padding (distance between border and checkmark)
    - Background & Border (same options as Button)
    - Corner radii
    - SetPosition, SetSize, SetRotation
  - RadioButton
    - A single option belonging to a RadioButtonGroup
    - Signals: OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - SetEnabled, SetValue, GetValue
    - Indicator field with per-state colors (Enabled, Disabled, Hover, Clicked), Solid/Linear Shift/Radial/HueStar Gradient fill, gradient offset/degree/animation & rotation speed
    - Indicator padding (distance between border and indicator)
    - Background & Border (same options as Button)
    - Corner radii
    - SetPosition, SetSize, SetRotation
  - RadioButtonGroup
    - Owns a set of RadioButtons and enforces mutual exclusivity, deselecting all others when one is selected
    - Owns its RadioButtons and their Labels: AddRadioButton(name, cell) builds a button and its Label (named name + "Label") in a cell of the grid; GetRadioButton, GetRadioButtonLabel and RemoveRadioButton find or remove them by the button's name. A removed pair is destroyed at the start of the next Interact, so the button's own slot may remove it, and SetAxisDefinitions keeps the pairs whose cells the new grid still has
    - Grid layout: row/column axis definitions, with each cell holding a RadioButton + Label pair
    - Per-cell content alignment (Left, Right, Center) and content padding
    - Group text label (shares Label's text properties via ITextProperties)
    - Border (same options as Button), with configurable border gaps
    - SetPosition, SetSize, SetRotation
  - ComboBox
    - Picks one entry out of a list of text items: AddItem, RemoveItem, GetItemCount, SetIndex, GetSelectedIndex, GetSelectedText
    - Drop-down panel hanging off the bottom edge of the closed box, as tall as the item count up to a cap of visible rows, with its own per-state background and border colors and corner radii. The open panel draws after every scene of its window, so it covers the widgets around it, and a click on it reaches only the drop-down: the widgets under it see the pointer as captured, until the press that picked a row is released
    - Clicking the box opens and closes the list, clicking a row selects it and closes it, a click anywhere outside puts it away
    - Menus: SetCaption shows a fixed title in the closed box ("File") instead of the selection, and OnItemClicked tells which item was picked; SetItemEnabled grays an item out (SetItemTextColors), and a disabled item is neither highlighted nor picked. SetDropDownWidth (IDropDown) lets the list be wider than the box, its left edge staying on the box's
    - The row under the cursor is highlighted, and the hit test walks the same row rectangles the renderer places, so what lights up is what gets picked
    - Arrow icon (shared SVG) with per-state colors, fitted to the icon's own aspect ratio inside the right edge of the box
    - Item text: font family, size, slant and weight, alignment and padding, inset by the border width so the frame never crosses a glyph
    - Signals: OnItemClicked, OnIndexChanged, OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation: the box tilts with all its parts, and the open list opens tilted with it, attached to the box (see Third dimension)
  - TextBox
    - Text input & editing (insert, backspace, key repeat). Everything typed in a frame goes in, in the order typed, and held keys repeat at the system's rate (the user's keyboard settings), not by frames
    - Keystrokes go to the text box or table cell clicked last, in whichever window it is; one that isn't being edited never takes them from another
    - Read-only mode
    - Caret: blinking with configurable blink time, colors, Solid/Linear Shift/Radial/HueStar Gradient fill, gradient animation speed
    - Text selection: mouse-drag selection, colors, Solid/Linear Shift/Radial/HueStar Gradient fill, gradient offset/degree/rotation speed, corner radii
    - Placerholder Text
    - Copy / Cut / Paste
    - Signals: OnTextChange, OnEnterPressed, OnEnterReleased, OnCopy, OnPaste
  - Slider
    - Represents a numeric value of any type (Bool, Int8-64, UInt8-64, Float32/64) with configurable min/max
    - Horizontal and Vertical orientation: the value always runs along the track's local x-axis, so a vertical slider is a horizontal one turned a quarter turn, while its own rotation stays at whatever the caller set. SetSize takes the on-screen footprint either way, so a layout container can size a vertical slider without knowing about the turn
    - Track direction (Normal/Inverted) chooses which end of the track holds the minimum, for values that count the opposite way to the track - a scrollbar's offset, for instance. Affects where the indicator and graph are drawn, not the value
    - Drag the indicator or click anywhere on the track to set the value
    - Step the value by a configurable step size, in whatever type the value currently holds
    - Indicator: Stroke or Circle type, sized per axis in pixels or percent of the track, per-state colors (Enabled, Disabled, Clicked, Hover), Solid/Linear Shift/Radial/HueStar Gradient fill, Raster, Fog Effect, corner radii, border width and border gaps. Sized and positioned inside the track's border rather than over it
    - Graph (the filled part of the track): Bar or Wave mode, the wave's thickness, height and length, per-state colors, width, all fill types, corner radii
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - ProgressBar
    - Shares the Slider's value, indicator, graph and stepping machinery (IRangedValue) without the dragging
    - Horizontal and Vertical orientation, Normal or Inverted track direction
    - Signals: OnValueChanged, OnValueSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation, corner radii
  - Histogram
    - One bar per value of a bound array (SetData), of any numeric type (Bool, Int8-64, UInt8-64, Float32/64), mapped into a min/max range and clamped to the track
    - The data isn't copied but read every frame, so a producer that updates its buffer in place - a Video's frequency bands, for instance - shows live without being bound again
    - Spacing between the bars (SetBarInterstice), Horizontal and Vertical orientation
    - Graph colors and fill options from IRangedValue
    - Signals: OnDataSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation
  - LineDiagram
    - Draws every bound data set as its own line (SetData with a set index). Each set is its own buffer with its own length, read every frame without copying, so lines follow data updated in place
    - Any numeric type (Bool, Int8-64, UInt8-64, Float32/64), shared by all sets; values are mapped into one min/max range and clamped to the track, while the data itself is left as it is
    - The lines share the x spacing of the longest set, so the same index lines up across all of them. A shorter set ends where its data does, or holds its last value to the end (EnableClampToLastValue)
    - Per-line color (SetLineColor) and one line width for all (SetLineThickness). Lines have mitered joins and anti-aliased edges (Polyline and the Line shaders), and are inset from the border far enough that a line at the minimum or maximum isn't cut off
    - The line under the cursor is drawn 2 px wider and on top of the others. It keeps the hover while the cursor is on its wider self, so it doesn't flicker at its edge
    - Pointer on the hovered line's data point nearest the cursor. It is the indicator, so ShowIndicator and the SetIndicator setters style it - a 10 px circle by default
    - Labels (SetLabel, SetLabelFontFamily): at the start of each line (ShowLineLabels), where labels of lines starting together simply overlap, and next to the pointer as "Label: value" (EnableHoverLineLabel and EnableHoverValue, each also on its own)
    - The hover text flips to the other side of the pointer where it would leave the widget, and sits on a translucent box (SetHoverTextBackgroundColors) so it stays readable across its own line
    - Hexadecimal values (ShowHexNumbers): an integer as its value (17 is 0x11, -1 in an Int8 is 0xFF), a float as its bits in full (1.0f is 0x3F800000, 1.0 is 0x3FF0000000000000)
    - Lines, labels and pointer appear once ShowGraph(true) is called
    - Signals: OnDataSet
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation
  - ColorPreview
    - Shows a color against a checkerboard, so an alpha below 1 reads as transparency instead of as a different color
    - The board is drawn by the Checkered raster pattern rather than by a texture, so its square size and its two colors are shader uniforms
    - Optional dual preview (EnableDualPreviewBackground): the widget splits in half and the same color is drawn over the checkerboard on one half and over white on the other, side by side or stacked (SetDualPreviewAlignment)
    - The previewed color and the border always span the whole widget, so the two halves differ only in what lies behind the color
    - Border (same options as Button)
    - SetPreviewColor, GetColor, SetPosition, SetSize, SetRotation
  - Table
    - Grid of typed cells, each created on demand the first time a value is assigned to it. The cell type follows from what is assigned: a string or a number makes a TableText, a color makes a TableColor, a bool makes a TableCheckBox, a texture makes a TableImage
    - TableText cells: editable text (see the text interaction entry below)
    - Cells hold only their values: text with its value store, a color, a checked state. The text cells in view draw through a pool of TableTexts, about as many as fit in the view and handed on to other cells while scrolling, and the checkbox and color cells through one shared widget each, so a table costs the same however many rows it holds. The tracks in view and the cell under the pointer are found by halving over the tracks' offsets: a scroll step through 35951 rows takes no measurable time, and filling them, four cells each, about 110 ms in a Debug build
    - TableColor cells: a solid color swatch. The cell stores only its color value and draws through a single rectangle shared by the whole table, so a color column costs one set of GL buffers no matter how many rows it has
    - TableImage cells (SetValue(const ITexture*, index)): a texture fitted inside the cell less its padding (SetImagePadding, 4 px by default), its aspect ratio kept, drawn through one rectangle shared by the whole table. The table doesn't own the texture: whoever sets it keeps it alive while the cell shows it. GetDisplayedRows tells which rows are in view, for filling image cells lazily
    - TableCheckBox cells: a checkbox sized as the largest centered square that fits the cell minus a configurable margin, so it keeps its shape as the tracks resize. The cell stores only its checked state and draws through one background rectangle, one border rectangle and one SVG checkmark shared by the whole table, so a checkbox column costs one set of GL buffers regardless of row count
    - Checkbox interaction: the click target is the box itself rather than the surrounding cell, with hover and pressed states tracked per cell. Per-state colors (Enabled, Disabled, Hover, Clicked) for background, border and checkmark, plus corner radii and border width
    - Per-track sizing policies: Star (divides whatever viewport space the fixed tracks left over) and Fixed (absolute pixels, the mode that lets content outgrow the table and become scrollable)
    - Scrolling on both axes, with content extent, maximum scroll and scroll position exposed so an external Slider can drive it through the signal/slot mechanism
    - Only the tracks overlapping the viewport are drawn; partially visible ones are cut by clip planes (Shader::ClipScope) inside whatever clips surround the table, so a half-scrolled row can't spill past the border, tilted or not
    - Alternating track coloring with any number of color variants, banding by row or by column (TableOrientation)
    - Per-variant background, inner border and text colors
    - Column and row headers, placeable Top/Bottom and Left/Right, with their own font, alignment, padding, colors and border widths. The band is taken out of the content viewport and stays pinned while the content scrolls underneath it
    - Text interaction per cell: click to place the caret, drag to mark, arrow keys with key repeat, select all, copy / cut / paste, backspace. One caret and one selection highlight are shared by every cell, reused by whichever one currently has focus
    - Read-only tracks: marking and copying still work, only the edits are rejected. A checkbox in a read-only track still shows its value but won't toggle
    - Row selection (SetRowSelection: None, Single, Multiple): a press on a row selects it, whatever its cells hold, Alt+press adds or removes one, Shift+press selects every row from the last one pressed, adding them with Alt. A press past the rows drops the selection. In Column orientation it selects columns, the tracks the colors band along. A press on a read-only track then only selects, without a caret. Selected rows draw in colors of their own (SetSelectedRowBackgroundColors, SetSelectedRowTextColors); GetSelectedRows, SelectRows and ClearSelection reach the selection from code, GetRowAt finds the row under a point, and redefining the tracks drops the selection
    - Selection band (Multiple): a press off the selection that moves past a 5 px drag threshold draws a translucent band (SetSelectionBandColors, SetSelectionBandBorderColors) and selects every row it touches, added to the selection from before with Alt. Its start stays with its row, and the table scrolls while the pointer is above or below the rows, faster the farther it is
    - Dragging the selection: a press on a selected row, or on any row in Single selection, that moves past the drag threshold drags the selected rows. OnSelectionDrag reports Started, Moved, Released and Cancelled (Escape, or the pointer taken away) with the pointer in window coordinates; the press stays with the table until it is let go, wherever the pointer goes, and the table scrolls while the pointer is above or below the rows, as for a band. Let go without moving, the press selects that row alone. SetDropTargetRow and ClearDropTargetRow mark the row a drop would go on, in colors of their own drawn over the selection's (SetDropTargetRowBackgroundColors); redefining the tracks takes the mark away
    - Keys (TakeKeyboardFocus, HasKeyboardFocus; a selecting press takes them): the arrow keys, Home, End, PageUp and PageDown move the selection, Shift extends it from the anchor, Ctrl+A selects every row. The row they reach is scrolled into view (ScrollRowIntoView). With Ctrl or Alt held they are left to the app, Ctrl+A aside, and while a cell is edited they move its caret
    - Scrolls by the mouse wheel while the mouse is over it, before a ScrollView around it does: a notch scrolls as many lines as the system is set to (3 unless changed) of SetWheelScrollStep pixels each (20 by default), or a whole viewport when the system scrolls a page per notch. Held navigation keys repeat at the system's rate
    - Editing from code: EditCell(index, selectedLength) opens a text cell for typing, a read-only track's too, with its first selectedLength code points selected (all of it by default), the way a file list starts a rename. Enter keeps the edit and Escape puts the old text back; EndEdit keeps it, as a press elsewhere does. OnCellEditEnded reports the cell, how the edit ended (Kept, Cancelled) and the text it left
    - Signals: OnTextChange, OnEnterPressed, OnEnterReleased, OnCopy, OnPaste, OnSelectionChanged, OnRowDoubleClicked (a second press on the same row within the system's double-click time, PlatformBridge::Input::GetDoubleClickTime), OnRowSlowDoubleClicked (a second press on the row that is the only one selected, too late for a double-click but 500 to 1000 ms after the first, never before the double-click time is over, and let go without a drag: what a file list renames with; SetSlowDoubleClickTime), OnSelectionDrag, OnHorizontalHeaderClicked (the column), OnScrolled (the table scrolled itself, for the keys, the wheel, a band or a drag, so an external Slider can follow)
    - Background & Border (same options as Button)
    - SetPosition, SetSize, SetRotation
  - Image
    - Every format stb_image reads (PNG, JPG, BMP, GIF's first frame, TGA, PSD, HDR, PIC, PNM) and SVG, always as RGBA, so grayscale and gray-and-alpha pictures upload whole. Paths are UTF-8, so names outside the system's code page open too
    - Decoding without GL (Texture::DecodeFile, Texture::DecodeMemory into a DecodedImage, which keeps the picture's own size too), so a worker thread decodes and SetImage uploads. A size limit scales big pictures down, each pixel averaged by alpha so edges don't darken, and draws an SVG sharp at that size. Files over 256 MB and pictures over 64 megapixels (8192 x 8192) are refused before anything is allocated for them, since an untrusted file can claim a picture of gigabytes in a few bytes
    - Built without a path it shows nothing until SetImage; ClearImage, GetAspectRatio, GetResolution
    - A file comes from ResourceManager, shared by every widget showing it and kept until ResourceManager::Free(path); a picture given to SetImage is the Image's own
    - Drawn as a Plane's image, on the quad all rectangles share
    - Fits its picture inside its size, the aspect ratio kept
    - SetPosition, SetSize, SetRotation
  - Video
    - Plays a file with its sound (VideoPlayback, over an AudioPlayback of its own): OpenVideoFile, Close, IsOpen, Play, Pause, Stop, Seek, GetDuration. After an open or a seek its first picture shows even while paused
    - The sound's channel levels and frequency bands, for meters and a Histogram
    - Drawn as a Plane's image, on the quad all rectangles share
  - SvgImage
    - Rasterizes SVG vector graphics (lunasvg) into an SvgTexture of its own, which ResourceManager parses from the file's text, read once however many widgets show it
    - Fit widget geometry to the SVG's intrinsic aspect ratio
    - Per-path runtime recoloring of named SVG paths (Solid, Linear Shift, Radial, HueStar Gradient). Only switching a path's override on or off rasterizes the SVG again; changing its colors doesn't
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
    - Owns its widgets: AddWidget&lt;T&gt;(name, CellPlacement, extra constructor arguments) builds the widget in place, with the Lasagna as its parent, and hands back a borrowed pointer. A taken name, an out-of-range placement or an occupied cell fails visibly with nullptr, and nothing is built
    - GetWidget&lt;T&gt;(name), ShowWidget and IsWidgetShown find a widget again by name. A hidden widget neither draws nor takes input
    - RemoveWidget(name) frees the widget's cells at once, so another widget can take them, and destroys the widget at the start of the next Interact, so a widget's own slot may remove it
    - Cell lookup by TrackIndex, and the current track count per axis
    - Debug lines
  - Prefab
    - A group of widgets laid out in its own Lasagna and treated as a single widget by whatever contains it
    - Composed, not subclassed: children are added with AddWidget&lt;T&gt;(name, placement), so a prefab is a tree of widgets rather than a new C++ type per kind. A subclass per kind would have to expose its children through the C ABI, which addresses widgets by string and cannot hand back a pointer, so every prefab would grow the binding surface
    - Children are reached by name afterwards: GetChildWidget&lt;T&gt;, ShowChildWidget, IsChildWidgetShown, and removed with RemoveChildWidget. A hidden child neither draws nor takes input
    - Its Lasagna builds, owns and parents the children, so a child's path runs through it (TestStepper/TestStepper_Lasagna/Increase); a name that is already taken, or a cell that is out of range or occupied, builds nothing
    - SetPosition, SetSize, SetRotation, forwarded to its Lasagna
  - Environment
    - An empty 3D room in which models and widgets sit freely, placed in its own units (-1 ... 1 by default, SetExtent) by position, size and rotation (EnvironmentPlacement). Moving, resizing or rotating the Environment moves, resizes and rotates everything inside it alike
    - Scaled into its rectangle uniformly, by a fit: Contain, Cover, KeepHeight or KeepWidth (SetFit), with optional clipping to the rectangle (clip planes, so the cut turns with a tilted Environment) and a debug box
    - Owns its widgets: AddWidget&lt;T&gt;(name, placement, extra constructor arguments) builds the widget in place, with the Environment as its parent, and refuses a taken name. Widgets draw in the order they were attached
    - By name afterwards: GetWidget&lt;T&gt;, SetPlacement, GetPlacement, ShowWidget, IsWidgetShown, and RemoveWidget, which destroys the widget at the start of the next Interact, so its own slot may remove it
  - ScrollView
    - Cuts its content to the inside of its border with clip planes, exact when it is tilted. Where the content is cut away it takes no pointer either, apart from an open drop-down list, which draws uncut over everything
    - Scrolls by dragging: a press is held back from the widgets until it moves past a 5 px drag threshold along an axis it can scroll (EnableScrolling: X, Y, Z), or released as a click
    - Scrolls by the mouse wheel while the mouse is over it: vertical, and horizontal from tilting wheels, two-finger swipes and Shift with the wheel. A notch scrolls as many lines as the system is set to (3 unless changed) of SetWheelScrollStep pixels each (20 by default), or a whole view when the system scrolls a page per notch
    - Scrolls along Z by a pinch (touchscreen, precision touchpad, or Ctrl with the wheel): spreading the fingers to twice the distance goes one view deeper
    - Stacks its cells along any axis (SetAlignmentDirection: Vertical by default, Horizontal, or Z). Along Z they stack away from the eye from the view's front face, each cell's own depth spacing it from the next, and draw from the back. With Z scrolling on, the cells scrolled past the front or lying beyond the view's depth are left out, the frontmost cell under the pointer covers the ones behind it, and a view that scrolls in depth only takes the plain wheel for it: away from the user goes deeper. Under perspective the deeper cells shrink toward the vanishing point
    - Nested ScrollViews: the innermost one under the mouse that can scroll along an axis takes that axis of the wheel, after its widgets drew, so the ones around it don't scroll too; an axis it can't scroll stays for them
    - Owns its widgets: AddWidget&lt;T&gt;(name, extra constructor arguments) stacks a widget after the last; GetWidget&lt;T&gt;(name), and RemoveWidget(name), which destroys the widget at the start of the next Interact
    - GetScrollPosition, SetScrollPosition per axis; signals OnXScrolled, OnYScrolled, OnZScrolled
  - MenuBar
    - A bar of flex cells along an edge (SetFlexDefinition: Fixed, Auto, Star or Square sizing per cell), with item backgrounds, borders and separator lines
    - Owns its widgets: AddWidget&lt;T&gt;(name, cell, extra constructor arguments) builds a widget in a cell, with the bar as its parent, and refuses a taken cell or name. GetWidget&lt;T&gt;(cell) finds it again; RemoveWidget(cell) frees the cell at once and destroys the widget at the start of the next Interact, as do cells a smaller flex drops
  - ExtendedComboBox
    - A drop-down whose rows are whole widgets: Labels, Buttons, TextBoxes, ComboBoxes, other ExtendedComboBoxes and more
    - Owns its rows: AddItem&lt;T&gt;(name, extra constructor arguments) builds each with the box as its parent, so an open list counts the row as its own, and applies the box's font to it; a taken name builds nothing. RemoveItem(index) drops the row and destroys the widget at the start of the next Interact, so a row's own slot may remove it
  - WindowTab
    - Docks windows as tabs, like a file explorer: a strip of fields along the top, each with a window's native title and a close button, and the selected window's content in the area below (AddWindow, InsertWindow, RemoveWindow, SelectWindow, GetSelectedWindow, GetWindow, GetWindowCount)
    - Drawn the way ImGui draws docked windows: the window holding the WindowTab draws the selected window's scenes itself, in its own GL context, with the viewport and scissor box set to the content area. Its own later scenes, a drop-down for instance, draw over the content. The tabbed window keeps its native window, hidden and kept right under the content area, so its widgets read the cursor as on the window itself, and keys reach its text boxes through the focused window (Window::KeyboardWindowOf)
    - Only the selected window draws; the windows in the other tabs pause until they are selected, as in ImGui
    - A press on a field selects it, the close button closes its window the way the window's own close button does, and the window's owner destroys it as usual
    - Dragging a field sideways reorders the fields. Dragging it 1.5 title font sizes above or below the strip, or 2.2 past the first or last field (ImGui's thresholds), pulls its window out: it gets its old size back, with the cursor holding it where it held the field, and follows the cursor until let go, docking to an edge or into a tab where its preview showed
    - Dragging a free window by its bar over a strip shows a translucent preview over the content area and a preview field at the end of the strip, and letting go docks it in at the end. Over a strip, docking into the tab wins over docking to an edge
    - A window in a tab hides the scenes with ShowInTab(false) (its window bar), draws no frame, takes the content area's corner radii (SetContentCornerRadii), can't be resized or docked to an edge, and its popups go on the stack of the window holding the tab, so they block the strip too
    - A window leaving a tab shows only once it has drawn a frame of its own, so it never shows a stale picture, and nothing shows when that happens during shutdown
    - Styling: strip height, field width range, spacing, corner radii, per-state field background, border and title colors plus the selected field's, title font, close button size, colors and icon color, and the WindowTab's own background and border
    - Signals: OnSelectionChanged, OnWindowAdded, OnWindowRemoved
    - Can be a scene's root container (Scene::SetRootContainer<WindowTab>) as well as sit in a Lasagna cell
  - Window
    - Background color or image
      - Background image ID can be used to create glass effects on widgets
    - Background & Border (same options as Button), with a radius per corner (SetCornerRadii). The border is 0 px wide until it is set, and is drawn over the scenes
    - Transparent framebuffer: what the background leaves out - the area outside its rounded corners, or all of it with a translucent background color - shows what lies behind the window. Alpha blends with a function of its own (glBlendFuncSeparate), so a semi-transparent widget on an opaque background leaves the window opaque instead of letting the desktop through
    - Resizeable, by presses inside the window only. The mouse state is read for the whole screen, and outside the window every position would count as near an edge
    - The mouse reaches only the topmost window under the cursor, whatever application it belongs to (PlatformBridge::Input::IsWindowUnderMouse), and a press stays with the window it landed on until it is let go. A click never reaches a window lying underneath, whose widgets see the mouse as outside and let go of their hover, and a drag carries on over other windows
    - The mouse wheel and the pinch reach the widgets of the window under the mouse too, through PointerState (_Wheel, _PinchScale). A widget that scrolls by a part takes it (Window::TakeWheel with WheelParts: Horizontal, Vertical, Pinch) after the widgets inside it interacted, so the widgets read after it, the containers around it, see none of it. The app's loop calls PlatformBridge::Input::NewFrame() once per frame, before the first window interacts, so every window sees the same frame of typed text, wheel and key strokes
    - Three passes per frame: Interact() has the widgets react to the frame's input (hover, clicks, keys, the wheel, tooltips, context menus), Update() does the per-frame work apart from input and drawing (a video advancing, the scenes' OnUpdate, every AudioPlayback's frequency bands through AudioPlayback::UpdateAll), and Draw() only draws. The app's loop calls Interact on every window, then Update on every window, then Draw, so whatever one window's input changes, in that window or another, shows in the same frame. Interact passes down the tree as Draw does: IWidget::Interact updates the widget's tooltip and context menu, then calls its protected virtual interact(), in which a container calls its widgets' Interact in the order they draw. A window in a tab interacts through its WindowTab, as it draws through it. Update runs for every window and every widget, hidden ones and windows in background tabs included, so a video in a background tab keeps advancing and its scenes keep ticking. A window that was hidden (in a background tab, minimized, not shown yet) draws its first frame back without transitions, so it shows at once what changed meanwhile
    - Movable, by dragging a MenuBar cell marked for it (FlexDefinition::_IsWindowDrag)
    - Several windows at once: every window shares textures, buffers and shaders with the first one created, so fonts, images and shaders load once. Each window makes its own GL context current before it draws. VAOs and framebuffers, the only GL objects windows don't share, are kept per context (VertexArray, Framebuffer): every context that draws an object makes its own on first use and deletes it only while that context is current, so a widget can be drawn by another window - a window in a tab is - and destroyed with any context current
    - Destroying a Window destroys its native window. FromGlfwWindow returns the Window behind a GLFW window
    - Dropped files: OnFilesDropped reports files dropped on the window, from another program or another of the app's windows, with their UTF-8 paths and where the pointer was, sent from the window's next Interact rather than from inside the system's message handling. A window under a popup takes no drops
    - Layout file, the way ImGui keeps imgui.ini: RetroFuturaGUI.ini in the working directory keeps each window's position, size, maximized state, docking (host, edge and place on that edge) and the size before docking or tabbing, and each WindowTab's windows in order with the selected one. Window::SetLayoutFile(path) sets another file (a relative path starts at the working directory) or, empty, switches it off; set it before the first frame. Window::SaveLayout() writes it at once
      - Read on the first frame, when the app's windows exist and nothing has drawn yet; created then when it doesn't exist, with the layout the code set up. Windows are found again by name (GetName) and WindowTabs by path (GetPath), so both must be unique and stay the same between runs; ones the file names that don't exist are skipped, and windows the file doesn't name keep the code's layout
      - Applied in order: each free window's size and place, then docking in each edge's saved order, then the tabs, then maximized. A window saved where no monitor shows its top anymore comes back centered on the primary monitor's work area (PlatformBridge::Monitors). Minimized isn't restored
      - Written on events: docking and undocking, a window going into or out of a tab, tabs reordered or selected, maximizing, minimizing and restoring, a window asked to close (before the app destroys it), and a move or resize once the mouse button is up. At most once per frame
    - Docking (DockWindow, Undock, IsDocked)
      - A window docks to one of another window's edges (DockEdge). The windows on one edge share its whole length, minus the gaps between them, and keep their own depth away from it - like the columns of Niri
      - A window has one host at most, the window whose edge it sits on, and never one docked to it, so a docking group is a tree. It can still border several windows: in a row of three, the middle one is docked to the first and the last one to the middle one
      - The group moves and resizes together: moving or resizing any of its windows carries the others along. A docked window can only be resized on its far edge, since its host sets its length and where its near edge sits
      - The gap is the largest docking padding (SetDockingPadding) among the host and the windows on its edge, so it doesn't matter which window was docked onto which
      - A window that undocks or closes gets its length from before docking back, the windows left on its edge grow to fill the gap, and the windows docked to it are let go where they are
      - The corners where docked windows meet are squared: a host's corners along an edge with windows on it, a window's corners facing its host, and those facing its neighbors on the same edge. EnableSquaredCornersWhenDocked(false) turns it off for every window. GetDrawnCornerRadii returns the radii actually drawn, so a widget lining the window's edge - a window bar, for instance - can match its own corners to them
      - Drag to dock: a window bar calls BeginDrag and EndDrag around a drag. A window docked to nothing that is dragged within 32 px of where it would sit docked on another window's edge shows a translucent preview of the exact slot, and docks there when it is let go - among the windows already on that edge, in the order of their positions. Dragging a docked window moves its group instead
      - Undock cells (FlexDefinition::_IsWindowUndock): a drag that starts in one pulls the window out of its group, then drags it alone, so it can be docked somewhere else. The bar disables the cell's widget while the window is docked to nothing, and a cell holding a disabled widget draws in the disabled item colors and takes no presses
      - Docking into tabs: a window also docks into a WindowTab, by its API or by dropping it on the strip. It leaves its docking group when it goes in, and Undock takes it out of the tab again (IsInTab)
    - Maximize and restore (ToggleMaximize, IsMaximized), with the state read from GLFW rather than kept by the window
    - Holds an ordered list of Scenes it draws over its root Lasagna (AddScene, RemoveScene), borrowed rather than owned
    - Divides its surface into a client area: each docked Scene takes its strip, and whatever is left goes to the overlay Scenes and the root Lasagna. Recomputed on resize, on adding or removing a Scene, and whenever a Scene changes what it reserves or whether it is active
- Scenes
  - Scene
    - A named group of widgets with its own root container, hanging under a Window: Window / Scene / Container / Widgets. Meant for scene loading and for HUDs
    - The root container is a Lasagna (SetLasagnaAxis), an Environment, a ScrollView or a WindowTab (SetRootContainer&lt;T&gt;), so a whole scene can be a 3D room, scroll as one, or hold other windows in tabs
    - Hidden while its window sits in a tab (ShowInTab(false)), for a window's own bar, whose title and close button the tab's field shows instead. It gives back its strip meanwhile
    - Deliberately not an IWidget, only an IHierarchyNode. A scene has no geometry, no hit-testing and no color state of its own, and staying off IWidget keeps it out of Lasagna::AddWidget - so a Scene can never end up nested inside a cell, and the hierarchy stays at a fixed depth
    - Contributes its name to its widgets' paths, so GetPath composes MainWindow/MyScene/RootLasagna/TestButton for the interoperability API and .bechaml to address
    - Active or Inactive (SetActive), which is a different axis from a widget's Enabled or Disabled: Active decides whether the scene runs at all, Enabled decides whether a widget reacts and which ColorState it renders in. Since an inactive scene neither interacts, updates nor draws, it stops responding as well as drawing. A disabled widget still draws - so an active scene full of disabled widgets is exactly the dimmed-behind-a-modal case, and a Scene needs no separate visibility flag
    - Docks to an edge of the client area (SetReservedEdge with a DockEdge), reserving a strip in pixels that everything laid out after it is fitted around, so a menu bar or status bar can't hide or swallow clicks meant for the content behind it. ClearReservedEdge turns it back into an overlay
    - Overlay is the default: the scene covers the whole client area and reserves nothing, for floating panels and modals
    - Docked scenes reserve in the order they were added, so the first one to claim an edge also owns the corner where two claims meet
    - An inactive scene reserves nothing, and is laid out again when it is reactivated, so activating one never shows a layout computed for a different window size
    - SetPosition, SetSize, SetRotation, forwarded to its root container
    - Per-frame signal (Connect_OnUpdate / Disconnect_OnUpdate), emitted from Update after the input and before drawing, so a value written by a slot lands in the same frame instead of trailing one behind. Meant for widgets that track a live value, such as an audio level meter. It ticks while the scene's window sits in a background tab too; an inactive scene doesn't tick
    - Input signal (Connect_OnInteract / Disconnect_OnInteract), emitted from Interact before the widgets take the input, while the scene shows. For keys a widget would take too: the FileExplorer reads its keys there, so the Enter or Escape that ends a rename in its list doesn't also open the file or close the explorer
- Popups
  - Popup&lt;Ts...&gt; is a window of its own that closes with values of any types: Popup&lt;bool, std::string&gt; closes with Close(true, name), and the values reach the slots connected with Connect_OnResult. Dismiss, or the window bar's close button, closes it without values and fires Connect_OnDismiss instead
  - A popup owns its Window and the scenes added to it (AddScene). IPopup is its base without the value types, which is what a window's popup stack holds
  - Window::PushPopup opens one and takes ownership. A window has one popup stack: a popup opened from a popup's window goes on top of the same stack. Popups close from the top down - Close and Dismiss return false on a popup with another one over it, and only the stack's owner ever destroys one, always the top. A window destroyed with popups open destroys them top-down, without sending results
  - Only the top popup takes the mouse. Every window below it ignores the mouse, can't be resized and drops close requests, like a window under a native modal dialog. The window does this as an IPointerFilter at the root of every widget's pointer chain, so no widget needed changing
  - The click that opens or closes a popup doesn't click again in the window that takes the mouse next: a press held from before is ignored until the button is released
  - Each popup is natively owned by the window it opens over (PlatformBridge::NativeWindow::SetOwner), so it stays above it, minimizes with it and has no taskbar button. It opens centered over that window, and shows once it has drawn its first frame
  - Results are sent once the popup is off the stack, after its frame is drawn, so a result slot can open a follow-up popup. PushPopup makes the GL context current again that was current when the popup was created, because it usually runs inside a button's slot in the middle of another window's draw. A popup destroyed without being pushed, after a refused Open say, brings that context back itself
  - Popups' windows stay out of docking
  - A popup with content of its own derives from Popup&lt;Ts...&gt;, as FileExplorer does. Dismiss and the sending of results are final, so every popup closes and reports the same way
  - When the system closes a popup's window (Alt+F4, the taskbar) instead of Close or Dismiss, the window asks the popup first (IPopup::allowsSystemClose): one with work to finish can stay open, ask the user, and close itself once done. Window::TakeKeyboardInput makes a popup's window the one the keys arrive at
- Animation
  - Transitions carry colors and corner radii to new values over a set time instead of letting them jump: a runtime SetCornerRadii or SetBackgroundColors, a widget's hover, pressed and disabled colors, text colors, and a window's corners squaring when it docks
  - Transitions::SetDefault(TransitionProperty::Colors | TransitionProperty::CornerRadii, { 200ms, Easing::EaseInOut }) sets the global default, off (0 ms) until set. A widget overrides it per property with SetTransition and goes back to the default with ResetTransition; a Window has the same for its background and border, and a widget's tooltip and context menu follow the widget. Easing: Linear, EaseIn, EaseOut, EaseInOut (cubic)
  - Worked out from the clock whenever a value is drawn, so it runs the same at any frame rate, needs no update step and costs nothing once it has arrived. A new target set midway starts from where the value is, and values set before a widget first draws show at once, so nothing fades in at startup. In a hidden window (in a background tab, minimized, not shown yet) every transition takes no time: its first frame back shows what changed meanwhile at once (Transitions::InstantScope)
  - Colors blend with their alpha premultiplied, so a fade from transparent has no dark fringe. Gradients blend stop by stop: when the counts differ, both sides are sampled at as many evenly spaced stops as the larger one has, the way the gradient shader reads them. The fill type itself switches at once
  - Elements drawn more than once per frame in different colors switch at once, since they would restart at every draw: a Table's cells, a MenuBar's items, a WindowTab's fields, a LineDiagram's labels and a ComboBox's items. So do SVG icon colors, raster and fog colors, and a ColorPreview's color, which is a value rather than a style
  - Positions, sizes and rotations (TransitionProperty::Position, Size, Rotation): SetPosition, SetSize and SetRotation move a widget there over its transition, and IWidget::Update applies the shown value every frame until it arrives, so its hit box and whatever it carries along (its elements, a container's widgets) follow it. GetPosition, GetSize and GetRotation return where it is now. Layout never animates: containers place their widgets, and windows their scenes, inside an InstantScope, so resizing a window or scrolling a ScrollView moves everything at once, and a widget's own transition only plays for changes the app makes. The widgets' geometry overrides are the protected applySize, applyPosition and applyRotation behind the one public setter
  - WindowTab's fields follow its Position transition: they slide into their slots when the fields reorder, one closes or a window is dropped (its field grows out of the preview's), the dragged field glides home once let go, and the drop preview opens its gap while the other fields narrow. The dragged field itself follows the cursor at once
  - Fades (TransitionProperty::Opacity): IWidget::SetOpacity fades a widget and everything inside it, 0 to 1, over its Opacity transition (GetOpacity returns where it is now); a fully transparent widget is skipped when drawn but still takes the input. Tooltips and context menus fade in when shown and out when dismissed, over their widget's Opacity transition; a drop-down's list fades in when it opens and out when it closes, and takes no input while it fades out. Drawn opacity is Shader::OpacityScope: scopes multiply, and every on-screen shader takes the product as uOpacity, so images, SVG icons, video and 3D models fade with the rest
  - Popups and the docking preview fade through the native window's opacity (glfwSetWindowOpacity), over their window's Opacity transition (Window::SetTransition). A popup fades in as it first shows; once closed it leaves the stack and sends its result at once, then its window lets the mouse through to whatever lies under it and fades out before it is destroyed. The docking preview fades in as it shows and out before it hides
- FileExplorer
  - A popup that browses the file system and closes with the chosen paths as UTF-8 strings. Open(startPath, flags, extensions) sets it up, then Window::PushPopup shows it. It derives from Popup&lt;std::vector&lt;std::string&gt;&gt;, so the paths arrive through Connect_OnResult and a cancel through Connect_OnDismiss
  - Everything it offers is one bit mask, FileExplorer::FileExplorerFlags: a mode (SingleFile by default, MultiFile, Folder, WriteFile; Open refuses two), whether the three toggles start on (ShowHiddenFiles, ShowFileExtensions, Thumbnails; the toggles are always there, and the user can switch them), the previews (ImagePreview, MediaPreview, together Previews) and the file operations (NewFolder, Rename, Delete, DeletePermanently, CopyAndMove, DragAndDrop, Undo, AskOnNameClash, SkipDeleteConfirmation; FileOperations sets the usual ones). With using enum, a call reads Open("C:/Music", MultiFile | ShowHiddenFiles). A feature not in the mask has no button and no menu item
  - A menu strip under the address box: File (New folder, Rename, Delete, Delete permanently, Close) and Edit (Undo, Cut, Copy, Paste, Select all) menus, ComboBoxes with a caption, each listing only what its flags turn on and graying what can't be done right now; a Rename button; and the Thumbnails, Extensions and Hidden files toggles
  - The file types are one string: Open("C:/Code", SingleFile, ".cpp, .hpp, .mp3"). Any case, and the dot may be left out or written as "*.mp3"; ".*", the default, lists every file. The filter box offers them all together, selected, then each one alone, and "All files" when ".*" is among them. For named choices, the overload Open(startPath, flags, std::vector<FileFilter>) takes { ._Name = "Audio", ._Extensions = ".mp3, .flac" } per choice, the first one selected and only these offered; a filter without a name shows its extensions
  - A popup like any other, not a prefab: the explorer builds its own widgets into its scene's root Lasagna, one grid of 40 columns where each row is a set of column spans
  - Places: the known folders (Home, Desktop, Documents, Downloads, Music, Pictures, Videos) and the drives or mounted volumes. Back, Forward and Up, and an address box: Enter opens a typed folder; a typed file closes with it in the open modes, is the file to write in WriteFile mode, or opens its folder in Folder mode; a path that leads nowhere does nothing
  - The list: folders first, then by name, with digit runs compared as numbers ("Track 2" before "Track 10") and letters without case. Name, size, last write time in local time and type, the last three as wide as the font needs. Toggles for hidden files and for extensions (the names without them, folders unchanged), drawn in an accent color while on; the filter box narrows the list to its extensions, without case
  - Folders are listed, filtered, sorted and written out on a worker thread (BackgroundWork), so a slow drive doesn't stop any window from drawing. The list shows the old folder until the new one arrives, and the status line says "Opening ..." once that takes longer than 150 ms. A newer request drops the older ones, without waiting behind a listing still stuck on a slow drive: that one is let go to finish on its own, and closing the explorer doesn't wait for it either. The address box and the history change only once the folder could be listed
  - The folder shown is watched (PlatformBridge::FileSystem::DirectoryWatcher) and listed again once its changes pause for 300 ms, or a second after the first at the latest, so a long copy into it shows its progress; the selection and the scroll position stay. When the folder itself is deleted or moved away, the list goes to the nearest folder above it that still exists
  - Sorting: a click on a column header sorts by it (Name, Size, Modified or Type), folders first either way and names breaking ties; a second click reverses the order. The header marks the column with ▲ or ▼, from the Geometric Shapes block the explorer adds to its font
  - Selecting: a press selects a file, Alt+press adds or removes one, Shift+press a range, and a drag from off the selection draws a band (MultiFile; see Table)
  - Keys, from the moment it opens: the list's arrows, Home, End, PageUp, PageDown, Shift and Ctrl+A, plus Enter (opens the selected folder, otherwise does what OK does), Backspace and Ctrl+Up (up), Ctrl+Left and Ctrl+Right (back, forward), Escape (cancel) and F5 (lists again, keeping the selection and the scroll position). Going up or back out of a folder selects it in the list. The file operations' keys work while their flags are set: F2 (rename), Ctrl+Shift+N (new folder), Delete, Shift+Delete (delete permanently), Ctrl+C, Ctrl+X, Ctrl+V and Ctrl+Z
  - New folder asks for a name in a prompt, a free one filled in and selected ("New folder (2)" when taken), and selects the folder once made. Rename edits the name right in the list (F2, the Rename button, the menu, or a slow double-click): with extensions shown only the name before the extension is selected, with them hidden the extension stays. A taken name, an empty one or one with \ or / is refused in the status line and the old name comes back; a change of case alone is a rename
  - Delete moves the selection to the recycle bin and selects the row taking its place, so Delete can be pressed again. Items the bin can't take (a drive without one, an item bigger than the bin, or a bin switched off) are asked about in a prompt, to be deleted permanently, instead of being deleted without a word as Windows' own file operation does. Delete permanently (Shift+Delete) asks first, unless SkipDeleteConfirmation is set
  - Cut, Copy and Paste use the system clipboard on Windows, the way Windows Explorer does, so files copied in either one paste in the other; a cut is taken off the clipboard once moved. A folder can't be pasted into itself. A name taken where an item goes keeps both ("Song (2).mp3"); with AskOnNameClash a prompt asks instead: Replace, Skip or Keep both, for this item or the remaining ones, and closing it stops the paste. Replace moves the old item aside until the new one is in, and puts it back when the new one doesn't make it. A move to another drive copies, then deletes the original once the copy is whole
  - Copies, moves and deletes run on a thread of their own (FileTransfer), one at a time: the status line shows "Copying Song.mp3 (3 of 12)" and a Stop button, which ends the operation after the 1 MB chunk being copied and removes the file copied halfway. Closing the explorer while one runs asks first, Alt+F4 included; confirmed, it stops the operation and closes once that has
  - Drag and drop (DragAndDrop): selected rows dragged onto a folder row or a place button move there on the same volume and are copied onto another; Shift moves and Ctrl copies either way. The folder row or the place is marked while the drag is over it, and the status line says "Move to Music" or "Copy to Music". Past the window's edge the drag becomes the system's (Windows), with its drag image, so the files can be dropped into Windows Explorer or any program taking files, which copies or moves them itself. Files dropped on the explorer from another program are copied into the folder row or place under the pointer, otherwise into the open folder, and moved with Shift held; rows dragged out and let go over the explorer again drop as they would inside. Drops run as transfers, so they show progress, can be stopped, ask about name clashes with AskOnNameClash and can be undone
  - Preview pane (ImagePreview, MediaPreview) beside the list: the picture of an image (PNG, JPG, BMP, GIF, TGA, SVG), a song's cover art (MP3, FLAC, WAV, Ogg, Opus, M4A, ADX, AHX, HCA, AIFF, MKA, WavPack, CAF, MP2, and tracker modules by every extension libopenmpt knows when the build has them) or a video's frame from 10% in (WebM, MKV, MP4, MOV, OGV, MPG, AVI), shown by a Video widget and for royalty-free codecs only; an H.264 video says it isn't royalty-free. A WebM or MKV with sound only is shown as a song. Below it the details: dimensions; title, artist, album, length and format (a module's by its tracker, "Impulse Tracker 2.14"); length, size, frame rate and codecs; a folder's item count; several items' count and size. Files are read and decoded by the decoder helper, a process of its own (see IsolatedDecoder), from a worker thread; a stuck download is let go after two seconds, and only the newest selection is shown. A cloud file that isn't on this computer is downloaded to be previewed. The selected video stays open and paused, the groundwork for playing it in the pane - opened only once the helper decoded a frame of it without crashing; a row for its controls is kept free
  - Thumbnails (the Thumbnails toggle, or the flag to start with it on): a 72 px column first and rows 64 px tall, showing an image scaled down, a video's frame from 10% in for royalty-free codecs, a song's cover art, and an icon for folders, other files and whatever has no picture (Resources/img: FolderIcon, FileIcon, ImageIcon, AudioIcon, VideoIcon). Only the rows in view are decoded, by the decoder helper from a worker thread, a few at a time so scrolling fast doesn't queue rows long gone; a file changed on disk gets a new one, a job stuck for five seconds is let go, and a cloud file that isn't on this computer keeps its icon rather than being downloaded. The 512 thumbnails seen most recently are kept (about 8 MB), the rows in view always among them
  - Undo (Ctrl+Z, the Edit menu) takes back the latest operation, whichever explorer in the app did it (FileUndoHistory): it removes a new folder that is still empty, renames back while the old name is free, sends copies to the recycle bin, moves moved items back, and restores deleted ones from the bin, under a free name when theirs is taken now
  - The status line shows a message (an error, an outcome, a question) until the next one, until another folder opens, or until the user selects other rows; under it a note: why the list is empty, "Opening Music...", or "3 selected"
  - A double-click opens a folder, and in the open modes picks the file. WriteFile mode adds the filter's extension to a name without one, and asks once ("Replace") before naming a file that exists
  - The listing comes from PlatformBridge::FileSystem, which reports the hidden attribute with each name, and paths stay native until they are handed out as UTF-8, so names outside the system's code page and decomposed (NFD) names come back the way they are stored
  - SetFontFamily (Arial 18 by default; the explorer adds Latin-1 to its font itself)
- Media Playback
  - AudioPlayback
    - Plays a file through miniaudio's high-level engine, which owns the output device, decoder, resampler and mixer. WAV, AIFF (AIFF-C too: little-endian "sowt", float, A-law and µ-law, not IMA4), FLAC and MP3 decode inside miniaudio itself, with no FFmpeg involved
    - Every other format is decoded by FFmpeg (AudioStream): Opus, Vorbis, and audio in Ogg, Matroska/WebM and MP4 alike. A reader thread feeds the audio stream's packets to a decode thread (AudioTrack, AudioDecodeThread), which converts them to 32-bit float and writes them into a ring buffer miniaudio reads as a data source (RingBufferSource); position, duration, seeking, the end of the stream and starting over after it come from that path's own clock. Codecs outside DecodingPolicy's allowlist are refused (an MP4's AAC, for instance)
    - Open / Start / Stop, with Stop rewinding to the start and the file staying loaded
    - Seamless loops (SetLooping, IsLooping, SetLoopPoints, GetLoopPoints): between the file's own loop points (the HCA or ADX header, a WAV smpl chunk, LOOPSTART tags), between points set in samples at the file's rate, or over the whole file. On the FFmpeg path the reader goes back to the loop start at the loop end and the decode thread cuts the frame the end falls inside, so the loop is sample-exact; the clock goes back with it. Checked sample for sample against plain decodes (HCA, Ogg Vorbis with both points inside packets, a WAV whose samples count their own position)
    - HCA (with the royalty-free checks off): encrypted files play with the game's key (OpenAudioFile(file, HcaKey { ._KeyCode, ._Subkey })), cipher types 0, 1 and 56. A missing or wrong key fails the open, since the first block's checksum doesn't match, rather than playing noise
    - Tracker modules (MOD, XM, S3M, IT, MPTM, MO3, UMX and about 60 more formats) through libopenmpt (BSD), recognised by content like everything else: ModuleSource is a miniaudio data source that renders the module at the device's rate, so seeking, position, length and the end work as for a WAV. Looping repeats the module from its own restart position, as its composer set it, and the clock goes back there with it. Multi-song modules (GetSubsongCount, SelectSubsong; -1 plays them all in a row). The CMake option RETROFUTURAGUI_TRACKER_MODULES (on) leaves libopenmpt and what it brings along (mpg123 under LGPL 2.1, linked dynamically, libvorbis, libogg, zlib - for modules with compressed samples) out of a build entirely
    - Seeking and position reporting in **milliseconds** (Seek, GetPosition, GetDuration). miniaudio's own seconds-based calls convert against the file's sample rate rather than the device's, so a 44.1 kHz track on a 48 kHz device still lands where it should
    - Master volume, and playback speed via resampling (SetPlaybackSpeed). Speed and pitch are coupled, the way a record player couples them
    - Streamed rather than fully decoded into RAM, so an album-length FLAC costs no more memory than a short one. Seeking still works while streaming
    - Per-channel level metering (GetChannelVolume, GetChannelCount): a passthrough node measures RMS on the audio thread and publishes it through atomics, so reading a level from the GUI thread never touches an audio buffer and never blocks. Fast attack, slow decay, so it stays readable when polled once a frame
    - The engine is deliberately neither copyable nor movable: miniaudio objects are transparent structs whose address has to stay put
  - MediaSource
    - The demuxer, one per file. FFmpeg recognises the format from the content rather than the extension, so a WAV renamed to .bin still opens
    - Stream discovery (FindBestStream, GetStream, GetStreamCount), ReadPacket, and read-only access to the format context for metadata. FindVideoStream finds the first moving picture, skipping the cover art songs carry as a one-picture video stream, which FindBestStream can return
    - Seeking in milliseconds: lands on the keyframe at or before the target, and allows for containers whose timeline doesn't start at zero
  - Decoder
    - One per stream: packets in, frames out, the same calls for audio and video. Checks the codec against the licensing allowlist when it opens (IsCodecAllowed)
    - Royalty-free codecs only, by default: the CMake option RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY (on) becomes a compile definition, so the check folds away at compile time. Switched off, a build for personal or non-commercial use decodes every codec its FFmpeg build has (H.264, HEVC, AAC, ...), and CMake warns that shipping it means taking on those codecs' licences. AudioMetadata and VideoMetadata report both whether a codec is free and whether this build decodes it (_CodecIsAllowed, _VideoCodecIsAllowed, _AudioCodecIsAllowed); the FileExplorer's preview and thumbnails go by the latter
    - The royalty-free list (FreeCodecs): VP8, VP9, AV1, MPEG-2, MJPEG, FFV1, Theora; FLAC, Vorbis, Opus, MP3, MP2, ALAC, WavPack, IMA and Microsoft ADPCM, and every kind of uncompressed PCM (all sizes, both byte orders, float, A-law, µ-law) - so WavPack, CAF and lossless M4A play, and PCM in Matroska
    - SendPacket / ReceiveFrame pass FFmpeg's codes through, so "needs more input", "finished" and "error" stay distinguishable; Flush discards buffered frames after a seek
    - Keeps the stream's index and time base, so a caller can route packets and turn frame timestamps into seconds
    - Verified sample-exact: a FLAC decodes to exactly the 16,689,792 samples its STREAMINFO header states
  - Packet: owns an AVPacket for its whole lifetime, so an early return can't leak one
  - AudioMetadata (AudioMetadataReader::Read)
    - Everything a file says about itself, without playing it. Tells WAV, FLAC (native or in Ogg), MP3, Ogg Vorbis, Opus, M4A, ADX, AHX, HCA, AIFF, WebM, Matroska and tracker modules apart by container and codec, never by extension - WebM from other Matroska by its EBML header's DocType
    - Common fields: duration, sample rate, channels and layout, bit depth, bit rate, all tags (merged from file and stream - Ogg keeps its comments on the stream), chapters and cue markers, embedded cover art, and whether the codec is on the allowlist
    - Per-format details: WAV smpl loops; FLAC block/frame sizes and audio MD5; MP3 ID3 versions, VBR/CBR and LAME gapless delay/padding; Vorbis vendor and bit-rate hints; Opus pre-skip, input rate and output gain; M4A brand, AAC profile and iTunes gapless info; ADX version, encryption and loop points (samples and bytes); HCA version, block count and size, encoder delay and padding, cipher type, loop blocks and comment (AudioMetadataReader::ReadHcaHeader reads the header alone); a tracker module's format and the tracker that saved it, song message, channel, order, pattern, instrument and sample counts, and subsong, instrument and sample names (ModuleMetadata, read by libopenmpt); AIFF and AIFF-C compression type and name, markers with their names, and the INST chunk's base note and sustain and release loops (FFmpeg reads only COMM); WebM's and Matroska's DocType and version and the muxing and writing applications (AudioMetadataReader::ReadMatroskaHeader reads a file's start alone)
    - The sample count is what Decoder actually produces: Opus pre-skip, LAME gapless trimming (including the MP3 decoder's own 529-sample delay) and iTunes priming are taken out. Checked against full decodes of FLAC, MP3, Opus, Ogg Vorbis, Ogg FLAC and WAV files
    - One normalized loop region (_Loop, end exclusive) from wherever the format keeps it: the ADX or HCA header, a WAV smpl chunk, an AIFF's sustain loop between its markers, or LOOPSTART / LOOPLENGTH / LOOPEND tags. AudioPlayback loops between it
    - Reads FFmpeg leaves out: ADX loop points from the header (and its exact length - the demuxer only estimates one), WAV smpl chunks, the MP3 Xing/LAME header and the Vorbis vendor string
    - Still reads streams FFmpeg can't decode, such as AHX and type 4 ADX, whose channel count FFmpeg wipes once its decoder refuses them
    - Cover art is checked for stray bytes before the picture, so the image arrives whole: it starts at the picture's own JPEG or PNG signature, looked for in its first 16 bytes. An MP3 whose ID3 picture frame has an extra 00 after its description, as some taggers write it, shows its cover here, where Windows Explorer and online tag readers find none. The check covers every format's cover art, not only MP3's
  - VideoMetadata (ReadVideoMetadata)
    - What a video file says about itself without decoding a picture: container, video and audio codec and whether each is royalty-free, length, size, frame rate, sample rate, channels, bit rate and title. A song's cover art, stored as a one-picture video stream, isn't taken for a video. Read from the container's headers only (MediaSource's StreamProbing::HeadersOnly) wherever they tell codec, size, rate and length - Matroska, MP4, Ogg: a VP9 WebM opens in under a millisecond instead of 20, a screen recording reads half the bytes. A file without a header, an MPEG program stream, is still probed as before
  - VideoStill (DecodeVideoFrame)
    - The picture a video shows at a moment, as RGBA pixels without GL (a DecodedImage), so a worker thread can make a thumbnail: royalty-free codecs only, scaled down to fit a square when asked, widened where the video stores wide pixels, flipped for OpenGL when asked. Past the end it gives the last picture
    - DecodeCoverArt: a file's cover art (AudioMetadata's _CoverArt with its CoverArtCodec: JPEG, PNG, BMP, GIF, WebP, TIFF) decoded by FFmpeg into the same RGBA DecodedImage, scaled the same way. The picture codecs are on the royalty-free list (GIF's and TIFF's LZW patents expired in 2003-04, WebP is royalty-free)
  - Decoding apart from the program (IsolatedDecoder, MediaPreview)
    - The previews and thumbnails the file explorer shows of files the user never opened are decoded by the decoder helper, RetroFuturaGUIDecoder, a program built beside the one using RetroFuturaGUI. A file that crashes a decoder, or takes it over, takes only the helper down: that file's preview says it can't be read, and the next request starts a new helper. The program reads the helper's replies as untrusted too (MediaPreviewProtocol checks every length before allocating)
    - On Windows the helper runs at low integrity without administrator rights or privileges - it reads files but can't change the user's, even when the program runs elevated - can't start programs, gets 1 GB of memory at most, and ends with the program (PlatformBridge::ChildProcess, Sandboxed)
    - MediaPreview is the work itself, without GL or widgets: what the helper runs, and what the program runs itself when the helper isn't beside it. Measured on images, an SVG, an MP3, a module, an audio-only WebM, a VP9 WebM and an H.264 MP4: the helper's previews and thumbnails are byte for byte the program's, at the same speed
  - Fuzz targets (tools/fuzz, the CMake option RETROFUTURAGUI_FUZZING): the metadata readers on any file, the HCA and Matroska header parsers, everything the decoder helper does to a file, and the helper's protocol, under MSVC's AddressSanitizer and libFuzzer. A fuzzing build folder of its own builds just them. First runs (2026-11-02): the Matroska header parser read past its data within seconds (fixed); then 2 million header inputs, 665,000 protocol messages and 10 minutes each on the file targets found nothing more. The file targets are slow (a few inputs a second), so they want long runs
  - The libraries that read untrusted files are kept current through vcpkg (FFmpeg 9.0, stb 2026-08, lunasvg 3.5, libpng 1.6.59, zlib 1.3.2, FreeType 2.14, HarfBuzz 14.6, miniaudio 0.11.25, libopenmpt 0.8.7); FFmpeg is built with the zlib feature, for PNG covers
    - Exact, not the nearest keyframe: it decodes forward from the keyframe before the moment. Where a file has no index of its keyframes (an MPEG program stream) a seek can land past that keyframe, so it starts again further back. Checked on FFV1 and MPEG-2 files whose frames each have their own brightness, MPEG-2 with B-frames and a keyframe every 12 frames
  - Note for non-ASCII paths on Windows: file paths reach miniaudio as narrow strings and are opened with `fopen_s`, which reads them in the process codepage. An application that wants to open paths like `…/電気グルーヴ/…` needs an app manifest setting `activeCodePage` to UTF-8 (Windows 10 1903+), and MSVC's `/utf-8` on its own target too - CMake's add_compile_options only reaches targets created after it. Without `/utf-8`, std::print also writes raw bytes the console misreads
  - NTFS doesn't normalize Unicode in file names: a name stored decomposed (NFD - common for files that came from macOS) and the same name typed in code (NFC) look identical but are different files. Paths taken from a file picker always match
- ResourceManager
  - GetTexture(path): the texture of an image file (UTF-8 path), loaded once and shared by every widget showing it - Image, a widget's background picture, Model's material maps. It stays until Free(path), which finds it by the path's hash. Nothing is freed on its own, so an image shown again later isn't loaded again; whoever frees one makes sure no widget still shows it
  - CreateSvgTexture(path): a new SvgTexture for every caller, parsed from the file's text, which is read once and kept until Free(path). Each copy keeps its own colors and size and can be rasterized again at another size. SvgImage, CheckBox, Table (one copy for all its cells), the drop-downs and WindowTab each own theirs; CreateCheckmarkIcon and CreateDropDownArrowIcon make the library's icons
  - Release frees everything while GL still runs; GlfwTerminate calls it
- Rectangles (source/Rectangles): one class per role, each with only the features that role uses. All of them draw one shared quad
  - Plane: a filled rectangle (backgrounds, highlights, tracks). Fill, rounded corners, glass, raster, fog, background gaps, and an image of its own (SetBackgroundImageTextureID, SetBackgroundImagePadding) on a sampler apart from the glass, so the two show together
  - Frame: an outline (borders). Fill, rounded corners, glass, raster, border width and border gaps
  - WavePlane: a Plane that ShowWave cuts down to a sine wave through its middle, with its thickness, height and length. IRangedValue's graph is one
  - ColorField: the ColorPicker's field of every hue, a circle or a square (ColorFieldShape), with a brightness. It computes its colors, so it takes none
  - IRectangle is their base: size, position, rotation and the shared quad
- Third dimension
  - SetRotation turns a widget in all three axes around its center, with everything in it: its parts and, in a container, its widgets. The parts stay laid out untilted and a frame (Shader::FrameScope, uFrame in every vertex shader) tilts them, multiplied down the tree, so a rotated Lasagna, ScrollView or Environment turns what it holds. Rectangles, text and textures stay flat planes; inside an Environment they move, turn and tilt freely
  - Exact pointer at any tilt, under perspective too: readPointer casts the pointer as a ray through the projection and the widget's frame and hands the widget where it meets its untilted layout (PointerState::_Position), next to the window point (_WindowPosition). Hover, clicks, drags, sliders and text carets keep their flat math
  - A drop-down's open list opens tilted with its box and is hit-tested that way; tooltips and context menus stay flat
  - Clipping by clip planes (Shader::ClipScope): the Table, the ScrollView and the Environment cut along their own borders however they are tilted. The planes run from the eye through the borders, so under perspective 3D content is cut where it reaches past the rectangle on the screen, in front of it or behind it
  - Perspective per window: Window::SetPerspective(viewingDistance) puts the eye that many pixels in front of the window's middle, so what lies further back shrinks toward it while the screen plane stays pixel-exact; SetOrthographic() returns to the flat default. Layouts stack flat under perspective, so only what is meant to be 3D shows depth
  - Perspective per Environment: Environment::SetPerspective(viewingDistance) gives it its own, vanishing at its center, whatever its window shows; FollowWindowProjection() returns to the window's. Its widgets draw and take the pointer through it
  - Projection: ProjectionMode (Orthographic, Perspective), SetPerspective with an optional vanishing point, GetEyePosition, GetDepth (the depth layouts spread over)
- Shaders
  - Built from modules: one shader per rectangle class (Quad.vs, Plane.fs, Frame.fs, ColorField.fs). Each feature lives once in ShaderSource/Modules and each fill in ShaderSource/Fills, pulled in with #include, and every class and fill pair compiles to a program of its own, picked by a define
  - Solid Fill
    - Glass Effect
    - Rounded corners
    - Raster (Dotted, Checkered)
    - Fog Effect
  - Linear Shift
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
    - Raster (Dotted, Checkered)
    - Fog Effect
  - Radial Gradient
    - Animated (rotation, motion)
    - up to 256 colors (RGBA)
    - Glass Effect
    - Rounded corners
    - Raster (Dotted, Checkered)
    - Fog Effect
  - HueStar Gradient
    - Animated (rotation, motion)
    - Glass Effect
    - Rounded corners
    - Raster (Dotted, Checkered)
    - Fog Effect
  - Raster (combinable overlay, works on every fill, on Plane and Frame alike)
    - Two patterns share the feature, picked with SetRasterPattern: Dotted and Checkered
    - One width curve drives both, sampled along a configurable direction (SetRasterDegree): each value is a dot's full width across, or a checkerboard square's side length, so the pattern's elements grow and shrink smoothly from one side of an element to the other
    - One grid for every pattern: a cell is the largest width plus the spacing (SetRasterSpacing), so 0 lets the largest dots or squares touch and a larger spacing spreads the grid without changing any element's size. Each element sits centered in its cell, and the fill shows between them
    - That same direction is the axis the pattern animates along, and it turns the checkerboard as a whole
    - Seamless, endless scrolling animation along that direction
    - Dotted: configurable color and opacity falloff (hard edge to soft center-only glow)
    - Checkered: a second color for the other square of each pair. Square edges are antialiased, a gap thinner than a pixel fades in rather than leaving a seam where squares touch, and once squares fall below a pixel the board settles into an even mix of both colors rather than aliasing into moire
    - Alpha-blends over whatever the fill/border/glass effect already rendered; clipped by rounded corners
  - Fog Effect (combinable overlay, works on every fill; Plane only)
    - Dynamic, irregular cloud-like density from a multi-octave fractal noise field
    - Configurable overall opacity, drift speed, per-octave density/weight curve, and clearing threshold (how much clear/fog-free area shows through)
    - Alpha-blends over whatever the fill/raster pattern already rendered; clipped by rounded corners
  - Background Gaps (combinable, works on every fill; Plane only, a Frame carries its own Border Gaps instead)
    - Skips sections of the fill in a repeating solid/gap pattern: no gap, a single one, a set number of them, or tiled across the whole element
    - Offset and length are absolute pixels measured from the edge the pattern starts at, so each segment keeps its size and its distance from that edge as the element resizes, rather than stretching with it
    - The pattern runs along X and can be rotated to any angle, so the same definition gives vertical stripes, horizontal bands or anything between
    - A background is one element rather than four edges, so a single definition covers it - unlike Border Gaps, which describe one edge at a time
  - Frame variants of the fills above (Solid, Linear Shift, Radial, HueStar), including Raster. The outline takes each corner's own radius, so a frame can be rounded on some corners and square on others
  - Line Fill (used by Polyline)
    - A geometry shader widens each segment in world space, so the width stays in pixels however the line is scaled
    - Mitered joins between segments; turns too sharp for a miter get a bevel instead of a spike
    - Anti-aliased edges, faded over about a pixel
  - Text (Text.vs, Text.fs): every fill, Raster, Fog Effect, glass, the image and the mask from the same modules, inside the glyphs and measured across the whole text. Rounded corners and border gaps are left out, since they don't fit glyphs. A color glyph keeps its own colors (uColorGlyphs), tinted toward the fill by uColorGlyphTint, with the raster, fog, glass and mask over them; the text border is drawn with the same shader just behind the glyphs
  - Mask (Modules/Mask.glsl, Plane and Text): a grayscale texture as alpha or as shading, stretched or keeping its proportions
  - Texture
  - Shader Manager (compiles and caches all shaders once)
- Text Renderer
  - Full Unicode codepoint range support via configurable glyph atlasses (FreeType)
  - Big ranges cost nothing until shown: a range of more than 256 code points, CJK's 20,940 ideographs say, renders each glyph the first time a text shows it, into 1024 x 1024 pages added as they fill (about 1 MB and 700 glyphs each at size 25). Loading the CJK block takes 2 ms instead of 2.6 s and about 260 MB; 3000 kanji take 184 ms and 4 pages. Smaller ranges are rendered at once into one atlas, which grows to fit its glyphs. Every atlas and gray page holds one channel, coverage only, a quarter of RGBA's memory; only color glyphs get four-channel pages
  - Multiple font families, weights and slants, resolved from installed system fonts: each weight and slant loads its own file (Arial Bold is arialbd.ttf), and a style the family lacks shows its closest one, the same slant first, then the closest weight. A variable font (Noto Serif JP's single file, say) is set to the weight asked for, as far as its weight axis reaches, rather than drawing its default instance
  - A family that isn't installed shows the default font (FontManager::SetDefaultFont), or else the first font loaded, or else Arial (DejaVu Sans on Linux), and is kept under the name asked for, so it's found again rather than loaded on every call. A debug build says which font stands in
  - Font sizes are pixels. Every font setter takes the size with a FontSizeUnit (Pixels or Points, points at 96 dots per inch, so 25 points are 33.3 pixels as before) and keeps it in pixels; the getters (GetFontSize, GetHeaderFontSize, GetLabelFontSize, GetTitleFontSize) take the unit to answer in. FontManager::ToPixels and FromPixels convert, and GetOfferedSizes lists a bitmap font's fixed sizes in either unit
  - Line layout from the font's own metrics: lines are a line height apart (ascender, descender and the font's gap), the first line of a top-anchored text starts an ascender below the top, and a single line is centered from its lowest to its highest glyph
  - Glyph sources behind an interface (IGlyphSource): FreeTypeGlyphSource renders TrueType and OpenType fonts and single faces of collections (.ttc); a bitmap font source for GameCube and Wii can follow without changing FontManager or Text
  - Glyphs can be rendered at any time: when no GL context is current, they go up through the first window's context (FontManager::SetUploadContext, which Window hands over and passes on when that window closes)
  - Text shaping with HarfBuzz (MIT): kerning, ligatures, combining marks composed (a decomposed ヘ゛ draws ベ), and emoji sequences drawn as one glyph - joiner families, skin tones, keycaps - each line shaped in runs per font file, joiners and marks staying with the character before them
  - Color emoji: a color font's glyphs (gradient paint, flat color layers, color bitmaps) go into four-channel pages and keep their own colors; every other glyph stays on one-channel pages. The raster, fog, glass, mask and border apply to them as to the letters (over the glass they're as see-through as the fill); the fill only tints them, as far as SetTextColorGlyphTint says (0 by default, 1 for the fill's color), and the text's image leaves them out, since either would hide their colors
  - Gradient emoji (COLR version 1), the look of Windows 11's Segoe UI Emoji: ColorGlyphPainter draws a glyph's paint as HarfBuzz walks it - transforms, clips to outlines and rectangles, solid colors, linear, radial and sweep gradients with their pad, repeat and reflect ends, and groups laid together in every composite and blend mode. FreeType fills the outlines; nothing else is needed. Glyphs without it fall back to their flat layers (COLR version 0). Each glyph is drawn once and kept in its page; a note full of new emoji at 53 pixels takes about 85 ms in Debug. The pixel loops step through the paint's space with plain floats, every glyph source of one font file shares one HarfBuzz face (Segoe UI Emoji's megabytes of COLR are checked once, not once per Unicode block; the first emoji shown after starting pays that, about 130 ms in Debug), and whether a glyph has gradient paint is asked of FreeType, which reads the table only as far as it needs
  - Vertical flow (SetTextFlow): lines become columns read top to bottom, laid out left to right or right to left (Japanese, Chinese), with the font's vertical metrics and its vertical forms of punctuation and brackets. Text boxes edit in columns too: the caret lies across the column, a selection runs down it in tall boxes, Up and Down step along the column and Left and Right go to the next column that way
  - Right-to-left scripts (Arabic, Hebrew) with SheenBidi (Apache 2.0): each line is split into runs of one direction by the Unicode bidirectional algorithm, its direction taken from its first strong character, and each run is shaped its way (brackets mirrored, Arabic joined) and laid out as it's drawn. A right-to-left line is set against the text's right edge, where it starts; boundaries sit at each character's start, the arrow keys walk the line as it's drawn (Text::GetBoundaryBeside), and a selection across both directions shows as one box per stretch drawn without a break (Text::GetSelectionAreas)
  - Per-character fallback: a character its block's font lacks, or one no block covers, is drawn by the first other font of the fontset that has it, else by the default font (SetDefaultFont, else the first font loaded), each shaped in a run of its own; what was found is kept per character until the fontset is extended
  - Text border (SetTextBorderWidth): every glyph's outline widened by FreeType's stroker and drawn behind it, with colors per state, every fill and its own image (SetTextBorderColors, SetTextBorderFillType, SetTextBorderBackgroundImageTextureID and the rest); borders of each width are rendered once into the glyph pages
  - Fallback/extended fontsets for filling in missing glyph ranges
  - Dual UTF-8 / UTF-32 string encoding (DoubleEncodedString), always kept in sync
  - Text Alignment (Left, Right, Center)
  - Text Padding & horizontal scroll offset for overflowing text
  - Glyph boundary lookup (used for caret positioning & selection); the characters inside a shaped cluster share the boundary at its end, and the arrow keys step over a cluster at once
  - Multiple colors, fills and effects (Text shares FillStyle and FogStyle with Plane): the widget owns the colors and the text keeps a span over them
  - Image fit (SetTextBackgroundImageFit, also on Plane and Background): an image stretched, the default, or keeping its proportions while filling (cropped) or fitting into what it covers, so a text's image no longer widens as the text grows
  - Grayscale masks (SetTextMaskTextureID, SetTextMaskMode, SetTextMaskFit; on Background and Plane too): the mask's gray as transparency, or as shading that darkens below mid-gray and lightens above it. Metal text is the metal as the text's image with a chrome mask of bright sky, dark horizon and rising ground over it (the Notes scene's Metal button). A text's mask can be laid over each line on its own (SetTextMaskSpan(MaskSpan::EachLine); each column across its width), so every line of a multiline text is shaded like a one-line logo
  - Test samples in the TestProject's Notes scene: a second row of buttons puts emoji (skin tones, joined sequences), kanji with kana, Thai with its stacked marks, mixed scripts on one line, Hebrew and Arabic with English mixed in, and vertical Japanese into the note, each combinable with the effect buttons above
- Signals
  - Thread-safe Connect/Disconnect
  - Synchronous (Emit) and asynchronous, threaded (EmitAsync) dispatch
- BackgroundWork
  - One worker thread running jobs in the order they were posted: Post(work, done) runs work(std::stop_token) on the worker, and done(result) on the thread that calls Drain, once per frame, so the slot can touch widgets and GL objects
  - ClearPending drops the jobs not yet started and the results not yet drained; IsBusy tells whether anything is queued, running or waiting
  - Destroying it sets the running job's stop token and drops everything not yet run or drained, so an owner holding one can't receive a result after it's gone. Whether it waits for the running job is chosen at construction (WhenDestroyed): WaitForJob, for jobs that use what their owner holds, or LetJobFinish, which lets a job stuck on a slow drive finish on its own thread and drops its result. The worker's queues are shared with its thread for that, so nothing dangles
- Input Manager
  - Per-window mouse position & button state
  - Focused/hovered window tracking
- PlatformBridge (cross-platform system layer)
  - Windows & Linux support
  - Clipboard: text on Windows, and files and folders the way Windows Explorer puts them there (CF_HDROP with the copy or cut effect); Linux keeps both inside the app for now
  - Recycle bin: moving items to it without a dialog, refusing those it would delete permanently, and restoring them (the Shell on Windows, the freedesktop trash on Linux)
  - Dragging files out of the app onto other programs, with the system's drag image (DragDrop, Windows)
  - System font enumeration (family, weight, slant, Unicode ranges). On Windows the names and the style come from the font file itself (the typographic family, the OS/2 weight and style bits), so Arial Bold is the family Arial at weight 700 rather than a family "Arial Bold"; older family names ("Segoe UI Semibold") still find their face. Every face of a collection (.ttc) is listed with its index, and fonts installed for the current user only are found too
  - Keyboard input, held-key tracking, per-window key capture. When the keys move to another window, keys let go meanwhile are let go here too, so none stays held. IsKeyPhysicallyDown asks the system directly, for moments the app's window has no keys, like a drop from another program
  - Mouse button state & window-relative cursor position, and whether a window is the topmost one under the cursor
  - Monitor enumeration (Windows)
  - Executable path & working directory utilities
- Cross Language Capabilities
  - dll/so/dylib compilation
  - Widget ID Manager (DynamicLibWidgetManager) addressing widgets by string ID
  - Signal connection via C callbacks (OnClick, OnRelease, OnMouseEnter, OnMouseLeave, WhileHover, OnTextChange, OnEnterPressed, OnEnterReleased)
  - OpenFileExplorer(title, width, height, startPath, flags, extensions, onResult, onDismiss): a FileExplorer over the main window, its flags as FileExplorerFlags' bits and its strings UTF-8. onResult gets the chosen paths as an array of UTF-8 C strings, valid during the call (PathsCallbackType); onDismiss hears it closed without. It returns false when Open refuses the flags, two modes for instance
  - The bindings carry FileExplorerFlags bit for bit (C# [Flags] enum, Python IntFlag, Rust constants in file_explorer_flags) and copy the paths while the callback runs: C# and Python take a function getting a list of strings and keep their callbacks alive while the explorer is open, Rust reads them with paths_from_callback. Tried from C#: a selection of three files, one with a Japanese name, arrives whole after a forced garbage collection, Escape arrives as a dismiss
  - A host exits cleanly: Draw destroys the main window, and GlfwTerminate the textures ResourceManager holds (ResourceManager::Release), while GL still runs; left to the unloading DLL, textures crashed it on the way out
  - C#
  - Python
  - Rust

</details>
  


### ToDos before milestone 5
<details><summary>CLICK TO EXPAND</summary>
  <ul>
<li>Audio formats, once they're needed</li>
  <ul>
  <li>MIDI (needs a synthesizer such as TinySoundFont - and a SoundFont whose sample sources are clear for commercial use) and chiptune formats each need a new dependency</li>
  </ul>
<li>Check whether loading a video's preview picture can be improved. Today the frame from 10% in is decoded up to three times: the decoder helper's test decode, the preview pane's own Video opening the file in the program, and the thumbnail. Each seeks to the keyframe before the moment and decodes forward, up to 300 frames. Leads: show the helper's picture in the pane until playback is asked for, keep the stills already made, take the nearest keyframe instead of the exact moment, decode at a lower resolution, use a picture the file carries itself (a Matroska or MP4 cover)</li>
<li>Text</li>
  <ul>
  <li>A text box's selection boxes are worked out in world space when the selection changes, so they stay behind when the box is moved or resized (by its layout, say) while text is selected; they should follow the text</li>
  <li>Backspace should delete a whole combined emoji sequence, not just its last code-point, which can leave the emoji changed or corrupted</li>
  <li>Bitmap fonts for GameCube and Wii: an IGlyphSource that hands out fixed sizes (GetPixelSizes) and can't shape; FontManager, Text and the size conversions are ready for one</li>
  </ul>
<li>Shaders</li>
  <ul>
  <li>cut corners</li>
  <li>radial shift</li>
  <li>conic radiant shift</li>
  <li>noise options</li>
  <li>Double-check how the raster spacing is described: SetRasterSpacing gives the gap between the largest elements (0 lets them touch), and the distance between element centers might suit the patterns better</li>
  <li>optimize shader handling</li>
    <ul>
    <li>only pass parameters on initialization and on parameter value change. Skipping uploads that repeat what the program holds doesn't pay off: 620 of the 948 uploads per frame do, but comparing each value cost as much as the uploads it saved (Debug, 2026-10-26). Uniform buffers per rectangle, updated only on a change, could get there</li>
    </ul>
  </ul>
  <li>Custom icons from images</li>
  </ul>
<li>Animation Engine: what transitions don't cover yet (colors, corner radii, positions, sizes, rotations and opacity have them, see Animation)</li>
  <ul>
  <li>Per-cell hover in elements drawn once per cell (Table cells, MenuBar items, WindowTab fields, ComboBox items), which needs a shown state per cell</li>
  <li>SVG icon colors (checkmark, drop-down arrow, tab close icon), whose every change rasterizes the SVG again; raster and fog colors</li>
  </ul>
<li>Test Carbon compatibility</li>
<li>Add more widget interactions to the DynamicLibWidgetManager</li>
  <ul>
  <li>SetRotation, SetPosition, SetSize</li>
  <li>Set Color/Decoration parameters</li>
  </ul>
<li>C++ modules once ALL common compilers (GCC, CLang, MSVC) support them</li>
<li>Wiki/Documentation</li>
<li>Complete Python binding</li>
<li>Goose binding, built the way Goose's documentation describes its interoperability</li>
<li>Clean up API smell before the interoperability binding below, so each function is exported once, in its final form: public data members and helpers on IWidget, signals without Connect_ functions (ScrollView's OnXScrolled/OnYScrolled/OnZScrolled), SetEnabled ignoring emitSignal, misspelled names (ColorSate, DualPrevieAlignment), and mixed parameter types for the same thing (const std::string&amp; and std::string_view names, float beside f32)</li>
<li>General refactor across the framework</li>
  <ul>
  <li>windows.h still reaches Table.hpp and FileExplorer.hpp, through PlatformBridge's Definitions.hpp, for the PB_KEY_* values of their key members. Resolve this entirely once Android support is tackled. NOMINMAX keeps its min and max macros out everywhere, but its other macros still come along. Only PlatformBridge can offer key names without windows.h</li>
  <li>Refactor MenuBar's SetItemCornerRadii, setItemBackgroundCornerRadii and setItemBorderCornerRadii functions</li>
  </ul>
<li>SVGs on meshes, once they are needed: SvgTexture keeps only the SVG data - the picture, the masks of recolored paths and their colors - and the geometry that shows it binds to that data in one universal way, whether it is the quad inside SvgImage or a Mesh. Today SvgTexture owns the quad it draws on, so a Mesh can't show a recolored SVG</li>
<li>Overload Window::SetBackgroundImage so it also takes a Texture*, letting a texture that is already loaded (ResourceManager::GetTexture, a decoded picture) be the window's background, and with it the glass backdrop GetBackgroundImageId hands out, without going through a path</li>
<li>PlatformBridge::Input on Linux: the wheel and the pinch (done on Windows), untested and never compiled</li>
  <ul>
  <li>The wheel only reaches the window keys go to, since PlatformBridge selects button events on that window alone; it has to come from every window of the app, like the Windows message hook</li>
  <li>Two-finger stretch (pinch) on touchpads and touchscreens needs XInput 2.4 gesture events (XI_GesturePinchUpdate) and libXi; only Ctrl+wheel pinches so far</li>
  </ul>
<li>Bring the interoperability binding (DYNLIB_MODE, DynamicLibWidgetManager, the C#, Python and Rust bindings) up to date with scenes and the window mechanisms. Last on the list, since it builds on the items above</li>
  <ul>
  <li>Widgets are registered under their bare name (AddWidget(widget->GetName(), ...)), so of two widgets with the same name in different scenes or windows only the first one registered is reachable. Register them by path instead (GetPath: MainWindow/MyScene/RootLasagna/TestButton), which is what the path was made for</li>
  <li>Nothing is ever unregistered, so a closed scene (SceneLoader::CloseScene), a closed window or a destroyed prefab leaves the manager holding freed pointers. A scene's widgets have to leave the registry along with the scene</li>
  <li>The exported functions reach single widgets and the one MainWindow only. Scenes (create, close, activate, reserve an edge), several windows, docking, WindowTabs and popups can't be used from another language yet</li>
  <li>Callbacks are void(*)() only, apart from the file explorer's paths (PathsCallbackType), so signals that carry values - WindowTab's OnSelectionChanged with its Window*, other popups' results - have no way out</li>
  <li>The Rust binding doesn't compile: new() moves the Library into the struct and then borrows it for its symbols, and the libloading::Symbol fields have no lifetime. Its file explorer parts were checked on their own</li>
  <li>DynamicLibWidgetManager's setters take no emitSignal, so a binding can't choose whether a change fires the widget's signal: SetText passes false to Button::SetText and Label::SetText, and TextBox's SetValue goes without one. Add an emitSignal bool to the setters whose widgets have one, to the exports in main.cpp, and to the C#, Python and Rust bindings, with no default, as everywhere else</li>
  </ul>
</ul>
</details>

### ToDos independent of milestones
<details><summary>CLICK TO EXPAND</summary>
  <ul>
<li>Animated text ("boiling" letters, as in hand-made video captions): every glyph gets its own small rotation, scale and offset around its center, changing over time</li>
  <ul>
  <li>Worked out in the text's vertex shader from each glyph's index, a seed and the clock, so the mesh isn't rebuilt for every pose and the border, emoji and shaped clusters move along with their glyphs</li>
  <li>Snapping (the pose jumps and holds, from the time times the rate, rounded down) or transferring (eased from one pose to the next with the transitions' Easing)</li>
  <li>Settings: largest rotation, scale and offset, poses per second, snap or transfer with its easing, a stagger so glyphs or words change one after another instead of all at once, per glyph or per word, the seed; set through ITextProperties like the raster and fog, and switched off by default</li>
  <li>Only how the glyphs are drawn changes: the caret, the selection and clicks keep the glyphs' resting places</li>
  <li>The reference (a caption video, measured 2026-11-01): poses snap every 5 frames at 30 fps (6 per second) and hold still in between, all glyphs change on the same frame, about 5 degrees of rotation, a slight scale and a vertical nudge each; captions cut in and out without a transition</li>
  </ul>
<li>A caption background following the text's lines: a rounded box per line, merged into one stepped shape with rounded outer corners and curved inner joins where lines of different lengths meet; it stays still while the letters move</li>
<li>Drop shadow and glow, inner shadow and inner glow, for rectangles (IRectangle) and text</li>
  <ul>
  <li>Drop shadow and glow behind the shape: color, offset (none for a glow), blur radius and spread; inner shadow and inner glow inside its edges: the same settings, drawn over the fill</li>
  <li>Rectangles: their rounded shape has an exact distance field, so a blurred shadow can be worked out in the fragment shader of a quad grown by the blur and offset, with no extra pass</li>
  <li>Text: glyphs have no such shape in the shader, so the coverage has to be blurred - a separable blur of the text into a texture of its own, or glyph pages holding distance fields - and the color glyphs (emoji) cast theirs from their alpha</li>
  <li>Transitions for their colors, offsets and radii, like the other properties</li>
  </ul>
<li>Naming that may confuse: across code, comments and docs</li>
  <ul>
  <li>Say "font family" instead of "face" where a family is meant</li>
  <li>Write "code-point" instead of "code point"</li>
  </ul>
  </ul>
</details>

#### Nice-to-have's
- Better (relative) Viewports
- Custom text renderer with many IANA character sets

### Why?
There are many C++ GUI frameworks available, but most come with one or more drawbacks: restrictive licensing, enforced use of proprietary containers, inflexible window layouts, limited design capabilities, or poor cross-platform support, often confined to C++ only.
RetroFuturaGUI aims to break these barriers!

### Known Bugs
- SvgTexture.cpp
  - Draw() switches the depth test off for its whole draw so that its coplanar mask passes can blend over the base layer. That also lets any SVG drawn later in the frame paint over whatever is already there, whatever the z - an SvgImage in a later Lasagna cell paints over a widget reaching into it, for instance. Keeping the test on and using GL_LEQUAL for the mask passes would keep the blending and respect depth
- TextBox.cpp
  - A TextBox reports WidgetTypeID::Button, so DynamicLibWidgetManager::SetText and ConnectSlot take the Button branch, dynamic_cast to Button* yields null and is then dereferenced. The WidgetTypeID::TextBox branches are unreachable as a result

### Is AI used in this project?
AI is often used for repititive tasks like adding code comments to functions, classes, and structs and updating this readme's ToDo and feature list. Shaders are mostly written by AI and AI is sometimes used for finding bugs and to assist with complicated calculations. The architecture of font loading and Scene management was partially planned with AI. AudioMetadata.hpp/cpp are entirely written by AI. YUV to RGB conversion was done by AI. Multi-window support, transparent windows, window docking, window tabs and popups were written by AI, following a design of mine