//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public ::Test::TestIntf
{
public:

    TestI(const Ice::PropertiesPtr&);

    virtual void shutdown(const Ice::Current&);
    virtual std::string getProperty(const std::string&, const Ice::Current&);

private:

    Ice::PropertiesPtr _properties;
};

#endif
