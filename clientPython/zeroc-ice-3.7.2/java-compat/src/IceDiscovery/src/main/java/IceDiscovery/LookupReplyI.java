//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceDiscovery;

class LookupReplyI extends _LookupReplyDisp
{
    public LookupReplyI(LookupI lookup)
    {
        _lookup = lookup;
    }

    @Override
    public void
    foundObjectById(Ice.Identity id, Ice.ObjectPrx proxy, Ice.Current current)
    {
        _lookup.foundObject(id, current.id.name, proxy);
    }

    @Override
    public void
    foundAdapterById(String adapterId, Ice.ObjectPrx proxy, boolean isReplicaGroup, Ice.Current current)
    {
        _lookup.foundAdapter(adapterId, current.id.name, proxy, isReplicaGroup);
    }

    private final LookupI _lookup;
}
