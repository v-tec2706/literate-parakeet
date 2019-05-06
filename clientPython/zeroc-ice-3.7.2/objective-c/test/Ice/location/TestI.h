//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <LocationTest.h>
#import <location/ServerLocator.h>

@interface ServerManagerI : TestLocationServerManager<TestLocationServerManager>
{
    NSMutableArray* communicators_;
    ServerLocatorRegistry* registry_;
    ICEInitializationData* initData_;
    int nextPort_;
}
-(id)init:(ServerLocatorRegistry*)registry initData:(ICEInitializationData*)d;
-(void)terminate;
@end

@interface HelloI : TestLocationHello<TestLocationHello>
@end

@interface TestLocationI : TestLocationTestIntf<TestLocationTestIntf>
{
    id<ICEObjectAdapter> adapter1_;
    id<ICEObjectAdapter> adapter2_;
    ServerLocatorRegistry* registry_;
}
-(id) init:(id<ICEObjectAdapter>)adapter1 adapter2:(id<ICEObjectAdapter>)adpt2 registry:(ServerLocatorRegistry*)r;
@end
