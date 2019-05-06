//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/IceIAP/ConnectionInfo.h>
#import <ConnectionI.h>
#import <LocalObjectI.h>
#import <Util.h>

#include <IceIAP/ConnectionInfo.h>

using namespace std;

@implementation ICEIAPConnectionInfo (IceIAP)

+(void) load
{
    IceObjC::registerConnectionInfoClass([ICEIAPConnectionInfo class]);
}

-(id) initWithIAPConnectionInfo:(IceIAP::ConnectionInfo*)iapConnectionInfo
{
    self = [super initWithConnectionInfo:iapConnectionInfo];
    if(self)
    {
         self->name = toNSString(iapConnectionInfo->name);
         self->manufacturer = toNSString(iapConnectionInfo->manufacturer);
         self->modelNumber = toNSString(iapConnectionInfo->modelNumber);
         self->firmwareRevision = toNSString(iapConnectionInfo->firmwareRevision);
         self->hardwareRevision = toNSString(iapConnectionInfo->hardwareRevision);
         self->protocol = toNSString(iapConnectionInfo->protocol);
    }
    return self;
}

+(id) checkedConnectionInfoWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo
{
    IceIAP::ConnectionInfo* iapConnectionInfo = dynamic_cast<IceIAP::ConnectionInfo*>(connectionInfo);
    if(iapConnectionInfo)
    {
        return [[ICEIAPConnectionInfo alloc] initWithIAPConnectionInfo:iapConnectionInfo];
    }
    return nil;
}

@end
