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
#include "SourceOp.h"
#include "SafeRelease.h"

/* SourceOp class */


//-------------------------------------------------------------------
// CreateOp
// Static method to create a SourceOp instance.
//
// op: Specifies the async operation.
// ppOp: Receives a pointer to the SourceOp object.
//-------------------------------------------------------------------

HRESULT SourceOp::CreateOp(SourceOp::Operation op, SourceOp **ppOp)
{
    if (ppOp == NULL)
    {
        return E_POINTER;
    }

    SourceOp *pOp = new (std::nothrow) SourceOp(op);
    if (pOp  == NULL)
    {
        return E_OUTOFMEMORY;
    }
    *ppOp = pOp;

    return S_OK;
}

//-------------------------------------------------------------------
// CreateStartOp:
// Static method to create a SourceOp instance for the Start()
// operation.
//
// pPD: Presentation descriptor from the caller.
// ppOp: Receives a pointer to the SourceOp object.
//-------------------------------------------------------------------

HRESULT SourceOp::CreateStartOp(IMFPresentationDescriptor *pPD, SourceOp **ppOp)
{
    if (ppOp == NULL)
    {
        return E_POINTER;
    }

    SourceOp *pOp = new (std::nothrow) StartOp(pPD);
    if (pOp == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *ppOp = pOp;
    return S_OK;
}


ULONG SourceOp::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG SourceOp::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

HRESULT SourceOp::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_POINTER;
    }
    HRESULT hr = E_NOINTERFACE;
    (*ppv) = nullptr;
    if (riid == IID_IUnknown)
    {
        (*ppv) = static_cast<IUnknown *>(this);
        AddRef();
        hr = S_OK;
    }
    return hr;
}

SourceOp::SourceOp(Operation op) : m_cRef(1), m_op(op)
{
    PropVariantInit(&m_data);
}

SourceOp::~SourceOp()
{
    PropVariantClear(&m_data);
}

HRESULT SourceOp::SetData(const PROPVARIANT& var)
{
    return PropVariantCopy(&m_data, &var);
}


StartOp::StartOp(IMFPresentationDescriptor *pPD) : SourceOp(SourceOp::OP_START), m_pPD(pPD)
{
    if (m_pPD)
    {
        m_pPD->AddRef();
    }
}

StartOp::~StartOp()
{
    SafeRelease(&m_pPD);
}


HRESULT StartOp::GetPresentationDescriptor(IMFPresentationDescriptor **ppPD)
{
    if (ppPD == NULL)
    {
        return E_POINTER;
    }
    if (m_pPD == NULL)
    {
        return MF_E_INVALIDREQUEST;
    }
    *ppPD = m_pPD;
    (*ppPD)->AddRef();
    return S_OK;
}

#pragma warning( pop )
