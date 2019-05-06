//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>
#include <SystemFailure.h>

RetryI::RetryI() : _counter(0)
{
}

void
RetryI::op(bool kill, const Ice::Current& current)
{
    if(kill)
    {
        if(current.con)
        {
            current.con->close(Ice::ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        }
        else
        {
            throw Ice::ConnectionLostException(__FILE__, __LINE__);
        }
    }
}

int
RetryI::opIdempotent(int nRetry, const Ice::Current&)
{
    if(nRetry < 0)
    {
        _counter = 0;
        return 0;
    }

    if(nRetry > _counter)
    {
        ++_counter;
        throw Ice::ConnectionLostException(__FILE__, __LINE__);
    }
    int counter = _counter;
    _counter = 0;
    return counter;
}

void
RetryI::opNotIdempotent(const Ice::Current&)
{
    throw Ice::ConnectionLostException(__FILE__, __LINE__);
}

void
RetryI::opSystemException(const Ice::Current&)
{
    throw SystemFailure(__FILE__, __LINE__);
}

void
RetryI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
