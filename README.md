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
| 4 | More Widgets (TextBox ✅, Table ✅, 3D Model ✅, Slider ✅, CheckBox ✅, ComboBox ✅, ExtendedComoBox ✅, RadioButton ✅, RadioButtonGroup ✅, SeparatorLine ✅, ColorPreview ✅, change Grid2d to "Lasagna" and add a 3rd dimension ✅, MenuBar ✅, MultilineTextBox ✅, Histogram ✅, VideoPlayer ✅, Environment ✅, LineDiagram ✅, ScrollView ✅, Color Pickers ✅, WindowTab ✅, window docking ✅, ContextMenu ✅), Scene ✅, SceneLoader ✅, AudioPlayback ✅, Popups ✅, Tooltip ✅, FileExplorer ✅, Big Refactor ✅ | 1 | ✅ | 
| 5 | .bechaml markup language for GUI design 🥣 (**B**eautifully **E**xtended **C**ascading but **H**airbally **A**pplication **M**arkup **L**anguage), StylingManager with flavors ✅ (named, shared styles for colors, corners and shader settings, in place of the planned ColorManager and CornerRadiiManager), DropShadow ✅ and InnerShade ✅ (Text, Border, Background; InnerGlow serves as the inner shade) | 4 | |
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
- EnvironmentView
  - Input doesn't go through a view: hover and clicks reach the world only in the Environment's own window
  - A view draws into its rectangle as laid out, so a tilted container around it doesn't turn it, and its window's perspective doesn't reach it
  - The world's own clipping (a clipped Environment) isn't applied in a view; a ScrollView or Table inside the world is untested there, and glass, pixelation and CRT read the view's window behind them
- SeparatorLine
  - The caption's left padding isn't clamped to the line's width, so a long caption or a large padding runs the gap and its text off the right end
  - The gap always starts from the left edge, so centering or right-aligning a caption means working out the padding by hand. GetText().SetAlignment doesn't do it either: the caption is always centered inside its own gap, and the alignment passed in is overwritten the next time the layout runs
- ColorPreview
  - The checkerboard's two colors and its square size are fixed internals, so neither the board's contrast nor its scale can be set from outside
  - The dual preview is on by default, so a preview that was never asked to be split is
  - Against the light checkerboard the white half reads almost the same as the checkered one - compositing over white and over a white/light-grey board differ only inside the grey squares. Drawing the color at full opacity there instead would carry information the checkered half can't
- Window
  - The docking preview is a window of its own, so it gets a taskbar button while it shows. Hiding it needs WS_EX_TOOLWINDOW on Windows (_NET_WM_STATE_SKIP_TASKBAR on X11), which belongs in PlatformBridge
  - A minimized or maximized window stays out of the docking layout until it is restored, and minimizing a group's root leaves the rest of the group on screen
- Tab (items: root containers it owns, one per field)
  - Dragging a field only reorders the fields: an item can't be dragged out of the strip or into another Tab
  - The strip runs along the top only, and fields that don't fit at their minimum width are left out; the strip doesn't scroll
  - Selecting, reordering and removing by the pointer are untested by input (selection and removal by code are)
  - Not addressable through the C binding (DynamicLibWidgetManager) yet
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
  - Styling is partly there: a popup's window takes flavors like every window (Window::AddFlavor, or .default for all of them) for its background, border and transitions. Its bar has no styling of its own yet, and the library's own popups (the FileExplorer and its prompts) set fixed colors as their own values, which flavors can't override
  - Linux is untested: the owner is set with XSetTransientForHint, and whether the popup stays off the taskbar is up to the window manager
- Third dimension
  - Rectangles, text and textures keep a two-dimensional size: they are flat, and the frame tilts them, so a third size component would go unused
  - Tooltips and context menus stay flat over a tilted widget, by design, and so does a tabbed window's content (see WindowTab)
  - Nested clips in one frame and depth cut to their overlap, but only two differently tilted ones count at once: a third drops the outermost
  - Under perspective, layouts stack within 1 px (Projection::GetDepth), so a Lasagna with many layers gives each a slice close to the widgets' own stacking steps
  - A tilted widget is depth-tested against everything drawn, so a part tilting toward the eye draws over what lies in front of the widget's plane, the window bar included
  - A widget seen exactly edge-on takes no pointer, and under perspective a part projected past the window's edge can't be reached there
- SvgImage
  - No disabled look of its own. Graying it out means recoloring its named paths, which would overwrite path colors set by the application, so whoever owns the image does it
- FileExplorer
  - No styling API: only the font can be set (SetFontFamily). Every color is fixed in FileExplorer.cpp - the bar, the buttons and menus, the list's row, header, selection and drop target colors, the toggles' accent, the scrollbar, the boxes - and the prompts (NamePrompt, ConfirmPrompt, ClashPrompt) copy them. A style set before Open, per widget kind and color state, which the prompts take over too, would let an app match its own look
  - With thumbnails on, the Name column gets what the thumbnails (72 px), Size, Modified and Type leave: about 70 px in the default 1000 px window with the preview pane, a few characters
  - The Thumbnails toggle was tried only by opening with the Thumbnails flag; clicking it, which keeps the selection and the top row in view, hasn't been done yet
  - The preview shows as many detail rows as fit, the most telling first; the rest can't be scrolled to yet. The row for playback controls stays empty until playing is built
  - A cloud file that isn't on this computer is previewed only when its provider downloads just the ranges read (PARTIAL or PROGRESSIVE in the Cloud Files API: the headers and one picture); one that downloads whole files (FULL, such as kDrive) shows "Not on this computer" with the file's details. The partial case hasn't been tried, for want of such a provider here
  - The file operations were tried through their keys and prompts only, by posting key messages to the windows: the menu items, the Rename and Stop buttons, a slow double-click to rename and the clash prompt's Replace and Skip buttons haven't been clicked yet. Of drag and drop only a drop from another program was tried, called the way the system's drop calls it; dragging rows inside, the drop target marks and dragging out haven't been done with the mouse yet
  - Files dragged in from another program show where they would go only once dropped: GLFW takes drops as WM_DROPFILES, which tells nothing while they hover. While rows are dragged out, the system runs the drag in a loop of its own, so the app's windows don't draw until the drop
  - Linux can't drag files out yet: a drag past the window's edge stays the explorer's own
  - A Replace can't be undone in full: Undo takes the copy away, but the item it replaced was deleted. Undo covers what explorers with the Undo flag did, and only while the app runs
  - Redo (Ctrl+Y, and a Redo item in the Edit menu) is still to come: an undone step should go onto a redo list that the next new operation clears
  - The Hidden files and Extensions toggles are buttons drawn in an accent color while on; they become labeled checkboxes once a LabeledCheckBox prefab exists (roadmap 7)
  - Renaming has its own button in the menu strip. Now that the ContextMenu reports clicks, Rename can move to a right-click menu on the rows and the button go; Delete, Cut, Copy and Paste join that menu too
  - Typing a name doesn't jump to it
  - File icons show only with thumbnails on; the plain list has none
  - The menu strip's titles and buttons use the font at 0.8 of its size so they fit; long drive labels still run past their place buttons (see Button)
  - Only filling the table is left on the main thread: storing the cells' strings and resolving the track sizes. In a Debug build the frame that shows WinSxS (35951 entries) takes about 140 ms, System32 (4781) under 40 ms
  - A folder still being listed when the app exits is cut off wherever its thread is, since a stuck listing is let go rather than waited for
  - Names in other scripts need a font holding their glyphs, set with SetFontFamily; the explorer only adds Latin-1 to its font itself
  - Its widgets are sized from the window size given to the constructor, so resizing the explorer's window doesn't refit them
  - From another language it opens over the main window only, and with an extensions string: the named filters stay C++-only. The Python binding's OpenFileExplorer hasn't been run, since no Python is installed here
  - Linux is untested: PlatformBridge::FileSystem's Linux side (listing, watching, the trash) and the in-app clipboard are written but have never been compiled
- AudioPlayback / MediaSource / Decoder
  - Packet owns an AVPacket's lifetime, but MediaSource::ReadPacket still takes a raw AVPacket* and leaves the unref to the caller. A Frame wrapper and a reader that runs the send / receive / drain / flush-after-seek protocol behind one call aren't built yet, so every caller currently repeats it by hand
  - A reader built on FindBestStream drops the packets of every other stream. That's right for audio alone; audio and video from one file will need a packet queue per stream
  - A WAV, FLAC or MP3 with loop points plays through FFmpeg instead of miniaudio: miniaudio's streamed sounds loop without a gap only over the whole file (a seek inside a stream is asynchronous, and its cursor wraps at the file's end, not the loop's). Loop points set on a file miniaudio plays reopen it through FFmpeg, which restarts its audio briefly
  - HCA cipher type 1 (the fixed table) is built but untested, for want of a sample file
  - Tracker modules seek to the nearest row, not the exact sample (libopenmpt seeks by pattern position), and their length is libopenmpt's estimate: a song played once runs a few hundredths of a second past it, to the end of its last row. SetLoopPoints refuses a module, which loops by its own patterns
  - AudioMetadata doesn't read the Opus vendor string (it sits in OpusTags, which FFmpeg skips) or APE tags; WAV loops come from the first smpl loop only, and RF64/BW64/RIFX files aren't scanned for smpl at all
  - The decoder helper restricts itself on Windows only (low integrity, no programs of its own, 1 GB at most, ending with the program); on Linux it runs as a separate process with the program's rights. The preview pane shows the picture the helper decoded, so the program never opens a video just to show it; playing one in the pane (still to come) will open it in the program, only once the helper decoded a frame of it without crashing
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

- Styling (every widget, Window, Tooltip and ContextMenu)
  - A widget's look is a set of parts reached by getters: `button.GetBackground().SetColors(...)`, `textBox.GetBorder().SetWidth(...)`, `label.GetText().SetFontFamily(...)`. Beyond Background, Border and Text, widgets hand out their own: GetIndicator and GetGraph (Slider, ProgressBar, Histogram, LineDiagram), GetCheckmark (CheckBox), GetArrow and GetList (drop-downs), GetItemText (ComboBox), GetPreview (ExtendedComboBox), GetItemStyle and GetSeparatorLine (MenuBar), GetFieldStyle and GetCloseButton (WindowTab), GetTrackStyle, GetCellStyle, GetHeaderStyle, GetCellCheckBox and GetSelectionBand (Table), GetHoverText (LineDiagram), GetPlaceholder, GetCaret and GetSelectedArea (text boxes, Table)
  - Colors per state (Enabled, Disabled, Hover, Clicked, Selected), each a list of colors: solid, animated Linear, Radial and Conic Shift gradients, raster patterns (Dotted, Checkered, Striped, Grid), fog, glass, images and recolorable SVGs, grayscale masks
  - Shape: corner radii per corner, each corner rounded or cut off straight (SetCutCorners, the radius as the length cut off), border widths and gaps, and a gap between the border and the background (SetBorderGap on the widget, per side), which moves over its own transition
  - Effects on backgrounds, borders and text, each with every fill and colors per state that follow their part's state (a state without colors shows the Enabled ones):
    - Drop shadow (GetDropShadow()): angle, distance, blur and scale
    - Glow (GetGlow()): light fading outward over a width. A border glows inward too while it has a gap to the background; EnableGlowOverBackground and EnableGlowOverBorder decide whether the border's and the background's glow cover the other
    - Inner glow (GetInnerGlow()): light fading inward over a width; set off by an angle and a distance in a dark color, it is an inner shadow
    - Their angles, distances, blurs, widths and scales move over the Effects transition. A text renders its effect glyphs once per blur, width and offset, so for those it crossfades from the glyphs at the old values to the new ones; its shadow's angle, distance and scale move as on rectangles
  - Text: any installed font family, weight and slant, sizes in pixels or points, alignment, padding, outline, vertical flow, color emoji, shaping and right-to-left scripts (see Text Renderer)
  - Transitions carry colors, corner radii, positions, sizes, rotations, opacity, border gaps and the effects' geometry to new values over time (see Animation)
- Widgets
  - Label: shows text
  - SeparatorLine: a line dividing content, with an optional caption set into it
  - Button: click, release, hover, mouse enter and leave actions (OnClick, OnRelease, WhileHover, OnMouseEnter, OnMouseLeave) and a text
  - CheckBox: a boolean value toggled by clicking, the Button's actions, optionally mirroring a bool owned elsewhere (EnableInheritedValue)
  - RadioButton and RadioButtonGroup: options of which only one is picked; the group builds and owns its buttons and their labels in a grid (AddRadioButton, GetRadioButton, RemoveRadioButton)
  - ComboBox: picks one text item from a drop-down list (AddItem, RemoveItem, SetIndex, OnIndexChanged, OnItemClicked). With a caption it is a menu, items can be disabled, and the open list covers the widgets around it. SetIndex clamps to the last item and says whether OnIndexChanged fires; removing items keeps the selection on its item, and removing the selected one hands it to the item moving into its place. The list shows up to SetMaxVisibleItemCount rows (8 by default), scrolls by the wheel and opens with the selected item in view; the closed box cuts its text off before the arrow, and a disabled box shows its text disabled and closes its list
  - ExtendedComboBox: a drop-down whose rows are whole widgets (AddItem&lt;T&gt;, RemoveItem), with the same row cap and wheel scrolling; removing rows keeps the selection on its row, and a removed selected row hands it to the row moving into its place, as in the ComboBox
  - TextBox and MultilineTextBox: typing with the system's key repeat, Backspace deleting a whole emoji sequence or flag (a diacritic goes on its own), the caret placed by clicks and arrow keys, mouse-drag selection, select all, copy, cut and paste, read-only mode and a placeholder (OnTextChange, OnEnterPressed, OnEnterReleased, OnCopy, OnPaste). The keys go to the text box clicked last, in any window. A TextBox can hold a typed value
    - Insert mode (the Insert key): typing overwrites the characters after the caret, up to the line's end, and the caret becomes a block behind the character it replaces, reaching halfway into the gaps beside it, in columns too (a bar under it in Table cells)
    - The caret can glide to its new place (GetCaret().SetMoveTransition): every move takes the same time however far it goes, and moves coming faster than that, as with a held arrow key or fast typing, take only the time between them, so the caret keeps pace
  - Slider: a value of any numeric type between a min and a max, set by dragging the indicator, clicking the track or stepping, horizontal or vertical, either direction (OnValueChanged, OnValueSet)
  - ProgressBar: a Slider's value and display without the dragging
  - Their indicator is a Stroke (square corners), a Circle (round, its diameter the smaller of its two sizes) or a Pill (its size kept, rounded fully at the shorter sides); the indicator and the graph stay inside the track's border, following its width when it changes
  - Histogram: one bar per value of a bound array, read every frame, so data updated in place shows live (SetData)
  - LineDiagram: one line per bound data set, live like the Histogram; the hovered line is highlighted with its nearest value, labels and hexadecimal values on request; signals tell which line and data point the pointer hovers or presses (OnHoverChanged, OnHoverEnded, OnPointClicked; GetHoveredPoint asks at any time); lines of different lengths are spaced by the longest set or each stretched across the whole track (SetLineSpacing); a glow all around the lines, their ends and turns included, in their own colors or the glow's (GetLineGlow, EnableGlowLineColor); translucent lines blend once where their pieces overlap at a turn
  - ColorPicker: picks a color from a field of every hue, with brightness and optional alpha (SetColor, SetBrightness, SetAlpha, OnColorChanged)
  - ColorPreview: a color over a checkerboard, so transparency reads as such, optionally split against white
  - Table: a grid of text, number, color, checkbox and image cells
    - Cells are typed by what is assigned to them, and editable text cells work like a TextBox, per cell; read-only tracks
    - Row selection (single or multiple, Alt and Shift, a drag band), dragging the selection, keyboard navigation (arrows, Home, End, PageUp, PageDown, Ctrl+A), the mouse wheel and scrolling from outside (a Slider)
    - Rename-style editing from code (EditCell, EndEdit), Enter keeps and Escape cancels (OnCellEditEnded)
    - Tab and Shift+Tab keep the edit and open the next or previous editable text cell in reading order, wrapping around, skipping read-only tracks and other cell kinds; the new cell's text is selected, it scrolls into view and its row becomes the selected one. Without an edit, Tab opens the selected row's first editable cell and Shift+Tab its last. The arrow keys move the caret within a cell
    - Star and Fixed track sizes, headers on any side, alternating track colors, double-click and slow double-click (OnRowDoubleClicked, OnRowSlowDoubleClicked)
    - Track states: the track under the pointer shows its Hover colors and the pressed one its Clicked colors, over Selected; checkbox cells' checkmarks take the tracks' icon colors; the header bands get an outer border
    - Costs the same however many rows it holds: only what is in view is drawn, through a pool of shared elements
  - Image: shows a picture file (stb_image formats and SVG) or a decoded picture, its aspect ratio kept; pictures can be decoded on another thread
  - Video: plays a video with its sound (Play, Pause, Stop, Seek) and hands out the sound's levels and frequency bands
  - SvgImage: shows an SVG at any size, its named paths recolorable
  - Model: loads 3D models with their textures (Assimp)
  - Tooltip and ContextMenu: every widget can have a tooltip (SetTooltipText) and a context menu of items and submenus (AddContextMenuItem, AddContextMenuNode); a click on an item closes the menu and reports its index path from the root list down (Connect_OnItemClicked, GetClickedPath), a submenu's row only opens its list
- Containers
  - Every container builds and owns its widgets (AddWidget&lt;T&gt;(name, placement, arguments)), finds them by name (GetWidget&lt;T&gt;) and removes them safely, even from their own slots (RemoveWidget)
  - Lasagna (three-dimensional successor of Grid): lays widgets out in rows, columns and layers, with spans and sizing policies. SetAxisDefinition (or one axis at a time) changes the tracks, their number included, keeping each widget in its cell and span; a widget reaching past the new last track is removed, and its name returned
  - Prefab: a group of widgets in its own Lasagna, treated as one widget; its rows, columns and layers can be changed after construction, as a Lasagna's
  - Environment: a 3D room in which widgets and models sit freely, scaled into its rectangle, optionally clipped, with its own perspective if wanted
  - EnvironmentView: shows an Environment, usually one in another window, through a camera of its own (SetWorld, SetCamera(eye, target, up) in the world's units, SetPerspective(fieldOfView) or SetOrthographic(height)): one world, owned once, seen from several windows, so whatever changes in it shows in every view. Views only show the world: hover and clicks, and drop-down lists opened in it, stay with the Environment's own window
  - ScrollView: scrolls its widgets by dragging, the mouse wheel, touchpad swipes and pinch, along X, Y or into the depth (Z); nested ScrollViews each take what they can scroll
  - MenuBar: a bar of flexible cells holding widgets; a cell can move or undock the window
  - WindowTab: windows as tabs in a strip, selected, reordered, closed, pulled out and dropped back in by dragging, drawn the way ImGui draws docked windows
- WidgetRegistry: every widget is found from anywhere by its path ("MainWindow/MainScene/RootContainer/OkButton") or the path's hash (GetPathHash, Fnv1aHash), also as its type (Find<Button>); renames are followed
- Flavors (StylingManager), named style rules like CSS classes: a flavor sets colors per state, corner radii, cut corners, border widths, a background's or a text's image, every shader setting (fill, raster, glass, fog and their noise, masks, gaps, pixelation, CRT), the drop shadow, glow and inner glow (GetDropShadow() and so on, shown, colors per state, numbers, fill), and a text's font family, size, slant and weight, alignment, padding, flow, emoji tint, mask span and outline (GetText().GetOutline(): width, colors per state, fill), for its widgets' background, border and text; for the widget itself it sets the transitions (SetTransition, over the global default) and the border gap (SetBorderGap); the parts inside a widget have named getters, mirroring the widgets' own (GetIndicator().GetBackground(), GetGraph(), GetList().GetHighlight(), GetItemStyle(), GetSeparatorLine(), GetFieldStyle(), GetCloseButton(), GetSelectionBand(), GetHoverText(), GetPlaceholder(), GetPreview(), GetCheckmark(), GetArrow(), GetCloseButton().GetIcon(), GetCaret(), GetSelectedArea(), GetItemText(), and a table's GetTrackStyle(n) for its nth track variant, GetCellStyle(), GetHeaderStyle(), GetCellCheckBox()); a window takes flavors as well (Window::AddFlavor: its background, border and transitions; a docked window still squares the corners where it meets others); a widget takes several in order (AddFlavor(".primary")), any time after it is made. Each value shows the widget's own setting if set, else the first of its flavors that sets it, else .default, else the widget type's built-in look; widgets don't own what they take from a flavor, so a change to the flavor shows on all of them at once
- Window
  - Several windows sharing fonts, images and shaders; rounded corners and transparency to the desktop
  - Moving, resizing and maximizing; docking windows to each other's edges (like the columns of Niri) or into tabs, by API or by dragging with a preview
  - Three passes per frame: Interact (input), Update (per-frame work) and Draw
  - Files dropped from other programs (OnFilesDropped)
  - A layout file like imgui.ini (RetroFuturaGUI.ini) restores positions, sizes, docking and tabs between runs
  - Holds Scenes, docked to an edge or as overlays
- Scenes
  - A named group of widgets under a window with its own root container: Window / Scene / Container / Widgets. SetLasagnaAxis builds a root Lasagna, or changes the tracks of the one already there, keeping its widgets
  - Active or inactive, docked to an edge of the window (a menu bar, a status bar) or covering it as an overlay
  - Per-frame and input signals (OnUpdate, OnInteract), and lifecycle signals: OnLoad when the scene joins its window, OnUnload when it leaves (closed by SceneLoader::CloseScene, unregistered or removed), OnActivate and OnDeactivate with SetActive
  - Scene::SetEnabled disables every widget in the scene (shown disabled, no input, an edit in a text box or Table ends) and gives each its own state back when enabled again, emitting OnEnable/OnDisable only if asked. Overlay scenes stay click-through by design; a modal overlay disables the scene beneath it this way
  - The root container is any container (IContainer: Lasagna, Environment, ScrollView, Prefab, MenuBar, RadioButtonGroup, ExtendedComboBox, Tab, WindowTab); a new container implements IContainer to be one. A closed scene is freed by the release hook its owner registered (SceneLoader::SetReleaseHook), and OnUnload tells an owner without one
  - Assets: what a scene's widgets load through ResourceManager (images, SVGs, 3D models with their material textures) counts as the scene's, and SceneLoader::AddAsset(scene, path, AssetKind) loads more ahead; an asset is freed once the last open scene counting it has closed and its widgets are gone, while one loaded outside any scene stays until ResourceManager::Free. A scene's setup opens a SceneLoader::AssetScope right after CreateScene so its widgets' loads count. 3D models are imported once and shared by every Model showing the file (ResourceManager::GetModel)
  - Loading screens: SceneLoader::AddAsset loads on up to four loading threads (reading, image and SVG decoding, model import) and uploads on the GL thread at the end of DrainPending, about 4 ms a frame, so the screen keeps its frame rate; it works for a scene not created yet (by its ID), whose widgets then find their files loaded once it is built. SceneLoadingProgress reports ready/total, a failed file counting as done; a widget asking for a file still in preparation gets it finished at once instead of loading it twice
- Popups
  - Popup&lt;Ts...&gt;: a window of its own that closes with values of any types (Close, Connect_OnResult) or is dismissed (Dismiss, Connect_OnDismiss)
  - One popup stack per window; the windows below the top popup take no input, like under a native modal dialog
- Animation
  - Transitions for colors (the effects' too), corner radii, positions, sizes, rotations, opacity, border gaps and the effects' angle, distance, blur, width and scale (Effects; an angle turns the way the numbers run, so 0 to 360 is a full turn): a global default per property (Transitions::SetDefault), overridable per widget (SetTransition), with easing (Linear, EaseIn, EaseOut, EaseInOut). Off until set
  - Worked out from the clock, so they run the same at any frame rate; layout changes stay instant
  - Moving fills run by the clock too: gradient animation (runs through all colors per second) and rotation (degrees per second), raster animation and fog drift (pixels per second), and SVG icon gradients; something drawn more than once a frame doesn't move faster
  - Fades: widget opacity (SetOpacity), tooltips, context menus and drop-down lists fade in and out, popups and the docking preview through the native window's opacity
- Third dimension
  - Every widget can be rotated around all three axes with everything in it; the pointer stays exact at any tilt
  - Perspective per window (Window::SetPerspective) and per Environment; clipping that follows the tilt
- FileExplorer
  - A popup that browses the file system and closes with the chosen paths: Open(startPath, flags, ".cpp, .hpp")
  - Single file, multiple files, folder or save mode; places, history, an address box, sorting, filters, hidden files and extensions toggles
  - File operations: new folder, rename, delete to the recycle bin or permanently, cut, copy and paste through the system clipboard, drag and drop in and out, undo
  - Previews and thumbnails of images, songs (cover art) and videos, decoded by a separate, restricted helper process. A video's still is decoded once, at the keyframe before 10% in and at the pane's size, for the pane and its thumbnail alike
  - Listing and file operations run off the GUI thread, and the folder shown is watched for changes
- Media Playback
  - AudioPlayback: plays WAV, AIFF, FLAC and MP3 (miniaudio) and Opus, Vorbis and audio in Ogg, Matroska/WebM and MP4 (FFmpeg); HCA with keys and tracker modules (libopenmpt). Seeking in milliseconds, seamless loops between loop points, volume, speed, per-channel levels and frequency bands
    - AudioStream decrypts HCA blocks itself (HcaCipher) before they reach FFmpeg: FFmpeg 9 decrypts type 56 only when handed the key through its demuxer's options, and type 1 not at all (7.1.1 misread the header and never decrypted). FFmpeg is never handed the key, so nothing is decrypted twice.
    - FFmpeg doesn't trim HCA's encoder delay and padding, so an HCA file decodes to its block count times 1024 samples; the sample count and loop points in AudioMetadata count the same way, so they match what plays
  - AudioMetadata and VideoMetadata: everything a file says about itself without playing it - tags, chapters, loops, cover art, codecs, length
    - DecodeCoverArt decodes PNG only with an FFmpeg built with zlib (vcpkg's ffmpeg[zlib] feature), since FFmpeg's PNG decoder needs it; without, a PNG cover decodes to nothing
  - Royalty-free codecs only by default (RETROFUTURAGUI_ROYALTY_FREE_CODECS_ONLY), so a build can't ship a codec it isn't licensed for
    - With the royalty-free checks on (the default), Decoder::Open refuses anything outside the codec allowlist in DecodingPolicy.hpp, so a build can't quietly start distributing a decoder it isn't licensed for. NonCommercialUseCodecs names the encumbered ones (H.264, HEVC, VVC, VC1, AAC) so a refusal can say which codec it refused and why. The FFmpeg build itself is LGPL 2.1 with GPL and nonfree off, and has to stay dynamically linked
    - ADX, HCA and Nintendo DSP ADPCM aren't on the royalty-free list, so they decode only with the checks off. Each addition to the list is a deliberate licensing decision rather than a code change
  - Untrusted files are decoded in a restricted helper process, the parsers are fuzzed (tools/fuzz) and the decoding libraries kept current
  - On Windows, paths outside the system's code page need an app manifest setting activeCodePage to UTF-8 and MSVC's /utf-8
- ResourceManager: picture files loaded once and shared until freed (GetTexture, Free); every SVG user gets its own copy (CreateSvgTexture)
- Shaders
  - Built from modules, one program per rectangle class and fill
  - Fills: Solid, Linear Shift, Radial Shift, Conic Shift, animated, up to 256 colors
  - Effects on every fill: glass, rounded or cut corners, raster (Dotted, Checkered, Striped, Grid), fog, background gaps, border gaps, images and masks
  - Glass and fog noise settings (SetGlassNoise, SetFogNoise): cell sizes in pixels, the glass's distortion, smoothing (Linear, Cubic, Quintic) and a seed
  - Pixelation of whatever was drawn behind, 3D models included (SetPixelateBlockSize on Background, Graph and Plane): the window under the plane is copied right before it draws and shown in square blocks on the plane's own grid, so they tilt with it; the fill's alpha decides how much of it shows. It shows only what was drawn before it
  - A CRT look of whatever was drawn behind (SetCrtEffect with a CrtEffect on the same parts): red and blue pulled apart sideways by a displacement in pixels, and dark horizontal scanlines of a given width and alpha; never pixelated, it takes the pixelation's place where both are set
  - Drop shadows, glows and inner glows of rectangles from the shape's own distance field (Shadow.fs, with GLOW and INNER_GLOW), in one draw each and rounded or cut like the shape
  - Anti-aliased rectangle edges: rounded and cut corners, turned, tilted and perspective edges, borders, shadows and glows fade over one screen pixel; straight edges of flat rectangles land on whole pixels, so thin lines stay crisp
  - Anti-aliased lines with mitered joins (Polyline)
- Text Renderer
  - All of Unicode, glyphs rendered when first shown; installed font families with their weights and slants, variable fonts, fallback fonts per character
  - Shaping with HarfBuzz (kerning, ligatures, emoji sequences), right-to-left scripts (SheenBidi), color emoji including gradient emoji (COLR v1), vertical flow
  - Every fill and effect inside the glyphs, outlines, images and masks (metal text)
  - Drop shadows, glows and inner glows: their glyphs are blurred on the CPU into the glyph pages, once per blur, width and offset
- Signals: thread-safe Connect/Disconnect, synchronous (Emit) and threaded (EmitAsync) dispatch
- BackgroundWork: a worker thread whose results are handed back on the GUI thread
- PlatformBridge (cross-platform system layer, Windows and Linux)
  - Keyboard, mouse, wheel and pinch input; clipboard; recycle bin; dragging files out; system fonts; monitors; child processes
- Cross Language Capabilities
  - dll/so/dylib compilation, widgets addressed by string ID (DynamicLibWidgetManager), signals through C callbacks, the FileExplorer
  - C#, Python and Rust bindings

</details>
  


### ToDos before milestone 5
<details><summary>CLICK TO EXPAND</summary>
<li>Nothing open: what didn't block .bechaml moved to the independent list</li>
</details>

### ToDos independent of milestones
<details><summary>CLICK TO EXPAND</summary>
<li>Interoperability and bindings (roadmap 8 and 11)</li>
  <ul>
  <li>Flavors: C exports and bindings (the C++ side is complete)</li>
  <li>Add more widget interactions to the DynamicLibWidgetManager</li>
    <ul>
    <li>SetRotation, SetPosition, SetSize</li>
    <li>Set Color/Decoration parameters</li>
    </ul>
  <li>Complete Python binding</li>
  <li>Goose binding, built the way Goose's documentation describes its interoperability</li>
  <li>Test Carbon compatibility</li>
  <li>Bring the interoperability binding (DYNLIB_MODE, DynamicLibWidgetManager, the C#, Python and Rust bindings) up to date with scenes and the window mechanisms. Last on the list, since it builds on the items above</li>
    <ul>
    <li>Widgets are registered under their bare name (AddWidget(widget->GetName(), ...)), so of two widgets with the same name in different scenes or windows only the first one registered is reachable. Find them through WidgetRegistry instead, by path or path hash (MainWindow/MyScene/RootLasagna/TestButton)</li>
    <li>Nothing is ever unregistered, so a closed scene (SceneLoader::CloseScene), a closed window or a destroyed prefab leaves the manager holding freed pointers. WidgetRegistry already drops every widget as it is destroyed, so the manager can rely on it instead of keeping its own list</li>
    <li>The exported functions reach single widgets and the one MainWindow only. Scenes (create, close, activate, reserve an edge), several windows, docking, WindowTabs and popups can't be used from another language yet</li>
    <li>Callbacks are void(*)() only, apart from the file explorer's paths (PathsCallbackType), so signals that carry values - WindowTab's OnSelectionChanged with its Window*, other popups' results - have no way out</li>
    <li>The Rust binding doesn't compile: new() moves the Library into the struct and then borrows it for its symbols, and the libloading::Symbol fields have no lifetime. Its file explorer parts were checked on their own</li>
    <li>DynamicLibWidgetManager's setters take no emitSignal, so a binding can't choose whether a change fires the widget's signal: SetText passes false to Button::SetText and Label::SetText, and TextBox's SetValue goes without one. Add an emitSignal bool to the setters whose widgets have one, to the exports in main.cpp, and to the C#, Python and Rust bindings, with no default, as everywhere else</li>
    </ul>
  </ul>
<li>Animation Engine: what transitions don't cover yet (colors, corner radii, positions, sizes, rotations, opacity, border gaps and the effects' geometry have them, see Animation)</li>
  <ul>
  <li>Per-cell hover in elements drawn once per cell (Table cells, MenuBar items, WindowTab fields, ComboBox items), which needs a shown state per cell</li>
  <li>SVG icon colors (checkmark, drop-down arrow, tab close icon), whose every change rasterizes the SVG again; raster and fog colors</li>
  </ul>
<li>Shaders</li>
  <ul>
  <li>LineDiagram and Polyline: where points sit closer together than the line is wide, the inner side of a mitered corner can fold back over the previous segment</li>
  <li>Anti-aliasing of the cuts inside a shape: border gaps, background gaps and the wave line are still cut hard, while the edges and corners fade over a pixel</li>
  <li>Pixelate and CRT: only Plane-based parts have them (Background, Graph), not Border or Text; untested under tilt and perspective, which the shaders account for</li>
  <li>Glass could show the live copy of the window behind, as Pixelate and CRT do, instead of a fixed backdrop image set by the app</li>
  <li>optimize shader handling</li>
    <ul>
    <li>only pass parameters on initialization and on parameter value change. Skipping uploads that repeat what the program holds doesn't pay off: 620 of the 948 uploads per frame do, but comparing each value cost as much as the uploads it saved (Debug, 2026-10-26). Uniform buffers per rectangle, updated only on a change, could get there</li>
    </ul>
  </ul>
<li>General refactor across the framework</li>
  <ul>
  <li>windows.h still reaches Table.hpp and FileExplorer.hpp, through PlatformBridge's Definitions.hpp, for the PB_KEY_* values of their key members. Resolve this entirely once Android support is tackled. NOMINMAX keeps its min and max macros out everywhere, but its other macros still come along. Only PlatformBridge can offer key names without windows.h</li>
  <li>The build copies ShaderSource next to the executable but never removes anything there, so renamed or deleted shaders (RadialGradient.glsl, the old Border*.fs) stay behind. Mirror the folder instead of copying over it</li>
  </ul>
<li>Overload Background::SetImage so it also takes a Texture*, letting a texture that is already loaded (ResourceManager::GetTexture, a decoded picture) be a background, the window's included, and with it the glass backdrop GetBackgroundImageId hands out, without going through a path</li>
<li>PlatformBridge::Input on Linux: the wheel and the pinch (done on Windows), untested and never compiled</li>
  <ul>
  <li>The wheel only reaches the window keys go to, since PlatformBridge selects button events on that window alone; it has to come from every window of the app, like the Windows message hook</li>
  <li>Two-finger stretch (pinch) on touchpads and touchscreens needs XInput 2.4 gesture events (XI_GesturePinchUpdate) and libXi; only Ctrl+wheel pinches so far</li>
  </ul>
<li>Wiki/Documentation</li>
<li>Switchable C++ modules (RETROFUTURAGUI_MODULES), needed once PS2 support is tackled: one CMake option builds the library as one module where the toolchain supports it, and as headers everywhere else (PS2, older toolchains). Proven on a slice (IncludeHelper, the transitions; RetroFuturaGUI.cppm, ModuleExternals.hpp, RFG_EXPORT); left: the RFG_EXPORT markers and .cpp preambles for the rest of the library, the CMake option, module builds in CI, the TestProject on import, and how VS Code's IntelliSense copes. One module rebuilds everything on any header change; partitions could narrow that. PlatformBridge stays plain headers</li>
<li>SVGs on meshes, once they are needed: SvgTexture keeps only the SVG data - the picture, the masks of recolored paths and their colors - and the geometry that shows it binds to that data in one universal way, whether it is the quad inside SvgImage or a Mesh. Today SvgTexture owns the quad it draws on, so a Mesh can't show a recolored SVG</li>
<li>Custom icons from images</li>
<li>Audio formats, once they're needed</li>
  <ul>
  <li>MIDI (needs a synthesizer such as TinySoundFont - and a SoundFont whose sample sources are clear for commercial use) and chiptune formats each need a new dependency</li>
  </ul>
<li>Animated text ("boiling" letters, as in hand-made video captions): every glyph gets its own small rotation, scale and offset around its center, changing over time</li>
  <ul>
  <li>Worked out in the text's vertex shader from each glyph's index, a seed and the clock, so the mesh isn't rebuilt for every pose and the outline, emoji and shaped clusters move along with their glyphs</li>
  <li>Snapping (the pose jumps and holds, from the time times the rate, rounded down) or transferring (eased from one pose to the next with the transitions' Easing)</li>
  <li>Settings: largest rotation, scale and offset, poses per second, snap or transfer with its easing, a stagger so glyphs or words change one after another instead of all at once, per glyph or per word, the seed; set through WidgetText (GetText()) like the raster and fog, and switched off by default</li>
  <li>Only how the glyphs are drawn changes: the caret, the selection and clicks keep the glyphs' resting places</li>
  <li>The reference (a caption video, measured 2026-11-01): poses snap every 5 frames at 30 fps (6 per second) and hold still in between, all glyphs change on the same frame, about 5 degrees of rotation, a slight scale and a vertical nudge each; captions cut in and out without a transition</li>
  </ul>
<li>A caption background following the text's lines: a rounded box per line, merged into one stepped shape with rounded outer corners and curved inner joins where lines of different lengths meet; it stays still while the letters move</li>
<li>Naming that may confuse: across code, comments and docs</li>
  <ul>
  <li>Say "font family" instead of "face" where a family is meant</li>
  <li>Write "code-point" instead of "code point"</li>
  </ul>
<li>Bitmap fonts for GameCube and Wii: an IGlyphSource that hands out fixed sizes (GetPixelSizes) and can't shape; FontManager, Text and the size conversions are ready for one</li>
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