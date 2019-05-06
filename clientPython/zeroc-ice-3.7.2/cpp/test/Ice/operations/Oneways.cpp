//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

void
oneways(const Ice::CommunicatorPtr&, const Test::MyClassPrxPtr& proxy)
{
    Test::MyClassPrxPtr p = ICE_UNCHECKED_CAST(Test::MyClassPrx, proxy->ice_oneway());

    {
        p->ice_ping();
    }

    {
        try
        {
            p->ice_isA("dummy");
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_id();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        try
        {
            p->ice_ids();
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

    {
        p->opVoid();
    }

    {
        p->opIdempotent();
    }

    {
        p->opNonmutating();
    }

    {
        Ice::Byte b;

        try
        {
            p->opByte(Ice::Byte(0xff), Ice::Byte(0x0f), b);
            test(false);
        }
        catch(const Ice::TwowayOnlyException&)
        {
        }
    }

}
