//////////////////////////////////////////////////////////////////////////
//
// PpboxMediaSource.h
// Implements the Ppbox media source object.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "PpboxMediaType.h"
#include <InitGuid.h>
#include <wmcodecdsp.h>

#include "SafeRelease.h"

#define PPBOX_EXTERN
#include "plugins/ppbox/ppbox.h"


/*  Static functions */

//-------------------------------------------------------------------
// CreateVideoMediaType:
// Create a media type from an Ppbox video sequence header.
//-------------------------------------------------------------------

HRESULT CreateVideoMediaType(const PPBOX_StreamInfoEx& info, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;

    IMFMediaType *pType = NULL;

    hr = MFCreateMediaType(&pType);

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    }

    if (SUCCEEDED(hr))
    {
        if (info.sub_type == ppbox_video_avc)
            hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        else
            hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_WMV3);
    }

    // Format details.
    if (SUCCEEDED(hr))
    {
        // Frame size

        hr = MFSetAttributeSize(
            pType,
            MF_MT_FRAME_SIZE,
            info.video_format.width,
            info.video_format.height
            );
    }

    if (SUCCEEDED(hr))
    {
        // Frame rate

        hr = MFSetAttributeRatio(
            pType,
            MF_MT_FRAME_RATE,
            info.video_format.frame_rate,
            1
            );
    }

    if (SUCCEEDED(hr))
    {
        // foramt data

        hr = pType->SetBlob(
            MF_MT_USER_DATA,
            info.format_buffer,
            info.format_size
            );
    }

    if (SUCCEEDED(hr))
    {
        *ppType = pType;
        (*ppType)->AddRef();
    }

    SafeRelease(&pType);
    return hr;
}

//-------------------------------------------------------------------
// CreateAudioMediaType:
// Create a media type from an Ppbox audio frame header.
//
// Note: This function fills in an PpboxWAVEFORMAT structure and then
// converts the structure to a Media Foundation media type
// (IMFMediaType). This is somewhat roundabout but it guarantees
// that the type can be converted back to an PpboxWAVEFORMAT by the
// decoder if need be.
//
// The WAVEFORMATEX portion of the PpboxWAVEFORMAT structure is
// converted into attributes on the IMFMediaType object. The rest of
// the struct is stored in the MF_MT_USER_DATA attribute.
//-------------------------------------------------------------------
/*
HRESULT LogMediaType(IMFMediaType *pType);
HRESULT CreateAudioMediaType(const PPBOX_StreamInfoEx& info, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;
    IMFMediaType *pType = NULL;
    DWORD dwSize = sizeof(WAVEFORMATEX) + info.format_size;

    WAVEFORMATEX  * wf = (WAVEFORMATEX  *)new BYTE[dwSize];
    if (wf == 0) 
        return(E_OUTOFMEMORY);
    memset(wf, 0, dwSize);

    wf->wFormatTag = WAVE_FORMAT_WMAUDIO2;
    wf->nChannels = info.audio_format.channel_count;
    wf->nSamplesPerSec = info.audio_format.sample_rate;
    wf->nAvgBytesPerSec = 3995;
    wf->nBlockAlign = 742;
    wf->wBitsPerSample = info.audio_format.sample_size;
    wf->cbSize = info.format_size;
    memcpy(wf + 1, info.format_buffer, info.format_size);

    // Use the structure to initialize the Media Foundation media type.
    hr = MFCreateMediaType(&pType);
    if (SUCCEEDED(hr))
    {
        hr = MFInitMediaTypeFromWaveFormatEx(pType, wf, dwSize);
    }

    if (SUCCEEDED(hr))
    {
        *ppType = pType;
        (*ppType)->AddRef();
    }
    
    LogMediaType(pType);

    SafeRelease(&pType);
    return hr;
}
//*/
/*
DEFINE_GUID(MEDIASUBTYPE_RAW_AAC1, 0x000000FF, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
HRESULT CreateAudioMediaType(const PPBOX_StreamInfoEx& info, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;
    IMFMediaType *pType = NULL;

    hr = MFCreateMediaType(&pType);

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    }

    // Subtype = Ppbox payload
    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_RAW_AAC1);
    }

    // Format details.
    if (SUCCEEDED(hr))
    {
        // Sample size

        hr = pType->SetUINT32(
            MF_MT_AUDIO_BITS_PER_SAMPLE,
            info.audio_format.sample_size
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS,
            info.audio_format.channel_count
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND,
            info.audio_format.sample_rate
            );
    }
    if (SUCCEEDED(hr))
    {
        // foramt data

        hr = pType->SetBlob(
            MF_MT_USER_DATA,
            info.format_buffer,
            info.format_size
            );
    }
    if (SUCCEEDED(hr))
    {
        *ppType = pType;
        (*ppType)->AddRef();
    }

    SafeRelease(&pType);
    return hr;
}
//*/
//*
HRESULT Fill_HEAACWAVEFORMAT(const PPBOX_StreamInfoEx& info, PHEAACWAVEFORMAT format)
{
    PWAVEFORMATEX wf = &format->wfInfo.wfx;
    wf->wFormatTag = WAVE_FORMAT_MPEG_HEAAC;
    wf->nChannels = (WORD)info.audio_format.channel_count;
    wf->nSamplesPerSec = info.audio_format.sample_rate;
    wf->nAvgBytesPerSec = 0;
    wf->nBlockAlign = 1;
    wf->wBitsPerSample = (WORD)info.audio_format.sample_size;
    wf->cbSize = (WORD)(sizeof(format->wfInfo) - sizeof(format->wfInfo.wfx) + info.format_size);
	PHEAACWAVEINFO hawi = &format->wfInfo;
	hawi->wPayloadType = 0; // The stream contains raw_data_block elements only. 
	hawi->wAudioProfileLevelIndication = 0x29;
	hawi->wStructType = 0;
	hawi->wReserved1 = 0;
	hawi->dwReserved2 = 0;
	memcpy(format->pbAudioSpecificConfig, info.format_buffer, info.format_size);
	return S_OK;
}

HRESULT CreateAudioMediaType(const PPBOX_StreamInfoEx& info, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;
    IMFMediaType *pType = NULL;

    hr = MFCreateMediaType(&pType);

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    }

    // Subtype = Ppbox payload
    if (SUCCEEDED(hr))
    {
        if (info.sub_type == ppbox_audio_aac)
            hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        else if (info.sub_type == ppbox_audio_mp3)
            hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_MP3);
        else
            hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_WMAudioV8);
    }

    // Format details.
    if (SUCCEEDED(hr))
    {
        // Sample size

        hr = pType->SetUINT32(
            MF_MT_AUDIO_BITS_PER_SAMPLE,
            info.audio_format.sample_size
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS,
            info.audio_format.channel_count
            );
    }
    if (SUCCEEDED(hr))
    {
        // Sample rate

        hr = pType->SetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND,
            info.audio_format.sample_rate
            );
    }

    if (SUCCEEDED(hr))
    {
        // foramt data

        hr = pType->SetBlob(
            MF_MT_USER_DATA,
            info.format_buffer,
            info.format_size
            );
    }

    if (info.sub_type == ppbox_audio_aac)
    {
        if (SUCCEEDED(hr))
        {
            hr = pType->SetUINT32(
                MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION,
                0x29
                );
        }
        if (SUCCEEDED(hr))
        {
            hr = pType->SetUINT32(
                MF_MT_AAC_PAYLOAD_TYPE,
                0
                );
        }
        if (SUCCEEDED(hr))
        {
            // foramt data
		    struct {
			    HEAACWAVEFORMAT format;
			    char aac_config_data_pad[256];
		    } format;

		    Fill_HEAACWAVEFORMAT(info, &format.format);

            hr = pType->SetBlob(
                MF_MT_USER_DATA,
                (UINT8 const *)&format.format.wfInfo.wPayloadType,
                sizeof(HEAACWAVEINFO) - sizeof(WAVEFORMATEX) + info.format_size
                );
        }
    } // if (info.sub_type == ppbox_audio_aac)

    if (SUCCEEDED(hr))
    {
        *ppType = pType;
        (*ppType)->AddRef();
    }

    SafeRelease(&pType);
    return hr;
}
