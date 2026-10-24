using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public class RetroFuturaGuiBinding
{
    public enum WidgetAction : Int32
    {
        OnClick,
        OnRelease,
        OnMouseEnter,
        OnMouseLeave,
        WhileHover,
        OnTextChanged,
        OnEnterPressed,
        OnEnterReleased,
        Unknown = -1
    };   

    public enum ColorState : UInt32
    {
        Enabled,
        Disabled,
        Clicked,
        Hover
    };

    [Flags]
    public enum FileExplorerFlags : UInt32
    {
        SingleFile             = 0,
        MultiFile              = 1,
        Folder                 = 1 << 1,
        WriteFile              = 1 << 2, //new or existing
        ShowHiddenFiles        = 1 << 3,
        ShowFileExtensions     = 1 << 4,
        Thumbnails             = 1 << 5,
        ImagePreview           = 1 << 6,
        MediaPreview           = 1 << 7, //audio and video metadata, cover art, the frame from 10% in
        NewFolder              = 1 << 8,
        Rename                 = 1 << 9,
        Delete                 = 1 << 10, //to the recycle bin
        DeletePermanently      = 1 << 11, //Shift+Delete, after a confirmation
        CopyAndMove            = 1 << 12, //cut, copy, paste
        DragAndDrop            = 1 << 13,
        Undo                   = 1 << 14,
        AskOnNameClash         = 1 << 15, //pasting asks Replace, Skip or Keep both; without it a clash keeps both
        SkipDeleteConfirmation = 1 << 16, //don't ask for permanent deletion
        Previews               = ImagePreview | MediaPreview,
        FileOperations         = NewFolder | Rename | Delete | CopyAndMove | DragAndDrop | Undo
    };

    public delegate void Callback();

    //The paths are UTF-8 C strings, valid during the call only: ReadPaths copies them
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void PathsCallback(IntPtr paths, UInt32 pathCount);

    const string dllName = "TestProjectNative";

    //The delegates of each open explorer, so the garbage collector can't free them while native code still calls them
    private static readonly Dictionary<int, Delegate[]> explorerCallbacks = new();
    private static int explorerCount = 0;

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWorkingDirectory(
        [MarshalAs(UnmanagedType.LPStr)] string dir
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void InitRetroFuturaGUI();

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void Draw();

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ConnectSlot(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        Callback callback,
        int action,
        bool async
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void DisconnectSlot(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        Callback callback,
        int action
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetRotation(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float degree
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetSize(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float width,
        float height
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBackgroundColors(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        IntPtr colors,
        UInt32 colorCount,
        UInt32 colorState
    );


    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBackgroundGradientOffset(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float gradientOffset
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBackgroundGradientAnimationSpeed(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float animationSpeed
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBackgroundGradientDegree(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float degree
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBackgroundGradientRotationSpeed(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float rotationSpeed
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWindowBackgroundImageTextureID(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        UInt32 textureID
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBorderColors(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        IntPtr colors,
        UInt32 colorCount,
        UInt32 colorState
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBorderGradientOffset(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float gradientOffset
    );
    
    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBorderGradientAnimationSpeed(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float animationSpeed
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBorderGradientDegree(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float degree
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetBorderGradientRotationSpeed(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        float rotationSpeed
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetWindowBorderImageTextureID(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        UInt32 textureID
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetEnabled(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        bool enable
    );

    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetTextColors(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        IntPtr colors,
        UInt32 colorCount,
        UInt32 colorState
    );
    
    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetText(
        [MarshalAs(UnmanagedType.LPStr)] string id,
        [MarshalAs(UnmanagedType.LPStr)] string text
    );

    //Opens a file explorer over a window. The caller keeps both delegates alive until one of them was called; the
    //overload taking an Action does that itself. A null startPath starts in the working directory, null extensions list every file.
    [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool OpenFileExplorer(
        [MarshalAs(UnmanagedType.LPUTF8Str)] string title,
        Int32 width,
        Int32 height,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string startPath,
        FileExplorerFlags flags,
        [MarshalAs(UnmanagedType.LPUTF8Str)] string extensions,
        PathsCallback onResult,
        Callback onDismiss
    );

    //Opens a file explorer over the main window: onResult gets the chosen paths, onDismiss hears it closed without
    public static bool OpenFileExplorer(string title, Int32 width, Int32 height, string startPath, FileExplorerFlags flags, string extensions, Action<string[]> onResult, Action onDismiss)
    {
        int explorer = explorerCount++;
        PathsCallback resultCallback = (paths, pathCount) =>
        {
            string[] copied = ReadPaths(paths, pathCount);
            explorerCallbacks.Remove(explorer);
            onResult?.Invoke(copied);
        };
        Callback dismissCallback = () =>
        {
            explorerCallbacks.Remove(explorer);
            onDismiss?.Invoke();
        };
        explorerCallbacks[explorer] = new Delegate[] { resultCallback, dismissCallback };
        bool isOpened = OpenFileExplorer(title, width, height, startPath, flags, extensions, resultCallback, dismissCallback);

        if(!isOpened)
            explorerCallbacks.Remove(explorer);

        return isOpened;
    }

    //Copies the paths a PathsCallback gets
    public static string[] ReadPaths(IntPtr paths, UInt32 pathCount)
    {
        string[] copied = new string[pathCount];

        for(int index = 0; index < pathCount; ++index)
            copied[index] = Marshal.PtrToStringUTF8(Marshal.ReadIntPtr(paths, index * IntPtr.Size)) ?? "";

        return copied;
    }
};