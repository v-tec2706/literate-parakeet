//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["java:package:test.Ice.hold"]]
module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    int set(int value, int delay);
    void setOneway(int value, int expected);
    void shutdown();
}

}
