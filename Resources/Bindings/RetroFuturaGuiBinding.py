import os
from ctypes import CDLL, c_char_p
from enum import Enum
from ctypes import CDLL, c_char_p, c_int, c_bool, c_void_p, CFUNCTYPE, WINFUNCTYPE

Callback = CFUNCTYPE(None, c_void_p)

class RetroFuturaGuiBinding:

    class WidgetAction(Enum):
        OnClick = 0
        OnRelease = 1
        OnMouseEnter = 2
        OnMouseLeave = 3
        WhileHover = 4
        Unknown = -1

    class ColorState(Enum):
        Enabled = 0
        Disabled = 1
        Clicked = 2
        Hover = 3

    def __init__(self):
        self.RetroFuturaGuiDLL = None
        self.workingDirectory = ""
        os.chdir(os.path.dirname(os.path.abspath(__file__)));
        self.LoadDll();
        self.SetDllWorkingDirectory()
        self.SetupFunctionPrototypes()
        self.callbacks = {}

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