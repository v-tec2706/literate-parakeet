//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/DispatchInterceptor.h>

#import <Request.h>

#import <Foundation/NSThread.h>
#import <Foundation/NSInvocation.h>

@implementation ICEDispatchInterceptor
-(void) iceDispatch:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICERequest* request = [ICERequest request:current is:is os:os];
    id<ICEDispatchInterceptor> dispatchInterceptor = (id<ICEDispatchInterceptor>)self;
    [dispatchInterceptor dispatch:request];
}
@end

@implementation ICEMainThreadDispatch

-(id)init:(ICEObject*)s
{
    servant = [s retain];
    return self;
}

+(id)mainThreadDispatch:(ICEObject*)s
{
    return [[[self alloc] init:s] autorelease];
}

-(void) dispatch:(id<ICERequest>)request
{
    SEL selector = @selector(ice_dispatch:);
    NSMethodSignature* sig = [[servant class] instanceMethodSignatureForSelector:selector];
    NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];
    [inv setTarget:servant];
    [inv setSelector:selector];
    [inv setArgument:&request atIndex:2];
    [inv performSelectorOnMainThread:@selector(invokeWithTarget:) withObject:servant waitUntilDone:YES];
}

-(void)dealloc
{
    [servant release];
    [super dealloc];
}
@end
