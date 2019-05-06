//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

class EventHandlerOpPair
{
    EventHandlerOpPair(EventHandler handler, int op)
    {
        this.handler = handler;
        this.op = op;
    }

    EventHandler handler;
    int op;
}
