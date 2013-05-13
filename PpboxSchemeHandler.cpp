//////////////////////////////////////////////////////////////////////////
//
// PpboxByteStreamHandler.cpp
// Implements the byte-stream handler for the Ppbox source.
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
#include "PpboxMediaSource.h"
#include "PpboxSchemeHandler.h"

#include "SafeRelease.h"

#include <atlconv.h>

#include <plugins/ppbox/ppbox_dynamic.h>

//-------------------------------------------------------------------
// PpboxByteStreamHandler  class
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

PpboxSchemeHandler::PpboxSchemeHandler()
    : m_cRef(1), m_pSource(NULL), m_pResult(NULL)
{
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

PpboxSchemeHandler::~PpboxSchemeHandler()
{
    SafeRelease(&m_pSource);
    SafeRelease(&m_pResult);
}


// IMediaExtension methods
IFACEMETHODIMP PpboxSchemeHandler::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
    return S_OK;
}

//-------------------------------------------------------------------
// IMFByteStreamHandler methods
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// BeginCreateObject
// Starts creating the media source.
//-------------------------------------------------------------------

HRESULT PpboxSchemeHandler::BeginCreateObject(
    /* [in] */ LPCWSTR pwszURL,
    /* [in] */ DWORD dwFlags,
    /* [in] */ IPropertyStore *pProps,
    /* [out] */ IUnknown **ppIUnknownCancelCookie,
    /* [in] */ IMFAsyncCallback *pCallback,
    /* [in] */ IUnknown *punkState
    )
{
    USES_CONVERSION;

    if (pCallback == NULL)
    {
        return E_POINTER;
    }

    if ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    IMFAsyncResult * pResult = NULL;

    hr = MFCreateAsyncResult(NULL, pCallback, punkState, &pResult);

    if (SUCCEEDED(hr))
    {
        m_pResult = pResult;
        m_pResult->AddRef();

		*ppIUnknownCancelCookie = pResult;
		(*ppIUnknownCancelCookie)->AddRef();

        LPSTR pszPlaylink = W2A(pwszURL);
        if (strncmp(pszPlaylink, "identify:", 9) == 0) {
            char * p = strchr(pszPlaylink, '#');
            *p++ = 0;
            int l = strlen(p);
            char * q = pszPlaylink = pszPlaylink + 8 - l;
            strcpy_s(q, l + 1, p);
            q[l] = ':';
        }

		AddRef();
        PPBOX_AsyncOpenEx(
			pszPlaylink, 
			"format=raw&demux.Source.time_out=0&mux.RawMuxer.video_format=es&mux.RawMuxer.time_scale=10000000", // &mux.TimeScale.time_adjust_mode=2
			this, 
			&PpboxSchemeHandler::StaticOpenCallback);
    }

    return hr;
}

void __cdecl PpboxSchemeHandler::StaticOpenCallback(void * user, long err)
{
	PpboxSchemeHandler * inst = (PpboxSchemeHandler *)user;
    if (err != ppbox_success && err != ppbox_already_open && err != ppbox_operation_canceled)
    {
        PPBOX_Close();
    }
    inst->OpenCallback(err == ppbox_success ? S_OK : E_FAIL);
	SafeRelease(&inst);
}

void PpboxSchemeHandler::OpenCallback(HRESULT hr)
{
    PpboxMediaSource    *pSource = NULL;

    if (SUCCEEDED(hr)) {
        // Create an instance of the media source.
        hr = PpboxMediaSource::CreateInstance(&pSource);
    }

    if (SUCCEEDED(hr)) {
        m_pSource = pSource;
        m_pSource->AddRef();
    }

    m_pResult->SetStatus(hr);

    MFInvokeCallback(m_pResult);

    SafeRelease(&pSource);
}

//-------------------------------------------------------------------
// EndCreateObject
// Completes the BeginCreateObject operation.
//-------------------------------------------------------------------

HRESULT PpboxSchemeHandler::EndCreateObject(
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ MF_OBJECT_TYPE *pObjectType,
        /* [out] */ IUnknown **ppObject)
{
    if (pResult == NULL || pObjectType == NULL || ppObject == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    *pObjectType = MF_OBJECT_INVALID;
    *ppObject = NULL;

    hr = pResult->GetStatus();

    if (SUCCEEDED(hr))
    {
        *pObjectType = MF_OBJECT_MEDIASOURCE;
        assert(m_pSource != NULL);
        hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppObject));
    }

    SafeRelease(&m_pSource);
    SafeRelease(&m_pResult);

    return hr;
}

HRESULT PpboxSchemeHandler::CancelObjectCreation(
    IUnknown *pIUnknownCancelCookie)
{
    PPBOX_Close();
    return S_OK;
}
