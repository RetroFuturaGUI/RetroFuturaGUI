// This prevents recompilation of the library
#define MINIAUDIO_IMPLEMENTATION
//Single-threaded COM on the thread that makes the audio context, the GUI thread, which dragging files out needs (OleInitialize).
#define MA_COINIT_VALUE 2 //COINIT_APARTMENTTHREADED.
#include <miniaudio.h>