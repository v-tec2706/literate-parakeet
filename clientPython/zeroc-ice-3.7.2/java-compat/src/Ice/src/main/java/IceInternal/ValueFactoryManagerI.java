//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

final class ValueFactoryManagerI implements Ice.ValueFactoryManager
{
    public synchronized void add(Ice.ValueFactory factory, String id)
    {
        Object o = _factoryMap.get(id);
        if(o != null)
        {
            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
            ex.id = id;
            ex.kindOfObject = "value factory";
            throw ex;
        }
        _factoryMap.put(id, factory);
    }

    public synchronized Ice.ValueFactory find(String id)
    {
        return _factoryMap.get(id);
    }

    private java.util.Map<String, Ice.ValueFactory> _factoryMap = new java.util.HashMap<String, Ice.ValueFactory>();
}
