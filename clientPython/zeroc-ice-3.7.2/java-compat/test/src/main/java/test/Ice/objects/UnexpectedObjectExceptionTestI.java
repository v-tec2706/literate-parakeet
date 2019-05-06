//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.AlsoEmpty;

public final class UnexpectedObjectExceptionTestI extends Ice.Blobject
{
    @Override
    public boolean
    ice_invoke(byte[] inParams, Ice.ByteSeqHolder outParams, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.OutputStream out = new Ice.OutputStream(communicator);
        out.startEncapsulation(current.encoding, Ice.FormatType.DefaultFormat);
        out.writeValue(new AlsoEmpty());
        out.writePendingValues();
        out.endEncapsulation();
        outParams.value = out.finished();
        return true;
    }
}
