//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    void allTests(Test::TestHelper*);
    allTests(this);
    //
    // Shutdown the IceBox server.
    //
    ICE_UNCHECKED_CAST(Ice::ProcessPrx,
                       communicator->stringToProxy("DemoIceBox/admin -f Process:default -p 9996"))->shutdown();
}

DEFINE_TEST(Client)
