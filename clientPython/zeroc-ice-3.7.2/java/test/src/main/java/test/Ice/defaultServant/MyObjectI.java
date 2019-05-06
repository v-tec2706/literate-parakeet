//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.defaultServant;

import test.Ice.defaultServant.Test.*;

public final class MyObjectI implements MyObject
{
    @Override
    public void ice_ping(com.zeroc.Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new com.zeroc.Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new com.zeroc.Ice.FacetNotExistException();
        }
    }

    @Override
    public String getName(com.zeroc.Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new com.zeroc.Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new com.zeroc.Ice.FacetNotExistException();
        }

        return name;
    }
}
