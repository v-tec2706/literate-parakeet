//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTOR_F_H
#define ICE_CONNECTOR_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class Connector;
ICE_API IceUtil::Shared* upCast(Connector*);
typedef Handle<Connector> ConnectorPtr;

}

#endif
