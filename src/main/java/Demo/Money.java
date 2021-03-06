//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.2
//
// <auto-generated>
//
// Generated from file `bank.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

package Demo;

public class Money implements java.lang.Cloneable,
                              java.io.Serializable
{
    public Currency currency;

    public int value;

    public Money()
    {
        this.currency = Currency.PLN;
    }

    public Money(Currency currency, int value)
    {
        this.currency = currency;
        this.value = value;
    }

    public boolean equals(java.lang.Object rhs)
    {
        if(this == rhs)
        {
            return true;
        }
        Money r = null;
        if(rhs instanceof Money)
        {
            r = (Money)rhs;
        }

        if(r != null)
        {
            if(this.currency != r.currency)
            {
                if(this.currency == null || r.currency == null || !this.currency.equals(r.currency))
                {
                    return false;
                }
            }
            if(this.value != r.value)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    public int hashCode()
    {
        int h_ = 5381;
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, "::Demo::Money");
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, currency);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, value);
        return h_;
    }

    public Money clone()
    {
        Money c = null;
        try
        {
            c = (Money)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    public void ice_writeMembers(com.zeroc.Ice.OutputStream ostr)
    {
        Currency.ice_write(ostr, this.currency);
        ostr.writeInt(this.value);
    }

    public void ice_readMembers(com.zeroc.Ice.InputStream istr)
    {
        this.currency = Currency.ice_read(istr);
        this.value = istr.readInt();
    }

    static public void ice_write(com.zeroc.Ice.OutputStream ostr, Money v)
    {
        if(v == null)
        {
            _nullMarshalValue.ice_writeMembers(ostr);
        }
        else
        {
            v.ice_writeMembers(ostr);
        }
    }

    static public Money ice_read(com.zeroc.Ice.InputStream istr)
    {
        Money v = new Money();
        v.ice_readMembers(istr);
        return v;
    }

    static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, java.util.Optional<Money> v)
    {
        if(v != null && v.isPresent())
        {
            ice_write(ostr, tag, v.get());
        }
    }

    static public void ice_write(com.zeroc.Ice.OutputStream ostr, int tag, Money v)
    {
        if(ostr.writeOptional(tag, com.zeroc.Ice.OptionalFormat.FSize))
        {
            int pos = ostr.startSize();
            ice_write(ostr, v);
            ostr.endSize(pos);
        }
    }

    static public java.util.Optional<Money> ice_read(com.zeroc.Ice.InputStream istr, int tag)
    {
        if(istr.readOptional(tag, com.zeroc.Ice.OptionalFormat.FSize))
        {
            istr.skip(4);
            return java.util.Optional.of(Money.ice_read(istr));
        }
        else
        {
            return java.util.Optional.empty();
        }
    }

    private static final Money _nullMarshalValue = new Money();

    /** @hidden */
    public static final long serialVersionUID = -1655887568992353868L;
}
