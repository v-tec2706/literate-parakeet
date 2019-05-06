//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>

#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>

@protocol ICECommunicator;

@interface ICEInputStream : NSObject<ICEInputStream>
{
    Ice::InputStream* is_;
    Ice::InputStream stream_;
    std::vector<IceUtil::Handle<IceUtil::Shared> >* objectReaders_;
    NSDictionary* prefixTable_;
    NSData* data_;
}
+(Ice::Object*)createObjectReader:(ICEObject*)obj;
-(id)initWithCxxCommunicator:(Ice::Communicator*)com data:(const std::pair<const Byte*, const Byte*>&)data;
-(id)initWithCommunicator:(id<ICECommunicator>)com data:(NSData*)data encoding:(ICEEncodingVersion*)e;
-(Ice::InputStream*) is;
@end

@interface ICEOutputStream : NSObject<ICEOutputStream>
{
    Ice::OutputStream* os_;
    Ice::OutputStream stream_;
    std::map<ICEObject*, Ice::ObjectPtr>* objectWriters_;
}
-(id)initWithCxxCommunicator:(Ice::Communicator*)communicator;
-(id)initWithCxxStream:(Ice::OutputStream*)stream;
-(id)initWithCommunicator:(id<ICECommunicator>)com encoding:(ICEEncodingVersion*)e;
-(Ice::OutputStream*) os;
@end
