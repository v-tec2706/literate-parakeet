//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceSSL.configuration;

import test.IceSSL.configuration.Test.Server;

class ServerI implements Server
{
    ServerI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void noCert(com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.ConnectionInfo info = (com.zeroc.IceSSL.ConnectionInfo)current.con.getInfo();
            test(info.certs == null);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void
    checkCert(String subjectDN, String issuerDN, com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.ConnectionInfo info = (com.zeroc.IceSSL.ConnectionInfo)current.con.getInfo();
            java.security.cert.X509Certificate cert = (java.security.cert.X509Certificate)info.certs[0];
            test(info.verified);
            test(info.certs.length == 2 &&
                 cert.getSubjectDN().toString().equals(subjectDN) &&
                 cert.getIssuerDN().toString().equals(issuerDN));
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void checkCipher(String cipher, com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.ConnectionInfo info = (com.zeroc.IceSSL.ConnectionInfo)current.con.getInfo();
            test(info.cipher.indexOf(cipher) >= 0);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    public void destroy()
    {
        _communicator.destroy();
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private com.zeroc.Ice.Communicator _communicator;
}
