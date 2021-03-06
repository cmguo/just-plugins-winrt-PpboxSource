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
#include <InitGuid.h>
#include <wmcodecdsp.h>

#include "SafeRelease.h"

#include "PpboxMediaType.h"


/*  Static functions */

//-------------------------------------------------------------------
// CreateVideoMediaType:
// Create a media type from an Ppbox video sequence header.
//-------------------------------------------------------------------

struct CodecType {
    PP_uint pp_type;
    GUID mf_type;
} const codec_type_table[] = {
    {JUST_VideoSubType::AVC1, MFVideoFormat_H264}, 
    {JUST_VideoSubType::MP4V, MFVideoFormat_MP4V}, 
    {JUST_VideoSubType::WMV2, MFVideoFormat_WMV2}, 
    {JUST_VideoSubType::WMV3, MFVideoFormat_WMV3}, 
    {JUST_VideoSubType::I420, MFVideoFormat_I420}, 
    {JUST_VideoSubType::RGBT, MFVideoFormat_RGB24}, 

    {JUST_AudioSubType::MP4A, MFAudioFormat_AAC}, 
    {JUST_AudioSubType::MP3,  MFAudioFormat_MP3}, 
    {JUST_AudioSubType::MP2,  MFAudioFormat_MPEG}, 
    {JUST_AudioSubType::WMA2, MFAudioFormat_WMAudioV8}, 
    {JUST_AudioSubType::AC3,  MFAudioFormat_Dolby_AC3}, 
    {JUST_AudioSubType::EAC3, MFAudioFormat_Dolby_DDPlus}, 
    {JUST_AudioSubType::FLT,  MFAudioFormat_Float}, 
    {JUST_AudioSubType::PCM,  MFAudioFormat_PCM}, 
};

static GUID find_codec_by_pp_type(
    PP_uint pp_type)
{
    size_t n = sizeof(codec_type_table) / sizeof(codec_type_table[0]);
    for (size_t i = 0; i < n; ++i) {
        if (codec_type_table[i].pp_type == pp_type) {
            return codec_type_table[i].mf_type;
        }
    }
    GUID guid = {pp_type, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};
    return guid;
}

static PP_uint find_codec_by_mf_type(
    GUID mf_type)
{
    size_t n = sizeof(codec_type_table) / sizeof(codec_type_table[0]);
    for (size_t i = 0; i < n; ++i) {
        if (codec_type_table[i].mf_type == mf_type) {
            return codec_type_table[i].pp_type;
        }
    }
    return 0;
}

HRESULT CreateVideoMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType)
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
        GUID mf_type = find_codec_by_pp_type(info.sub_type);
        hr = pType->SetGUID(MF_MT_SUBTYPE, mf_type);
    }

    // Format details.
    if (SUCCEEDED(hr))
    {
        // Frame size

        hr = MFSetAttributeSize(
            pType,
            MF_MT_FRAME_SIZE,
            info.format.video.width,
            info.format.video.height
            );
    }

    if (SUCCEEDED(hr))
    {
        // Frame rate

        hr = MFSetAttributeRatio(
            pType,
            MF_MT_FRAME_RATE,
            info.format.video.frame_rate_num,
            info.format.video.frame_rate_den
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
HRESULT CreateAudioMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType)
{
    HRESULT hr = S_OK;
    IMFMediaType *pType = NULL;
    DWORD dwSize = sizeof(WAVEFORMATEX) + info.format_size;

    WAVEFORMATEX  * wf = (WAVEFORMATEX  *)new BYTE[dwSize];
    if (wf == 0) 
        return(E_OUTOFMEMORY);
    memset(wf, 0, dwSize);

    wf->wFormatTag = WAVE_FORMAT_WMAUDIO2;
    wf->nChannels = info.format.audio.channel_count;
    wf->nSamplesPerSec = info.format.audio.sample_rate;
    wf->nAvgBytesPerSec = 3995;
    wf->nBlockAlign = 742;
    wf->wBitsPerSample = info.format.audio.sample_size;
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
HRESULT CreateAudioMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType)
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
            info.format.audio.sample_size
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS,
            info.format.audio.channel_count
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND,
            info.format.audio.sample_rate
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
HRESULT Fill_HEAACWAVEFORMAT(const JUST_StreamInfo& info, PHEAACWAVEFORMAT format)
{
    PWAVEFORMATEX wf = &format->wfInfo.wfx;
    wf->wFormatTag = WAVE_FORMAT_MPEG_HEAAC;
    wf->nChannels = (WORD)info.format.audio.channel_count;
    wf->nSamplesPerSec = info.format.audio.sample_rate;
    wf->nAvgBytesPerSec = 0;
    wf->nBlockAlign = 1;
    wf->wBitsPerSample = (WORD)info.format.audio.sample_size;
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

HRESULT CreateAudioMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType)
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
        GUID mf_type = find_codec_by_pp_type(info.sub_type);
        hr = pType->SetGUID(MF_MT_SUBTYPE, mf_type);
    }

    // Format details.
    if (SUCCEEDED(hr) && info.format.audio.sample_size)
    {
        // Sample size

        hr = pType->SetUINT32(
            MF_MT_AUDIO_BITS_PER_SAMPLE,
            info.format.audio.sample_size
            );
    }
    if (SUCCEEDED(hr))
    {
        // Channel count

        hr = pType->SetUINT32(
            MF_MT_AUDIO_NUM_CHANNELS,
            info.format.audio.channel_count
            );
    }
    if (SUCCEEDED(hr))
    {
        // Sample rate

        hr = pType->SetUINT32(
            MF_MT_AUDIO_SAMPLES_PER_SECOND,
            info.format.audio.sample_rate
            );
    }

    if (SUCCEEDED(hr) && info.format.audio.block_align)
    {
        // Block alignment

        hr = pType->SetUINT32(
            MF_MT_AUDIO_BLOCK_ALIGNMENT,
            info.format.audio.block_align
            );
    }
    if (SUCCEEDED(hr) && info.bitrate)
    {
        // Bytes per second

        hr = pType->SetUINT32(
            MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
            info.bitrate / 8
            );
    }

    if (SUCCEEDED(hr) && info.format_size)
    {
        // foramt data

        hr = pType->SetBlob(
            MF_MT_USER_DATA,
            info.format_buffer,
            info.format_size
            );
    }

    if (info.sub_type == JUST_AudioSubType::MP4A)
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
    } // if (info.sub_type == JUST_AudioSubType::MP4A)

    if (SUCCEEDED(hr))
    {
        *ppType = pType;
        (*ppType)->AddRef();
    }

    SafeRelease(&pType);
    return hr;
}


HRESULT CreateSample(JUST_Sample const & sample, IMFSample **ppSample)
{
    HRESULT hr = S_OK;
    IMFMediaBuffer      *pBuffer = NULL;
    IMFSample           *pSample = NULL;
    BYTE                *pData = NULL;      // Pointer to the IMFMediaBuffer data.

    // Create a media buffer for the payload.
    if (SUCCEEDED(hr))
    {
		hr = MFCreateMemoryBuffer(sample.size, &pBuffer);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBuffer->Lock(&pData, NULL, NULL);
    }

    if (SUCCEEDED(hr))
    {
        CopyMemory(pData, sample.buffer, sample.size);
    }

    if (SUCCEEDED(hr))
    {
        hr = pBuffer->Unlock();
    }

    if (SUCCEEDED(hr))
    {
        hr = pBuffer->SetCurrentLength(sample.size);
    }

    // Create a sample to hold the buffer.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateSample(&pSample);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->AddBuffer(pBuffer);
    }

    // Time stamp the sample.
    if (SUCCEEDED(hr))
    {
        INT64 time = (sample.decode_time + sample.composite_time_delta);
        hr = pSample->SetSampleTime(time);
    }

    if (SUCCEEDED(hr))
    {
        *ppSample = pSample;
        (*ppSample)->AddRef();
    }

    SafeRelease(&pBuffer);
    SafeRelease(&pSample);

    return hr;
}
