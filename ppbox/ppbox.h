// ppbox.h

#include "ppbox/include/IDemuxer.h"

#ifndef WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#ifndef PPBOX_EXTERN
#define IMPORT_FUNC(name) \
static FT_ ## name FF_ ## name = NULL; \
FT_ ## name fp_ ## name() \
{ \
    if (FF_ ## name == NULL) \
        FF_ ## name = (FT_ ## name)dlsym(PPBOX_Load(), #name); \
    return FF_ ## name; \
}

#else
#define IMPORT_FUNC(name) \
extern FT_ ## name fp_ ## name();
#endif

#ifndef WIN32
typedef void * HMODULE;
#define PPBOX_LIB_NAME "libppbox-linux-x86-gcc44-mt-gd-1.0.0.so"
#else
#define PPBOX_LIB_NAME TEXT("ppbox.dll")
#define dlopen(n, f) LoadPackagedLibrary(n, 0)
#define dlsym(m, s) GetProcAddress(m, s)
#define dlclose(m) FreeLibrary(m)
#define RTLD_LAZY 0
#endif

extern HMODULE PPBOX_Load();
extern void __cdecl PPBOX_Unload();

IMPORT_FUNC(PPBOX_StartP2PEngine);
#define PPBOX_StartP2PEngine fp_PPBOX_StartP2PEngine()
IMPORT_FUNC(PPBOX_StopP2PEngine);
#define PPBOX_StopP2PEngine fp_PPBOX_StopP2PEngine()
IMPORT_FUNC(PPBOX_GetLastError);
#define PPBOX_GetLastError fp_PPBOX_GetLastError()
IMPORT_FUNC(PPBOX_GetLastErrorMsg);
#define PPBOX_ScheduleCallback fp_PPBOX_ScheduleCallback()
IMPORT_FUNC(PPBOX_ScheduleCallback);
#define PPBOX_CancelCallback fp_PPBOX_CancelCallback()
IMPORT_FUNC(PPBOX_CancelCallback);

IMPORT_FUNC(PPBOX_Open);
#define PPBOX_Open fp_PPBOX_Open()
IMPORT_FUNC(PPBOX_AsyncOpen);
#define PPBOX_AsyncOpen fp_PPBOX_AsyncOpen()
IMPORT_FUNC(PPBOX_OpenEx);
#define PPBOX_OpenEx fp_PPBOX_OpenEx()
IMPORT_FUNC(PPBOX_AsyncOpenEx);
#define PPBOX_AsyncOpenEx fp_PPBOX_AsyncOpenEx()
IMPORT_FUNC(PPBOX_Close);
#define PPBOX_Close fp_PPBOX_Close()
IMPORT_FUNC(PPBOX_GetDuration);
#define PPBOX_GetDuration fp_PPBOX_GetDuration()
IMPORT_FUNC(PPBOX_GetStreamCount);
#define PPBOX_GetStreamCount fp_PPBOX_GetStreamCount()
IMPORT_FUNC(PPBOX_GetStreamInfoEx);
#define PPBOX_GetStreamInfoEx fp_PPBOX_GetStreamInfoEx()
IMPORT_FUNC(PPBOX_ReadSampleEx2);
#define PPBOX_ReadSampleEx2 fp_PPBOX_ReadSampleEx2()
IMPORT_FUNC(PPBOX_Seek);
#define PPBOX_Seek fp_PPBOX_Seek()

#ifndef PPBOX_EXTERN
HMODULE hMod = NULL;
void __cdecl PPBOX_Unload()
{
  if (hMod) {
    PPBOX_StopP2PEngine();
    dlclose(hMod);
  }
  hMod = NULL;
}
extern "C" _onexit_t (__cdecl *_imp___onexit)(_onexit_t func);
HMODULE PPBOX_Load()
{
    if (hMod == NULL) {
        hMod = dlopen(PPBOX_LIB_NAME, RTLD_LAZY);
        PP_int32 ret = PPBOX_StartP2PEngine("12", "161", "08ae1acd062ea3ab65924e07717d5994");
        if (ret != ppbox_success && ret != ppbox_already_start) {
            PPBOX_Unload();
            _exit(1);
        }
        //(*_imp___onexit)((_onexit_t)PPBOX_Unload);
    }
    return hMod;
}
#endif

