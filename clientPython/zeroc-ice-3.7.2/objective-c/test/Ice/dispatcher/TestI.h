//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <DispatcherTest.h>

@interface TestDispatcherTestIntfI : TestDispatcherTestIntf
-(void) op:(ICECurrent *)current;
-(void) opWithPayload:(ICEMutableByteSeq*)seq current:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end

@interface TestDispatcherTestIntfControllerI : TestDispatcherTestIntfController
{
    id<ICEObjectAdapter> _adapter;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
-(void) holdAdapter:(ICECurrent*)current;
-(void) resumeAdapter:(ICECurrent*)current;
@end
