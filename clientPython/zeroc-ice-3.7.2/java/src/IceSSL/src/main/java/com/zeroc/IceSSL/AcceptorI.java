//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

final class AcceptorI implements com.zeroc.IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _delegate.fd();
    }

    @Override
    public void setReadyCallback(com.zeroc.IceInternal.ReadyCallback callback)
    {
        _delegate.setReadyCallback(callback);
    }

    @Override
    public void close()
    {
        _delegate.close();
    }

    @Override
    public com.zeroc.IceInternal.EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    @Override
    public com.zeroc.IceInternal.Transceiver accept()
    {
        //
        // The plug-in may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            com.zeroc.Ice.PluginInitializationException ex = new com.zeroc.Ice.PluginInitializationException();
            ex.reason = "IceSSL: plug-in is not initialized";
            throw ex;
        }

        return new TransceiverI(_instance, _delegate.accept(), _adapterName, true);
    }

    @Override
    public String protocol()
    {
        return _delegate.protocol();
    }

    @Override
    public String toString()
    {
        return _delegate.toString();
    }

    @Override
    public String toDetailedString()
    {
        return _delegate.toDetailedString();
    }

    AcceptorI(EndpointI endpoint, Instance instance, com.zeroc.IceInternal.Acceptor delegate, String adapterName)
    {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = delegate;
        _adapterName = adapterName;
    }

    private EndpointI _endpoint;
    private Instance _instance;
    private com.zeroc.IceInternal.Acceptor _delegate;
    private String _adapterName;
}
