//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Properties.ice>

["objc:prefix:TestAdmin"]
module Test
{

interface RemoteCommunicator
{
    Object* getAdmin();

    Ice::PropertyDict getChanges();

    //
    // Logger operations
    //
    void print(string message);
    void trace(string category, string message);
    void warning(string message);
    void error(string message);

    void shutdown();

    void waitForShutdown();

    void destroy();
}

interface RemoteCommunicatorFactory
{
    RemoteCommunicator* createCommunicator(Ice::PropertyDict props);

    void shutdown();
}

interface TestFacet
{
    void op();
}

}
