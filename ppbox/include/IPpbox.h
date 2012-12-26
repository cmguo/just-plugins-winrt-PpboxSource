// IPpbox.h

#ifndef _PPBOX_PPBOX_I_PPBOX_H_
#define _PPBOX_PPBOX_I_PPBOX_H_

#ifdef WIN32
//# define PPBOX_DECL __declspec(dllimport)
# define PPBOX_DECL
#else
# define PPBOX_DECL
#endif

#define EXPORT_FUNC(type, name) typedef type (* FT_ ## name)
//#define EXPORT_FUNC(type, name) type name

typedef char PP_char;
typedef char PP_bool;
typedef unsigned char PP_uchar;
typedef long PP_int32;
typedef unsigned short PP_uint16;
typedef unsigned long PP_uint32;
typedef unsigned long long PP_uint64;
typedef PP_int32 PP_err;

typedef void * PPBOX_HANDLE;

enum PPBOX_ErrorEnum
{
    ppbox_success = 0, 
    ppbox_not_start, 
    ppbox_already_start, 
    ppbox_not_open, 
    ppbox_already_open, 
    ppbox_operation_canceled, 
    ppbox_would_block, 
    ppbox_stream_end, 
    ppbox_logic_error, 
    ppbox_network_error, 
    ppbox_demux_error, 
    ppbox_certify_error, 
    ppbox_other_error = 1024, 
};

#if __cplusplus
extern "C" {
#endif // __cplusplus

    PPBOX_DECL EXPORT_FUNC(PP_int32, PPBOX_StartP2PEngine)(
        PP_char const * gid, 
        PP_char const * pid, 
        PP_char const * auth);

    PPBOX_DECL EXPORT_FUNC(void, PPBOX_StopP2PEngine)(void);

    //��ô�����
    //����ֵ: ������
    PPBOX_DECL EXPORT_FUNC(PP_int32, PPBOX_GetLastError)(void);

    PPBOX_DECL EXPORT_FUNC(PP_char const *, PPBOX_GetLastErrorMsg)(void);

    PPBOX_DECL EXPORT_FUNC(PP_char const *, PPBOX_GetVersion)(void);

    PPBOX_DECL EXPORT_FUNC(void, PPBOX_SetConfig)(
        PP_char const * module, 
        PP_char const * section, 
        PP_char const * key, 
        PP_char const * value);

    PPBOX_DECL EXPORT_FUNC(void, PPBOX_DebugMode)(
        PP_bool mode);

    typedef struct tag_DialogMessage
    {
        PP_uint32 time;         // time(NULL)���ص�ֵ
        PP_char const * module; // \0��β
        PP_uint32 level;        // ��־�ȼ�
        PP_uint32 size;         // msg�ĳ��ȣ�������\0��β
        PP_char const * msg;    // \0��β
    } DialogMessage;

    // vector:������־��Ϣ��buffer
    // size:������ȡ��־������
    // module:��ȡ��־��ģ�����ƣ���NULL ��ȡ���⣩
    // level <= ��־�ȼ�
    // ����ֵ��ʵ�ʻ�ȡ��־������
    // ����0��ʾû�л�ȡ���κ���־��
    PPBOX_DECL EXPORT_FUNC(PP_uint32, PPBOX_DialogMessage)(
        DialogMessage * vector, 
        PP_int32 size, 
        PP_char const * module, 
        PP_int32 level);

    PPBOX_DECL EXPORT_FUNC(void, PPBOX_SubmitMessage)(
        PP_char const * msg, 
        PP_int32 size);

    typedef void (*PPBOX_Callback)(void *, PP_int32);

    typedef void (*PPBOX_OnLogDump)( PP_char const *, PP_int32 );

    PPBOX_DECL EXPORT_FUNC(void, PPBOX_LogDump)(
        PPBOX_OnLogDump callback,
        PP_int32 level);

    PPBOX_DECL EXPORT_FUNC(PPBOX_HANDLE, PPBOX_ScheduleCallback)(
        PP_uint32 delay, 
        void * user_data, 
        PPBOX_Callback callback);

    PPBOX_DECL EXPORT_FUNC(PP_err, PPBOX_CancelCallback)(
        PPBOX_HANDLE handle);

#if __cplusplus
}
#endif // __cplusplus

#endif // _PPBOX_PPBOX_I_PPBOX_H_
