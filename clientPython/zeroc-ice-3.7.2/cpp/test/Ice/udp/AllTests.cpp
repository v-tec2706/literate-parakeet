//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

class PingReplyI : public PingReply, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    virtual void
    reply(const Ice::Current&)
    {
        Lock sync(*this);
        ++_replies;
        notify();
    }

    void
    reset()
    {
         _replies = 0;
    }

    bool
    waitReply(int expectedReplies, const IceUtil::Time& timeout)
    {
        Lock sync(*this);
        IceUtil::Time end = IceUtil::Time::now() + timeout;
        while(_replies < expectedReplies)
        {
            IceUtil::Time delay = end - IceUtil::Time::now();
            if(delay > IceUtil::Time::seconds(0))
            {
                timedWait(delay);
            }
            else
            {
                break;
            }
        }
        return _replies == expectedReplies;
    }

private:

    int _replies;
};
ICE_DEFINE_PTR(PingReplyIPtr, PingReplyI);

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    communicator->getProperties()->setProperty("ReplyAdapter.Endpoints", "udp");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ReplyAdapter");
    PingReplyIPtr replyI = ICE_MAKE_SHARED(PingReplyI);
    PingReplyPrxPtr reply = ICE_UNCHECKED_CAST(PingReplyPrx, adapter->addWithUUID(replyI))->ice_datagram();
    adapter->activate();

    cout << "testing udp... " << flush;
    ObjectPrxPtr base = communicator->stringToProxy("test -d:" + helper->getTestEndpoint("udp"));
    TestIntfPrxPtr obj = ICE_UNCHECKED_CAST(TestIntfPrx, base);

    int nRetry = 5;
    bool ret = false;
    while(nRetry-- > 0)
    {
        replyI->reset();
        obj->ping(reply);
        obj->ping(reply);
        obj->ping(reply);
        ret = replyI->waitReply(3, IceUtil::Time::seconds(2));
        if(ret)
        {
            break; // Success
        }

        // If the 3 datagrams were not received within the 2 seconds, we try again to
        // receive 3 new datagrams using a new object. We give up after 5 retries.
        replyI = ICE_MAKE_SHARED(PingReplyI);
        reply = ICE_UNCHECKED_CAST(PingReplyPrx, adapter->addWithUUID(replyI))->ice_datagram();
    }
    test(ret);

    if(communicator->getProperties()->getPropertyAsInt("Ice.Override.Compress") == 0)
    {
        //
        // Only run this test if compression is disabled, the test expect fixed message size
        // to be sent over the wire.
        //

        Test::ByteSeq seq;
        try
        {
            seq.resize(1024);
            while(true)
            {
                seq.resize(seq.size() * 2 + 10);
                replyI->reset();
                obj->sendByteSeq(seq, reply);
                replyI->waitReply(1, IceUtil::Time::seconds(10));
            }
        }
        catch(const DatagramLimitException&)
        {
            test(seq.size() > 16384);
        }
        obj->ice_getConnection()->close(ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));
        communicator->getProperties()->setProperty("Ice.UDP.SndSize", "64000");
        seq.resize(50000);
        try
        {
            replyI->reset();
            obj->sendByteSeq(seq, reply);
            test(!replyI->waitReply(1, IceUtil::Time::milliSeconds(500)));
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
    }

    cout << "ok" << endl;

    ostringstream endpoint;
    if(communicator->getProperties()->getProperty("Ice.IPv6") == "1")
    {
        endpoint << "udp -h \"ff15::1:1\" -p " << helper->getTestPort(10);
#if defined(__APPLE__) || defined(_WIN32)
        endpoint << " --interface \"::1\""; // Use loopback to prevent other machines to answer. the multicast requests.
#endif
    }
    else
    {
        endpoint << "udp -h 239.255.1.1 -p " << helper->getTestPort(10);
#if defined(__APPLE__) || defined(_WIN32)
        endpoint << " --interface 127.0.0.1"; // Use loopback to prevent other machines to answer.
#endif
    }
    base = communicator->stringToProxy("test -d:" + endpoint.str());
    TestIntfPrxPtr objMcast = ICE_UNCHECKED_CAST(TestIntfPrx, base);
#if (!defined(__APPLE__) || (defined(__APPLE__) && !TARGET_OS_IPHONE))
    cout << "testing udp multicast... " << flush;

    nRetry = 5;
    while(nRetry-- > 0)
    {
        replyI->reset();
        objMcast->ping(reply);
        ret = replyI->waitReply(5, IceUtil::Time::seconds(2));
        if(ret)
        {
            break; // Success
        }
        replyI = ICE_MAKE_SHARED(PingReplyI);
        reply = ICE_UNCHECKED_CAST(PingReplyPrx, adapter->addWithUUID(replyI))->ice_datagram();
    }
    if(!ret)
    {
        cout << "failed (is a firewall enabled?)" << endl;
    }
    else
    {
        cout << "ok" << endl;
    }
#endif

    cout << "testing udp bi-dir connection... " << flush;
    obj->ice_getConnection()->setAdapter(adapter);
    objMcast->ice_getConnection()->setAdapter(adapter);
    nRetry = 5;
    while(nRetry-- > 0)
    {
        replyI->reset();
        obj->pingBiDir(reply->ice_getIdentity());
        obj->pingBiDir(reply->ice_getIdentity());
        obj->pingBiDir(reply->ice_getIdentity());
        ret = replyI->waitReply(3, IceUtil::Time::seconds(2));
        if(ret)
        {
            break; // Success
        }

        // If the 3 datagrams were not received within the 2 seconds, we try again to
        // receive 3 new datagrams using a new object. We give up after 5 retries.
        replyI = ICE_MAKE_SHARED(PingReplyI);
        reply = ICE_UNCHECKED_CAST(PingReplyPrx, adapter->addWithUUID(replyI))->ice_datagram();
    }
    test(ret);
    cout << "ok" << endl;

    //
    // Sending the replies back on the multicast UDP connection doesn't work for most
    // platform (it works for macOS Leopard but not Snow Leopard, doesn't work on SLES,
    // Windows...). For Windows, see UdpTransceiver constructor for the details. So
    // we don't run this test.
    //
//     cout << "testing udp bi-dir connection... " << flush;
//     nRetry = 5;
//     while(nRetry-- > 0)
//     {
//         replyI->reset();
//         objMcast->pingBiDir(reply->ice_getIdentity());
//         ret = replyI->waitReply(5, IceUtil::Time::seconds(2));
//         if(ret)
//         {
//             break; // Success
//         }
//         replyI = new PingReplyI;
//         reply = PingReplyPrx::uncheckedCast(adapter->addWithUUID(replyI))->ice_datagram();
//     }
//     if(!ret)
//     {
//         cout << "failed (is a firewall enabled?)" << endl;
//     }
//     else
//     {
//         cout << "ok" << endl;
//     }
}
