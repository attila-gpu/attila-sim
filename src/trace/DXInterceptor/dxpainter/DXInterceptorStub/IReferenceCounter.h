////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

extern const IID IID_IReferenceCounter;

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorStub;

class IReferenceCounter : public IUnknown
{
public:

  IReferenceCounter(DXInterceptorStub* owner);
  ~IReferenceCounter();

  DXInterceptorStub* GetOwner();

  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
  STDMETHOD_(ULONG,AddRef)(THIS);
  STDMETHOD_(ULONG,Release)(THIS);

protected:

  DWORD m_referenceCount;
  DXInterceptorStub* m_owner;

};

////////////////////////////////////////////////////////////////////////////////
