#pragma once
#include "BackgroundWork.hpp"
#include "Clipboard.hpp"
#include "FileSystem.hpp"
#include "FileTransfer.hpp"
#include "FileUndoHistory.hpp"
#include "ISceneHost.hpp"
#include "Lasagna.hpp"
#include "Popup.hpp"
#include "Table.hpp"
#include <array>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>
#include "Definitions.hpp"

namespace RetroFuturaGUI
{
    class CheckBox;
    class ComboBox;

    /// @brief A popup that browses the file system and closes with the chosen paths, UTF-8 encoded. Connect to OnResult and
    ///        OnDismiss, call Open with the features it offers, then hand it to Window::PushPopup. WriteFile and Folder close
    ///        with one path.
    class FileExplorer final : public Popup<std::vector<std::string>>
    {
    public:
        /// @brief What Open sets up, combined with |. A feature not in the mask is off: it has no button and no key.
        enum class FileExplorerFlags : u32
        {
            SingleFile = 0,
            MultiFile = 1,
            Folder = 1 << 1,
            WriteFile = 1 << 2,
            ShowHiddenFiles = 1 << 3,
            ShowFileExtensions = 1 << 4,
            Thumbnails = 1 << 5,
            ImagePreview = 1 << 6,
            MediaPreview = 1 << 7, //audio and video metadata, cover art, the frame from 10% in
            NewFolder = 1 << 8,
            Rename = 1 << 9,
            Delete = 1 << 10, //to the recycle bin
            DeletePermanently = 1 << 11, //Shift+Delete, after a confirmation
            CopyAndMove = 1 << 12, //cut, copy, paste
            DragAndDrop = 1 << 13,
            Undo = 1 << 14,
            AskOnNameClash = 1 << 15, //pasting asks Replace, Skip or Keep both; without it a clash keeps both
            SkipDeleteConfirmation = 1 << 16, //don't ask for permanent deletion
            Previews = ImagePreview | MediaPreview,
            FileOperations = NewFolder | Rename | Delete | CopyAndMove | DragAndDrop | Undo
        };

        friend constexpr FileExplorerFlags operator |(const FileExplorerFlags left, const FileExplorerFlags right)
        {
            return static_cast<FileExplorerFlags>(static_cast<u32>(left) | static_cast<u32>(right));
        }

        friend constexpr FileExplorerFlags operator &(const FileExplorerFlags left, const FileExplorerFlags right)
        {
            return static_cast<FileExplorerFlags>(static_cast<u32>(left) & static_cast<u32>(right));
        }

        /// @brief Returns whether every bit of flag is set in flags.
        static constexpr bool HasFlag(const FileExplorerFlags flags, const FileExplorerFlags flag)
        {
            return static_cast<u32>(flag) == (static_cast<u32>(flags) & static_cast<u32>(flag));
        }

        /// @brief One named choice in the filter box: { ._Name = "Audio", ._Extensions = ".mp3, .flac" }.
        struct FileFilter
        {
            std::string _Name {}; //UTF-8; empty shows the extensions instead
            std::string _Extensions {}; //written as Open's extensions; ".*" among them, or none, lists every file
        };

        /// @brief Creates the explorer's window, hidden until it is pushed, and leaves its GL context current.
        FileExplorer(std::string_view title, const i32 width, const i32 height);
        FileExplorer() = delete;
        FileExplorer(const FileExplorer&) = delete;
        FileExplorer(FileExplorer&&) = delete;
        auto operator =(const FileExplorer&) = delete;
        auto operator =(FileExplorer&&) = delete;
        ~FileExplorer() override; //where the preview pane's type is whole

        /// @brief Builds the explorer for the flags and lists the start folder. Call it once, then hand the explorer to
        ///        Window::PushPopup, which takes ownership of it.
        /// @param startPath UTF-8. A file opens its folder and, with WriteFile, fills in its name. Empty, or a folder that can't be
        ///        listed, starts in the working directory.
        /// @param extensions The file types listed, comma-separated: ".cpp, .hpp, .mp3". Any case, the dot may be left out. ".*"
        ///        lists every file. The filter box offers all of them together, selected, then each one alone, and "All files"
        ///        when ".*" is among them.
        /// @return false when the explorer has no window, Open was called before, or more than one mode is set
        bool Open(std::string_view startPath, const FileExplorerFlags flags, std::string_view extensions = ".*");

        /// @brief Open with named file types: each filter is one choice in the filter box, the first one selected, and only
        ///        these are offered. Without any, every file is listed.
        bool Open(std::string_view startPath, const FileExplorerFlags flags, const std::vector<FileFilter>& filters);

        /// @brief Sets the font every text in the explorer uses; table rows grow with its size. Fonts hold only the code points
        ///        they were loaded with, so names in other scripts, like Japanese, need a font set the app loaded or extended
        ///        with those blocks (FontManager::LoadFont, ExtendFontset). The explorer adds Latin-1 itself. Arial 18 when
        ///        not set. Call before Open.
        void SetFontFamily(std::string_view fontFamily, const f32 fontSize, const PlatformBridge::Fonts::Slant slant, const PlatformBridge::Fonts::Weight weight);

    private:
        using Entry = PlatformBridge::FileSystem::Entry;

        /// @brief The one mode Open's flags pick.
        enum class Mode : u32
        {
            SingleFile,
            MultiFile,
            Folder,
            WriteFile
        };

        /// @brief One filter box choice, its extensions as native paths with the dot. No extensions lists every file.
        struct Filter
        {
            std::string _Name {};
            std::vector<std::filesystem::path> _Extensions {};
        };

        /// @brief The column the list is sorted by. A click on a header picks it; a second click on the same one reverses the order.
        enum class SortColumn : u32
        {
            Name,
            Size,
            Modified,
            Type
        };

        /// @brief One shown entry's sort keys, taken once per sort, since a path takes its name apart again on every comparison.
        struct SortKey
        {
            std::filesystem::path::string_type _Name {};
            std::string _Type {};
            uSize _Index { 0 }; //into _shownEntries
        };

        /// @brief Turns file times into local time. Each offset lookup searches the time zone database, so the last offset is
        ///        kept for as long as the times fall in the span it holds for.
        struct LocalClock
        {
            const std::chrono::time_zone* _Zone { nullptr }; //nullptr: UTC
            std::chrono::sys_info _Info {};
            bool _HasInfo { false };
        };

        /// @brief A key the explorer takes while the list has the keyboard, each press once.
        struct KeyPress
        {
            u32
                _Key { 0 },
                _PressCountSeen { 0 };
        };

        class NamePrompt;
        class ConfirmPrompt;
        class ClashPrompt;
        class FilePreview;
        class ThumbnailCache;

        /// @brief What a file is, told by its extension, for the preview and the thumbnails.
        enum class FileKind : u32
        {
            Folder,
            Image,
            Audio,
            Video,
            Other
        };

        using ClashAnswer = FileTransfer::ClashAnswer;
        using ClashScope = FileTransfer::ClashScope;

        /// @brief What a menu item does.
        enum class MenuAction : u32
        {
            NewFolder,
            Rename,
            Delete,
            DeletePermanently,
            Close,
            Undo,
            Cut,
            Copy,
            Paste,
            SelectAll
        };

        /// @brief What the worker lists and how it shows it. All copies, so the job never touches the explorer.
        struct ListingRequest
        {
            u64 _Generation { 0 };
            std::filesystem::path _Directory {};
            std::optional<std::vector<Entry>> _Entries {}; //listed before: shown again with other settings, without asking the disk
            Filter _Filter {};
            Mode _Mode { Mode::SingleFile };
            SortColumn _SortColumn { SortColumn::Name };
            bool
                _ShowsHidden { false },
                _IsSortDescending { false };
        };

        /// @brief One shown entry's cells, written by the worker. Both names, so the extensions toggle only picks one.
        struct RowTexts
        {
            std::string
                _Name {},
                _ShortName {}, //without its extension; a folder keeps its name
                _Size {},
                _Modified {},
                _Type {};
        };

        /// @brief What the worker hands back: the folder's entries, the ones shown, sorted, and their cells.
        struct Listing
        {
            u64 _Generation { 0 };
            std::filesystem::path _Directory {};
            std::error_code _Error {};
            bool _IsListed { false };
            std::vector<Entry>
                _Listed {},
                _Shown {};
            std::vector<RowTexts> _Rows {}; //one per shown entry
        };

        /// @brief What the explorer does with a listing once it arrives.
        struct ListingIntent
        {
            std::vector<std::filesystem::path> _Fallbacks {}; //the folders to try next when this one can't be listed
            std::optional<uSize> _HistoryIndex {}; //Back and Forward: where in the history the folder is
            std::filesystem::path _WriteTarget {}; //the file WriteFile's address box named, confirmed once its folder shows
            std::vector<std::filesystem::path> _Select {}; //what to select once shown, a folder just made or a file just renamed
            bool
                _AddsToHistory { false },
                _KeepsView { false }; //the same folder again: the selection and the scroll position stay
        };

        /// @brief The listing on its way, the latest one asked for.
        struct PendingListing
        {
            std::filesystem::path _Directory {};
            ListingIntent _Intent {};
            std::chrono::steady_clock::time_point _Since {};
            bool _IsShowingProgress { false };
        };

        /// @brief The one font every text in the view uses, so a font set the app prepared covers all of it.
        struct ViewFont
        {
            std::string _Family { "Arial" };
            f32 _Size { 18.0f };
            PlatformBridge::Fonts::Slant _Slant { PlatformBridge::Fonts::Slant::Roman };
            PlatformBridge::Fonts::Weight _Weight { PlatformBridge::Fonts::Weight::Normal };
        };

        /// @brief Where a widget sits in the view's grid: one row tall, a number of columns wide.
        struct ViewCell
        {
            uSize
                _Row { 0 },
                _Column { 0 },
                _ColumnSpan { 1 };
            SizingMode _SizingMode { SizingMode::FIXED };
            uSize _Layer { 0 }; //widgets sharing a place sit in layers of one cell
        };

        /// @brief The view's widgets the logic works with. The ViewHost owns them; a feature that is off leaves its widget null.
        struct ViewWidgets
        {
            MenuBar* _Bar { nullptr };
            Button
                * _Close { nullptr },
                * _Back { nullptr },
                * _Forward { nullptr },
                * _Up { nullptr },
                * _Rename { nullptr },
                * _Thumbnails { nullptr },
                * _Extensions { nullptr },
                * _Hidden { nullptr },
                * _Stop { nullptr }, //shown while a file operation runs
                * _Ok { nullptr },
                * _Cancel { nullptr };
            TextBox
                * _Address { nullptr },
                * _Name { nullptr }; //WriteFile only
            ScrollView* _Places { nullptr };
            Table* _Files { nullptr };
            Slider* _Scroll { nullptr };
            ComboBox
                * _FileMenu { nullptr },
                * _EditMenu { nullptr }, //null when it would hold nothing
                * _Filter { nullptr };
            Label* _Status { nullptr };
        };

        /// @brief Owns the explorer's scene and every widget in it. IPopup::AddScene takes it, so the popup destroys the widgets
        ///        while its GL context is current.
        class ViewHost final : public ISceneHost
        {
        public:
            /// @brief Creates the scene in the window, with the explorer's grid as its root Lasagna.
            ViewHost(std::string_view sceneName, Window* window, const AxisDefinition& gridAxis);
            ViewHost() = delete;
            ViewHost(const ViewHost&) = delete;
            ViewHost(ViewHost&&) = delete;
            auto operator =(const ViewHost&) = delete;
            auto operator =(ViewHost&&) = delete;
            ~ViewHost() override;

            Scene* GetScene() const override;

            /// @brief The scene's root Lasagna, laid out by _viewAxis.
            Lasagna* GetGrid() const;

            /// @brief Builds a widget in the given cell of a Lasagna and keeps it.
            /// @return The widget, or nullptr, keeping nothing, when the cell is out of range or taken.
            template<typename T, typename... Args>
            T* Attach(std::string_view name, Lasagna& lasagna, const ViewCell& cell, Args&&... args)
            {
                if(!_projection)
                    return nullptr;

                if(!_glfwWindow)
                    return nullptr;

                //Most widget constructors take the name as const std::string&
                const std::string widgetName { name };
                std::unique_ptr<T> widget { std::make_unique<T>(widgetName, _projection, &lasagna, WidgetTypeID::Lasagna, _glfwWindow, std::forward<Args>(args)...) };
                T* attached { widget.get() };

                //Lasagna counts in u32; the view's grid is far below its range
                if(!lasagna.AttachWidget(static_cast<u32>(cell._Row), static_cast<u32>(cell._Column), static_cast<u32>(cell._Layer), attached, cell._SizingMode, 1, static_cast<u32>(cell._ColumnSpan), 1))
                    return nullptr;

                _widgets.push_back(std::move(widget));
                return attached;
            }

        private:
            std::unique_ptr<Scene> _scene { nullptr };
            std::vector<std::unique_ptr<IWidget>> _widgets {}; //after the scene, so they are destroyed first
            Projection* _projection { nullptr };
            GLFWwindow* _glfwWindow { nullptr };
        };

    //View
        /// @brief The frame every window of the explorer has, its prompts' too: rounded corners and a thin border.
        static void styleWindow(Window& window, std::string_view title);

        /// @brief Builds the bar across a grid's first row: the title, which drags the window, and the close button in cell
        ///        _barCloseIndex, which the caller connects.
        static MenuBar* attachBar(ViewHost& host, Lasagna& grid, Window& window, std::string_view title, const uSize columnCount, const f32 barHeight, const glm::vec2& windowSize, const ViewFont& font);

        /// @brief Builds the view's widgets, styled, placed and connected, for the mode and flags. Only a feature that is on gets
        ///        its widget.
        /// @return false when one of the widgets every mode needs couldn't be built
        bool buildView(ViewHost& host, std::string_view title);
        static void styleButton(Button& button, std::string_view text, const glm::vec3& size, const ViewFont& font);

        /// @brief Draws a toggle in the accent colors while it is on, so it needs no "on"/"off" text.
        static void styleToggle(Button& toggle, const bool isOn);

        /// @brief Makes a ComboBox a menu: flat under its caption, one item per action, its list as wide as given.
        static void styleMenu(ComboBox& menu, std::string_view caption, std::span<const MenuAction> actions, const glm::vec3& size, const f32 listWidth, const ViewFont& font);
        static void styleTextBox(TextBox& textBox, std::string_view placeholder, const glm::vec3& size, const ViewFont& font);

        /// @brief A square box in the explorer's colors, the accent's blue while checked.
        static void styleCheckBox(CheckBox& checkBox, const f32 side);

        /// @brief A line of text on the window's background, a prompt's message for instance.
        static void styleLabel(Label& label, std::string_view text, const glm::vec3& size, const ViewFont& font);

        /// @brief The size of one button in the places list, which the logic fills.
        static glm::vec3 placeButtonSize(const glm::vec2& windowSize, const ViewFont& font);

        /// @brief A table row's height, enough for the font's ascenders and descenders.
        static f32 rowHeight(const ViewFont& font);

        /// @brief The toolbar's font: the view's at 0.8 times the size, so ten labels fit one row. They are plain ASCII, so a font
        ///        set prepared for other scripts loses nothing.
        static ViewFont toolbarFont(const ViewFont& font);

        /// @brief Size, Modified and Type wide enough for "1023.9 KB", a date with minutes and a short type in the font, each with the sort arrow in its header; Name takes the rest.
        ///        With thumbnails, their column comes first.
        static std::vector<Table::TrackDefinition> columnDefinition(const ViewFont& font, const bool hasThumbnails);

        /// @brief The size of a fixed-size widget in a cell of a grid laid out by the axis, a margin smaller than the cell on every side.
        static glm::vec3 cellSize(const AxisDefinition& axis, const glm::vec2& windowSize, const ViewCell& cell);
        bool hasPreview() const;

    //Setup
        /// @brief Returns the mode the flags pick: the one mode bit set, or SingleFile without any.
        static Mode modeOf(const FileExplorerFlags flags);

        /// @brief What both Opens do once the filter box's choices are known.
        bool openWithFilters(std::string_view startPath, const FileExplorerFlags flags, std::vector<Filter>&& filters);

        /// @brief The filter box's choices for Open's extensions: all of them together, each one alone, and "All files" for ".*"
        ///        or when none is usable.
        static std::vector<Filter> filtersOf(std::string_view extensions);

        /// @brief The filter box's choices for named filters, "All files" when there are none.
        static std::vector<Filter> filtersOf(const std::vector<FileFilter>& fileFilters);

        /// @brief Splits Open's extensions at the commas into lowercase ".ext", without blanks or repeats. "*.mp3" and "mp3" become ".mp3".
        static std::vector<std::string> splitExtensions(std::string_view extensions);

        /// @brief The extensions as a filter box shows them: ".cpp, .hpp".
        static std::string joinedExtensions(const std::vector<std::filesystem::path>& extensions);

        /// @brief Loads a Unicode block into the font unless it has it: Latin-1 for accented names, Geometric Shapes for the sort
        ///        arrows. Only once, since extending again would replace the block.
        void ensureFontBlock(const u32 first, const u32 last) const;

        /// @brief Picks the table's selection mode and names the OK button.
        void applyMode();
        void addPlaces();
        void addPlaceButton(std::string_view text, const std::filesystem::path& place, const uSize index);
        void addFilters();

    //Browsing
        /// @brief Opens the start folder, or the first of the working directory, home and the roots that can be listed.
        void navigateToStart(std::string_view startPath);

        /// @brief Has the folder listed and shown. When it can't be listed the explorer stays where it was and says why.
        void navigateTo(const std::filesystem::path& directory, const bool addToHistory);

        /// @brief Lists the open folder again, keeping the selection and the scroll position. Nothing while another listing is
        ///        on its way, since that one is newer.
        void relist();

        /// @brief Shows the listed entries again with the current hidden toggle, filter and sort order, without asking the disk.
        ///        While a listing is on its way, that one is asked for again with them instead.
        void showEntries();

        /// @brief Has the worker list a folder and show it; the listing arrives in update. Any listing asked for before is dropped.
        void requestListing(const std::filesystem::path& directory, ListingIntent&& intent);

        /// @brief Posts the job for a listing; the latest generation is the one shown.
        void postListing(ListingRequest&& request);

        /// @brief The listing request with the current settings.
        ListingRequest listingRequest(const std::filesystem::path& directory) const;

        /// @brief The worker's side: lists the folder unless the entries came along, then filters, sorts and formats them.
        static Listing makeListing(ListingRequest&& request, std::stop_token stop);

        /// @brief The main thread's side: shows a listing that arrived, unless a newer one was asked for since.
        void showListing(Listing&& listing);

        /// @brief Runs each frame from the scene's update: takes arrived listings, shows the progress, then the keys.
        void update();

        /// @brief Shows the listings that arrived since the last frame.
        void drainListings();

        /// @brief Lists the folder again once it changed: when the changes pause, or a second after the first at the latest,
        ///        so a long copy into it still shows its progress.
        void watchChanges();

        /// @brief Says "Opening" in the status line once a listing has taken long enough to notice.
        void showProgress();

        /// @brief The folder the list shows, or the one on its way, which Up goes above.
        const std::filesystem::path& targetDirectory() const;

        /// @brief Fills the list with the shown rows' texts, as the worker wrote them.
        void fillTable();

        /// @brief Sets the note the status line shows while no message is up: "Opening Music...", why the list is empty, or "3
        ///        selected" in MultiFile.
        void updateStatusNote();

        /// @brief Has the preview pane show the selected rows' entries.
        void showPreview();

        /// @brief Shows the message in the status line, or the note while there is none.
        void showStatus();

        /// @brief Names the columns, the sort column with ▲ or ▼ for its direction.
        void updateHeaderTexts();

        /// @brief The paths of the selected rows.
        std::vector<std::filesystem::path> selectedPaths() const;

        /// @brief Selects the rows showing the given paths, and scrolls the first into view.
        void selectPaths(const std::vector<std::filesystem::path>& paths, const bool emitSignal);

        /// @brief The name the list shows: without its extension while the extensions toggle is off. Folders keep theirs.
        const std::string& displayName(const RowTexts& texts) const;

        /// @brief Writes the names again after the extensions toggle changed, leaving the selection and scroll position alone.
        void updateNameColumn();

        /// @brief The column showing the names, after the thumbnails' while they show.
        uSize nameColumn() const;

        /// @brief The rows' height: taller while thumbnails show.
        f32 listRowHeight() const;

        /// @brief Gives the rows in view their thumbnails, made on the first use.
        void showThumbnails();
        void updateScrollSlider();

        /// @brief Moves the slider to where the list is scrolled, without scrolling it again.
        void syncScrollSlider();
        void updateNavigationButtons();

        /// @brief Shows a message in the status line: an error, an outcome or a question. It stays until the next one, until
        ///        another folder is opened, or until the user selects other rows. Empty takes it down.
        void setStatus(std::string_view text);
        const Filter& currentFilter() const;

    //Closing
        void openSelectedFiles();
        void openSelectedFolder();

        /// @brief Returns the file the name box names in the current folder, the filter's extension added when it has none.
        std::filesystem::path nameBoxTarget() const;

        /// @brief Closes with the file to write, after asking once when it exists already.
        void confirmWrite(const std::filesystem::path& target);

        /// @brief Takes back the question whether to replace a file, since the name it was about changed.
        void disarmReplace();
        void closeWith(const std::vector<std::filesystem::path>& paths);
        std::string_view okButtonText() const;

    //New folder and Rename
        /// @brief Asks for the new folder's name, a free one filled in, and makes the folder in the open folder.
        void askNewFolderName();

        /// @brief Makes a folder in the open folder and selects it once it shows.
        void createFolder(std::string_view name);

        /// @brief Starts editing the selected row's name in the list. With extensions shown only the name before the extension
        ///        is selected, so typing keeps it.
        void startRename();

        /// @brief Renames the file the edit started on to what the edit left in its cell, and selects it once it shows.
        void finishRename(std::string_view text);

        /// @brief Returns whether a typed name can name a file in a folder; says why not in the status line.
        bool isValidName(std::string_view name);

        /// @brief The number of code points in UTF-8 text, which is what a text box counts in.
        static uSize codePointCount(std::string_view text);

        /// @brief Makes a prompt the top popup over the explorer, its window holding the keys, and gives them back to the list
        ///        once it's dismissed.
        template<typename Prompt>
        Prompt* pushPrompt(std::unique_ptr<Prompt> prompt);

        /// @brief Gives the keys back to the list, after a prompt or an edit.
        void giveKeysToList();

    //Deleting and transfers
        /// @brief Moves the selected items to the recycle bin. Those the bin can't take are asked about, to delete them for good.
        void deleteSelected();

        /// @brief Deletes items for good, after asking unless SkipDeleteConfirmation is set.
        void askDeletePermanently(std::vector<std::filesystem::path>&& paths);

        /// @brief Puts the selected items on the clipboard, to be copied or moved where they are pasted, here or in the system's
        ///        file manager.
        void putSelectedOnClipboard(const PlatformBridge::Clipboard::FileOperation operation);

        /// @brief Copies or moves the files on the clipboard into the open folder. A folder pasted into itself is refused.
        void pasteFromClipboard();

        /// @brief Runs each frame from update: the Paste item follows what the clipboard holds, which other programs change too.
        void followClipboard();

    //Drag and drop
        /// @brief Follows a drag of the selected rows: marks the folder row or place a drop would go into and says "Move to Music"
        ///        or "Copy to Music", hands the drag to the system once the pointer leaves the window, and moves or copies on the
        ///        drop.
        void on_filesTable_selectionDrag(const Table::SelectionDragPhase phase, const glm::vec2& point);

        /// @brief Takes files dropped on the explorer's window from another program: copies them into the folder under the
        ///        pointer, or the open one, and moves them with Shift held. Rows dragged out and dropped back are taken as a drag
        ///        inside.
        void on_window_filesDropped(const std::vector<std::string>& texts, const glm::vec2& point);

        /// @brief The folder a drop at the point goes into: a folder row, or a place button, and with isOpenFolderTaken the open
        ///        folder anywhere else. Empty when there is none. A dragged item, and the open folder for rows dragged from it,
        ///        never are.
        std::filesystem::path dropFolderAt(const glm::vec2& point, const std::vector<std::filesystem::path>& dragged, const bool isOpenFolderTaken, std::optional<uSize>& rowOut, Button*& placeOut) const;

        /// @brief Marks where a drop would go: the folder row, the place button, or nothing when both are empty.
        void showDropTarget(const std::optional<uSize> row, Button* place);

        /// @brief Draws a place button in the accent colors while a drop would go into it, and in its own otherwise.
        static void markPlace(Button* place, const bool isMarked);

        /// @brief The window's size now, in the coordinates the widgets and the pointer use.
        glm::vec2 windowSize() const;

        /// @brief Whether dropping the items into the folder moves them: Shift moves and Ctrl copies, otherwise items move
        ///        within their volume and are copied onto another.
        static bool isDropMove(const std::vector<std::filesystem::path>& items, const std::filesystem::path& folder, const bool isShiftHeld, const bool isControlHeld);

        /// @brief Runs the system's drag of the dragged rows, from update once the pointer has left the window: away from the
        ///        table's handling of the press, since the system's drag loop blocks until the drop.
        void dragOut();

        /// @brief Copies or moves dropped or dragged items into a folder, unless a folder would go into itself or another
        ///        operation runs.
        void dropInto(std::vector<std::filesystem::path>&& items, const std::filesystem::path& folder, const bool isMove);

        /// @brief Takes the drop target's marks away and forgets the drag.
        void endDrag();

        /// @brief Undoes the newest step in the app's file history, whichever explorer did it: removes a new folder still empty,
        ///        renames back, sends copies to the recycle bin, moves items back, or restores them from the bin.
        void undoLast();

        /// @brief Adds a step to the app's file history while Undo is on; steps an undo does aren't added.
        void recordUndoStep(const FileUndoHistory::Kind kind, std::vector<FileUndoHistory::Item>&& items);

        /// @brief Runs a file operation on the transfer's thread, unless one runs already, and shows its progress.
        void startTransfer(FileTransfer::Request&& request);

        /// @brief Runs each frame from update: shows the progress, asks about a name clash, and takes the outcome once it ends.
        void followTransfer();

        /// @brief Says how the operation went, lists the folder again, and asks about the items the bin refused.
        void showTransferOutcome(FileTransfer::Outcome&& outcome);

        /// @brief Asks the user what to do about a name clash the transfer waits on. Closing the prompt stops the transfer.
        void askAboutClash(const FileTransfer::Clash& clash);

        /// @brief Shows the Stop button while an operation runs and hides it otherwise.
        void showStopButton(const bool isShown);

        /// @brief Closes the explorer, after asking while a file operation runs: confirmed, the operation stops after its current
        ///        chunk and the explorer closes once it has.
        void closeAfterAsking(std::function<void()>&& close);

        /// @brief Alt+F4 and the taskbar close the explorer through closeAfterAsking too.
        bool allowsSystemClose() override;

        /// @brief "Copying 3 of 12: Song.mp3".
        static std::string progressText(const FileTransfer::Progress& progress);

        /// @brief "Moved 3 items to the recycle bin", "Can't copy Song.mp3: Access is denied." and so on.
        static std::string outcomeText(const FileTransfer::Outcome& outcome);

        /// @brief The row that takes the removed rows' place, so the keys go on from there: the first row after the first removed
        ///        one that is still there, or else the last one before it. Empty when none is left.
        std::vector<std::filesystem::path> rowAfterRemoved(const std::vector<std::filesystem::path>& removed) const;

        /// @brief "Song.mp3" for one item, "3 items" for more.
        static std::string itemsText(const std::vector<std::filesystem::path>& paths);

    //Menus
        /// @brief The actions of a menu's choices the explorer offers: those whose feature is on.
        std::vector<MenuAction> menuActionsOf(std::span<const MenuAction> choices) const;

        /// @brief Returns whether an action is on: its flag is set, or it needs none.
        bool isMenuActionOffered(const MenuAction action) const;

        /// @brief Returns whether an action can be done right now: Paste needs files on the clipboard, Rename one selected, and so on.
        bool isMenuActionAvailable(const MenuAction action) const;

        /// @brief Returns whether an action is both offered and available, which a key needs as much as a menu item.
        bool canDo(const MenuAction action) const;

        /// @brief Enables the menu items that can be done right now and disables the rest.
        void updateMenus();
        void runMenuAction(const MenuAction action);
        void selectAllRows();
        static std::string_view menuText(const MenuAction action);

    //Keys
        /// @brief Takes the explorer's keys while the list has the keyboard: Enter, Backspace, Ctrl+Up, Ctrl+Left, Ctrl+Right,
        ///        Escape and F5. The list moves the selection itself. Runs each frame from the scene's update.
        void handleKeys();

        /// @brief Enter: opens the one selected folder, otherwise does what OK does.
        void openFromKeyboard();

        /// @brief Returns whether the key was pressed since the last call.
        static bool wasPressed(KeyPress& key);

        /// @brief Takes the press counts as seen, so keys typed while a box had the keyboard do nothing.
        void syncKeys();

    //Slots
        void on_backButton_click();
        void on_forwardButton_click();
        void on_upButton_click();
        void on_addressTextBox_enterPressed();
        void on_placeButton_click(const std::filesystem::path& place);
        void on_filesTable_selectionChanged();
        void on_filesTable_rowDoubleClicked(const uSize row);
        void on_filesTable_headerClicked(const uSize column);
        void on_fileMenu_itemClicked();
        void on_editMenu_itemClicked();
        void on_renameButton_click();
        void on_filesTable_rowSlowDoubleClicked(const uSize row);
        void on_filesTable_cellEditEnded(const TrackIndex& index, const Table::EditEnd end, const std::string& text);
        void on_scrollSlider_valueChanged();
        void on_filterComboBox_indexChanged();
        void on_hiddenButton_click();
        void on_extensionsButton_click();
        void on_thumbnailsButton_click();
        void on_nameTextBox_textChanged();
        void on_okButton_click();
        void on_cancelButton_click();
        void on_stopButton_click();

    //Helpers
        /// @brief Sorts entries by a column, folders first whichever way the list runs.
        static void sortEntries(std::vector<Entry>& entries, const SortColumn column, const bool isDescending);

        /// @brief Folders first, then by the sort column in its direction, then by name.
        static bool isSortedBefore(const SortKey& left, const SortKey& right, const std::vector<Entry>& entries, const SortColumn column, const bool isDescending);

        /// @brief Digit runs compared as numbers ("Track 2" before "Track 10"), letters A to Z without case.
        static bool isNameBefore(const std::filesystem::path::string_type& left, const std::filesystem::path::string_type& right);
        static std::filesystem::path::value_type foldCase(const std::filesystem::path::value_type character);
        static bool matchesFilter(const std::filesystem::path& path, const Filter& filter);

        /// @brief Removes a trailing separator, so the parent of "C:\Users\" is "C:\", not "C:\Users".
        static std::filesystem::path normalizedDirectory(const std::filesystem::path& directory);
        static std::string toUtf8(const std::filesystem::path& path);
        static std::filesystem::path fromUtf8(std::string_view text);
        static std::string formatSize(const u64 size);

        /// @brief Writes a file time in local time, to the minute; in UTC when the clock has no zone.
        static std::string formatTime(const std::filesystem::file_time_type time, LocalClock& clock);

        /// @brief The local time zone, or nullptr when the system has no time zone database.
        static const std::chrono::time_zone* localZone();
        /// @brief "Folder", "File" without an extension, otherwise the extension as the file spells it: "png", "JPG".
        static std::string typeName(const Entry& entry);

        /// @brief The ASCII letters in lowercase, every other character as it is.
        static std::string lowerCase(std::string text);
        static FileKind fileKindOf(const Entry& entry);
        static std::string describeError(const std::error_code& error);

    //Settings
        ViewFont _font {};
        uSize _explorerNumber { 0 }; //part of its scene's name, which the scene loader keys across every window
        i32
            _width { 0 },
            _height { 0 };
        FileExplorerFlags _flags { FileExplorerFlags::SingleFile };
        Mode _mode { Mode::SingleFile };

    //State
        ViewHost* _viewHost { nullptr }; //owned by the popup, through AddScene
        std::unique_ptr<FilePreview> _preview; //with ImagePreview or MediaPreview; its widgets are the view host's. No {}, which MSVC takes to need the whole type
        std::unique_ptr<ThumbnailCache> _thumbnails; //made once thumbnails first show, since it runs a thread of its own
        std::unique_ptr<BackgroundWork> _listingWork { std::make_unique<BackgroundWork>(BackgroundWork::WhenDestroyed::LetJobFinish) }; //its jobs use only copies, so a stuck one is let go
        u64 _listingGeneration { 0 }; //of the latest listing asked for; an older one that arrives is dropped
        std::optional<PendingListing> _pending {};
        std::optional<std::filesystem::path> _renamingPath {}; //the file whose name is being edited in the list
        FileTransfer _transfer {}; //copies, moves and deletes; stopped and joined when the explorer goes
        std::function<void()> _closeAfterTransfer {}; //the close the user confirmed while an operation ran, done once it has stopped
        std::vector<std::filesystem::path>
            _selectAfterTransfer {}, //a delete's: the row that takes the deleted ones' place
            _pastedCut {},           //the cut files being moved, taken off the clipboard once they are
            _draggedPaths {},        //the rows a drag carries
            _draggedOutPaths {};     //the rows the system's drag carried last, which may come back as a drop on the explorer
        std::vector<std::pair<Button*, std::filesystem::path>> _placeButtons {}; //owned by the places ScrollView
        Button* _dropTargetPlace { nullptr }; //marked while a drag is over it
        std::chrono::steady_clock::time_point _draggedOutAt {};
        bool
            _isDraggingRows { false },
            _isDragOutPending { false }, //the pointer left the window; the system's drag starts from the next update
            _hasDraggedOut { false };    //the system ran the drag, so the release the table reports after it is no drop
        bool
            _hadFilesOnClipboard { false },
            _isTransferUndoing { false }; //the operation running undoes a step, so its outcome isn't recorded
        std::string
            _statusMessage {}, //an error, an outcome or a question; stays until the folder changes, the user selects, or the next one
            _statusNote {};    //shown while there is no message
        bool
            _isAskingAboutClash { false },
            _isSelectingByCode { false }; //the explorer selects rows, which keeps the status message
        PlatformBridge::FileSystem::DirectoryWatcher _watcher {}; //the folder the list shows
        std::chrono::steady_clock::time_point
            _firstUnlistedChange {},
            _lastChange {};
        bool _hasUnlistedChange { false };
        ViewWidgets _widgets {};
        std::vector<Filter> _filters {};
        std::vector<MenuAction>
            _fileMenuActions {}, //item n of the File menu does _fileMenuActions[n]
            _editMenuActions {};
        std::filesystem::path
            _currentDirectory {},
            _replaceTarget {}; //the existing file the next OK overwrites
        std::vector<std::filesystem::path> _history {};
        uSize _historyIndex { 0 };
        std::vector<Entry>
            _listedEntries {}, //everything in the current folder
            _shownEntries {};  //what passes the toggle, mode and filter, sorted; table row n shows _shownEntries[n]
        std::vector<RowTexts> _shownRows {}; //row n's texts, as the worker wrote them
        SortColumn _sortColumn { SortColumn::Name };
        bool
            _showHidden { false },
            _showExtensions { false },
            _showThumbnails { false },
            _isReplaceArmed { false },
            _isSortDescending { false },
            _hadListFocus { false };
        KeyPress
            _enterKey { ._Key = PB_KEY_RETURN, ._PressCountSeen = 0 },
            _backspaceKey { ._Key = PB_KEY_BACKSPACE, ._PressCountSeen = 0 },
            _escapeKey { ._Key = PB_KEY_ESCAPE, ._PressCountSeen = 0 },
            _upKey { ._Key = PB_KEY_UP, ._PressCountSeen = 0 },
            _leftKey { ._Key = PB_KEY_LEFT, ._PressCountSeen = 0 },
            _rightKey { ._Key = PB_KEY_RIGHT, ._PressCountSeen = 0 },
            _refreshKey { ._Key = PB_KEY_F5, ._PressCountSeen = 0 },
            _renameKey { ._Key = PB_KEY_F2, ._PressCountSeen = 0 },
            _newFolderKey { ._Key = PB_KEY_N, ._PressCountSeen = 0 }, //with Ctrl and Shift
            _deleteKey { ._Key = PB_KEY_DELETE, ._PressCountSeen = 0 }, //with Shift deletes for good
            _copyKey { ._Key = PB_KEY_C, ._PressCountSeen = 0 },  //with Ctrl
            _cutKey { ._Key = PB_KEY_X, ._PressCountSeen = 0 },   //with Ctrl
            _pasteKey { ._Key = PB_KEY_V, ._PressCountSeen = 0 }, //with Ctrl
            _undoKey { ._Key = PB_KEY_Z, ._PressCountSeen = 0 };  //with Ctrl

        inline static uSize
            _createdCount { 0 },
            _promptCount { 0 }; //part of each prompt's scene name, which the scene loader keys across every window

    //Design
        static constexpr uSize _viewColumnCount { 40 }; //one grid for every row, so each row's split is a set of column spans
        inline static const AxisDefinition _viewAxis
        {
            ._RowDefinition = { 0.06f, 0.07f, 0.07f, 0.66f, 0.07f, 0.07f }, //bar, navigation, toolbar, places and files, options, buttons
            ._ColumnDefinition = std::vector<f32>(_viewColumnCount, 1.0f / static_cast<f32>(_viewColumnCount)),
            ._LayerDefinition = { 1.0f }
        };
        static constexpr uSize _fileColumnCount { 4 }; //name, size, modified, type
        static constexpr f32
            _margin { 6.0f },
            _thumbnailColumnWidth { 72.0f },
            _thumbnailRowHeight { 64.0f };
        static constexpr std::chrono::milliseconds
            _progressDelay { 150 },    //a listing that takes longer says "Opening" meanwhile
            _relistQuietTime { 300 },  //the folder is listed again once its changes pause this long,
            _relistLongestWait { 1000 }; //or this long after the first change at the latest
        static constexpr uSize
            _placesColumnCount { 7 },   //of the 40: places 7, files 22, scrollbar 1, preview 10
            _filesColumnCount { 22 },
            _previewColumnCount { 10 }, //without a preview the files take its columns
            _barCloseIndex { 2 },       //the bar's flex cell holding the close button
            _menuColumnCount { 4 },     //a menu title's columns in the menu strip: its caption and the arrow side by side
            _menuListColumnCount { 9 }, //an open menu's width, enough for "Delete permanently"
            _statusColumnCount { 19 },  //the status line's columns beside the Stop button; it takes the Stop button's too without operations
            _stopColumnCount { 5 };
        static constexpr std::chrono::milliseconds _ownDropWindow { 2000 }; //a drop of the rows dragged out this soon after is their own
        static constexpr std::array<MenuAction, 5>
            _fileMenuChoices { MenuAction::NewFolder, MenuAction::Rename, MenuAction::Delete, MenuAction::DeletePermanently, MenuAction::Close },
            _editMenuChoices { MenuAction::Undo, MenuAction::Cut, MenuAction::Copy, MenuAction::Paste, MenuAction::SelectAll };
        inline static const glm::vec4
            _toggleOnColor { 0.22f, 0.32f, 0.55f, 1.0f },
            _toggleOnHoverColor { 0.3f, 0.42f, 0.68f, 1.0f },
            _toggleOnClickedColor { 0.38f, 0.5f, 0.78f, 1.0f };
    };
}