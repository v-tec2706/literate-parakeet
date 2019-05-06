//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Holder class for Ice objects that are out- or inout-parameters.
 **/
public final class ObjectHolder extends ObjectHolderBase<Ice.Object>
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public ObjectHolder()
    {
    }

    /**
     * Instantiates the class with the passed Ice object.
     *
     * @param value The <code>Object</code> value for this holder.
     **/
    public ObjectHolder(Ice.Object value)
    {
        super(value);
    }

    /**
     * Sets the value of this holder to the passed instance.
     *
     * @param v The new value for this holder.
     **/
    public void valueReady(Ice.Object v)
    {
        value = v;
    }
}
