//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_SCOPED_ARRAY_H
#define ICE_UTIL_SCOPED_ARRAY_H

#include <IceUtil/Config.h>

#ifndef ICE_CPP11_MAPPING

//
// Only for the C++98 mapping
//

namespace IceUtil
{

template<typename T>
class ScopedArray
{
public:

    explicit ScopedArray(T* ptr = 0) :
        _ptr(ptr)
    {
    }

    ScopedArray(const ScopedArray& other)
    {
        _ptr = other._ptr;
        const_cast<ScopedArray&>(other)._ptr = 0;
    }

    ~ScopedArray()
    {
        if(_ptr != 0)
        {
            delete[] _ptr;
        }
    }

    void reset(T* ptr = 0)
    {
        assert(ptr == 0 || ptr != _ptr);
        if(_ptr != 0)
        {
            delete[] _ptr;
        }
        _ptr = ptr;
    }

    ScopedArray& operator=(const ScopedArray& other)
    {
        if(_ptr != 0)
        {
            delete[] _ptr;
        }
        _ptr = other._ptr;
        const_cast<ScopedArray&>(other)._ptr = 0;
        return *this;
    }

    T& operator[](size_t i) const
    {
        assert(_ptr != 0);
        assert(i >= 0);
        return _ptr[i];
    }

    T* get() const
    {
        return _ptr;
    }

    void swap(ScopedArray& a)
    {
        T* tmp = a._ptr;
        a._ptr = _ptr;
        _ptr = tmp;
    }

    T* release()
    {
        T* tmp = _ptr;
        _ptr = 0;
        return tmp;
    }

private:

    T* _ptr;
};

} // End of namespace IceUtil

#endif
#endif
