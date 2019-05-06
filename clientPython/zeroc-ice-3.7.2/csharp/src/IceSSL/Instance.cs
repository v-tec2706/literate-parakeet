//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;

    internal class Instance : IceInternal.ProtocolInstance
    {
        internal Instance(SSLEngine engine, short type, string protocol) :
            base(engine.communicator(), type, protocol, true)
        {
            _engine = engine;
        }

        internal SSLEngine engine()
        {
            return _engine;
        }

        internal int securityTraceLevel()
        {
            return _engine.securityTraceLevel();
        }

        internal string securityTraceCategory()
        {
            return _engine.securityTraceCategory();
        }

        internal bool initialized()
        {
            return _engine.initialized();
        }

        internal X509Certificate2Collection certs()
        {
            return _engine.certs();
        }

        internal SslProtocols protocols()
        {
            return _engine.protocols();
        }

        internal int checkCRL()
        {
            return _engine.checkCRL();
        }

        internal void traceStream(System.Net.Security.SslStream stream, string connInfo)
        {
            _engine.traceStream(stream, connInfo);
        }

        internal void verifyPeer(string address, IceSSL.ConnectionInfo info, string desc)
        {
            _engine.verifyPeer(address, info, desc);
        }

        private SSLEngine _engine;
    }
}
