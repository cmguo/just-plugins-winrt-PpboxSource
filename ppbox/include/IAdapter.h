// IAdapter.h

#ifndef _PPBOX_PPBOX_I_ADAPTER_H_
#define _PPBOX_PPBOX_I_ADAPTER_H_

#include "IPpbox.h"

#if __cplusplus
extern "C" {
#endif // __cplusplus

    //打开一个适配视频
    PPBOX_DECL PP_int32 Adapter_Open(
                           PP_char const * playlink,
                           PP_char const * format);

    typedef void (*Adapter_Open_Callback)(PP_int32);

    //异步打开一个适配视频
    PPBOX_DECL void Adapter_AsyncOpen(
                         PP_char const * playlink, 
                         PP_char const * format,
                         Adapter_Open_Callback callback);

    //适配暂停
    PPBOX_DECL PP_int32 Adapter_Pause();

    //适配恢复
    PPBOX_DECL PP_int32 Adapter_Resume();

    //强制适配结束
    PPBOX_DECL void Adapter_Close();

    //跳到某个时刻开始播放
    PPBOX_DECL PP_int32 Adapter_Seek(
                          PP_uint32 start_time);

    //重新获取头部数据
    PPBOX_DECL void Adapter_Reset();

    //读取适配影片数据
    PPBOX_DECL PP_int32 Adapter_Read(
                         unsigned char * buffer,
                         PP_uint32 buffer_size,
                         PP_uint32 * read_size);

    enum Adapter_PlayStatusEnum
    {
        ppbox_adapter_closed = 0, 
        ppbox_adapter_playing, 
        ppbox_adapter_buffering, 
        ppbox_adapter_paused, 
    };

    typedef struct tag_Apapter_PlayStatistic
    {
        PP_uint32 length;           //本结构体的长度
        PP_int32  play_status;       //播放状态 0-未启动 1-playing态 2-buffering态 3-Pausing态
        PP_uint32 buffering_present;//播放缓冲百分比 10 表示 10%
        PP_uint32 buffer_time;      //下载缓冲区数据的总时间
    } Adapter_PlayStatistic;

    //获得播放信息
    //返回值: 错误码
    //    ppbox_success      表示成功
    //    其他数值表示失败
    PPBOX_DECL PP_int32 Adapter_GetPlayMsg(
        Adapter_PlayStatistic * statistic_Msg);

    typedef struct TMediaFileInfo
    {
        PP_uint32 duration;
        // video
        PP_uint32 frame_rate;
        PP_uint32 width;
        PP_uint32 height;
        // audio
        PP_uint32   channel_count;
        PP_uint32   sample_size;
        PP_uint32   sample_rate;
    }Adapter_Mediainfo;

    PPBOX_DECL PP_int32 Adapter_GetMediaInfo(
        Adapter_Mediainfo * media_info);

    PPBOX_DECL PP_int32 Adapter_GetPlayTime(
        PP_uint64 * time);

#if __cplusplus
}
#endif // __cplusplus

#endif // _PPBOX_PPBOX_I_ADAPTER_H_
