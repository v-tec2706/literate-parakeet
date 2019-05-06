//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["js:es6-module"]]

module Test
{

interface Hold
{
    void putOnHold(int seconds);
    void waitForHold();
    void setOneway(int value, int expected);
    int set(int value, int delay);
    void shutdown();
}

}
