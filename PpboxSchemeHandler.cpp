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

#include <SafeRelease.h>
#include <Trace.h>

//-------------------------------------------------------------------
// PpboxByteStreamHandler  class
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

PpboxSchemeHandler::PpboxSchemeHandler()
    : m_cRef(1)
    , m_pCallback(this, &PpboxSchemeHandler::OpenCallback)
    , m_pSource(NULL)
{
    TRACE(3, L"PpboxSchemeHandler::PpboxSchemeHandler\r\n");
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

PpboxSchemeHandler::~PpboxSchemeHandler()
{
    SafeRelease(&m_pSource);
    SafeRelease(&m_pConfiguration);
    TRACE(3, L"PpboxSchemeHandler::~PpboxSchemeHandler\r\n");
}


// IMediaExtension methods
IFACEMETHODIMP PpboxSchemeHandler::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
    m_pConfiguration = pConfiguration;
    m_pConfiguration->AddRef();
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
    if (pCallback == NULL)
    {
        return E_POINTER;
    }

    if ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    IMFAsyncResult      *pResult = NULL;
    PpboxMediaSource    *pSource = NULL;

    hr = MFCreateAsyncResult(NULL, pCallback, punkState, &pResult);

    if (SUCCEEDED(hr))
    {
        hr = PpboxMediaSource::CreateInstance(&pSource);
    }
    if (SUCCEEDED(hr))
    {
        m_pSource = pSource;
        m_pSource->AddRef();
        hr = m_pSource->SetProperties(m_pConfiguration);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pSource->AsyncOpen(pwszURL, ppIUnknownCancelCookie, &m_pCallback, pResult);
    }

    SafeRelease(&pResult);
    SafeRelease(&pSource);

    TRACEHR_RET(hr);
}

HRESULT PpboxSchemeHandler::OpenCallback(IMFAsyncResult* pAsyncResult)
{
    HRESULT hr = S_OK;
    IUnknown* pUnknownResult = NULL;
    IMFAsyncResult* pResult = NULL;
    
    hr = pAsyncResult->GetState(&pUnknownResult);
    if (SUCCEEDED(hr))
    {
        hr = pUnknownResult->QueryInterface(IID_PPV_ARGS(&pResult));
    }
    if (SUCCEEDED(hr))
    {
        hr = pResult->SetStatus(pAsyncResult->GetStatus());
    }

    MFInvokeCallback(pResult);

    SafeRelease(&pResult);
    SafeRelease(&pUnknownResult);

    TRACEHR_RET(hr);
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

    TRACEHR_RET(hr);
}

HRESULT PpboxSchemeHandler::CancelObjectCreation(
    IUnknown *pIUnknownCancelCookie)
{
    HRESULT hr = m_pSource->CancelOpen(pIUnknownCancelCookie);
    TRACEHR_RET(hr);
}
