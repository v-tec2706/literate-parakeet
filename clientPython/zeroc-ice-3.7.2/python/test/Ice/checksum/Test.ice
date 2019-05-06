//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/SliceChecksumDict.ice>

module Test
{

interface Checksum
{
    idempotent Ice::SliceChecksumDict getSliceChecksums();

    void shutdown();
}

}
