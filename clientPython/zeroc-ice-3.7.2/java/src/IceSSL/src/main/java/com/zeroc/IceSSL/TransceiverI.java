//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

import java.nio.*;
import javax.net.ssl.*;
import javax.net.ssl.SSLEngineResult.*;
import com.zeroc.IceInternal.SocketOperation;

final class TransceiverI implements com.zeroc.IceInternal.Transceiver
{
    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        return _delegate.fd();
    }

    @Override
    public void setReadyCallback(com.zeroc.IceInternal.ReadyCallback callback)
    {
        _readyCallback = callback;
        _delegate.setReadyCallback(callback);
    }

    @Override
    public int initialize(com.zeroc.IceInternal.Buffer readBuffer, com.zeroc.IceInternal.Buffer writeBuffer)
    {
        if(!_isConnected)
        {
            int status = _delegate.initialize(readBuffer, writeBuffer);
            if(status != SocketOperation.None)
            {
                return status;
            }
            _isConnected = true;

            com.zeroc.Ice.IPConnectionInfo ipInfo = null;
            for(com.zeroc.Ice.ConnectionInfo p = _delegate.getInfo(); p != null; p = p.underlying)
            {
                if(p instanceof com.zeroc.Ice.IPConnectionInfo)
                {
                    ipInfo = (com.zeroc.Ice.IPConnectionInfo)p;
                }
            }
            final String host = _incoming ? (ipInfo != null ? ipInfo.remoteAddress : "") : _host;
            final int port = ipInfo != null ? ipInfo.remotePort : -1;
            _engine = _instance.createSSLEngine(_incoming, host, port);
            _appInput = ByteBuffer.allocate(_engine.getSession().getApplicationBufferSize() * 2);

            // Require BIG_ENDIAN byte buffers. This is needed for Android >= 8.0 which can read
            // the SSL messages directly with these buffers.
            int bufSize = _engine.getSession().getPacketBufferSize() * 2;
            _netInput = new com.zeroc.IceInternal.Buffer(ByteBuffer.allocateDirect(bufSize * 2),
                                                         java.nio.ByteOrder.BIG_ENDIAN);
            _netOutput = new com.zeroc.IceInternal.Buffer(ByteBuffer.allocateDirect(bufSize * 2),
                                                          java.nio.ByteOrder.BIG_ENDIAN);
        }

        int status = handshakeNonBlocking();
        if(status != SocketOperation.None)
        {
            return status;
        }

        assert(_engine != null);

        SSLSession session = _engine.getSession();
        _cipher = session.getCipherSuite();
        try
        {
            java.security.cert.Certificate[] pcerts = session.getPeerCertificates();
            java.security.cert.Certificate[] vcerts = _instance.engine().getVerifiedCertificateChain(pcerts);
            _verified = vcerts != null;
            _certs = _verified ? vcerts : pcerts;
        }
        catch(javax.net.ssl.SSLPeerUnverifiedException ex)
        {
            // No peer certificates.
        }

        //
        // Additional verification.
        //
        _instance.verifyPeer(_host, (com.zeroc.IceSSL.ConnectionInfo)getInfo(), _delegate.toString());

        if(_instance.securityTraceLevel() >= 1)
        {
            _instance.traceConnection(_delegate.toString(), _engine, _incoming);
        }
        return SocketOperation.None;
    }

    @Override
    public int closing(boolean initiator, com.zeroc.Ice.LocalException ex)
    {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close()
    {
        if(_engine != null)
        {
            try
            {
                //
                // Send the close_notify message.
                //
                _engine.closeOutbound();
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((java.nio.Buffer)_netOutput.b).clear();
                while(!_engine.isOutboundDone())
                {
                    _engine.wrap(_emptyBuffer, _netOutput.b);
                    try
                    {
                        //
                        // Note: we can't block to send the close_notify message. In some cases, the
                        // close_notify message might therefore not be received by the peer. This is
                        // not a big issue since the Ice protocol isn't subject to truncation attacks.
                        //
                        flushNonBlocking();
                    }
                    catch(com.zeroc.Ice.LocalException ex)
                    {
                        // Ignore.
                    }
                }
            }
            catch(SSLException ex)
            {
                //
                // We can't throw in close.
                //
                // Ice.SecurityException se = new Ice.SecurityException(
                //     "IceSSL: SSL failure while shutting down socket", ex);
                //
            }

            try
            {
                _engine.closeInbound();
            }
            catch(SSLException ex)
            {
                //
                // SSLEngine always raises an exception with this message:
                //
                // Inbound closed before receiving peer's close_notify: possible truncation attack?
                //
                // We would probably need to wait for a response in shutdown() to avoid this.
                // For now, we'll ignore this exception.
                //
                //_instance.logger().error("IceSSL: error during close\n" + ex.getMessage());
            }
        }

        _delegate.close();
    }

    @Override
    public com.zeroc.IceInternal.EndpointI bind()
    {
        assert(false);
        return null;
    }

    @Override
    public int write(com.zeroc.IceInternal.Buffer buf)
    {
        if(!_isConnected)
        {
            return _delegate.write(buf);
        }

        int status = writeNonBlocking(buf.b);
        assert(status == SocketOperation.None || status == SocketOperation.Write);
        return status;
    }

    @Override
    public int read(com.zeroc.IceInternal.Buffer buf)
    {
        if(!_isConnected)
        {
            return _delegate.read(buf);
        }

        _readyCallback.ready(SocketOperation.Read, false);

        //
        // Try to satisfy the request from data we've already decrypted.
        //
        fill(buf.b);

        //
        // Read and decrypt more data if necessary. Note that we might read
        // more data from the socket than is actually necessary to fill the
        // caller's stream.
        //
        try
        {
            while(buf.b.hasRemaining())
            {
                _netInput.flip();
                SSLEngineResult result = _engine.unwrap(_netInput.b, _appInput);
                _netInput.b.compact();

                Status status = result.getStatus();
                assert status != Status.BUFFER_OVERFLOW;

                if(status == Status.CLOSED)
                {
                    throw new com.zeroc.Ice.ConnectionLostException();
                }
                // Android API 21 SSLEngine doesn't report underflow, so look at the absence of
                // network data and application data to signal a network read.
                else if(status == Status.BUFFER_UNDERFLOW || (_appInput.position() == 0 && _netInput.b.position() == 0))
                {
                    int s = _delegate.read(_netInput);
                    if(s != SocketOperation.None && _netInput.b.position() == 0)
                    {
                        return s;
                    }
                    continue;
                }

                fill(buf.b);
            }

            // If there is no more application data, do one further unwrap to ensure
            // that the SSLEngine has no buffered data (Android R21 and greater only).
            if(_appInput.position() == 0)
            {
                _netInput.flip();
                _engine.unwrap(_netInput.b, _appInput);
                _netInput.b.compact();

                // Don't check the status here since we may have already filled
                // the buffer with a complete request which must be processed.
            }
        }
        catch(SSLException ex)
        {
            throw new com.zeroc.Ice.SecurityException("IceSSL: error during read", ex);
        }

        //
        // Indicate whether more data is available.
        //
        if(_netInput.b.position() > 0 || _appInput.position() > 0)
        {
            _readyCallback.ready(SocketOperation.Read, true);
        }

        return SocketOperation.None;
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
        return toString();
    }

    @Override
    public com.zeroc.Ice.ConnectionInfo getInfo()
    {
        ConnectionInfo info = new ConnectionInfo();
        info.underlying = _delegate.getInfo();
        info.incoming = _incoming;
        info.adapterName = _adapterName;
        info.cipher = _cipher;
        info.certs = _certs;
        info.verified = _verified;
        return info;
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize)
    {
        _delegate.setBufferSize(rcvSize, sndSize);
    }

    @Override
    public void checkSendSize(com.zeroc.IceInternal.Buffer buf)
    {
        _delegate.checkSendSize(buf);
    }

    TransceiverI(Instance instance, com.zeroc.IceInternal.Transceiver delegate, String hostOrAdapterName,
                 boolean incoming)
    {
        _instance = instance;
        _delegate = delegate;
        _incoming = incoming;
        if(_incoming)
        {
            _adapterName = hostOrAdapterName;
        }
        else
        {
            _host = hostOrAdapterName;
        }
    }

    private int handshakeNonBlocking()
    {
        try
        {
            HandshakeStatus status = _engine.getHandshakeStatus();
            while(!_engine.isOutboundDone() && !_engine.isInboundDone())
            {
                SSLEngineResult result = null;
                switch(status)
                {
                case FINISHED:
                case NOT_HANDSHAKING:
                {
                    return SocketOperation.None;
                }
                case NEED_TASK:
                {
                    Runnable task;
                    while((task = _engine.getDelegatedTask()) != null)
                    {
                        task.run();
                    }
                    status = _engine.getHandshakeStatus();
                    break;
                }
                case NEED_UNWRAP:
                {
                    if(_netInput.b.position() == 0)
                    {
                        int s = _delegate.read(_netInput);
                        if(s != SocketOperation.None && _netInput.b.position() == 0)
                        {
                            return s;
                        }
                    }

                    //
                    // The engine needs more data. We might already have enough data in
                    // the _netInput buffer to satisfy the engine. If not, the engine
                    // responds with BUFFER_UNDERFLOW and we'll read from the socket.
                    //
                    _netInput.flip();
                    result = _engine.unwrap(_netInput.b, _appInput);
                    _netInput.b.compact();
                    //
                    // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                    //
                    status = result.getHandshakeStatus();
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                    {
                        assert(false);
                        break;
                    }
                    case BUFFER_UNDERFLOW:
                    {
                        assert(status == javax.net.ssl.SSLEngineResult.HandshakeStatus.NEED_UNWRAP);
                        int position = _netInput.b.position();
                        int s = _delegate.read(_netInput);
                        if(s != SocketOperation.None && _netInput.b.position() == position)
                        {
                            return s;
                        }
                        break;
                    }
                    case CLOSED:
                    {
                        throw new com.zeroc.Ice.ConnectionLostException();
                    }
                    case OK:
                    {
                        break;
                    }
                    }
                    break;
                }
                case NEED_WRAP:
                {
                    //
                    // The engine needs to send a message.
                    //
                    result = _engine.wrap(_emptyBuffer, _netOutput.b);
                    if(result.bytesProduced() > 0)
                    {
                        int s = flushNonBlocking();
                        if(s != SocketOperation.None)
                        {
                            return s;
                        }
                    }

                    //
                    // FINISHED is only returned from wrap or unwrap, not from engine.getHandshakeResult().
                    //
                    status = result.getHandshakeStatus();
                    break;
                }
                }

                if(result != null)
                {
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                        assert(false);
                        break;
                    case BUFFER_UNDERFLOW:
                        // Need to read again.
                        assert(status == HandshakeStatus.NEED_UNWRAP);
                        break;
                    case CLOSED:
                        throw new com.zeroc.Ice.ConnectionLostException();
                    case OK:
                        break;
                    }
                }
            }
        }
        catch(SSLException ex)
        {
            throw new com.zeroc.Ice.SecurityException("IceSSL: handshake error", ex);
        }
        return SocketOperation.None;
    }

    private int writeNonBlocking(ByteBuffer buf)
    {
        //
        // This method has two purposes: encrypt the application's message buffer into our
        // _netOutput buffer, and write the contents of _netOutput to the socket without
        // blocking.
        //
        try
        {
            while(buf.hasRemaining() || _netOutput.b.position() > 0)
            {
                if(buf.hasRemaining())
                {
                    //
                    // Encrypt the buffer.
                    //
                    SSLEngineResult result = _engine.wrap(buf, _netOutput.b);
                    switch(result.getStatus())
                    {
                    case BUFFER_OVERFLOW:
                        //
                        // Need to make room in _netOutput.b.
                        //
                        break;
                    case BUFFER_UNDERFLOW:
                        assert(false);
                        break;
                    case CLOSED:
                        throw new com.zeroc.Ice.ConnectionLostException();
                    case OK:
                        break;
                    }
                }

                //
                // Write the encrypted data to the socket. We continue writing until we've written
                // all of _netOutput, or until flushNonBlocking indicates that it cannot write
                // (i.e., by returning SocketOperation.Write).
                //
                if(_netOutput.b.position() > 0)
                {
                    int s = flushNonBlocking();
                    if(s != SocketOperation.None)
                    {
                        return s;
                    }
                }
            }
        }
        catch(SSLException ex)
        {
            throw new com.zeroc.Ice.SecurityException("IceSSL: error while encoding message", ex);
        }

        assert(_netOutput.b.position() == 0);
        return SocketOperation.None;
    }

    private int flushNonBlocking()
    {
        _netOutput.flip();

        try
        {
            int s = _delegate.write(_netOutput);
            if(s != SocketOperation.None)
            {
                _netOutput.b.compact();
                return s;
            }
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            throw new com.zeroc.Ice.ConnectionLostException(ex);
        }
        // Cast to java.nio.Buffer to avoid incompatible covariant
        // return type used in Java 9 java.nio.ByteBuffer
        ((java.nio.Buffer)_netOutput.b).clear();
        return SocketOperation.None;
    }

    private void fill(ByteBuffer buf)
    {
        ((java.nio.Buffer)_appInput).flip();
        if(_appInput.hasRemaining())
        {
            int bytesAvailable = _appInput.remaining();
            int bytesNeeded = buf.remaining();
            if(bytesAvailable > bytesNeeded)
            {
                bytesAvailable = bytesNeeded;
            }
            if(buf.hasArray())
            {
                //
                // Copy directly into the destination buffer's backing array.
                //
                byte[] arr = buf.array();
                _appInput.get(arr, buf.arrayOffset() + buf.position(), bytesAvailable);
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((Buffer)buf).position(buf.position() + bytesAvailable);
            }
            else if(_appInput.hasArray())
            {
                //
                // Copy directly from the source buffer's backing array.
                //
                byte[] arr = _appInput.array();
                buf.put(arr, _appInput.arrayOffset() + _appInput.position(), bytesAvailable);
                // Cast to java.nio.Buffer to avoid incompatible covariant
                // return type used in Java 9 java.nio.ByteBuffer
                ((Buffer)_appInput).position(_appInput.position() + bytesAvailable);
            }
            else
            {
                //
                // Copy using a temporary array.
                //
                byte[] arr = new byte[bytesAvailable];
                _appInput.get(arr);
                buf.put(arr);
            }
        }
        _appInput.compact();
    }

    private Instance _instance;
    private com.zeroc.IceInternal.Transceiver _delegate;
    private javax.net.ssl.SSLEngine _engine;
    private String _host = "";
    private String _adapterName = "";
    private boolean _incoming;
    private com.zeroc.IceInternal.ReadyCallback _readyCallback;
    private boolean _isConnected = false;

    private ByteBuffer _appInput; // Holds clear-text data to be read by the application.
    private com.zeroc.IceInternal.Buffer _netInput; // Holds encrypted data read from the socket.
    private com.zeroc.IceInternal.Buffer _netOutput; // Holds encrypted data to be written to the socket.
    private static ByteBuffer _emptyBuffer = ByteBuffer.allocate(0); // Used during handshaking.

    private String _cipher;
    private java.security.cert.Certificate[] _certs;
    private boolean _verified;
}
