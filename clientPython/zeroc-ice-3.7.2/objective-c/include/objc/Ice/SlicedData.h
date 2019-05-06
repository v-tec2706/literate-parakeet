//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Config.h>

#import <objc/Ice/Object.h>

//
// SlicedData holds the slices of unknown types.
//
ICE_API @protocol ICESlicedData<NSObject>
//
// Clear the slices to break potential cyclic references.
//
-(void) clear;
@end

//
// Unknown sliced object holds instance of unknown type.
//
ICE_API @interface ICEUnknownSlicedValue : ICEObject
{
@private
    NSString* unknownTypeId_;
    id<ICESlicedData> slicedData_;
}
@end
