//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

import java.nio.channels.SelectionKey;

public class SocketOperation
{
    public static final int None = 0;
    public static final int Read = SelectionKey.OP_READ;
    public static final int Write = SelectionKey.OP_WRITE;
    public static final int Connect = SelectionKey.OP_CONNECT;
}
