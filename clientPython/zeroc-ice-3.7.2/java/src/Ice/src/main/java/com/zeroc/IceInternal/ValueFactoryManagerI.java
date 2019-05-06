//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import com.zeroc.Ice.ValueFactory;

final class ValueFactoryManagerI implements com.zeroc.Ice.ValueFactoryManager
{
    public synchronized void add(ValueFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
        if(o != null)
        {
            com.zeroc.Ice.AlreadyRegisteredException ex = new com.zeroc.Ice.AlreadyRegisteredException();
            ex.id = id;
            ex.kindOfObject = "value factory";
            throw ex;
        }
        _factoryMap.put(id, factory);
    }

    public synchronized ValueFactory find(String id)
    {
        return _factoryMap.get(id);
    }

    private java.util.Map<String, ValueFactory> _factoryMap = new java.util.HashMap<>();
}
