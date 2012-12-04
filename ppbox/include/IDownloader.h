// IDownloader.h

#ifndef _PPBOX_PPBOX_I_DOWNLOADER_H_
#define _PPBOX_PPBOX_I_DOWNLOADER_H_

#include "IPpbox.h"

#if __cplusplus
extern "C" {
#endif // __cplusplus

    // refine
    typedef void * PPBOX_Download_Handle;
    typedef void(*PPBOX_Download_Callback)(PP_int32);
    static const PPBOX_Download_Handle PPBOX_INVALID_DOWNLOAD_HANDLE = NULL;

    //打开一个下载用例
    PPBOX_DECL PPBOX_Download_Handle PPBOX_DownloadOpen(
        char const * playlink,
        char const * format,
        char const * save_filename,
        PPBOX_Download_Callback resp);

    //关闭指定的下载用例
    PPBOX_DECL void PPBOX_DownloadClose(PPBOX_Download_Handle hander);

    typedef struct tag_PPBOX_DownloadStatistic
    {
        PP_uint64 total_size;
        PP_uint64 finish_size;
        PP_uint32 speed; 
    } PPBOX_DownloadStatistic;

    // 获取指定下载用例的实时统计信息
    PPBOX_DECL PP_int32 PPBOX_GetDownloadInfo(
        PPBOX_Download_Handle hander,
        PPBOX_DownloadStatistic * stat);

#if __cplusplus
}
#endif // __cplusplus

#endif // _PPBOX_PPBOX_I_DOWNLOADER_H_
