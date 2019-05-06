//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Net;

    sealed class UdpConnector : Connector
    {
        public Transceiver connect()
        {
            return new UdpTransceiver(_instance, _addr, _sourceAddr, _mcastInterface, _mcastTtl);
        }

        public short type()
        {
            return _instance.type();
        }

        //
        // Only for use by UdpEndpointI
        //
        internal UdpConnector(ProtocolInstance instance, EndPoint addr, EndPoint sourceAddr, string mcastInterface,
                              int mcastTtl, string connectionId)
        {
            _instance = instance;
            _addr = addr;
            _sourceAddr = sourceAddr;
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;
            _connectionId = connectionId;

            _hashCode = 5381;
            HashUtil.hashAdd(ref _hashCode, _addr);
            if(sourceAddr != null)
            {
                HashUtil.hashAdd(ref _hashCode, _sourceAddr);
            }
            HashUtil.hashAdd(ref _hashCode, _mcastInterface);
            HashUtil.hashAdd(ref _hashCode, _mcastTtl);
            HashUtil.hashAdd(ref _hashCode, _connectionId);
        }

        public override bool Equals(object obj)
        {
            if(!(obj is UdpConnector))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            UdpConnector p = (UdpConnector)obj;
            if(!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            if(!_mcastInterface.Equals(p._mcastInterface))
            {
                return false;
            }

            if(_mcastTtl != p._mcastTtl)
            {
                return false;
            }

            if(!Network.addressEquals(_sourceAddr, p._sourceAddr))
            {
                return false;
            }

            return _addr.Equals(p._addr);
        }

        public override string ToString()
        {
            return Network.addrToString(_addr);
        }

        public override int GetHashCode()
        {
            return _hashCode;
        }

        private ProtocolInstance _instance;
        private EndPoint _addr;
        private EndPoint _sourceAddr;
        private string _mcastInterface;
        private int _mcastTtl;
        private string _connectionId;
        private int _hashCode;
    }
}
