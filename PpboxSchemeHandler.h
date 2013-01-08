//////////////////////////////////////////////////////////////////////////
//
// PpboxByteStreamHandler.h
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


#pragma once

#include <wrl\client.h>
#include <wrl\implements.h>
#include <wrl\ftm.h>
#include <wrl\event.h>
#include <wrl\wrappers\corewrappers.h>

#include "Windows.Media.h"

//-------------------------------------------------------------------
// PpboxByteStreamHandler  class
//
// Byte-stream handler for Ppbox streams.
//-------------------------------------------------------------------
#ifndef RUNTIMECLASS_GeometricSource_GeometricSchemeHandler_DEFINED
#define RUNTIMECLASS_GeometricSource_GeometricSchemeHandler_DEFINED
extern const __declspec(selectany) WCHAR RuntimeClass_PpboxSource_PpboxSchemeHandler[] = L"PpboxSource.PpboxSchemeHandler";
#endif

class PpboxMediaSource;

class PpboxSchemeHandler
    : public Microsoft::WRL::RuntimeClass<
		Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >, 
		ABI::Windows::Media::IMediaExtension,
		IMFSchemeHandler >
{
    InspectableClass(RuntimeClass_PpboxSource_PpboxSchemeHandler, BaseTrust)

public:
    PpboxSchemeHandler();

    ~PpboxSchemeHandler();

    IFACEMETHOD (SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration);

    virtual HRESULT STDMETHODCALLTYPE BeginCreateObject( 
        /* [in] */ LPCWSTR pwszURL,
        /* [in] */ DWORD dwFlags,
        /* [in] */ IPropertyStore *pProps,
        /* [out] */ IUnknown **ppIUnknownCancelCookie,
        /* [in] */ IMFAsyncCallback *pCallback,
        /* [in] */ IUnknown *punkState);

    virtual HRESULT STDMETHODCALLTYPE EndCreateObject( 
        /* [in] */ IMFAsyncResult *pResult,
        /* [out] */ MF_OBJECT_TYPE *pObjectType,
        /* [out] */ IUnknown **ppObject);

    virtual HRESULT STDMETHODCALLTYPE CancelObjectCreation( 
        /* [in] */ IUnknown *pIUnknownCancelCookie);

private:
    static void __cdecl StaticOpenCallback(long err);

    void OpenCallback(HRESULT hr);

    long            m_cRef; // reference count
    PpboxMediaSource     *m_pSource;
    IMFAsyncResult  *m_pResult;
};
