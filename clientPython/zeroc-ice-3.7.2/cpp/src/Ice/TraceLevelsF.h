//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TRACE_LEVELS_F_H
#define ICE_TRACE_LEVELS_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class TraceLevels;
IceUtil::Shared* upCast(TraceLevels*);
typedef Handle<TraceLevels> TraceLevelsPtr;

}

#endif
