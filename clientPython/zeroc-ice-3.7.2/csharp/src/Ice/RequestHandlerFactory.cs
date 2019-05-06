//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace IceInternal
{
    public class RequestHandlerFactory
    {
        internal RequestHandlerFactory(Instance instance)
        {
            _instance = instance;
        }

        public RequestHandler
        getRequestHandler(RoutableReference rf, Ice.ObjectPrxHelperBase proxy)
        {
            if(rf.getCollocationOptimized())
            {
                Ice.ObjectAdapter adapter = _instance.objectAdapterFactory().findObjectAdapter(proxy);
                if(adapter != null)
                {
                    return proxy.iceSetRequestHandler(new CollocatedRequestHandler(rf, adapter));
                }
            }

            bool connect = false;
            ConnectRequestHandler handler;
            if(rf.getCacheConnection())
            {
                lock(this)
                {
                    if(!_handlers.TryGetValue(rf, out handler))
                    {
                        handler = new ConnectRequestHandler(rf, proxy);
                        _handlers.Add(rf, handler);
                        connect = true;
                    }
                }
            }
            else
            {
                handler = new ConnectRequestHandler(rf, proxy);
                connect = true;
            }

            if(connect)
            {
                rf.getConnection(handler);
            }
            return proxy.iceSetRequestHandler(handler.connect(proxy));
        }

        internal void
        removeRequestHandler(Reference rf, RequestHandler handler)
        {
            if(rf.getCacheConnection())
            {
                lock(this)
                {
                    ConnectRequestHandler h;
                    if(_handlers.TryGetValue(rf, out h) && h == handler)
                    {
                        _handlers.Remove(rf);
                    }
                }
            }
        }

        readonly Instance _instance;
        readonly Dictionary<Reference, ConnectRequestHandler> _handlers =
            new Dictionary<Reference, ConnectRequestHandler>();
    }
}
