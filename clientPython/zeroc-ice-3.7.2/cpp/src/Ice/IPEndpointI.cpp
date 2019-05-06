//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/IPEndpointI.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/PropertiesI.h>
#include <Ice/LoggerUtil.h>
#include <Ice/HashUtil.h>
#include <Ice/NetworkProxy.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* hashMutex = 0;

class Init
{
public:

    Init()
    {
        hashMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete hashMutex;
        hashMutex = 0;
    }
};

Init init;

}

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(IPEndpointI* p) { return p; }
#endif
IceUtil::Shared* IceInternal::upCast(EndpointHostResolver* p) { return p; }

IceInternal::IPEndpointInfoI::IPEndpointInfoI(const EndpointIPtr& endpoint) : _endpoint(endpoint)
{
}

IceInternal::IPEndpointInfoI::~IPEndpointInfoI()
{
}

Ice::Short
IceInternal::IPEndpointInfoI::type() const ICE_NOEXCEPT
{
    return _endpoint->type();
}

bool
IceInternal::IPEndpointInfoI::datagram() const ICE_NOEXCEPT
{
    return _endpoint->datagram();
}

bool
IceInternal::IPEndpointInfoI::secure() const ICE_NOEXCEPT
{
    return _endpoint->secure();
}

Ice::EndpointInfoPtr
IceInternal::IPEndpointI::getInfo() const ICE_NOEXCEPT
{
    Ice::IPEndpointInfoPtr info = ICE_MAKE_SHARED(IPEndpointInfoI, ICE_SHARED_FROM_CONST_THIS(IPEndpointI));
    fillEndpointInfo(info.get());
    return info;
}

Ice::Short
IceInternal::IPEndpointI::type() const
{
    return _instance->type();
}

const string&
IceInternal::IPEndpointI::protocol() const
{
    return _instance->protocol();
}

bool
IceInternal::IPEndpointI::secure() const
{
    return _instance->secure();
}

void
IceInternal::IPEndpointI::streamWriteImpl(OutputStream* s) const
{
    s->write(_host, false);
    s->write(_port);
}

const string&
IceInternal::IPEndpointI::connectionId() const
{
    return _connectionId;
}

EndpointIPtr
IceInternal::IPEndpointI::connectionId(const string& connectionId) const
{
    if(connectionId == _connectionId)
    {
        return ICE_SHARED_FROM_CONST_THIS(IPEndpointI);
    }
    else
    {
        return createEndpoint(_host, _port, connectionId);
    }
}

void
IceInternal::IPEndpointI::connectors_async(Ice::EndpointSelectionType selType, const EndpointI_connectorsPtr& cb) const
{
    _instance->resolve(_host, _port, selType, ICE_SHARED_FROM_CONST_THIS(IPEndpointI), cb);
}

vector<EndpointIPtr>
IceInternal::IPEndpointI::expandIfWildcard() const
{
    vector<EndpointIPtr> endps;
    vector<string> hosts = getHostsForEndpointExpand(_host, _instance->protocolSupport(), false);
    if(hosts.empty())
    {
        endps.push_back(ICE_SHARED_FROM_CONST_THIS(IPEndpointI));
    }
    else
    {
        for(vector<string>::const_iterator p = hosts.begin(); p != hosts.end(); ++p)
        {
            endps.push_back(createEndpoint(*p, _port, _connectionId));
        }
    }
    return endps;
}

vector<EndpointIPtr>
IceInternal::IPEndpointI::expandHost(EndpointIPtr& publish) const
{
    //
    // If this endpoint has an empty host (wildcard address), don't expand, just return
    // this endpoint.
    //
    if(_host.empty())
    {
        vector<EndpointIPtr> endpoints;
        endpoints.push_back(ICE_SHARED_FROM_CONST_THIS(IPEndpointI));
        return endpoints;
    }

    //
    // If using a fixed port, this endpoint can be used as the published endpoint to
    // access the returned endpoints. Otherwise, we'll publish each individual expanded
    // endpoint.
    //
    if(_port > 0)
    {
        publish = ICE_SHARED_FROM_CONST_THIS(IPEndpointI);
    }

    vector<Address> addrs = getAddresses(_host,
                                         _port,
                                         _instance->protocolSupport(),
                                         Ice::ICE_ENUM(EndpointSelectionType, Ordered),
                                         _instance->preferIPv6(),
                                         true);

    vector<EndpointIPtr> endpoints;
    if(addrs.size() == 1)
    {
        endpoints.push_back(ICE_SHARED_FROM_CONST_THIS(IPEndpointI));
    }
    else
    {
        for(vector<Address>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
        {
            string host;
            int port;
            addrToAddressAndPort(*p, host, port);
            endpoints.push_back(createEndpoint(host, port, _connectionId));
        }
    }
    return endpoints;
}

bool
IceInternal::IPEndpointI::equivalent(const EndpointIPtr& endpoint) const
{
    const IPEndpointI* ipEndpointI = dynamic_cast<const IPEndpointI*>(endpoint.get());
    if(!ipEndpointI)
    {
        return false;
    }
    return ipEndpointI->type() == type() && ipEndpointI->_host == _host && ipEndpointI->_port == _port;
}

Ice::Int
IceInternal::IPEndpointI::hash() const
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(hashMutex);
    if(!_hashInitialized)
    {
        _hashValue = 5381;
        hashAdd(_hashValue, type());
        hashInit(_hashValue);
        _hashInitialized = true;
    }
    return _hashValue;
}

string
IceInternal::IPEndpointI::options() const
{
    //
    // WARNING: Certain features, such as proxy validation in Glacier2,
    // depend on the format of proxy strings. Changes to toString() and
    // methods called to generate parts of the reference string could break
    // these features. Please review for all features that depend on the
    // format of proxyToString() before changing this and related code.
    //
    ostringstream s;

    if(!_host.empty())
    {
        s << " -h ";
        bool addQuote = _host.find(':') != string::npos;
        if(addQuote)
        {
            s << "\"";
        }
        s << _host;
        if(addQuote)
        {
            s << "\"";
        }
    }

    s << " -p " << _port;

    if(isAddressValid(_sourceAddr))
    {
        s << " --sourceAddress " << inetAddrToString(_sourceAddr);
    }

    return s.str();
}

bool
#ifdef ICE_CPP11_MAPPING
IceInternal::IPEndpointI::operator==(const Endpoint& r) const
#else
IceInternal::IPEndpointI::operator==(const LocalObject& r) const
#endif
{
    const IPEndpointI* p = dynamic_cast<const IPEndpointI*>(&r);
    if(!p)
    {
        return false;
    }

    if(this == p)
    {
        return true;
    }

    if(_host != p->_host)
    {
        return false;
    }

    if(_port != p->_port)
    {
        return false;
    }

    if(_connectionId != p->_connectionId)
    {
        return false;
    }

    if(compareAddress(_sourceAddr, p->_sourceAddr) != 0)
    {
        return false;
    }
    return true;
}

bool
#ifdef ICE_CPP11_MAPPING
IceInternal::IPEndpointI::operator<(const Endpoint& r) const
#else
IceInternal::IPEndpointI::operator<(const LocalObject& r) const
#endif
{
    const IPEndpointI* p = dynamic_cast<const IPEndpointI*>(&r);
    if(!p)
    {
        const EndpointI* e = dynamic_cast<const EndpointI*>(&r);
        if(!e)
        {
            return false;
        }
        return type() < e->type();
    }

    if(this == p)
    {
        return false;
    }

    if(type() < p->type())
    {
        return true;
    }
    else if(p->type() < type())
    {
        return false;
    }

    if(_host < p->_host)
    {
        return true;
    }
    else if(p->_host < _host)
    {
        return false;
    }

    if(_port < p->_port)
    {
        return true;
    }
    else if(p->_port < _port)
    {
        return false;
    }

    if(_connectionId < p->_connectionId)
    {
        return true;
    }
    else if(p->_connectionId < _connectionId)
    {
        return false;
    }

    int rc = compareAddress(_sourceAddr, p->_sourceAddr);
    if(rc < 0)
    {
        return true;
    }
    else if(rc > 0)
    {
        return false;
    }

    return false;
}

vector<ConnectorPtr>
IceInternal::IPEndpointI::connectors(const vector<Address>& addresses, const NetworkProxyPtr& proxy) const
{
    vector<ConnectorPtr> connectors;
    for(unsigned int i = 0; i < addresses.size(); ++i)
    {
        connectors.push_back(createConnector(addresses[i], proxy));
    }
    return connectors;
}

void
IceInternal::IPEndpointI::hashInit(Ice::Int& h) const
{
    hashAdd(h, _host);
    hashAdd(h, _port);
    hashAdd(h, _connectionId);
    if(isAddressValid(_sourceAddr))
    {
        hashAdd(h, inetAddrToString(_sourceAddr));
    }
}

void
IceInternal::IPEndpointI::fillEndpointInfo(Ice::IPEndpointInfo* info) const
{
    info->host = _host;
    info->port = _port;
    info->sourceAddress = inetAddrToString(_sourceAddr);
}

void
IceInternal::IPEndpointI::initWithOptions(vector<string>& args, bool oaEndpoint)
{
    EndpointI::initWithOptions(args);

    if(_host.empty())
    {
        const_cast<string&>(_host) = _instance->defaultHost();
    }
    else if(_host == "*")
    {
        if(oaEndpoint)
        {
            const_cast<string&>(_host) = string();
        }
        else
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "`-h *' not valid for proxy endpoint `" + toString() +
                                              "'");
        }
    }

    if(isAddressValid(_sourceAddr))
    {
        if(oaEndpoint)
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__,
                                              "`--sourceAddress' not valid for object adapter endpoint `" + toString() +
                                              "'");
        }
    }
    else if(!oaEndpoint)
    {
        const_cast<Address&>(_sourceAddr) = _instance->defaultSourceAddress();
    }
}

bool
IceInternal::IPEndpointI::checkOption(const string& option, const string& argument, const string& endpoint)
{
    if(option == "-h")
    {
        if(argument.empty())
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "no argument provided for -h option in endpoint " +
                                              endpoint);
        }
        const_cast<string&>(_host) = argument;
    }
    else if(option == "-p")
    {
        if(argument.empty())
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "no argument provided for -p option in endpoint " +
                                              endpoint);
        }
        istringstream p(argument);
        if(!(p >> const_cast<Ice::Int&>(_port)) || !p.eof())
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "invalid port value `" + argument + "' in endpoint " +
                                              endpoint);
        }
        else if(_port < 0 || _port > 65535)
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__, "port value `" + argument +
                                              "' out of range in endpoint " + endpoint);
        }
    }
    else if(option == "--sourceAddress")
    {
        if(argument.empty())
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__,
                                              "no argument provided for --sourceAddress option in endpoint " +
                                              endpoint);
        }
#ifndef ICE_OS_UWP
        const_cast<Address&>(_sourceAddr) = getNumericAddress(argument);
        if(!isAddressValid(_sourceAddr))
        {
            throw Ice::EndpointParseException(__FILE__, __LINE__,
                                              "invalid IP address provided for --sourceAddress option in endpoint " +
                                              endpoint);
        }
#endif
    }
    else
    {
        return false;
    }
    return true;
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance, const string& host, int port,
                                      const Address& sourceAddr, const string& connectionId) :
    _instance(instance),
    _host(host),
    _port(port),
    _sourceAddr(sourceAddr),
    _connectionId(connectionId),
    _hashInitialized(false)
{
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance) :
    _instance(instance),
    _port(0),
    _hashInitialized(false)
{
}

IceInternal::IPEndpointI::IPEndpointI(const ProtocolInstancePtr& instance, InputStream* s) :
    _instance(instance),
    _port(0),
    _hashInitialized(false)
{
    s->read(const_cast<string&>(_host), false);
    s->read(const_cast<Ice::Int&>(_port));
}

#ifndef ICE_OS_UWP

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    IceUtil::Thread("Ice.HostResolver"),
    _instance(instance),
    _protocol(instance->protocolSupport()),
    _preferIPv6(instance->preferIPv6()),
    _destroyed(false)
{
    updateObserver();
}

void
IceInternal::EndpointHostResolver::resolve(const string& host, int port, Ice::EndpointSelectionType selType,
                                           const IPEndpointIPtr& endpoint, const EndpointI_connectorsPtr& callback)
{
    //
    // Try to get the addresses without DNS lookup. If this doesn't work, we queue a resolve
    // entry and the thread will take care of getting the endpoint addresses.
    //
    NetworkProxyPtr networkProxy = _instance->networkProxy();
    if(!networkProxy)
    {
        try
        {
            vector<Address> addrs = getAddresses(host, port, _protocol, selType, _preferIPv6, false);
            if(!addrs.empty())
            {
                callback->connectors(endpoint->connectors(addrs, 0));
                return;
            }
        }
        catch(const Ice::LocalException& ex)
        {
            callback->exception(ex);
            return;
        }
    }

    Lock sync(*this);
    assert(!_destroyed);

    ResolveEntry entry;
    entry.host = host;
    entry.port = port;
    entry.selType = selType;
    entry.endpoint = endpoint;
    entry.callback = callback;

    const CommunicatorObserverPtr& obsv = _instance->initializationData().observer;
    if(obsv)
    {
        entry.observer = obsv->getEndpointLookupObserver(endpoint);
        if(entry.observer)
        {
            entry.observer->attach();
        }
    }

    _queue.push_back(entry);
    notify();
}

void
IceInternal::EndpointHostResolver::destroy()
{
    Lock sync(*this);
    assert(!_destroyed);
    _destroyed = true;
    notify();
}

void
IceInternal::EndpointHostResolver::run()
{
    while(true)
    {
        ResolveEntry r;
        ThreadObserverPtr threadObserver;
        {
            Lock sync(*this);
            while(!_destroyed && _queue.empty())
            {
                wait();
            }

            if(_destroyed)
            {
                break;
            }

            r = _queue.front();
            _queue.pop_front();
            threadObserver = _observer.get();
        }

        if(threadObserver)
        {
            threadObserver->stateChanged(ICE_ENUM(ThreadState, ThreadStateIdle), ICE_ENUM(ThreadState, ThreadStateInUseForOther));
        }

        try
        {
            NetworkProxyPtr networkProxy = _instance->networkProxy();
            ProtocolSupport protocol = _protocol;
            if(networkProxy)
            {
                networkProxy = networkProxy->resolveHost(_protocol);
                if(networkProxy)
                {
                    protocol = networkProxy->getProtocolSupport();
                }
            }

            vector<Address> addresses = getAddresses(r.host, r.port, protocol, r.selType, _preferIPv6, true);
            if(r.observer)
            {
                r.observer->detach();
                r.observer = 0;
            }

            r.callback->connectors(r.endpoint->connectors(addresses, networkProxy));

            if(threadObserver)
            {
                threadObserver->stateChanged(ICE_ENUM(ThreadState, ThreadStateInUseForOther),
                                             ICE_ENUM(ThreadState, ThreadStateIdle));
            }

        }
        catch(const Ice::LocalException& ex)
        {
            if(threadObserver)
            {
                threadObserver->stateChanged(ICE_ENUM(ThreadState, ThreadStateInUseForOther),
                                             ICE_ENUM(ThreadState, ThreadStateIdle));
            }
            if(r.observer)
            {
                r.observer->failed(ex.ice_id());
                r.observer->detach();
            }
            r.callback->exception(ex);
        }
    }

    for(deque<ResolveEntry>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        Ice::CommunicatorDestroyedException ex(__FILE__, __LINE__);
        if(p->observer)
        {
            p->observer->failed(ex.ice_id());
            p->observer->detach();
        }
        p->callback->exception(ex);
    }
    _queue.clear();

    if(_observer)
    {
        _observer.detach();
    }
}

void
IceInternal::EndpointHostResolver::updateObserver()
{
    Lock sync(*this);
    const CommunicatorObserverPtr& obsv = _instance->initializationData().observer;
    if(obsv)
    {
        _observer.attach(obsv->getThreadObserver("Communicator",
                                                 name(),
                                                 ICE_ENUM(ThreadState, ThreadStateIdle),
                                                 _observer.get()));
    }
}

#else

IceInternal::EndpointHostResolver::EndpointHostResolver(const InstancePtr& instance) :
    _instance(instance)
{
}

void
IceInternal::EndpointHostResolver::resolve(const string& host,
                                           int port,
                                           Ice::EndpointSelectionType selType,
                                           const IPEndpointIPtr& endpoint,
                                           const EndpointI_connectorsPtr& callback)
{
    //
    // No DNS lookup support with UWP.
    //
    callback->connectors(endpoint->connectors(getAddresses(host, port,
                                                           _instance->protocolSupport(),
                                                           selType,
                                                           _instance->preferIPv6(),
                                                           false),
                                              _instance->networkProxy()));
}

void
IceInternal::EndpointHostResolver::destroy()
{
}

void
IceInternal::EndpointHostResolver::run()
{
}

void
IceInternal::EndpointHostResolver::updateObserver()
{
}

#endif
