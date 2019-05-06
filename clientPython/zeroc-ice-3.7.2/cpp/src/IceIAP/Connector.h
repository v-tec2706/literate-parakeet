//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_CONNECTOR_H
#define ICE_IAP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Connector.h>

#import <Foundation/Foundation.h>
#import <ExternalAccessory/ExternalAccessory.h>

namespace IceObjC
{

class iAPEndpointI;

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class iAPConnector : public IceInternal::Connector
{
public:

    virtual IceInternal::TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

private:

    iAPConnector(const IceInternal::ProtocolInstancePtr&, Ice::Int, const std::string&, NSString*, EAAccessory*);
    virtual ~iAPConnector();
    friend class iAPEndpointI;

    const IceInternal::ProtocolInstancePtr _instance;
    const Ice::Int _timeout;
    const std::string _connectionId;
    NSString* _protocol;
    EAAccessory* _accessory;
};

}

#endif
