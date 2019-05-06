//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/Config.h>
#include <Ice/ProxyHandle.h>

#ifdef ICE_CPP11_MAPPING
namespace Ice
{

class ObjectPrx;
/// \cond INTERNAL
using ObjectPrxPtr = ::std::shared_ptr<ObjectPrx>;
/// \endcond

}

namespace IceInternal
{

template<typename P>
::std::shared_ptr<P> createProxy();

}

#else // C++98 mapping
namespace IceProxy
{

namespace Ice
{

class Object;
inline Object* upCast(Object* o) { return o; }

}

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;
typedef ObjectPrx ObjectPrxPtr;

}

#endif
#endif
