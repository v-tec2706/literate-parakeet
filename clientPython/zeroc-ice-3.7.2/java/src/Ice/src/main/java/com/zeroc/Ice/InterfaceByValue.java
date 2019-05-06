//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Base class for interoperating with existing applications that pass interfaces by value.
 **/
public class InterfaceByValue extends Value
{
    /**
     * The constructor accepts the Slice type ID of the interface being passed by value.
     *
     * @param id The Slice type ID of the interface.
     **/
    public InterfaceByValue(String id)
    {
        _id = id;
    }

    /**
     * Returns the Slice type ID of the interface being passed by value.
     *
     * @return The Slice type ID.
     **/
    public String ice_id()
    {
        return _id;
    }

    /** @hidden */
    @Override
    protected void _iceWriteImpl(OutputStream ostr)
    {
        ostr.startSlice(ice_id(), -1, true);
        ostr.endSlice();
    }

    /** @hidden */
    @Override
    protected void _iceReadImpl(InputStream istr)
    {
        istr.startSlice();
        istr.endSlice();
    }

    /** @hidden */
    public static final long serialVersionUID = 0L;

    private String _id;
}
