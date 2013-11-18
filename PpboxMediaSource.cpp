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
#include "PpboxMediaSource.h"
#include <InitGuid.h>
#include <wmcodecdsp.h>
#include <atlconv.h>

#include "SafeRelease.h"
#include "SourceOp.h"
#include "PropertySet.h"
#include "Trace.h"

#include "PpboxMediaType.h"
//-------------------------------------------------------------------
//
// Notes:
// This sample contains an Ppbox source.
//
// - The source parses Ppbox systems-layer streams and generates
//   samples that contain Ppbox payloads.
// - The source does not support files that contain a raw Ppbox
//   video or audio stream.
// - The source does not support seeking.
//
//-------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4355 )  // 'this' used in base member initializer list


/* Public class methods */

//-------------------------------------------------------------------
// Name: CreateInstance
// Static method to create an instance of the source.
//
// ppSource:    Receives a ref-counted pointer to the source.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::CreateInstance(PpboxMediaSource **ppSource)
{
    if (ppSource == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    PpboxMediaSource *pSource = new (std::nothrow) PpboxMediaSource(hr);
    if (pSource == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppSource = pSource;
        (*ppSource)->AddRef();
    }

    SafeRelease(&pSource);
    TRACEHR_RET(hr);
}


IFACEMETHODIMP PpboxMediaSource::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
    using namespace ABI::Windows::Foundation;
    using namespace ABI::Windows::Foundation::Collections;
    ComPtr<IPropertySet> spConfigurations(pConfiguration);

    HRESULT hr = PropertySetAddSubMap(spConfigurations, L"SourceStatistics", m_pStatMap);

    PropertySetSet(m_pStatMap, L"ConnectionStatus", m_uConnectionStatus);
    PropertySetSet(m_pStatMap, L"BytesRecevied", m_uBytesRecevied);
    PropertySetSet(m_pStatMap, L"DownloadSpeed", m_uDownloadSpeed);

    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// IUnknown methods
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_POINTER;
    }
    //TRACE(0, L"PpboxMediaSource::QueryInterface %s\r\n", GetGUIDNameConst(riid));
    HRESULT hr = E_NOINTERFACE;
    (*ppv) = nullptr;
    if (riid == IID_IMFGetService)
    {
        (*ppv) = static_cast<IMFGetService *>(this);
        AddRef();
        hr = S_OK;
    }

    if (riid == IID_IUnknown || 
        riid == IID_IMFMediaEventGenerator ||
        riid == IID_IMFMediaSource)
    {
        (*ppv) = static_cast<IMFMediaSource *>(this);
        AddRef();
        hr = S_OK;
    }

    //TRACEHR_RET(hr);
    return hr;
}

ULONG PpboxMediaSource::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG PpboxMediaSource::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP PpboxMediaSource::GetService( 
    /* [in] */ __RPC__in REFGUID guidService,
    /* [in] */ __RPC__in REFIID riid,
    /* [iid_is][out] */ __RPC__deref_out_opt LPVOID *ppvObject)
{
    //TRACE(0, L"PpboxMediaSource::GetService %s %s\r\n", GetGUIDNameConst(guidService), GetGUIDNameConst(riid));
    HRESULT hr = MF_E_UNSUPPORTED_SERVICE;
    if (guidService == MFNETSOURCE_STATISTICS_SERVICE && riid == IID_IPropertyStore)
    {
        (*ppvObject) = static_cast<IPropertyStore *>(this);
        AddRef();
        hr = S_OK;
    }
    //TRACEHR_RET(hr);
    return hr;
}

static DWORD const NetSourceStatisticsId[] =
{
    MFNETSOURCE_RECVRATE_ID, 
    MFNETSOURCE_BYTESRECEIVED_ID, 
    MFNETSOURCE_BUFFERSIZE_ID, 
    MFNETSOURCE_BUFFERPROGRESS_ID, 
    MFNETSOURCE_DOWNLOADPROGRESS_ID, 
};

STDMETHODIMP PpboxMediaSource::GetCount( 
    /* [out] */ __RPC__out DWORD *cProps)
{
    HRESULT hr = S_OK;
    *cProps = sizeof(NetSourceStatisticsId) / sizeof(NetSourceStatisticsId[0]);
    TRACEHR_RET(hr);
}

STDMETHODIMP PpboxMediaSource::GetAt( 
    /* [in] */ DWORD iProp,
    /* [out] */ __RPC__out PROPERTYKEY *pkey)
{
    HRESULT hr = S_OK;
    if (iProp < sizeof(NetSourceStatisticsId) / sizeof(NetSourceStatisticsId[0]))
    {
        pkey->fmtid = MFNETSOURCE_STATISTICS;
        pkey->pid = NetSourceStatisticsId[iProp];
    }
    else
    {
        hr = E_INVALIDARG ;
    }
    TRACEHR_RET(hr);
}

STDMETHODIMP PpboxMediaSource::GetValue( 
    /* [in] */ __RPC__in REFPROPERTYKEY key,
    /* [out] */ __RPC__out PROPVARIANT *pv)
{
    //TRACE(0, L"PpboxMediaSource::GetValue %s %u\r\n", GetGUIDNameConst(key.fmtid), key.pid);
    HRESULT hr = S_OK;
    pv->vt = VT_EMPTY;
    if (key.fmtid == MFNETSOURCE_STATISTICS)
    {
        for (int i = 0; i < sizeof(NetSourceStatisticsId) / sizeof(NetSourceStatisticsId[0]); ++i)
        {
            if (key.pid == NetSourceStatisticsId[i])
            {
                pv->vt = VT_I4;
                pv->lVal = (&m_uDownloadSpeed)[i];
            }
        }
    }
    TRACEHR_RET(hr);
}

STDMETHODIMP PpboxMediaSource::SetValue( 
    /* [in] */ __RPC__in REFPROPERTYKEY key,
    /* [in] */ __RPC__in REFPROPVARIANT propvar)
{
    HRESULT hr = STG_E_ACCESSDENIED;
    TRACEHR_RET(hr);
}

STDMETHODIMP PpboxMediaSource::Commit( void)
{
    HRESULT hr = STG_E_ACCESSDENIED;
    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// IMFMediaEventGenerator methods
//
// All of the IMFMediaEventGenerator methods do the following:
// 1. Check for shutdown status.
// 2. Call the event queue helper object.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->BeginGetEvent(pCallback, punkState);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}

HRESULT PpboxMediaSource::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->EndGetEvent(pResult, ppEvent);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}

HRESULT PpboxMediaSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent)
{
    // NOTE:
    // GetEvent can block indefinitely, so we don't hold the critical
    // section. Therefore we need to use a local copy of the event queue
    // pointer, to make sure the pointer remains valid.

    HRESULT hr = S_OK;

    IMFMediaEventQueue *pQueue = NULL;

    EnterCriticalSection(&m_critSec);

    // Check shutdown
    hr = CheckShutdown();

    // Cache a local pointer to the queue.
    if (SUCCEEDED(hr))
    {
        pQueue = m_pEventQueue;
        pQueue->AddRef();
    }

    LeaveCriticalSection(&m_critSec);

    // Use the local pointer to call GetEvent.
    if (SUCCEEDED(hr))
    {
        hr = pQueue->GetEvent(dwFlags, ppEvent);
    }

    SafeRelease(&pQueue);
    TRACEHR_RET(hr);
}

HRESULT PpboxMediaSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    LeaveCriticalSection(&m_critSec);

    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// IMFMediaSource methods
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// CreatePresentationDescriptor
// Returns a shallow copy of the source's presentation descriptor.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::CreatePresentationDescriptor(
    IMFPresentationDescriptor** ppPresentationDescriptor
    )
{
    if (ppPresentationDescriptor == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    // Fail if the source is shut down.
    hr = CheckShutdown();

    // Fail if the source was not initialized yet.
    if (SUCCEEDED(hr))
    {
        hr = IsInitialized();
    }

    // Do we have a valid presentation descriptor?
    if (SUCCEEDED(hr))
    {
        if (m_pPresentationDescriptor == NULL)
        {
            hr = MF_E_NOT_INITIALIZED;
        }
    }

    // Clone our presentation descriptor.
    if (SUCCEEDED(hr))
    {
        hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// GetCharacteristics
// Returns capabilities flags.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::GetCharacteristics(DWORD* pdwCharacteristics)
{
    if (pdwCharacteristics == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
		*pdwCharacteristics = 0;
		*pdwCharacteristics |= MFMEDIASOURCE_CAN_PAUSE;
        *pdwCharacteristics |= MFMEDIASOURCE_CAN_SEEK;
        if (m_bLive) {
			//*pdwCharacteristics |= MFMEDIASOURCE_IS_LIVE;
		}
    }

    // NOTE: This sample does not implement seeking, so we do not
    // include the MFMEDIASOURCE_CAN_SEEK flag.

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}

static void OnPpboxTimer(
	PP_context callback, 
	PP_err result)
{
	AsyncCallback<PpboxMediaSource> * async_callback = (AsyncCallback<PpboxMediaSource> *)callback;
	MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_STANDARD, 0, async_callback, NULL);
}


HRESULT PpboxMediaSource::AsyncOpen(
    /* [in] */ LPCWSTR pwszURL,
    /* [out] */ IUnknown **ppIUnknownCancelCookie,
    /* [in] */ IMFAsyncCallback *pCallback,
    /* [in] */ IUnknown *punkState)
{
    TRACE(3, L"PpboxMediaSource::AsyncOpen %p\r\n", this);

    USES_CONVERSION;

    HRESULT hr = S_OK;
    IMFAsyncResult * pResult = NULL;

    hr = MFCreateAsyncResult(NULL, pCallback, punkState, &pResult);

    if (SUCCEEDED(hr))
    {
        m_pOpenResult = pResult;
        m_pOpenResult->AddRef();

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

        m_state = STATE_OPENING;

		AddRef();
        PPBOX_AsyncOpenEx(
			pszPlaylink, 
            "format=raw"
                "&mux.RawMuxer.real_format=asf"
                "&mux.RawMuxer.time_scale=10000000"
                "&mux.Muxer.video_codec=AVC1,MP4V,WMV2,WMV3,I420,RGBT"
                "&mux.Muxer.audio_codec=MP4A,MP3,MP2,WMA2,AC3,EAC3,FLT,PCM"
                "&mux.Encoder.AVC1.param={profile:baseline,ref:2}", // &mux.TimeScale.time_adjust_mode=2
			this, 
			&PpboxMediaSource::StaticOpenCallback);
        m_OnScheduleTimer.AddRef();
		m_keyScheduleTimer = 
			PPBOX_ScheduleCallback(100, &m_OnScheduleTimer, OnPpboxTimer);
    }

	SafeRelease(&pResult);

    return hr;
}

void __cdecl PpboxMediaSource::StaticOpenCallback(PP_context user, PP_err err)
{
	PpboxMediaSource * inst = (PpboxMediaSource *)user;
    if (err != ppbox_success && err != ppbox_already_open && err != ppbox_operation_canceled)
    {
        PPBOX_Close();
    }
    inst->OpenCallback(err == ppbox_success ? S_OK : E_FAIL);
	SafeRelease(&inst);
}

void PpboxMediaSource::OpenCallback(HRESULT hr)
{
    TRACE(3, L"PpboxMediaSource::OpenCallback %p\r\n", this);

    EnterCriticalSection(&m_critSec);

    if (SUCCEEDED(hr))
    {
        hr = InitPresentationDescriptor();
    }
    else
    {
        m_state = STATE_INVALID;
    }

    m_pOpenResult->SetStatus(hr);

    MFInvokeCallback(m_pOpenResult);

    SafeRelease(&m_pOpenResult);

    LeaveCriticalSection(&m_critSec);
}

HRESULT PpboxMediaSource::CancelOpen(
    /* [in] */ IUnknown *pIUnknownCancelCookie)
{
    TRACE(3, L"PpboxMediaSource::CancelOpen %p\r\n", this);

    PPBOX_Close();
    return S_OK;
}

//-------------------------------------------------------------------
// RequestSample
// 
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::RequestSample()
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    hr = CheckShutdown();

    // Queue the operation.
    if (SUCCEEDED(hr))
    {
        hr = QueueAsyncOperation(SourceOp::OP_REQUEST_DATA);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// RequestSample
// 
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::EndOfStream()
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    hr = CheckShutdown();

    // Queue the operation.
    if (SUCCEEDED(hr))
    {
        hr = QueueAsyncOperation(SourceOp::OP_END_OF_STREAM);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// OnEndOfStream
// Called by each stream when it sends the last sample in the stream.
//
// Note: When the media source reaches the end of the MPEG-1 stream,
// it calls EndOfStream on each stream object. The streams might have
// data still in their queues. As each stream empties its queue, it
// notifies the source through an async OP_END_OF_STREAM operation.
//
// When every stream notifies the source, the source can send the
// "end-of-presentation" event.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::OnEndOfStream(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    hr = BeginAsyncOp(pOp);

    // Decrement the count of end-of-stream notifications.
    if (SUCCEEDED(hr))
    {
        --m_cPendingEOS;
        if (m_cPendingEOS == 0)
        {
            // No more streams. Send the end-of-presentation event.
            hr = m_pEventQueue->QueueEventParamVar(MEEndOfPresentation, GUID_NULL, S_OK, NULL);
        }

    }

    if (SUCCEEDED(hr))
    {
        hr = CompleteAsyncOp(pOp);
    }

    TRACEHR_RET(hr);
}


HRESULT PpboxMediaSource::OnScheduleTimer(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    hr = BeginAsyncOp(pOp);

    // Decrement the count of end-of-stream notifications.
    if (SUCCEEDED(hr))
    {
        if (m_state == STATE_OPENING)
        {
            UpdateNetStat();
			    //OutputDebugString(L"[DeliverPayload] would block\r\n");
			m_keyScheduleTimer = 
				PPBOX_ScheduleCallback(100, &m_OnScheduleTimer, OnPpboxTimer);
        }
        else if (m_state == STATE_STARTED)
        {
            m_keyScheduleTimer = 0;
            m_OnScheduleTimer.Release();
            DeliverPayload();
        }
        else
        {
            m_keyScheduleTimer = 0;
            m_OnScheduleTimer.Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = CompleteAsyncOp(pOp);
    }

    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// Pause
// Pauses the source.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::Pause()
{
    TRACE(0, L"PpboxMediaSource::Pause\r\n");

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    hr = CheckShutdown();

    // Queue the operation.
    if (SUCCEEDED(hr))
    {
        hr = QueueAsyncOperation(SourceOp::OP_PAUSE);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// Shutdown
// Shuts down the source and releases all resources.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::Shutdown()
{
    TRACE(0, L"PpboxMediaSource::Shutdown\r\n");

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    PpboxMediaStream *pStream = NULL;

    hr = CheckShutdown();

    if (SUCCEEDED(hr))
    {
        // Shut down the stream objects.

        for (DWORD i = 0; i < m_stream_number; i++)
        {
            pStream = m_streams[i];
            m_streams[i] = NULL;
            (void)pStream->Shutdown();
            SafeRelease(&pStream);
        }

        // Shut down the event queue.
        if (m_pEventQueue)
        {
            (void)m_pEventQueue->Shutdown();
        }

        // Release objects.

        SafeRelease(&m_pEventQueue);
        SafeRelease(&m_pPresentationDescriptor);
        SafeRelease(&m_pCurrentOp);

		if (m_keyScheduleTimer) {
            PPBOX_CancelCallback(m_keyScheduleTimer);
        }

        PPBOX_Close();

        // Set the state.
        m_state = STATE_SHUTDOWN;
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// Start
// Starts or seeks the media source.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::Start(
        IMFPresentationDescriptor* pPresentationDescriptor,
        const GUID* pguidTimeFormat,
        const PROPVARIANT* pvarStartPos
    )
{
    TRACE(0, L"PpboxMediaSource::Start\r\n");

    HRESULT hr = S_OK;
    SourceOp *pAsyncOp = NULL;

    // Check parameters.

    // Start position and presentation descriptor cannot be NULL.
    if (pvarStartPos == NULL || pPresentationDescriptor == NULL)
    {
        return E_INVALIDARG;
    }

    // Check the time format.
    if ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL))
    {
        // Unrecognized time format GUID.
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    // Check the data type of the start position.
    if ((pvarStartPos->vt != VT_I8) && (pvarStartPos->vt != VT_EMPTY))
    {
        return MF_E_UNSUPPORTED_TIME_FORMAT;
    }

    EnterCriticalSection(&m_critSec);

    // Check if this is a seek request. This sample does not support seeking.

    if (pvarStartPos->vt == VT_I8)
    {
        // If the current state is STOPPED, then position 0 is valid.
        // Otherwise, the start position must be VT_EMPTY (current position).

        //if ((m_state != STATE_STOPPED) || (pvarStartPos->hVal.QuadPart != 0))
        //{
        //    hr = MF_E_INVALIDREQUEST;
        //    goto done;
        //}
    }

    // Fail if the source is shut down.
    hr = CheckShutdown();
    if (FAILED(hr))
    {
        goto done;
    }

    // Fail if the source was not initialized yet.
    hr = IsInitialized();
    if (FAILED(hr))
    {
        goto done;
    }

    // Perform a sanity check on the caller's presentation descriptor.
    hr = ValidatePresentationDescriptor(pPresentationDescriptor);
    if (FAILED(hr))
    {
        goto done;
    }

    // The operation looks OK. Complete the operation asynchronously.

    hr = SourceOp::CreateStartOp(pPresentationDescriptor, &pAsyncOp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pAsyncOp->SetData(*pvarStartPos);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = QueueOperation(pAsyncOp);

done:
    SafeRelease(&pAsyncOp);
    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// Stop
// Stops the media source.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::Stop()
{
    TRACE(0, L"PpboxMediaSource::Stop\r\n");

    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    // Fail if the source is shut down.
    hr = CheckShutdown();

    // Fail if the source was not initialized yet.
    if (SUCCEEDED(hr))
    {
        hr = IsInitialized();
    }

    // Queue the operation.
    if (SUCCEEDED(hr))
    {
        hr = QueueAsyncOperation(SourceOp::OP_STOP);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// Public non-interface methods
//-------------------------------------------------------------------

/* Private methods */

PpboxMediaSource::PpboxMediaSource(HRESULT& hr) :
    OpQueue(m_critSec),
    m_cRef(1),
    m_pEventQueue(NULL),
    m_state(STATE_INVALID),
    m_pOpenResult(NULL),
    m_pPresentationDescriptor(NULL),
    m_streams(NULL),
    m_stream_number(0),
    m_pCurrentOp(NULL),
    m_cPendingEOS(0),
	m_bLive(FALSE),
    m_uDuration(0),
	m_uTime(0),
    m_uTimeGetBufferStat(GetTickCount64()),
    m_bBufferring(FALSE),
    m_uDownloadSpeed(0),
    m_uBytesRecevied(0),
    m_uBufferSize(0),
    m_uBufferProcess(0),
    m_uDownloadProcess(0),
    m_uConnectionStatus(0),
    m_OnScheduleTimer(this, &PpboxMediaSource::OnScheduleTimerCallback),
    m_keyScheduleTimer(0)
{
    TRACE(3, L"PpboxMediaSource::PpboxMediaSource %p\r\n", this);

    InitializeCriticalSectionEx(&m_critSec, 1000, 0);

    auto module = ::Microsoft::WRL::GetModuleBase();
    if (module != nullptr)
    {
        module->IncrementObjectCount();
    }

    // Create the media event queue.
    hr = MFCreateEventQueue(&m_pEventQueue);
}

PpboxMediaSource::~PpboxMediaSource()
{
    if (m_state != STATE_INVALID && m_state != STATE_SHUTDOWN)
    {
        Shutdown();
    }

    auto module = ::Microsoft::WRL::GetModuleBase();
    if (module != nullptr)
    {
        module->DecrementObjectCount();
    }

	DeleteCriticalSection(&m_critSec);

    TRACE(3, L"PpboxMediaSource::~PpboxMediaSource %p\r\n", this);
}


//-------------------------------------------------------------------
// IsInitialized:
// Returns S_OK if the source is correctly initialized with an
// Ppbox byte stream. Otherwise, returns MF_E_NOT_INITIALIZED.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::IsInitialized() const
{
    if (m_state == STATE_INVALID)
    {
        return MF_E_NOT_INITIALIZED;
    }
    else
    {
        return S_OK;
    }
}


//-------------------------------------------------------------------
// InitPresentationDescriptor
//
// Creates the source's presentation descriptor, if possible.
//
// During the BeginOpen operation, the source reads packets looking
// for headers for each stream. This enables the source to create the
// presentation descriptor, which describes the stream formats.
//
// This method tests whether the source has seen enough packets
// to create the PD. If so, it invokes the callback to complete
// the BeginOpen operation.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::InitPresentationDescriptor()
{
    HRESULT hr = S_OK;

    assert(m_pPresentationDescriptor == NULL);

    m_uDuration = PPBOX_GetDuration();
	m_bLive = m_uDuration == (PP_uint)-1;
    m_uDuration *= 10000;

	m_stream_number = PPBOX_GetStreamCount();
    // Ready to create the presentation descriptor.

    // Create an array of IMFStreamDescriptor pointers.
    IMFStreamDescriptor **ppSD =
            new (std::nothrow) IMFStreamDescriptor*[m_stream_number];

    if (ppSD == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory(ppSD, m_stream_number * sizeof(IMFStreamDescriptor*));
	m_streams = new PpboxMediaStream*[m_stream_number];
    // Fill the array by getting the stream descriptors from the streams.
    for (DWORD i = 0; i < m_stream_number; i++)
    {
        CreateStream(i, &m_streams[i]);
        hr = m_streams[i]->GetStreamDescriptor(&ppSD[i]);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // Create the presentation descriptor.
    hr = MFCreatePresentationDescriptor(m_stream_number, ppSD,
        &m_pPresentationDescriptor);

    if (FAILED(hr))
    {
        goto done;
    }

    if (!m_bLive)
    {
        hr = m_pPresentationDescriptor->SetUINT64(MF_PD_DURATION, m_uDuration);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // Select all streams.
    for (DWORD i = 0; i < m_stream_number; i++)
    {
        hr = m_pPresentationDescriptor->SelectStream(i);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // Switch state from "STATE_INVALID" to stopped.
    m_state = STATE_STOPPED;

done:
    // clean up:
    if (ppSD)
    {
        for (DWORD i = 0; i < m_stream_number; i++)
        {
            SafeRelease(&ppSD[i]);
        }
        delete [] ppSD;
    }
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// QueueAsyncOperation
// Queue an asynchronous operation.
//
// OpType: Type of operation to queue.
//
// Note: If the SourceOp object requires additional information, call
// OpQueue<SourceOp>::QueueOperation, which takes a SourceOp pointer.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::QueueAsyncOperation(SourceOp::Operation OpType)
{
    HRESULT hr = S_OK;
    SourceOp *pOp = NULL;

    hr = SourceOp::CreateOp(OpType, &pOp);

    if (SUCCEEDED(hr))
    {
        hr = QueueOperation(pOp);
    }

    SafeRelease(&pOp);
    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// BeginAsyncOp
//
// Starts an asynchronous operation. Called by the source at the
// begining of any asynchronous operation.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::BeginAsyncOp(SourceOp *pOp)
{
    // At this point, the current operation should be NULL (the
    // previous operation is NULL) and the new operation (pOp)
    // must not be NULL.

    if (pOp == NULL || m_pCurrentOp != NULL)
    {
        assert(FALSE);
        return E_FAIL;
    }

    // Store the new operation as the current operation.

    m_pCurrentOp = pOp;
    m_pCurrentOp->AddRef();

    return S_OK;
}

//-------------------------------------------------------------------
// CompleteAsyncOp
//
// Completes an asynchronous operation. Called by the source at the
// end of any asynchronous operation.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::CompleteAsyncOp(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    // At this point, the current operation (m_pCurrentOp)
    // must match the operation that is ending (pOp).

    if (pOp == NULL || m_pCurrentOp == NULL)
    {
        assert(FALSE);
        return E_FAIL;
    }

    if (m_pCurrentOp != pOp)
    {
        assert(FALSE);
        return E_FAIL;
    }

    // Release the current operation.
    SafeRelease(&m_pCurrentOp);

    // Process the next operation on the queue.
    hr = ProcessQueue();

    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// DispatchOperation
//
// Performs the asynchronous operation indicated by pOp.
//
// NOTE:
// This method implements the pure-virtual OpQueue::DispatchOperation
// method. It is always called from a work-queue thread.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::DispatchOperation(SourceOp *pOp)
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    if (m_state == STATE_SHUTDOWN)
    {
        LeaveCriticalSection(&m_critSec);

        return S_OK; // Already shut down, ignore the request.
    }

    switch (pOp->Op())
    {

    // IMFMediaSource methods:

    case SourceOp::OP_START:
        hr = DoStart((StartOp*)pOp);
        break;

    case SourceOp::OP_STOP:
        hr = DoStop(pOp);
        break;

    case SourceOp::OP_PAUSE:
        hr = DoPause(pOp);
        break;

    // Operations requested by the streams:

    case SourceOp::OP_REQUEST_DATA:
        hr = OnStreamRequestSample(pOp);
        break;

    case SourceOp::OP_END_OF_STREAM:
        hr = OnEndOfStream(pOp);
        break;

    case SourceOp::OP_TIMER:
        hr = OnScheduleTimer(pOp);
        break;

    default:
        hr = E_UNEXPECTED;
    }

    if (FAILED(hr))
    {
        StreamingError(hr);
    }

    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// ValidateOperation
//
// Checks whether the source can perform the operation indicated
// by pOp at this time.
//
// If the source cannot perform the operation now, the method
// returns MF_E_NOTACCEPTING.
//
// NOTE:
// Implements the pure-virtual OpQueue::ValidateOperation method.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::ValidateOperation(SourceOp *pOp)
{
    if (m_pCurrentOp != NULL)
    {
        return MF_E_NOTACCEPTING;
    }
    return S_OK;
}



//-------------------------------------------------------------------
// DoStart
// Perform an async start operation (IMFMediaSource::Start)
//
// pOp: Contains the start parameters.
//
// Note: This sample currently does not implement seeking, and the
// Start() method fails if the caller requests a seek.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::DoStart(StartOp *pOp)
{
    assert(pOp->Op() == SourceOp::OP_START);

    IMFPresentationDescriptor *pPD = NULL;
    IMFMediaEvent  *pEvent = NULL;

    HRESULT     hr = S_OK;
    LONGLONG    llStartOffset = 0;
    BOOL        bRestartFromCurrentPosition = FALSE;
    BOOL        bSentEvents = FALSE;

    hr = BeginAsyncOp(pOp);

    // Get the presentation descriptor from the SourceOp object.
    // This is the PD that the caller passed into the Start() method.
    // The PD has already been validated.
    if (SUCCEEDED(hr))
    {
        hr = pOp->GetPresentationDescriptor(&pPD);
    }
    // Because this sample does not support seeking, the start
    // position must be 0 (from stopped) or "current position."

    // If the sample supported seeking, we would need to get the
    // start position from the PROPVARIANT data contained in pOp.

    if (SUCCEEDED(hr))
    {
        // Select/deselect streams, based on what the caller set in the PD.
        // This method also sends the MENewStream/MEUpdatedStream events.
        hr = SelectStreams(pPD, &pOp->Data());
    }

    if (SUCCEEDED(hr))
    {
        m_state = STATE_STARTED;

        // Queue the "started" event. The event data is the start position.
        hr = m_pEventQueue->QueueEventParamVar(
            MESourceStarted,
            GUID_NULL,
            S_OK,
            &pOp->Data()
            );
    }

    if (FAILED(hr))
    {
        // Failure. Send the error code to the application.

        // Note: It's possible that QueueEvent itself failed, in which case it
        // is likely to fail again. But there is no good way to recover in
        // that case.

        (void)m_pEventQueue->QueueEventParamVar(
            MESourceStarted, GUID_NULL, hr, NULL);
    }

    CompleteAsyncOp(pOp);

    SafeRelease(&pEvent);
    SafeRelease(&pPD);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// DoStop
// Perform an async stop operation (IMFMediaSource::Stop)
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::DoStop(SourceOp *pOp)
{
    HRESULT hr = S_OK;
    QWORD qwCurrentPosition = 0;

    hr = BeginAsyncOp(pOp);

    // Stop the active streams.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; i < m_stream_number; i++)
        {
            if (m_streams[i]->IsActive())
            {
                hr = m_streams[i]->Stop();
            }
            if (FAILED(hr))
            {
                break;
            }
        }
    }

    m_state = STATE_STOPPED;

	if (m_keyScheduleTimer)
		PPBOX_CancelCallback(m_keyScheduleTimer);

    // Send the "stopped" event. This might include a failure code.
    (void)m_pEventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, hr, NULL);

    CompleteAsyncOp(pOp);

    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// DoPause
// Perform an async pause operation (IMFMediaSource::Pause)
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::DoPause(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    hr = BeginAsyncOp(pOp);

    // Pause is only allowed while running.
    if (SUCCEEDED(hr))
    {
        if (m_state != STATE_STARTED)
        {
            hr = MF_E_INVALID_STATE_TRANSITION;
        }
    }

    // Pause the active streams.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; i < m_stream_number; i++)
        {
            if (m_streams[i]->IsActive())
            {
                hr = m_streams[i]->Pause();
            }
            if (FAILED(hr))
            {
                break;
            }
        }
    }

    m_state = STATE_PAUSED;


    // Send the "paused" event. This might include a failure code.
    (void)m_pEventQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, hr, NULL);

    CompleteAsyncOp(pOp);

    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// StreamRequestSample
// Called by streams when they need more data.
//
// Note: This is an async operation. The stream requests more data
// by queueing an OP_REQUEST_DATA operation.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::OnStreamRequestSample(SourceOp *pOp)
{
    HRESULT hr = S_OK;

    hr = BeginAsyncOp(pOp);

    if (SUCCEEDED(hr))
    {
        DeliverPayload();

        CompleteAsyncOp(pOp);
    }

    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// SelectStreams
// Called during START operations to select and deselect streams.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::SelectStreams(
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    PROPVARIANT * varStart        // New start position.
    )
{
    HRESULT hr = S_OK;
    BOOL    fSelected = FALSE;
    BOOL    fWasSelected = FALSE;
    DWORD   stream_id = 0;

    IMFStreamDescriptor *pSD = NULL;

    PpboxMediaStream *pStream = NULL; // Not add-ref'd

    // Reset the pending EOS count.
    m_cPendingEOS = 0;

    if (varStart->vt == VT_I8 && !m_bLive)
    {
        hr = PPBOX_Seek((PP_uint)(varStart->hVal.QuadPart / 10000));
        if (hr == ppbox_success || hr == ppbox_would_block) {
			m_uTime = varStart->hVal.QuadPart;
            hr = S_OK;
        } else {
            hr = E_FAIL;
        }
    }
    else
    {
        //varStart->vt = VT_I8;
        //varStart->hVal.QuadPart = m_uTime;
    }

    if (FAILED(hr))
    {
        goto done;
    }

    // Loop throught the stream descriptors to find which streams are active.
    for (DWORD i = 0; i < m_stream_number; i++)
    {
        hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = pSD->GetStreamIdentifier(&stream_id);
		assert(stream_id == i);
        if (FAILED(hr))
        {
            goto done;
        }

        pStream = m_streams[i];
        if (pStream == NULL)
        {
            hr = E_INVALIDARG;
            goto done;
        }

        // Was the stream active already?
        fWasSelected = pStream->IsActive();

        // Activate or deactivate the stream.
        hr = pStream->Activate(fSelected);
        if (FAILED(hr))
        {
            goto done;
        }

        if (fSelected)
        {
            m_cPendingEOS++;

            // If the stream was previously selected, send an "updated stream"
            // event. Otherwise, send a "new stream" event.
            MediaEventType met = fWasSelected ? MEUpdatedStream : MENewStream;

            hr = m_pEventQueue->QueueEventParamUnk(met, GUID_NULL, hr, pStream);
            if (FAILED(hr))
            {
                goto done;
            }

            // Start the stream. The stream will send the appropriate event.
            hr = pStream->Start(*varStart);
            if (FAILED(hr))
            {
                goto done;
            }
        }
        SafeRelease(&pSD);
    }

done:
    SafeRelease(&pSD);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// EndOfMPEGStream:
// Called when the parser reaches the end of the Ppbox stream.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::EndOfPpboxStream()
{
    // Notify the streams. The streams might have pending samples.
    // When each stream delivers the last sample, it will send the
    // end-of-stream event to the pipeline and then notify the
    // source.

    // When every stream is done, the source sends the end-of-
    // presentation event.

    HRESULT hr = S_OK;

    for (DWORD i = 0; i < m_stream_number; i++)
    {
        if (m_streams[i]->IsActive())
        {
            hr = m_streams[i]->EndOfStream();
        }
        if (FAILED(hr))
        {
            break;
        }
    }

    TRACEHR_RET(hr);
}

HRESULT PpboxMediaSource::UpdatePlayStat()
{
    HRESULT hr = S_OK;
    PPBOX_PlayStatistic stat = {sizeof(stat)};
    hr = PPBOX_GetPlayStat(&stat);
    if (hr == ppbox_success || hr == ppbox_would_block)
    {
        m_uBufferSize = stat.buffer_time;
        m_uBufferProcess = stat.buffering_present;
        if (!m_bLive)
        {
            m_uDownloadProcess = (UINT32)((m_uTime + m_uBufferSize * 10000) * 100 / m_uDuration);
        }
        if (hr == ppbox_success)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_PENDING;
            return hr;
        }
    }
    else
    {
        hr = E_FAIL;
    }
    TRACEHR_RET(hr);
}


HRESULT PpboxMediaSource::UpdateNetStat()
{
    HRESULT hr = S_OK;
    PPBOX_DataStat stat;
    hr = PPBOX_GetDataStat(&stat);
    if (hr == ppbox_success || hr == ppbox_would_block)
    {
        m_uDownloadSpeed = stat.average_speed_five_seconds;
        m_uBytesRecevied = stat.total_download_bytes;
        m_uConnectionStatus = stat.connection_status;

        PropertySetSet(m_pStatMap, L"ConnectionStatus", m_uConnectionStatus);
        PropertySetSet(m_pStatMap, L"BytesRecevied", m_uBytesRecevied);
        PropertySetSet(m_pStatMap, L"DownloadSpeed", m_uDownloadSpeed);
        if (hr == ppbox_success)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_PENDING;
        }
    }
    else
    {
        hr = E_FAIL;
    }
    TRACEHR_RET(hr);
}

//-------------------------------------------------------------------
// StreamsNeedData:
// Returns TRUE if any streams need more data.
//-------------------------------------------------------------------

BOOL PpboxMediaSource::StreamsNeedData() const
{
    BOOL bNeedData = FALSE;

    switch (m_state)
    {
    case STATE_SHUTDOWN:
        // While shut down, we never need data.
        return FALSE;

    default:
        // If none of the above, ask the streams.
        for (DWORD i = 0; i < m_stream_number; i++)
        {
            if (m_streams[i]->NeedsData())
            {
                bNeedData = TRUE;
                break;
            }
        }
        return bNeedData;
    }
}

//-------------------------------------------------------------------
// DeliverPayload:
// Delivers an Ppbox payload.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::DeliverPayload()
{
    // When this method is called, the read buffer contains a complete
    // payload, and the payload belongs to a stream whose type we support.

    HRESULT             hr = S_OK;
    PPBOX_Sample        sample;

    IMFMediaBuffer      *pBuffer = NULL;
    IMFSample           *pSample = NULL;
    BYTE                *pData = NULL;      // Pointer to the IMFMediaBuffer data.

    if (m_bBufferring)
    {
        while (true)
        {
            hr = UpdatePlayStat();

            if (m_uBufferProcess >= 100)
            {
                m_bBufferring = FALSE;
                hr = m_pEventQueue->QueueEventParamVar(MEBufferingStopped, GUID_NULL, S_OK, NULL);
                hr = S_OK;
                break;
            }

            if (hr == S_OK)
            {
                continue;
            }
            else if (hr == E_PENDING)
            {
		        if (m_keyScheduleTimer == 0) {
			        //OutputDebugString(L"[DeliverPayload] would block\r\n");
                    m_OnScheduleTimer.AddRef();
			        m_keyScheduleTimer = 
				        PPBOX_ScheduleCallback(100, &m_OnScheduleTimer, OnPpboxTimer);
		        }
                UpdateNetStat();
                hr = S_OK;
                TRACEHR_RET(hr);
            }
            else
            {
                TRACEHR_RET(hr);
            }
        }
    }

    if (m_uTimeGetBufferStat <= GetTickCount64())
    {
        UpdatePlayStat();
        UpdateNetStat();
        m_uTimeGetBufferStat += 1000;
    }

    hr = PPBOX_ReadSample(&sample);

    if (hr == ppbox_success)
    {
        hr = S_OK;
    }
    else if (hr == ppbox_would_block)
    {
        //hr = MFScheduleWorkItem(&m_OnScheduleTimer, NULL, -100, NULL);
        //TRACEHR_RET(hr);
        m_bBufferring = TRUE;
        hr = m_pEventQueue->QueueEventParamVar(MEBufferingStarted, GUID_NULL, S_OK, NULL);
		if (m_keyScheduleTimer == 0) {
			//OutputDebugString(L"[DeliverPayload] would block\r\n");
            m_OnScheduleTimer.AddRef();
			m_keyScheduleTimer = 
				PPBOX_ScheduleCallback(100, &m_OnScheduleTimer, OnPpboxTimer);
		}
		hr = S_OK;
        TRACEHR_RET(hr);
    }
    else if (hr == ppbox_stream_end)
    {
        hr = EndOfPpboxStream();
        TRACEHR_RET(hr);
    }
    else
    {
        hr = E_FAIL;
        TRACEHR_RET(hr);
    }

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

    // Time stamp
    if (SUCCEEDED(hr))
    {
        m_uTime = (sample.decode_time + sample.composite_time_delta);
		
        hr = pSample->SetSampleTime(m_uTime);
    }

    // Duration
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleDuration(sample.duration);
    }

    // Deliver the payload to the stream.
    if (SUCCEEDED(hr))
    {
        assert(sample.itrack < m_stream_number);
        //TRACE(0, L"sample itrack = %u, pts = %lu\r\n", sample.itrack, sample.decode_time + sample.composite_time_delta);
		if (m_streams[sample.itrack]->IsActive())
			hr = m_streams[sample.itrack]->DeliverPayload(pSample);
    }

    if (SUCCEEDED(hr))
    {
        if (StreamsNeedData())
        {
            hr = RequestSample();
        }
    }

    if (FAILED(hr))
    {
        hr = EndOfPpboxStream();
    }

    SafeRelease(&pBuffer);
    SafeRelease(&pSample);
    TRACEHR_RET(hr);
}



//-------------------------------------------------------------------
// CreateStream:
// Creates a media stream, based on a packet header.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::CreateStream(long stream_id, PpboxMediaStream **ppStream)
{
    HRESULT hr = S_OK;

    PPBOX_StreamInfo info;

    IMFMediaType *pType = NULL;
    IMFStreamDescriptor *pSD = NULL;
    IMFMediaTypeHandler *pHandler = NULL;
    PpboxMediaStream *pStream = NULL;

    PPBOX_GetStreamInfo(stream_id, &info);

    switch (info.type)
    {
    case PPBOX_StreamType::VIDE:
        hr = CreateVideoMediaType(info, &pType);
        break;

    case PPBOX_StreamType::AUDI:
        hr = CreateAudioMediaType(info, &pType);
        break;

    default:
        assert(false); // If this case occurs, then IsStreamTypeSupported() is wrong.
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        // Create the stream descriptor from the media type.
        hr = MFCreateStreamDescriptor(stream_id, 1, &pType, &pSD);
    }

    // Set the default media type on the stream handler.
    if (SUCCEEDED(hr))
    {
        hr = pSD->GetMediaTypeHandler(&pHandler);
    }
    if (SUCCEEDED(hr))
    {
        hr = pHandler->SetCurrentMediaType(pType);
    }

    // Create the new stream.
    if (SUCCEEDED(hr))
    {
        pStream = new (std::nothrow) PpboxMediaStream(this, pSD, hr);
        if (pStream == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppStream = pStream;
		pStream->AddRef();
    }

    SafeRelease(&pStream);
    SafeRelease(&pHandler);
    SafeRelease(&pSD);
    SafeRelease(&pType);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// ValidatePresentationDescriptor:
// Validates the presentation descriptor that the caller specifies
// in IMFMediaSource::Start().
//
// Note: This method performs a basic sanity check on the PD. It is
// not intended to be a thorough validation.
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD)
{
    HRESULT hr = S_OK;
    BOOL fSelected = FALSE;
    DWORD stream_number = 0;

    IMFStreamDescriptor *pSD = NULL;

    // The caller's PD must have the same number of streams as ours.
    hr = pPD->GetStreamDescriptorCount(&stream_number);

    if (SUCCEEDED(hr))
    {
        if (stream_number != m_stream_number)
        {
            hr = E_INVALIDARG;
        }
    }

    // The caller must select at least one stream.
    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; i < stream_number; i++)
        {
            hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);
            if (FAILED(hr))
            {
                break;
            }
            if (fSelected)
            {
                break;
            }
            SafeRelease(&pSD);
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!fSelected)
        {
            hr = E_INVALIDARG;
        }
    }

    SafeRelease(&pSD);
    TRACEHR_RET(hr);
}


//-------------------------------------------------------------------
// StreamingError:
// Handles an error that occurs duing an asynchronous operation.
//
// hr: Error code of the operation that failed.
//-------------------------------------------------------------------

void PpboxMediaSource::StreamingError(HRESULT hr)
{
    if (m_state != STATE_SHUTDOWN)
    {
        // An error occurred during streaming. Send the MEError event
        // to notify the pipeline.

        QueueEvent(MEError, GUID_NULL, hr, NULL);
    }
}

//-------------------------------------------------------------------
// OnScheduleTimer
// Called when an asynchronous read completes.
//
// 
//-------------------------------------------------------------------

HRESULT PpboxMediaSource::OnScheduleTimerCallback(IMFAsyncResult *pResult)
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;
    DWORD cbRead = 0;

    IUnknown *pState = NULL;

    if (m_state == STATE_SHUTDOWN)
    {
        // If we are shut down, then we've already released the
        // byte stream. Nothing to do.
        LeaveCriticalSection(&m_critSec);
        return S_OK;
    }

    // Get the state object. This is either NULL or the most
    // recent OP_REQUEST_DATA operation.
    (void)pResult->GetState(&pState);

    // Complete the read opertation.
    hr = pResult->GetStatus();

    if (FAILED(hr))
    {
        StreamingError(hr);
    }

	//OutputDebugString(L"OnScheduleTimer\r\n");

    hr = QueueAsyncOperation(SourceOp::OP_TIMER);

    SafeRelease(&pState);
    LeaveCriticalSection(&m_critSec);
    TRACEHR_RET(hr);
}


#pragma warning( pop )
