//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <InvokeTest.h>

@interface BlobjectI : ICEBlobject<ICEBlobject>
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current;
@end
