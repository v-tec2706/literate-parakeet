//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

#include <fstream>

using namespace std;

TestI::TestI(const Ice::PropertiesPtr& properties) :
    _properties(properties)
{
}

string
TestI::getServerFile(const string& path, const Ice::Current&)
{
    string file = _properties->getProperty("ServerDistrib") + "/" + path;
    ifstream is(file.c_str());
    string content;
    if(is.good())
    {
        is >> content;
    }
    return content;
}

string
TestI::getApplicationFile(const string& path, const Ice::Current&)
{
    string file = _properties->getProperty("ApplicationDistrib") + "/" + path;
    ifstream is(file.c_str());
    string content;
    if(is.good())
    {
        is >> content;
    }
    return content;
}
