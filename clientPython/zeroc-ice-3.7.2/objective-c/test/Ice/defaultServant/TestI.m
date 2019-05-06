//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <defaultServant/TestI.h>

@implementation TestDefaultServantMyObjectI

-(void) ice_ping:(ICECurrent*)current
{
    NSString* name = current.id_.name;

    if([name isEqualToString:@"ObjectNotExist"])
    {
        @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
    }

    if([name isEqualToString:@"FacetNotExist"])
    {
        @throw [ICEFacetNotExistException facetNotExistException:__FILE__ line:__LINE__];
    }
}

-(NSString*) getName:(ICECurrent*)current
{
    NSString* name = current.id_.name;

    if([name isEqualToString:@"ObjectNotExist"])
    {
        @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
    }

    if([name isEqualToString:@"FacetNotExist"])
    {
        @throw [ICEFacetNotExistException facetNotExistException:__FILE__ line:__LINE__];
    }

    return name;
}
@end
