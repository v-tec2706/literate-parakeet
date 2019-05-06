//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SHA1_H
#define ICE_SHA1_H

#include <Ice/Config.h>
#include <Ice/UniquePtr.h>

#include <vector>

namespace IceInternal
{

ICE_API void
sha1(const unsigned char*, std::size_t, std::vector<unsigned char>&);

#ifndef ICE_OS_UWP
class ICE_API SHA1
{
public:

    SHA1();
    ~SHA1();

    void update(const unsigned char*, std::size_t);
    void finalize(std::vector<unsigned char>&);

private:

    // noncopyable
    SHA1(const SHA1&);
    SHA1 operator=(const SHA1&);

    class Hasher;
    UniquePtr<Hasher> _hasher;
};
#endif

}
#endif
