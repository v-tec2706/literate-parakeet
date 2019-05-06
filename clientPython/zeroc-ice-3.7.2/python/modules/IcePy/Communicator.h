//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_COMMUNICATOR_H
#define ICEPY_COMMUNICATOR_H

#include <Config.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

extern PyTypeObject CommunicatorType;

bool initCommunicator(PyObject*);

Ice::CommunicatorPtr getCommunicator(PyObject*);

PyObject* createCommunicator(const Ice::CommunicatorPtr&);
PyObject* getCommunicatorWrapper(const Ice::CommunicatorPtr&);

}

extern "C" PyObject* IcePy_initialize(PyObject*, PyObject*);
extern "C" PyObject* IcePy_initializeWithProperties(PyObject*, PyObject*);
extern "C" PyObject* IcePy_initializeWithLogger(PyObject*, PyObject*);
extern "C" PyObject* IcePy_initializeWithPropertiesAndLogger(PyObject*, PyObject*);

extern "C" PyObject* IcePy_identityToString(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringToIdentity(PyObject*, PyObject*);

#endif
