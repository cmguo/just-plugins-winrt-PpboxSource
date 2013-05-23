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


#pragma once

#include <new.h>
#include <windows.h>
#include <assert.h>

#ifndef _ASSERTE
#define _ASSERTE assert
#endif

#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mferror.h>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")      // Media Foundation GUIDs

#include "OpQueue.h"
#include "SourceOp.h"

// Forward declares
class PpboxSchemeHandler;
class PpboxMediaSource;
class PpboxMediaStream;
class SourceOp;

typedef ComPtrList<IMFSample>       SampleList;
typedef ComPtrList<IUnknown, true>  TokenList;    // List of tokens for IMFMediaStream::RequestSample

enum SourceState
{
    STATE_INVALID,      // Initial state. Have not started opening the stream.
    STATE_STOPPED,
    STATE_PAUSED,
    STATE_STARTED,
    STATE_SHUTDOWN
};

#include "PpboxMediaStream.h"    // Ppbox stream

#include <vector>

const UINT32 MAX_STREAMS = 32;


// Constants

const DWORD INITIAL_BUFFER_SIZE = 4 * 1024; // Initial size of the read buffer. (The buffer expands dynamically.)
const DWORD READ_SIZE = 4 * 1024;           // Size of each read request.
const DWORD SAMPLE_QUEUE = 2;               // How many samples does each stream try to hold in its queue?

// PpboxMediaSource: The media source object.
class PpboxMediaSource : public OpQueue<SourceOp>, public IMFMediaSource
{
public:
    static HRESULT CreateInstance(PpboxMediaSource **ppSource);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback* pCallback,IUnknown* punkState);
    STDMETHODIMP EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent);
    STDMETHODIMP GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent);
    STDMETHODIMP QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue);

    // IMFMediaSource
    STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor);
    STDMETHODIMP GetCharacteristics(DWORD* pdwCharacteristics);
    STDMETHODIMP Pause();
    STDMETHODIMP Shutdown();
    STDMETHODIMP Start(
        IMFPresentationDescriptor* pPresentationDescriptor,
        const GUID* pguidTimeFormat,
        const PROPVARIANT* pvarStartPosition
    );
    STDMETHODIMP Stop();

    // (This method is public because the streams call it.)
    HRESULT EndOfStream();

    HRESULT RequestSample();

    // Lock/Unlock:
    // Holds and releases the source's critical section. Called by the streams.
    void    Lock() { EnterCriticalSection(&m_critSec); }
    void    Unlock() { LeaveCriticalSection(&m_critSec); }

private:

    HRESULT QueueAsyncOperation(SourceOp::Operation OpType);

    PpboxMediaSource(HRESULT& hr);
    ~PpboxMediaSource();

    // CheckShutdown: Returns MF_E_SHUTDOWN if the source was shut down.
    HRESULT CheckShutdown() const
    {
        return ( m_state == STATE_SHUTDOWN ? MF_E_SHUTDOWN : S_OK );
    }

    HRESULT     CompleteOpen(HRESULT hrStatus);

    HRESULT     IsInitialized() const;
    BOOL        StreamsNeedData() const;

    HRESULT     DoStart(StartOp *pOp);
    HRESULT     DoStop(SourceOp *pOp);
    HRESULT     DoPause(SourceOp *pOp);
    HRESULT     OnStreamRequestSample(SourceOp *pOp);
    HRESULT     OnEndOfStream(SourceOp *pOp);

    HRESULT     InitPresentationDescriptor();
    HRESULT     SelectStreams(IMFPresentationDescriptor *pPD, PROPVARIANT * varStart);

    HRESULT     DeliverPayload();
    HRESULT     EndOfPpboxStream();

    HRESULT     CreateStream(long stream_id);

    HRESULT     ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);

    // Handler for async errors.
    void        StreamingError(HRESULT hr);

    HRESULT     BeginAsyncOp(SourceOp *pOp);
    HRESULT     CompleteAsyncOp(SourceOp *pOp);
    HRESULT     DispatchOperation(SourceOp *pOp);
    HRESULT     ValidateOperation(SourceOp *pOp);

    HRESULT     OnScheduleDelayRequestSample(IMFAsyncResult *pResult);

private:
    long                        m_cRef;                     // reference count

    CRITICAL_SECTION            m_critSec;                  // critical section for thread safety
    SourceState                 m_state;                    // Current state (running, stopped, paused)

    IMFMediaEventQueue          *m_pEventQueue;             // Event generator helper
    IMFPresentationDescriptor   *m_pPresentationDescriptor; // Presentation descriptor.

    PpboxMediaStream			**m_streams;                  // Array of streams.
	DWORD						m_stream_number;

    DWORD                       m_cPendingEOS;              // Pending EOS notifications.
    ULONG                       m_cRestartCounter;          // Counter for sample requests.

    SourceOp                    *m_pCurrentOp;
    SourceOp                    *m_pSampleRequest;

    BOOL                        m_bLive;
    UINT64                      m_uDuration;
    UINT64                      m_uTime;

    // Async callback helper.
    AsyncCallback<PpboxMediaSource>  m_OnScheduleDelayRequestSample;
    //MFWORKITEM_KEY              m_keyScheduleDelayRequestSample;
	void const *				m_keyScheduleDelayRequestSample;
};


