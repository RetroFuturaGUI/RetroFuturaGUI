use libloading::Library;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_uint, c_float, c_void};

#[repr(i32)]
#[derive(Debug, Clone, Copy)]
pub enum WidgetAction {
    OnClick = 0,
    OnRelease,
    OnMouseEnter,
    OnMouseLeave,
    WhileHover,
    OnTextChanged,
    OnEnterPressed,
    OnEnterReleased,
    Unknown = -1,
}

#[repr(u32)]
#[derive(Debug, Clone, Copy)]
pub enum ColorState {
    Enabled = 0,
    Disabled,
    Clicked,
    Hover,
}

pub type Callback = extern "C" fn();

/// Gets UTF-8 paths, valid during the call only: paths_from_callback copies them.
pub type PathsCallback = extern "C" fn(*const *const c_char, u32);

/// FileExplorer::FileExplorerFlags, bit for bit. Combine them with |.
pub mod file_explorer_flags {
    pub const SINGLE_FILE: u32 = 0;
    pub const MULTI_FILE: u32 = 1;
    pub const FOLDER: u32 = 1 << 1;
    pub const WRITE_FILE: u32 = 1 << 2;
    pub const SHOW_HIDDEN_FILES: u32 = 1 << 3;
    pub const SHOW_FILE_EXTENSIONS: u32 = 1 << 4;
    pub const THUMBNAILS: u32 = 1 << 5;
    pub const IMAGE_PREVIEW: u32 = 1 << 6;
    pub const MEDIA_PREVIEW: u32 = 1 << 7; // audio and video metadata, cover art, the frame from 10% in
    pub const NEW_FOLDER: u32 = 1 << 8;
    pub const RENAME: u32 = 1 << 9;
    pub const DELETE: u32 = 1 << 10; // to the recycle bin
    pub const DELETE_PERMANENTLY: u32 = 1 << 11; // Shift+Delete, after a confirmation
    pub const COPY_AND_MOVE: u32 = 1 << 12; // cut, copy, paste
    pub const DRAG_AND_DROP: u32 = 1 << 13;
    pub const UNDO: u32 = 1 << 14;
    pub const ASK_ON_NAME_CLASH: u32 = 1 << 15; // pasting asks Replace, Skip or Keep both; without it a clash keeps both
    pub const SKIP_DELETE_CONFIRMATION: u32 = 1 << 16; // Delete permanently without confirmation
    pub const PREVIEWS: u32 = IMAGE_PREVIEW | MEDIA_PREVIEW;
    pub const FILE_OPERATIONS: u32 = NEW_FOLDER | RENAME | DELETE | COPY_AND_MOVE | DRAG_AND_DROP | UNDO;
}

/// Copies the paths a PathsCallback gets.
///
/// # Safety
/// Only with the arguments the callback was called with, while it runs.
pub unsafe fn paths_from_callback(paths: *const *const c_char, path_count: u32) -> Vec<String> {
    (0..path_count as usize)
        .map(|index| CStr::from_ptr(*paths.add(index)).to_string_lossy().into_owned())
        .collect()
}

pub struct RetroFuturaGuiBinding {
    _lib: Library, // Keep library alive
    
    // Function pointers
    set_working_directory: libloading::Symbol<unsafe extern "C" fn(*const c_char)>,
    init_retro_futura_gui: libloading::Symbol<unsafe extern "C" fn()>,
    draw: libloading::Symbol<unsafe extern "C" fn()>,
    
    connect_slot: libloading::Symbol<unsafe extern "C" fn(*const c_char, Callback, c_int, bool)>,
    disconnect_slot: libloading::Symbol<unsafe extern "C" fn(*const c_char, Callback, c_int)>,
    
    set_rotation: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_size: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float, c_float)>,
    
    set_background_colors: libloading::Symbol<unsafe extern "C" fn(*const c_char, *const c_void, c_uint, c_uint)>,
    set_background_gradient_offset: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_background_gradient_animation_speed: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_background_gradient_degree: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_background_gradient_rotation_speed: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_window_background_image_texture_id: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_uint)>,
    
    set_border_colors: libloading::Symbol<unsafe extern "C" fn(*const c_char, *const c_void, c_uint, c_uint)>,
    set_border_gradient_offset: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_border_gradient_animation_speed: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_border_gradient_degree: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_border_gradient_rotation_speed: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_float)>,
    set_window_border_image_texture_id: libloading::Symbol<unsafe extern "C" fn(*const c_char, c_uint)>,
    
    set_enabled: libloading::Symbol<unsafe extern "C" fn(*const c_char, bool)>,
    
    set_text_colors: libloading::Symbol<unsafe extern "C" fn(*const c_char, *const c_void, c_uint, c_uint)>,
    set_text: libloading::Symbol<unsafe extern "C" fn(*const c_char, *const c_char)>,

    open_file_explorer: libloading::Symbol<unsafe extern "C" fn(*const c_char, i32, i32, *const c_char, u32, *const c_char, Option<PathsCallback>, Option<Callback>) -> bool>,
}

impl RetroFuturaGuiBinding {
    /// Loads the DLL and resolves all symbols.
    pub fn new() -> Result<Self, libloading::Error> {
        // On Windows, you might need ".dll" extension explicitly depending on the system config
        // On Linux/macOS, this would be ".so" or ".dylib" respectively.
        let lib = Library::new("TestProjectNative.dll")?;

        unsafe {
            Ok(RetroFuturaGuiBinding {
                _lib: lib, // Store library to prevent unloading
                
                set_working_directory: lib.get(b"SetWorkingDirectory")?,
                init_retro_futura_gui: lib.get(b"InitRetroFuturaGUI")?,
                draw: lib.get(b"Draw")?,
                
                connect_slot: lib.get(b"ConnectSlot")?,
                disconnect_slot: lib.get(b"DisconnectSlot")?,
                
                set_rotation: lib.get(b"SetRotation")?,
                set_size: lib.get(b"SetSize")?,
                
                set_background_colors: lib.get(b"SetBackgroundColors")?,
                set_background_gradient_offset: lib.get(b"SetBackgroundGradientOffset")?,
                set_background_gradient_animation_speed: lib.get(b"SetBackgroundGradientAnimationSpeed")?,
                set_background_gradient_degree: lib.get(b"SetBackgroundGradientDegree")?,
                set_background_gradient_rotation_speed: lib.get(b"SetBackgroundGradientRotationSpeed")?,
                set_window_background_image_texture_id: lib.get(b"SetWindowBackgroundImageTextureID")?,
                
                set_border_colors: lib.get(b"SetBorderColors")?,
                set_border_gradient_offset: lib.get(b"SetBorderGradientOffset")?,
                set_border_gradient_animation_speed: lib.get(b"SetBorderGradientAnimationSpeed")?,
                set_border_gradient_degree: lib.get(b"SetBorderGradientDegree")?,
                set_border_gradient_rotation_speed: lib.get(b"SetBorderGradientRotationSpeed")?,
                set_window_border_image_texture_id: lib.get(b"SetWindowBorderImageTextureID")?,
                
                set_enabled: lib.get(b"SetEnabled")?,
                
                set_text_colors: lib.get(b"SetTextColors")?,
                set_text: lib.get(b"SetText")?,

                open_file_explorer: lib.get(b"OpenFileExplorer")?,
            })
        }
    }

    // --- Wrapper Methods ---

    pub fn set_working_directory(&self, dir: &str) -> Result<(), libloading::Error> {
        let c_dir = CString::new(dir).unwrap();
        unsafe {
            (self.set_working_directory)(c_dir.as_ptr());
        }
        Ok(())
    }

    pub fn init_retro_futura_gui(&self) {
        unsafe {
            (self.init_retro_futura_gui)();
        }
    }

    pub fn draw(&self) {
        unsafe {
            (self.draw)();
        }
    }

    pub fn connect_slot(&self, id: &str, callback: Callback, action: WidgetAction, async_flag: bool) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.connect_slot)(c_id.as_ptr(), callback, action as c_int, async_flag);
        }
        Ok(())
    }

    pub fn disconnect_slot(&self, id: &str, callback: Callback, action: WidgetAction) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.disconnect_slot)(c_id.as_ptr(), callback, action as c_int);
        }
        Ok(())
    }

    pub fn set_rotation(&self, id: &str, degree: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_rotation)(c_id.as_ptr(), degree);
        }
        Ok(())
    }

    pub fn set_size(&self, id: &str, width: f32, height: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_size)(c_id.as_ptr(), width, height);
        }
        Ok(())
    }

    // Helper to convert Rust slice to pointer for color arrays
    // Assuming colors are passed as an array of u32 or similar struct
    pub fn set_background_colors(&self, id: &str, colors: &[u32], state: ColorState) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_background_colors)(
                c_id.as_ptr(),
                colors.as_ptr() as *const c_void,
                colors.len() as c_uint,
                state as c_uint,
            );
        }
        Ok(())
    }

    pub fn set_background_gradient_offset(&self, id: &str, offset: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_background_gradient_offset)(c_id.as_ptr(), offset);
        }
        Ok(())
    }

    pub fn set_background_gradient_animation_speed(&self, id: &str, speed: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_background_gradient_animation_speed)(c_id.as_ptr(), speed);
        }
        Ok(())
    }

    pub fn set_background_gradient_degree(&self, id: &str, degree: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_background_gradient_degree)(c_id.as_ptr(), degree);
        }
        Ok(())
    }

    pub fn set_background_gradient_rotation_speed(&self, id: &str, speed: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_background_gradient_rotation_speed)(c_id.as_ptr(), speed);
        }
        Ok(())
    }

    pub fn set_window_background_image_texture_id(&self, id: &str, texture_id: u32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_window_background_image_texture_id)(c_id.as_ptr(), texture_id);
        }
        Ok(())
    }

    pub fn set_border_colors(&self, id: &str, colors: &[u32], state: ColorState) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_border_colors)(
                c_id.as_ptr(),
                colors.as_ptr() as *const c_void,
                colors.len() as c_uint,
                state as c_uint,
            );
        }
        Ok(())
    }

    pub fn set_border_gradient_offset(&self, id: &str, offset: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_border_gradient_offset)(c_id.as_ptr(), offset);
        }
        Ok(())
    }

    pub fn set_border_gradient_animation_speed(&self, id: &str, speed: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_border_gradient_animation_speed)(c_id.as_ptr(), speed);
        }
        Ok(())
    }

    pub fn set_border_gradient_degree(&self, id: &str, degree: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_border_gradient_degree)(c_id.as_ptr(), degree);
        }
        Ok(())
    }

    pub fn set_border_gradient_rotation_speed(&self, id: &str, speed: f32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_border_gradient_rotation_speed)(c_id.as_ptr(), speed);
        }
        Ok(())
    }

    pub fn set_window_border_image_texture_id(&self, id: &str, texture_id: u32) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_window_border_image_texture_id)(c_id.as_ptr(), texture_id);
        }
        Ok(())
    }

    pub fn set_enabled(&self, id: &str, enable: bool) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_enabled)(c_id.as_ptr(), enable);
        }
        Ok(())
    }

    pub fn set_text_colors(&self, id: &str, colors: &[u32], state: ColorState) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        unsafe {
            (self.set_text_colors)(
                c_id.as_ptr(),
                colors.as_ptr() as *const c_void,
                colors.len() as c_uint,
                state as c_uint,
            );
        }
        Ok(())
    }

    pub fn set_text(&self, id: &str, text: &str) -> Result<(), libloading::Error> {
        let c_id = CString::new(id).unwrap();
        let c_text = CString::new(text).unwrap();
        unsafe {
            (self.set_text)(c_id.as_ptr(), c_text.as_ptr());
        }
        Ok(())
    }

    /// Opens a file explorer over the main window, with flags from file_explorer_flags. on_result gets the chosen paths
    /// (read them with paths_from_callback), on_dismiss hears it closed without. None for start_path starts in the working
    /// directory, None for extensions ("*.cpp, .hpp" for instance) lists every file.
    pub fn open_file_explorer(&self, title: &str, width: i32, height: i32, start_path: Option<&str>, flags: u32, extensions: Option<&str>, on_result: Option<PathsCallback>, on_dismiss: Option<Callback>) -> bool {
        let c_title = CString::new(title).unwrap();
        let c_start_path = start_path.map(|path| CString::new(path).unwrap());
        let c_extensions = extensions.map(|list| CString::new(list).unwrap());
        unsafe {
            (self.open_file_explorer)(
                c_title.as_ptr(),
                width,
                height,
                c_start_path.as_ref().map_or(std::ptr::null(), |path| path.as_ptr()),
                flags,
                c_extensions.as_ref().map_or(std::ptr::null(), |list| list.as_ptr()),
                on_result,
                on_dismiss,
            )
        }
    }
}
