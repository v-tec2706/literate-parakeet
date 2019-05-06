//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TCP_ACCEPTOR_H
#define ICE_TCP_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Acceptor.h>
#include <Ice/Network.h>

#if defined(ICE_OS_UWP)
#include <deque>
#endif

namespace IceInternal
{

class TcpEndpoint;

class TcpAcceptor : public Acceptor, public NativeInfo
{
public:

    virtual NativeInfoPtr getNativeInfo();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    virtual AsyncInfo* getAsyncInfo(SocketOperation);
#endif

    virtual void close();
    virtual EndpointIPtr listen();
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
    virtual void startAccept();
    virtual void finishAccept();
#endif

    virtual TransceiverPtr accept();
    virtual std::string protocol() const;
    virtual std::string toString() const;
    virtual std::string toDetailedString() const;

    int effectivePort() const;

private:

    TcpAcceptor(const TcpEndpointIPtr&, const ProtocolInstancePtr&, const std::string&, int);
    virtual ~TcpAcceptor();
    friend class TcpEndpointI;

    TcpEndpointIPtr _endpoint;
    const ProtocolInstancePtr _instance;
    const Address _addr;

    int _backlog;
#if defined(ICE_USE_IOCP)
    SOCKET _acceptFd;
    int _acceptError;
    std::vector<char> _acceptBuf;
    AsyncInfo _info;
#elif defined(ICE_OS_UWP)
    IceUtil::Mutex _mutex;
    bool _acceptPending;
    std::deque<Windows::Networking::Sockets::StreamSocket^> _accepted;
#endif
};

}
#endif
