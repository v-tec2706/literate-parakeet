//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_ENDPOINT_H
#define ICEPY_ENDPOINT_H

#include <Config.h>
#include <Ice/Endpoint.h>

namespace IcePy
{

extern PyTypeObject EndpointType;

bool initEndpoint(PyObject*);

PyObject* createEndpoint(const Ice::EndpointPtr&);
Ice::EndpointPtr getEndpoint(PyObject*);

bool toEndpointSeq(PyObject*, Ice::EndpointSeq&);

}

#endif
