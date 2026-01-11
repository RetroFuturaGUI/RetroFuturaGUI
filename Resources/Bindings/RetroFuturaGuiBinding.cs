using System;
using System.Runtime.InteropServices;

public class RetroFuturaGuiBinding
{
    public enum WidgetAction : int
    {
        OnClick,
        OnRelease,
        OnMouseEnter,
        OnMouseLeave,
        WhileHover,
        Unknown = -1
    };   

    public enum ColorState : UInt32
    {
        Enabled,
        Disabled,
        Clicked,
        Hover
    };

    public delegate void Callback();

    const string dllName = "TestProjectNative";

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
};