//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class Ex
{
    public static void throwUOE(String expectedType, Ice.Object v)
    {
        //
        // If the object is an unknown sliced object, we didn't find an
        // value factory, in this case raise a NoValueFactoryException
        // instead.
        //
        if(v instanceof Ice.UnknownSlicedValue)
        {
            Ice.UnknownSlicedValue usv = (Ice.UnknownSlicedValue)v;
            throw new Ice.NoValueFactoryException("", usv.ice_id());
        }

        String type = v.ice_id();
        throw new Ice.UnexpectedObjectException("expected element of type `" + expectedType + "' but received `" +
                                                type + "'", type, expectedType);
    }

    public static void throwMemoryLimitException(int requested, int maximum)
    {
        throw new Ice.MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                           " bytes (see Ice.MessageSizeMax)");
    }

    //
    // A small utility to get the strack trace of the exception (which also includes toString()).
    //
    public static String toString(java.lang.Throwable ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        return sw.toString();
    }
}
