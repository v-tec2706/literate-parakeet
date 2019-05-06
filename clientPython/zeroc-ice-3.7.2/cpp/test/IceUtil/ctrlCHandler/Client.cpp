//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <TestHelper.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>

using namespace IceUtil;
using namespace std;

void callback(int signal)
{
    cout << "Handling signal " << signal << endl;
}

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int, char**)
{
    {
        cout << "First ignore CTRL+C and the like for 10 seconds (try it!)" << endl;
        CtrlCHandler handler;
        ThreadControl::sleep(Time::seconds(10));

        cout << "Then handling them for another 30 seconds (try it)" << endl;
        handler.setCallback(callback);
        ThreadControl::sleep(Time::seconds(10));
    }
    cout << "And another 10 seconds after ~CtrlCHandler" << endl;
    ThreadControl::sleep(Time::seconds(10));
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
