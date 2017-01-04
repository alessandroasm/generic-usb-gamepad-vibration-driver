
// ComBase.h 
// a very simple COM server without MFC or ATL by :  Shoaib Ali
// you can send your comments at alleey@usa.net
//

#if !defined(AFX_COMBASE_H__369F4E0C_64A7_11D4_B0B4_0050BABFC904__INCLUDED_)
#define AFX_COMBASE_H__369F4E0C_64A7_11D4_B0B4_0050BABFC904__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <objbase.h>

#pragma warning(disable : 4250)

// class for single threaded Objects that uses c++ increment and decrement operators for reference 
// count purposes ..
// 
class CSingleThreaded
{
protected:
	STDMETHODIMP_(ULONG) Increment(long &reflong)  
	{
		reflong ++;
		return reflong; 
	}

	STDMETHODIMP_(ULONG) Decrement(long &reflong)
	{
		reflong --; 
		return reflong; 
	}
};

// class for multi threaded Objects that uses Win32 API for reference 
// count purposes ..
// 
class CMultiThreaded
{
protected:
	STDMETHODIMP_(ULONG) Increment(long &reflong) 
	{ 
		::InterlockedIncrement(&reflong); 
		return reflong; 
	}

	STDMETHODIMP_(ULONG) Decrement(long &reflong) 
	{
		::InterlockedDecrement(&reflong); 
		return reflong; 
	}
};

// the mother of all Objects this basically for object reference count management
// this is the only kool feature of this frame-work ; the way objects handle their lives
// 
class CObjRoot
{
protected:
	long	m_cRef;
protected:
	STDMETHOD_(ULONG,_AddRef)() =0;
	STDMETHOD_(ULONG,_Release)() =0;
public:
	static long *p_ObjCount;
};

// interface implementation class that delegates AddRef and Release to _AddRef and _Release in 
// ComBase that is inherited virtualy..
// 
template <class Interface>
class InterfaceImpl: public virtual CObjRoot, public Interface 
{
public:
	STDMETHOD(QueryInterface)(REFIID riid,LPVOID *ppv) = 0; 
	STDMETHODIMP_(ULONG) AddRef() 
	{
		return _AddRef(); 
	}
	STDMETHODIMP_(ULONG) Release() 
	{
		return _Release();
	}
};

// this mandatory class manages the life of the COM object it uses a perticular threading model
// for reference count management..
//
template <class ThreadModel = CSingleThreaded >
class CComBase  : public virtual CObjRoot ,  public ThreadModel
{

public:
	CComBase() {};
	virtual ~CComBase() {};

protected:

	STDMETHODIMP_(ULONG) _AddRef() 
	{
		if(p_ObjCount)
			ThreadModel::Increment(*p_ObjCount); 
		return ThreadModel::Increment(m_cRef); 
	}
	STDMETHODIMP_(ULONG) _Release() 
	{
		long Value = ThreadModel::Decrement(m_cRef); 
		if(!m_cRef)
			delete this;
		if(p_ObjCount)
			ThreadModel::Decrement(*p_ObjCount); 
		return Value;
	}
};


#endif // !defined(AFX_COMBASE_H__369F4E0C_64A7_11D4_B0B4_0050BABFC904__INCLUDED_)
