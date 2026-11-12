import os
from enum import Enum, IntFlag
from ctypes import CDLL, c_char_p, c_int32, c_uint, c_uint32, c_bool, c_float, POINTER, CFUNCTYPE

Callback = CFUNCTYPE(None) # basically a void(*)(), just what widget signals call

# The paths are UTF-8, valid during the call only
PathsCallback = CFUNCTYPE(None, POINTER(c_char_p), c_uint)
DismissCallback = CFUNCTYPE(None)

# Widgets are named by their full path, as the library finds them: "MainWindow/MainScene/RootLasagna/TestButton". A path no
# widget has does nothing. Colors, gradients, images and text reach a widget's main background, border and text. Signals
# reach Buttons, Labels and TextBoxes
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
        Selected = 4

    # How a background, border or text spreads its colors. One color, or a gradient across them
    class FillType(Enum):
        Solid = 0
        LinearShift = 1
        RadialShift = 2
        ConicShift = 3

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
        self.callbacks = {}  # (id, action) -> the ctypes callbacks connected there, which C calls until they're disconnected
        self.retiredCallbacks = []  # disconnected ones, let go once none runs: one may disconnect itself while it runs
        self.runningCallbackCount = 0
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
        try:
            self.RetroFuturaGuiDLL.InitRetroFuturaGUI();
            print("Python initialized RetroFuturaGUI")
        except Exception as e:
            print(f"Error initializing DLL: {e}")

    # Runs the window until it closes
    def DrawWindow(self):
        try:
            self.RetroFuturaGuiDLL.Draw();
            print("Window Terminated")
        except Exception as e:
            print(f"Error drawing window: {e}")

    def SetupFunctionPrototypes(self):
        dll = self.RetroFuturaGuiDLL
        colors = POINTER(c_float)
        prototypes = {
            "InitRetroFuturaGUI": [],
            "Draw": [],
            "ConnectSlot": [c_char_p, Callback, c_int32, c_bool],
            "DisconnectSlot": [c_char_p, Callback, c_int32],
            "SetRotation": [c_char_p, c_float, c_float, c_float],
            "SetPosition": [c_char_p, c_float, c_float, c_float],
            "SetSize": [c_char_p, c_float, c_float],
            "SetEnabled": [c_char_p, c_bool, c_bool],
            "SetBackgroundColors": [c_char_p, colors, c_uint32, c_uint32],
            "SetBackgroundFillType": [c_char_p, c_uint32],
            "SetBackgroundGradientOffset": [c_char_p, c_float],
            "SetBackgroundGradientAnimationSpeed": [c_char_p, c_float],
            "SetBackgroundGradientDegree": [c_char_p, c_float],
            "SetBackgroundGradientRotationSpeed": [c_char_p, c_float],
            "SetWindowBackgroundImageTextureID": [c_char_p, c_uint32],
            "SetBorderColors": [c_char_p, colors, c_uint32, c_uint32],
            "SetBorderFillType": [c_char_p, c_uint32],
            "SetBorderGradientOffset": [c_char_p, c_float],
            "SetBorderGradientAnimationSpeed": [c_char_p, c_float],
            "SetBorderGradientDegree": [c_char_p, c_float],
            "SetBorderGradientRotationSpeed": [c_char_p, c_float],
            "SetWindowBorderImageTextureID": [c_char_p, c_uint32],
            "SetTextColors": [c_char_p, colors, c_uint32, c_uint32],
            "SetTextFillType": [c_char_p, c_uint32],
            "SetText": [c_char_p, c_char_p, c_bool],
        }

        for name, argtypes in prototypes.items():
            function = getattr(dll, name)
            function.argtypes = argtypes
            function.restype = None

        dll.OpenFileExplorer.argtypes = [c_char_p, c_int32, c_int32, c_char_p, c_uint32, c_char_p, PathsCallback, DismissCallback]
        dll.OpenFileExplorer.restype = c_bool

    # Calls a function taking no arguments when the widget's signal for the action fires. Several may be connected to one action.
    def ConnectSlot(self, id, callback, action, shallBeAsync):
        try:
            self._letGoOfRetiredCallbacks()
            actionInt = self._valueOf(action)

            def counted():
                self.runningCallbackCount += 1

                try:
                    callback()
                finally:
                    self.runningCallbackCount -= 1

            cCallback = Callback(counted)
            self.callbacks.setdefault((id, actionInt), []).append(cCallback)  # kept alive while C may call it.
            self.RetroFuturaGuiDLL.ConnectSlot(id.encode("utf-8"), cCallback, actionInt, shallBeAsync)
        except Exception as e:
            print(f"Error connecting slot: {e}")

    # Disconnects every function this binding connected to the widget's signal for the action. A callback may disconnect itself
    def DisconnectSlot(self, id, action):
        try:
            self._letGoOfRetiredCallbacks()
            actionInt = self._valueOf(action)

            # The library tells slots apart by their function pointer, so each goes back as it was connected
            for cCallback in self.callbacks.pop((id, actionInt), []):
                self.RetroFuturaGuiDLL.DisconnectSlot(id.encode("utf-8"), cCallback, actionInt)
                self.retiredCallbacks.append(cCallback)  # ctypes frees one with its last reference, which would end it mid-run
        except Exception as e:
            print(f"Error disconnecting slot: {e}")

    # Degrees around x, y and z
    def SetRotation(self, id, x, y, z):
        try:
            self.RetroFuturaGuiDLL.SetRotation(id.encode("utf-8"), x, y, z)
        except Exception as e:
            print(f"Error setting rotation: {e}")

    # The widget's center in its window's pixels, and its depth
    def SetPosition(self, id, x, y, z):
        try:
            self.RetroFuturaGuiDLL.SetPosition(id.encode("utf-8"), x, y, z)
        except Exception as e:
            print(f"Error setting position: {e}")

    def SetSize(self, id, width, height):
        try:
            self.RetroFuturaGuiDLL.SetSize(id.encode("utf-8"), width, height)
        except Exception as e:
            print(f"Error setting size: {e}")

    def SetEnabled(self, id, enabled, emitSignal):
        try:
            self.RetroFuturaGuiDLL.SetEnabled(id.encode("utf-8"), enabled, emitSignal)
        except Exception as e:
            print(f"Error setting enabled: {e}")

    # colors is a list of (r, g, b, a) tuples from 0 to 1; several make a gradient
    def SetBackgroundColors(self, id, colors, colorState):
        self._setColors("SetBackgroundColors", id, colors, colorState)

    def SetBackgroundFillType(self, id, fillType):
        self._call("SetBackgroundFillType", id, self._valueOf(fillType))

    def SetBackgroundGradientOffset(self, id, gradientOffset):
        self._call("SetBackgroundGradientOffset", id, gradientOffset)

    def SetBackgroundGradientAnimationSpeed(self, id, animationSpeed):
        self._call("SetBackgroundGradientAnimationSpeed", id, animationSpeed)

    def SetBackgroundGradientDegree(self, id, degree):
        self._call("SetBackgroundGradientDegree", id, degree)

    def SetBackgroundGradientRotationSpeed(self, id, rotationSpeed):
        self._call("SetBackgroundGradientRotationSpeed", id, rotationSpeed)

    def SetWindowBackgroundImageTextureID(self, id, textureID):
        self._call("SetWindowBackgroundImageTextureID", id, textureID)

    def SetBorderColors(self, id, colors, colorState):
        self._setColors("SetBorderColors", id, colors, colorState)

    def SetBorderFillType(self, id, fillType):
        self._call("SetBorderFillType", id, self._valueOf(fillType))

    def SetBorderGradientOffset(self, id, gradientOffset):
        self._call("SetBorderGradientOffset", id, gradientOffset)

    def SetBorderGradientAnimationSpeed(self, id, animationSpeed):
        self._call("SetBorderGradientAnimationSpeed", id, animationSpeed)

    def SetBorderGradientDegree(self, id, degree):
        self._call("SetBorderGradientDegree", id, degree)

    def SetBorderGradientRotationSpeed(self, id, rotationSpeed):
        self._call("SetBorderGradientRotationSpeed", id, rotationSpeed)

    def SetWindowBorderImageTextureID(self, id, textureID):
        self._call("SetWindowBorderImageTextureID", id, textureID)

    def SetTextColors(self, id, colors, colorState):
        self._setColors("SetTextColors", id, colors, colorState)

    def SetTextFillType(self, id, fillType):
        self._call("SetTextFillType", id, self._valueOf(fillType))

    # A TextBox's value, any other widget's main text.
    def SetText(self, id, text, emitSignal):
        try:
            self.RetroFuturaGuiDLL.SetText(id.encode("utf-8"), text.encode("utf-8"), emitSignal)
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

    def _call(self, name, id, value):
        try:
            getattr(self.RetroFuturaGuiDLL, name)(id.encode("utf-8"), value)
        except Exception as e:
            print(f"Error calling {name}: {e}")

    def _setColors(self, name, id, colors, colorState):
        try:
            colorArray = (c_float * (len(colors) * 4))(*[component for color in colors for component in color])
            getattr(self.RetroFuturaGuiDLL, name)(id.encode("utf-8"), colorArray, len(colors), self._valueOf(colorState))
        except Exception as e:
            print(f"Error calling {name}: {e}")

    def _letGoOfRetiredCallbacks(self):
        if 0 == self.runningCallbackCount:
            self.retiredCallbacks.clear()

    @staticmethod
    def _valueOf(enumOrInt):
        return enumOrInt.value if hasattr(enumOrInt, 'value') else enumOrInt
