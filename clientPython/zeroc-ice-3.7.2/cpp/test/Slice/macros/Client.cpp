//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace Test;
using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int, char**)
{
    cout << "testing Slice predefined macros... " << flush;
    DefaultPtr d = ICE_MAKE_SHARED(Default);
    test(d->x == 10);
    test(d->y == 10);

    CppOnlyPtr c = ICE_MAKE_SHARED(CppOnly);
    test(c->lang == "cpp");
    test(c->version == ICE_INT_VERSION);
    cout << "ok" << endl;
}

DEFINE_TEST(Client)
