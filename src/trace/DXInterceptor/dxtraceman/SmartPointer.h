////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class SmartPointer
{
public:

  virtual ~SmartPointer()
  {
  };

  void AddRef()
  {
    m_refCount++;
  };
  
  void Release()
  {
    if (--m_refCount <= 0)
    {
      delete this;
    }
  };

protected:

  SmartPointer() :
  m_refCount(0)
  {
  };

private:

  SmartPointer(const SmartPointer &b) {};
  SmartPointer&	operator = (const SmartPointer &b) {};

  int m_refCount;

};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class smart_ptr
{
protected:    

  T* p;

public:
  smart_ptr() : p(NULL) {}
  smart_ptr( T* p_ ) : p(p_) { if (p) p->AddRef(); }
  smart_ptr( const smart_ptr<T>& p_ ) : p(p_.p) { if (p) p->AddRef(); } // Copy constructor.
  smart_ptr( int Null ) : p(NULL) {} // Assignment to NULL
  ~smart_ptr() { if (p) p->Release(); }

  // Casting to normal pointer.
  operator T*() const { return p; }

  // Casting to const pointer.
  operator const T*() const { return p; }

  // Dereference operator, allow dereferencing smart pointer just like normal pointer.
  T& operator*() const { return *p; }

  // Arrow operator, allow to use regular C syntax to access members of class.
  T* operator->(void) const { return p; }

  // Replace pointer.
  smart_ptr& operator = ( T* newp )
  {
    if (newp) newp->AddRef();
    if (p) p->Release();
    p = newp;
    return *this;
  }

  // Replace pointer.
  smart_ptr& operator = ( const smart_ptr<T> &newp )
  {
    if (newp.p) newp.p->AddRef();
    if (p) p->Release();
    p = newp.p;
    return *this;
  }

  // Cast to boolean, simplify if statements with smart pointers.
  operator bool() { return p != NULL; };
  operator bool() const { return p != NULL; };
  bool  operator !() { return p == NULL; };

  // Misc compare functions.
  bool  operator == ( const T* p1 ) const { return p == p1; };
  bool  operator != ( const T* p1 ) const { return p != p1; };
  bool  operator <  ( const T* p1 ) const { return p < p1; };
  bool  operator >  ( const T* p1 ) const { return p > p1; };

  bool operator == ( const smart_ptr<T> &p1 ) const { return p == p1.p; };
  bool operator != ( const smart_ptr<T> &p1 ) const { return p != p1.p; };
  bool operator < ( const smart_ptr<T> &p1 ) const { return p < p1.p; };
  bool operator > ( const smart_ptr<T> &p1 ) const { return p > p1.p; };

  friend bool operator == ( const smart_ptr<T> &p1,int null );
  friend bool operator != ( const smart_ptr<T> &p1,int null );
  friend bool operator == ( int null,const smart_ptr<T> &p1 );
  friend bool operator != ( int null,const smart_ptr<T> &p1 );
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline bool operator == ( const smart_ptr<T> &p1,int null )
{
  return p1.p == 0;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline bool operator != ( const smart_ptr<T> &p1,int null )
{
  return p1.p != 0;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline bool operator == ( int null,const smart_ptr<T> &p1 )
{
  return p1.p == 0;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline bool operator != ( int null,const smart_ptr<T> &p1 )
{
  return p1.p != 0;
}

////////////////////////////////////////////////////////////////////////////////
