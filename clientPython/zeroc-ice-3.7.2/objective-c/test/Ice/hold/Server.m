//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <hold/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter1.Endpoints" value:@"default -p 12010 -t 10000:udp"];
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.Size" value:@"5"];
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.SizeMax" value:@"5"];
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.SizeWarn" value:@"0"];
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.Serialize" value:@"0"];
    id<ICEObjectAdapter> adapter1 = [communicator createObjectAdapter:@"TestAdapter1"];

    [[communicator getProperties] setProperty:@"TestAdapter2.Endpoints" value:@"default -p 12011 -t 10000:udp"];
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.Size" value:@"5"];
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.SizeMax" value:@"5"];
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.SizeWarn" value:@"0"];
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.Serialize" value:@"1"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"TestAdapter2"];

    [adapter1 add:[HoldI hold] identity:[ICEUtil stringToIdentity:@"hold"]];
    [adapter2 add:[HoldI hold] identity:[ICEUtil stringToIdentity:@"hold"]];

    [adapter1 activate];
    [adapter2 activate];

    serverReady(communicator);

    [communicator waitForShutdown];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main holdServer
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
    ICEregisterIceUDP(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    @autoreleasepool
    {
        int status;
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestHold", @"::Test",
                                      nil];
#endif

            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
        return status;
    }
}
