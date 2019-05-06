//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_WS_ACCEPTOR_I_H
#define ICE_WS_ACCEPTOR_I_H

#include <Ice/LoggerF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Acceptor.h>
#include <Ice/Network.h>
#include <Ice/ProtocolInstance.h>

namespace IceInternal
{

class WSEndpoint;

class WSAcceptor : public Acceptor, public NativeInfo
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

private:

    WSAcceptor(const WSEndpointPtr&, const ProtocolInstancePtr&, const AcceptorPtr&);
    virtual ~WSAcceptor();
    friend class WSEndpoint;

    WSEndpointPtr _endpoint;
    const ProtocolInstancePtr _instance;
    const AcceptorPtr _delegate;
};

}

#endif
