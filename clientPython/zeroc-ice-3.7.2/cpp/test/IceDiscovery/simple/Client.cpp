//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
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
    //
    // Explicitly register the IceDiscovery plugin to test registerIceDiscovery.
    //
    Ice::registerIceDiscovery();

    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    int num = argc == 2 ? atoi(argv[1]) : 1;

    void allTests(Test::TestHelper*, int);
    allTests(this, num);
}

DEFINE_TEST(Client)
