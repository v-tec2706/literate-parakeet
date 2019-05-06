//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.invoke;

public class ServantLocatorI implements com.zeroc.Ice.ServantLocator
{
    public ServantLocatorI(boolean async)
    {
        _blobject = new com.zeroc.Ice.ServantLocator.LocateResult();
        if(async)
        {
            _blobject.returnValue = new BlobjectAsyncI();
        }
        else
        {
            _blobject.returnValue = new BlobjectI();
        }
    }

    @Override
    public com.zeroc.Ice.ServantLocator.LocateResult locate(com.zeroc.Ice.Current current)
    {
        return _blobject;
    }

    @Override
    public void finished(com.zeroc.Ice.Current current, com.zeroc.Ice.Object servant, java.lang.Object cookie)
    {
    }

    @Override
    public void deactivate(String category)
    {
    }

    private com.zeroc.Ice.ServantLocator.LocateResult _blobject;
}
