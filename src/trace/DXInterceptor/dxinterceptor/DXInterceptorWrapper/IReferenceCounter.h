////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

extern const IID IID_IReferenceCounter;

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorWrapper;

class IReferenceCounter : public IUnknown
{
public:

  IReferenceCounter(DXInterceptorWrapper* owner);
  ~IReferenceCounter();

  DXInterceptorWrapper* GetOwner();

  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
  STDMETHOD_(ULONG,AddRef)(THIS);
  STDMETHOD_(ULONG,Release)(THIS);

protected:

  DWORD m_referenceCount;
  DXInterceptorWrapper* m_owner;

};

////////////////////////////////////////////////////////////////////////////////
