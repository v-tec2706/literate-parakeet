//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <TestHelper.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Test;

namespace
{

void testContext(bool ssl, const Ice::CommunicatorPtr& communicator, const Ice::Context& context)
{
    Ice::Context ctx = context;
    if(!ssl)
    {
        test(ctx["_con.type"] == "tcp");
        ostringstream port;
        port << TestHelper::getTestPort(communicator->getProperties());
        test(ctx["_con.localPort"] == port.str());
    }
    else
    {
        test(ctx["_con.type"] == "ssl");
        ostringstream port;
        port << TestHelper::getTestPort(communicator->getProperties(), 1);
        test(ctx["_con.localPort"] == port.str());
    }
    test(ctx["_con.localAddress"] == "127.0.0.1");
    test(ctx["_con.remotePort"] != "");
    test(ctx["_con.remoteAddress"] == "127.0.0.1");
}

}

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string&, string&, const Ice::Current& current) const
    {
        testContext(userId == "ssl", current.adapter->getCommunicator(), current.ctx);
        return true;
    }
};

class SSLPermissionsVerifierI : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo& info, string&, const Ice::Current& current) const
    {
        testContext(true, current.adapter->getCommunicator(), current.ctx);

        IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
        test(cert->getIssuerDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
        test(cert->getSubjectDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
        test(cert->checkValidity());

        return true;
    }
};

class SessionI : public Glacier2::Session
{
public:

    SessionI(bool shutdown, bool ssl) : _shutdown(shutdown), _ssl(ssl)
    {
    }

    virtual void
    destroy(const Ice::Current& current)
    {
        testContext(_ssl, current.adapter->getCommunicator(), current.ctx);

        current.adapter->remove(current.id);
        if(_shutdown)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    virtual void
    ice_ping(const Ice::Current& current) const
    {
        testContext(_ssl, current.adapter->getCommunicator(), current.ctx);
    }

private:

    const bool _shutdown;
    const bool _ssl;
};

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
        testContext(userId == "ssl", current.adapter->getCommunicator(), current.ctx);

        Glacier2::SessionPtr session = new SessionI(false, userId == "ssl");
        return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    }
};

class SSLSessionManagerI : public Glacier2::SSLSessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const Glacier2::SSLInfo& info, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
        testContext(true, current.adapter->getCommunicator(), current.ctx);

        test(info.remoteHost == "127.0.0.1");
        test(info.localHost == "127.0.0.1");
        test(info.localPort == TestHelper::getTestPort(current.adapter->getCommunicator()->getProperties(), 1));

        try
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
            test(cert->getIssuerDN() == IceSSL::DistinguishedName(
                     "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
            test(cert->getSubjectDN() == IceSSL::DistinguishedName(
                     "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
            test(cert->checkValidity());
        }
        catch(const IceSSL::CertificateReadException&)
        {
            test(false);
        }

        Glacier2::SessionPtr session = new SessionI(true, true);
        return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    }
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("SessionServer",
                                                                                   getTestEndpoint(3, "tcp"));
    adapter->add(new PermissionsVerifierI, Ice::stringToIdentity("verifier"));
    adapter->add(new SSLPermissionsVerifierI, Ice::stringToIdentity("sslverifier"));
    adapter->add(new SessionManagerI, Ice::stringToIdentity("sessionmanager"));
    adapter->add(new SSLSessionManagerI, Ice::stringToIdentity("sslsessionmanager"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
