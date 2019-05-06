//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <LocationTest.h>

static int
run(id<ICECommunicator> communicator)
{
    void locationAllTests(id<ICECommunicator>, NSString*);
    locationAllTests(communicator, @"ServerManager:default -p 12010");
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main locationClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultClientProperties(&argc, argv);
            [initData.properties setProperty:@"Ice.Default.Locator" value:@"locator:default -p 12010"];
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestLocation", @"::Test",
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
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
