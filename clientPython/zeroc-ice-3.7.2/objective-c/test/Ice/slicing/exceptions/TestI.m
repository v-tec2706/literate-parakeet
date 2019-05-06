//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <slicing/exceptions/TestI.h>
#import <TestCommon.h>
#import <objc/Ice.h>

@implementation TestSlicingExceptionsServerI
-(void) baseAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerBase base:@"Base.b"];
}

-(void) unknownDerivedAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownDerived unknownDerived:@"UnknownDerived.b" ud:@"UnknownDerived.ud"];
}

-(void) knownDerivedAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownDerived knownDerived:@"KnownDerived.b" kd:@"KnownDerived.kd"];
}

-(void) knownDerivedAsKnownDerived:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownDerived knownDerived:@"KnownDerived.b" kd:@"KnownDerived.kd"];
}

-(void) unknownIntermediateAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownIntermediate unknownIntermediate:@"UnknownIntermediate.b" ui:@"UnknownIntermediate.ui"];
}

-(void) knownIntermediateAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownIntermediate knownIntermediate:@"KnownIntermediate.b" ki:@"KnownIntermediate.ki"];
}

-(void) knownMostDerivedAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownMostDerived knownMostDerived:@"KnownMostDerived.b" ki:@"KnownMostDerived.ki" kmd:@"KnownMostDerived.kmd"];
}

-(void) knownIntermediateAsKnownIntermediate:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownIntermediate knownIntermediate:@"KnownIntermediate.b" ki:@"KnownIntermediate.ki"];
}

-(void) knownMostDerivedAsKnownIntermediate:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownMostDerived knownMostDerived:@"KnownMostDerived.b" ki:@"KnownMostDerived.ki" kmd:@"KnownMostDerived.kmd"];
}

-(void) knownMostDerivedAsKnownMostDerived:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownMostDerived knownMostDerived:@"KnownMostDerived.b" ki:@"KnownMostDerived.ki" kmd:@"KnownMostDerived.kmd"];
}

-(void) unknownMostDerived1AsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownMostDerived1 unknownMostDerived1:@"UnknownMostDerived1.b" ki:@"UnknownMostDerived1.ki" umd1:@"UnknownMostDerived1.umd1"];
}

-(void) unknownMostDerived1AsKnownIntermediate:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownMostDerived1 unknownMostDerived1:@"UnknownMostDerived1.b"
                                                                            ki:@"UnknownMostDerived1.ki"
                                                                          umd1:@"UnknownMostDerived1.umd1"];
}

-(void) unknownMostDerived2AsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownMostDerived2 unknownMostDerived2:@"UnknownMostDerived2.b"
                                                                            ui:@"UnknownMostDerived2.ui"
                                                                          umd2:@"UnknownMostDerived2.umd2"];
}

-(void) unknownMostDerived2AsBaseCompact:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerUnknownMostDerived2 unknownMostDerived2:@"UnknownMostDerived2.b"
                                                                            ui:@"UnknownMostDerived2.ui"
                                                                          umd2:@"UnknownMostDerived2.umd2"];
}

-(void) knownPreservedAsBase:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownPreservedDerived knownPreservedDerived:@"base"
                                                                                kp:@"preserved"
                                                                               kpd:@"derived"];
}

-(void) knownPreservedAsKnownPreserved:(ICECurrent*)__unused current
{
    @throw [TestSlicingExceptionsServerKnownPreservedDerived knownPreservedDerived:@"base"
                                                                                kp:@"preserved"
                                                                               kpd:@"derived"];
}

-(void) relayKnownPreservedAsBase:(TestSlicingExceptionsServerRelayPrx*)relay current:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerRelayPrx* p =
        [TestSlicingExceptionsServerRelayPrx uncheckedCast:[current.con createProxy:[relay ice_getIdentity]]];
    [p knownPreservedAsBase];
    test(NO);
}

-(void) relayKnownPreservedAsKnownPreserved:(TestSlicingExceptionsServerRelayPrx*)relay current:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerRelayPrx* p =
        [TestSlicingExceptionsServerRelayPrx uncheckedCast:[current.con createProxy:[relay ice_getIdentity]]];
    [p knownPreservedAsKnownPreserved];
    test(NO);
}

-(void) unknownPreservedAsBase:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerSPreserved2* ex = [TestSlicingExceptionsServerSPreserved2 sPreserved2];
    ex.b = @"base";
    ex.kp = @"preserved";
    ex.kpd = @"derived";
    ex.p1 = [TestSlicingExceptionsServerSPreservedClass sPreservedClass:@"bc" spc:@"spc"];
    ex.p2 = ex.p1;
    @throw ex;
}

-(void) unknownPreservedAsKnownPreserved:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerSPreserved2* ex = [TestSlicingExceptionsServerSPreserved2 sPreserved2];
    ex.b = @"base";
    ex.kp = @"preserved";
    ex.kpd = @"derived";
    ex.p1 = [TestSlicingExceptionsServerSPreservedClass sPreservedClass:@"bc" spc:@"spc"];
    ex.p2 = ex.p1;
    @throw ex;
}

-(void) relayUnknownPreservedAsBase:(TestSlicingExceptionsServerRelayPrx*)relay current:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerRelayPrx* p =
        [TestSlicingExceptionsServerRelayPrx uncheckedCast:[current.con createProxy:[relay ice_getIdentity]]];
    [p unknownPreservedAsBase];
    test(NO);
}

-(void) relayUnknownPreservedAsKnownPreserved:(TestSlicingExceptionsServerRelayPrx*)relay current:(ICECurrent*)__unused current
{
    TestSlicingExceptionsServerRelayPrx* p =
        [TestSlicingExceptionsServerRelayPrx uncheckedCast:[current.con createProxy:[relay ice_getIdentity]]];
    [p unknownPreservedAsKnownPreserved];
    test(NO);
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end
