//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["objc:prefix:TestRetry"]
module Test
{

interface Retry
{
    void op(bool kill);

    idempotent int opIdempotent(int c);
    void opNotIdempotent();
    void opSystemException();

    idempotent void shutdown();
}

}
