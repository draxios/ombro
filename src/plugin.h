#pragma once
// Ombro — Winamp/Cabrio visualization plugin interface
// AGPL-3.0 License

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define VIS_HDRVER 0x102

typedef struct winampVisModule {
    char *description;
    HWND hwndParent;
    HINSTANCE hDllInstance;
    int sRate;
    int nCh;
    int latencyMs;
    int delayMs;
    int spectrumNch;
    int waveformNch;
    unsigned char spectrumData[2][576];
    unsigned char waveformData[2][576];
    void (__cdecl *Config)(struct winampVisModule *this_mod);
    int  (__cdecl *Init)(struct winampVisModule *this_mod);
    int  (__cdecl *Render)(struct winampVisModule *this_mod);
    void (__cdecl *Quit)(struct winampVisModule *this_mod);
    void *userData;
} winampVisModule;

typedef struct {
    int version;
    char *description;
    winampVisModule* (__cdecl *getModule)(int index);
} winampVisHeader;

// Host IPC
#define WM_WA_IPC WM_USER
#define IPC_GETINIDIRECTORYW  335
#define IPC_GETVISDIRECTORYW  339
#define IPC_SETVISWND         611
#define IPC_ISVISRUNNING      613

// Host commands sent via WM_COMMAND
#define ID_VIS_NEXT   40382
#define ID_VIS_PREV   40383
#define ID_VIS_RANDOM 40384
#define ID_VIS_FS     40389
#define ID_VIS_CFG    40390
#define ID_VIS_MENU   40391
