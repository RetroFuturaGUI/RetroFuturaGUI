import os
from ctypes import CDLL, c_char_p
from enum import Enum, IntFlag
from ctypes import CDLL, c_char_p, c_int, c_int32, c_uint, c_uint32, c_bool, c_float, c_void_p, POINTER, CFUNCTYPE, WINFUNCTYPE

Callback = CFUNCTYPE(None, c_void_p)

# The paths are UTF-8, valid during the call only
PathsCallback = CFUNCTYPE(None, POINTER(c_char_p), c_uint)
DismissCallback = CFUNCTYPE(None)

class RetroFuturaGuiBinding:

    class WidgetAction(Enum):
        OnClick = 0
        OnRelease = 1
        OnMouseEnter = 2
        OnMouseLeave = 3
        WhileHover = 4
        OnTextChanged = 5
        OnEnterPressed = 6
        OnEnterReleased = 7
        Unknown = -1

    class ColorState(Enum):
        Enabled = 0
        Disabled = 1
        Clicked = 2
        Hover = 3

    class FileExplorerFlags(IntFlag):
        SingleFile = 0
        MultiFile = 1
        Folder = 1 << 1
        WriteFile = 1 << 2 # write new or existing
        ShowHiddenFiles = 1 << 3
        ShowFileExtensions = 1 << 4
        Thumbnails = 1 << 5
        ImagePreview = 1 << 6
        MediaPreview = 1 << 7         # audio and video metadata, cover art, the frame from 10% in
        NewFolder = 1 << 8
        Rename = 1 << 9
        Delete = 1 << 10 # to the recycle bin
        DeletePermanently = 1 << 11 # Shift+Delete, after a confirmation
        CopyAndMove = 1 << 12 # cut, copy, paste
        DragAndDrop = 1 << 13
        Undo = 1 << 14
        AskOnNameClash = 1 << 15 # pasting asks Replace, Skip or Keep both; without it a clash keeps both
        SkipDeleteConfirmation = 1 << 16 # delete permanently without confimarion
        Previews = ImagePreview | MediaPreview
        FileOperations = NewFolder | Rename | Delete | CopyAndMove | DragAndDrop | Undo

    def __init__(self):
        self.RetroFuturaGuiDLL = None
        self.workingDirectory = ""
        os.chdir(os.path.dirname(os.path.abspath(__file__)));
        self.LoadDll();
        self.SetDllWorkingDirectory()
        self.SetupFunctionPrototypes()
        self.callbacks = {}
        self.explorerCallbacks = {}  # each open explorer's ctypes callbacks, which C calls until the explorer closes
        self.finishedExplorers = set()
        self.explorerCount = 0

    def LoadDll(self):
        try:
            self.RetroFuturaGuiDLL = CDLL("./TestProjectNative.dll")
            #print("DLL loaded successfully.")
        except Exception as e:
            print(f"Error loading DLL: {e}")
            exit(1)

    def SetDllWorkingDirectory(self):
        self.RetroFuturaGuiDLL.SetWorkingDirectory.argtypes = [c_char_p]
        self.RetroFuturaGuiDLL.SetWorkingDirectory.restype = None

        self.workingDirectory = os.getcwd().encode("utf-8")
        print(f"Setting working directory to: {self.workingDirectory}")

        try:
            self.RetroFuturaGuiDLL.SetWorkingDirectory(self.workingDirectory);
            #print("Working directory set successfully.")
        except Exception as e:
            print(f"Error setting working directory: {e}")

    def InitRetroFuturaGUI(self):
        self.RetroFuturaGuiDLL.InitRetroFuturaGUI.argtypes = []
        self.RetroFuturaGuiDLL.InitRetroFuturaGUI.restype = None

        try:
            self.RetroFuturaGuiDLL.InitRetroFuturaGUI();
            print("Python initialized RetroFuturaGUI")
        except Exception as e:
            print(f"Error initializing DLL: {e}")

    def DrawWindow(self):
        self.RetroFuturaGuiDLL.InitRetroFuturaGUI.argtypes = []
        self.RetroFuturaGuiDLL.InitRetroFuturaGUI.restype = None

        try:
            self.RetroFuturaGuiDLL.Draw();
            print("Window Terminated")
        except Exception as e:
            print(f"Error drawing window: {e}")

    def SetupFunctionPrototypes(self):
        self.Callback = WINFUNCTYPE(None, c_void_p)
        self.RetroFuturaGuiDLL.ConnectSlot.argtypes = [c_char_p, self.Callback, c_int, c_bool]
        self.RetroFuturaGuiDLL.ConnectSlot.restype = None
        self.RetroFuturaGuiDLL.OpenFileExplorer.argtypes = [c_char_p, c_int32, c_int32, c_char_p, c_uint32, c_char_p, PathsCallback, DismissCallback]
        self.RetroFuturaGuiDLL.OpenFileExplorer.restype = c_bool

    def ConnectSlot(self, id, callback, action, shallBeAsync):
        try:
            actionInt = action.value if hasattr(action, 'value') else action
            cCallback = self.Callback(callback)
            self.callbacks[id] = cCallback
            self.RetroFuturaGuiDLL.ConnectSlot(
                id.encode("utf-8"),
                cCallback,
                actionInt,
                shallBeAsync
            )
        except Exception as e:
            print(f"Error connecting slot: {e}")

    def DisconnectSlot(self, id, action):
        try:
            actionInt = action.value if hasattr(action, 'value') else action
            self.RetroFuturaGuiDLL.DisconnectSlot(
                id.encode("utf-8"),
                actionInt
            )
            if id in self.callbacks:
                del self.callbacks[id]
        except Exception as e:
            print(f"Error disconnecting slot: {e}")

    def SetRotation(self, id, rotation):
        try:
            self.RetroFuturaGuiDLL.SetRotation(
                id.encode("utf-8"),
                rotation
            )
        except Exception as e:
            print(f"Error setting rotation: {e}")

    def SetSize(self, id, width, height):
        try:
            self.RetroFuturaGuiDLL.SetSize(
                id.encode("utf-8"),
                width,
                height
            )
        except Exception as e:
            print(f"Error setting size: {e}")

    def SetTextColors(self, id, colors, colorState):
        try:
            colorCount = len(colors)
            colorArrayType = c_float * (colorCount * 4)
            colorArray = colorArrayType(*[component for color in colors for component in color])
            self.RetroFuturaGuiDLL.SetTextColors(
                id.encode("utf-8"),
                colorArray,
                colorCount,
                colorState.value if hasattr(colorState, 'value') else colorState
            )
        except Exception as e:
            print(f"Error setting text colors: {e}")

    def SetText(self, id, text):
        try:
            self.RetroFuturaGuiDLL.SetText(
                id.encode("utf-8"),
                text.encode("utf-8")
            )
        except Exception as e:
            print(f"Error setting text: {e}")

    # Opens a file explorer over the main window: onResult gets the chosen paths as a list of str, onDismiss hears it closed
    # without; either may be None. A startPath of None starts in the working directory, extensions of None list every file.
    def OpenFileExplorer(self, title, width, height, startPath, flags, extensions, onResult, onDismiss):
        # ctypes frees a callback with its last reference, so one can't let go of itself while it runs: finished ones go here
        for finished in self.finishedExplorers:
            self.explorerCallbacks.pop(finished, None)

        self.finishedExplorers.clear()
        explorer = self.explorerCount
        self.explorerCount += 1

        def resultCallback(paths, pathCount):
            copied = [paths[index].decode("utf-8") for index in range(pathCount)]
            self.finishedExplorers.add(explorer)

            if onResult:
                onResult(copied)

        def dismissCallback():
            self.finishedExplorers.add(explorer)

            if onDismiss:
                onDismiss()

        callbacks = (PathsCallback(resultCallback), DismissCallback(dismissCallback))
        self.explorerCallbacks[explorer] = callbacks

        try:
            isOpened = self.RetroFuturaGuiDLL.OpenFileExplorer(
                title.encode("utf-8"),
                width,
                height,
                startPath.encode("utf-8") if startPath is not None else None,
                int(flags),
                extensions.encode("utf-8") if extensions is not None else None,
                callbacks[0],
                callbacks[1]
            )
        except Exception as e:
            print(f"Error opening the file explorer: {e}")
            isOpened = False

        if not isOpened:
            self.explorerCallbacks.pop(explorer, None)

        return isOpened

    