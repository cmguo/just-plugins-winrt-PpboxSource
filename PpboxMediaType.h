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

#include <windows.h>
#include <mfapi.h>

HRESULT CreateVideoMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType);
HRESULT CreateAudioMediaType(const JUST_StreamInfo& info, IMFMediaType **ppType);
