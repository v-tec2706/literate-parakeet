//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <IceSSL/Plugin.h>
#include <Test.h>
#include <TestHelper.h>

using namespace std;

class AdminPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string& passwd, string&, const Ice::Current& c) const
    {
        if(c.ctx.find("throw") != c.ctx.end())
        {
            throw Test::ExtendedPermissionDeniedException("reason");
        }
        return (userId == "admin1" && passwd == "test1") ||
               (userId == "admin2" && passwd == "test2") ||
               (userId == "admin3" && passwd == "test3");
    }
};

class PermissionsVerifierServer : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
PermissionsVerifierServer::run(int argc, char** argv)
{
    shutdownOnInterrupt();
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("PermissionsVerifier");
    adapter->add(new AdminPermissionsVerifierI, Ice::stringToIdentity("AdminPermissionsVerifier"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(PermissionsVerifierServer)
