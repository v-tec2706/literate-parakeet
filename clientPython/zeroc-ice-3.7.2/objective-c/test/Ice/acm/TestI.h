//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <ACMTest.h>

#import <Foundation/Foundation.h>

@interface ACMRemoteCommunicatorI : TestACMRemoteCommunicator<TestACMRemoteCommunicator>
@end

@interface ACMRemoteObjectAdapterI : TestACMRemoteObjectAdapter<TestACMRemoteObjectAdapter>
{
    id<ICEObjectAdapter> _adapter;
    id<TestACMTestIntfPrx> _testIntf;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
@end

@interface ACMConnectionCallbackI : NSObject
{
    NSCondition* _cond;
    int _count;
}
-(void) waitForCount:(int)count;
@end

@interface TestACMTestIntfI : TestACMTestIntf<TestACMTestIntf>
{
    NSCondition* _cond;
    ACMConnectionCallbackI* _callback;
}
@end
