//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

std::string
AI::callA(const Ice::Current&)
{
    return "A";
}

std::string
BI::callB(const Ice::Current&)
{
    return "B";
}

std::string
CI::callC(const Ice::Current&)
{
    return "C";
}

std::string
DI::callD(const Ice::Current&)
{
    return "D";
}

std::string
EI::callE(const Ice::Current&)
{
    return "E";
}

std::string
FI::callF(const Ice::Current&)
{
    return "F";
}

void
GI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

std::string
GI::callG(const Ice::Current&)
{
    return "G";
}

std::string
HI::callH(const Ice::Current&)
{
    return "H";
}
