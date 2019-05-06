//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_F_H
#define ICE_OBJECT_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class Object;
#ifdef ICE_CPP11_MAPPING
/// \cond INTERNAL
using ObjectPtr = ::std::shared_ptr<Object>;
/// \endcond
#else
ICE_API Object* upCast(Object*);
typedef IceInternal::Handle<Object> ObjectPtr;
typedef ObjectPtr ValuePtr;
/// \cond INTERNAL
ICE_API void _icePatchObjectPtr(ObjectPtr&, const ObjectPtr&);
/// \endcond
#endif

}

#endif
