//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Controller.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

class Control : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Control::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    if(argc < 2)
    {
        ostringstream os;
        os << "Usage: " << argv[0] << " proxy";
        throw invalid_argument(os.str());
    }

    ControllerPrx control = ControllerPrx::uncheckedCast(communicator->stringToProxy(argv[1]));
    control->stop();
}

DEFINE_TEST(Control)
