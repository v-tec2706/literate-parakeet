//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// The following is required on HP-UX in order to bring in
// the definition for the ip_mreq structure.
//
#if defined(__hpux)
#  undef _XOPEN_SOURCE_EXTENDED
#  define _XOPEN_SOURCE
#  include <netinet/in.h>
#endif

#include <IceUtil/DisableWarnings.h>
#include <Ice/Network.h>
#include <Ice/NetworkProxy.h>
#include <IceUtil/StringUtil.h>
#include <Ice/StringConverter.h>
#include <Ice/LocalException.h>
#include <Ice/ProtocolInstance.h> // For setTcpBufSize
#include <Ice/Properties.h> // For setTcpBufSize
#include <Ice/LoggerUtil.h> // For setTcpBufSize
#include <Ice/Buffer.h>
#include <IceUtil/Random.h>
#include <functional>

// TODO: fix this warning
#if defined(_MSC_VER)
#   pragma warning(disable:4244) // 'argument': conversion from 'int' to 'u_short', possible loss of data
#endif

#if defined(ICE_OS_UWP)
#   include <IceUtil/InputUtil.h>
#elif defined(_WIN32)
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   ifdef __MINGW32__
#       include <wincrypt.h>
#   endif
#   include <iphlpapi.h>
#   include <Mswsock.h>
#   include <mstcpip.h>
#else
#   include <net/if.h>
#   include <sys/ioctl.h>
#endif

#if defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
#  include <ifaddrs.h>
#elif defined(__sun)
#  include <sys/sockio.h>
#endif

#if defined(__GNUC__) && (__GNUC__ < 5)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if defined(_WIN32)
#   ifndef SIO_LOOPBACK_FAST_PATH
#       define SIO_LOOPBACK_FAST_PATH _WSAIOW(IOC_VENDOR,16)
#   endif
#endif

#if defined(__MINGW32__)
//
// Work-around for missing definitions in MinGW Windows headers
//
#   ifndef IPV6_V6ONLY
#       define IPV6_V6ONLY 27
#   endif

extern "C"
{
    WINSOCK_API_LINKAGE int WSAAPI inet_pton(INT, PCTSTR, PVOID);
}
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifdef ICE_OS_UWP
using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Networking::Connectivity;
#endif

#ifdef _WIN32
int
IceInternal::getSystemErrno()
{
    return GetLastError();
}
#else
int
IceInternal::getSystemErrno()
{
    return errno;
}
#endif

namespace
{

struct AddressCompare
{
public:

    bool
    operator()(const Address& lhs, const Address& rhs) const
    {
        return compareAddress(lhs, rhs) < 0;
    }
};

#ifndef ICE_OS_UWP

#   ifndef ICE_CPP11_COMPILER

struct AddressIsIPv6 : public unary_function<Address, bool>
{
public:

    bool
    operator()(const Address& ss) const
    {
        return ss.saStorage.ss_family == AF_INET6;
    }
};
#   endif

void
sortAddresses(vector<Address>& addrs, ProtocolSupport protocol, Ice::EndpointSelectionType selType, bool preferIPv6)
{
    if(selType == Ice::ICE_ENUM(EndpointSelectionType, Random))
    {
        IceUtilInternal::shuffle(addrs.begin(), addrs.end());
    }

    if(protocol == EnableBoth)
    {
#ifdef ICE_CPP11_COMPILER
        if(preferIPv6)
        {
            stable_partition(addrs.begin(), addrs.end(),
                             [](const Address& ss)
                             {
                                 return ss.saStorage.ss_family == AF_INET6;
                             });
        }
        else
        {
            stable_partition(addrs.begin(), addrs.end(),
                             [](const Address& ss)
                             {
                                 return ss.saStorage.ss_family != AF_INET6;
                             });
        }
#else
        if(preferIPv6)
        {
            stable_partition(addrs.begin(), addrs.end(), AddressIsIPv6());
        }
        else
        {
            stable_partition(addrs.begin(), addrs.end(), not1(AddressIsIPv6()));
        }
#endif
    }
}

void
setTcpNoDelay(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

void
setKeepAlive(SOCKET fd)
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

#if defined(_WIN32) && !defined(ICE_OS_UWP)
void
setTcpLoopbackFastPath(SOCKET fd)
{
    int OptionValue = 1;
    DWORD NumberOfBytesReturned = 0;

    int status =
        WSAIoctl(fd, SIO_LOOPBACK_FAST_PATH, &OptionValue, sizeof(OptionValue), ICE_NULLPTR, 0, &NumberOfBytesReturned, 0, 0);
    if(status == SOCKET_ERROR)
    {
            // On platforms that do not support fast path (< Windows 8), WSAEONOTSUPP is expected.
        DWORD LastError = ::GetLastError();
        if(LastError != WSAEOPNOTSUPP)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
}
#endif

#ifdef ICE_OS_UWP
SOCKET
createSocketImpl(bool udp, int)
{
    if(udp)
    {
        return ref new DatagramSocket();
    }
    else
    {
        StreamSocket^ socket = ref new StreamSocket();
        socket->Control->KeepAlive = true;
        socket->Control->NoDelay = true;
        return socket;
    }
}
#else
SOCKET
createSocketImpl(bool udp, int family)
{
    SOCKET fd;
    if(udp)
    {
        fd = socket(family, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
    }

    if(fd == INVALID_SOCKET)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    if(!udp)
    {
        setTcpNoDelay(fd);
        setKeepAlive(fd);

#if defined(_WIN32) && !defined(ICE_OS_UWP)
        //
        // FIX: the fast path loopback appears to cause issues with
        // connection closure when it's enabled. Sometime, a peer
        // doesn't receive the TCP/IP connection closure (RST) from
        // the other peer and it ends up hanging. This is showing up
        // with the background test when ran with WS. The test
        // sporadically hangs on exit. See bug #6093.
        //
        //setTcpLoopbackFastPath(fd);
#endif
    }

    return fd;
}
#endif

#ifndef ICE_OS_UWP
vector<Address>
getLocalAddresses(ProtocolSupport protocol, bool includeLoopback, bool singleAddressPerInterface)
{
    vector<Address> result;

#if defined(_WIN32)
    DWORD family;
    switch(protocol)
    {
        case EnableIPv4:
            family = AF_INET;
            break;
        case EnableIPv6:
            family = AF_INET6;
            break;
        default:
            family = AF_UNSPEC;
            break;
    }

    DWORD size;
    DWORD rv = GetAdaptersAddresses(family, 0, ICE_NULLPTR, ICE_NULLPTR, &size);
    if(rv == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_ADDRESSES adapter_addresses = (PIP_ADAPTER_ADDRESSES) malloc(size);
        rv = GetAdaptersAddresses(family, 0, ICE_NULLPTR, adapter_addresses, &size);
        if(rv == ERROR_SUCCESS)
        {
            for(PIP_ADAPTER_ADDRESSES aa = adapter_addresses; aa != ICE_NULLPTR; aa = aa->Next)
            {
                if(aa->OperStatus != IfOperStatusUp)
                {
                    continue;
                }
                for(PIP_ADAPTER_UNICAST_ADDRESS ua = aa->FirstUnicastAddress; ua != ICE_NULLPTR; ua = ua->Next)
                {
                    Address addr;
                    memcpy(&addr.saStorage, ua->Address.lpSockaddr, ua->Address.iSockaddrLength);
                    if(addr.saStorage.ss_family == AF_INET && protocol != EnableIPv6)
                    {
                        if(addr.saIn.sin_addr.s_addr != 0 &&
                           (includeLoopback || addr.saIn.sin_addr.s_addr != htonl(INADDR_LOOPBACK)))
                        {
                            result.push_back(addr);
                            if(singleAddressPerInterface)
                            {
                                break; // One address is enough for each interface.
                            }
                        }
                    }
                    else if(addr.saStorage.ss_family == AF_INET6 && protocol != EnableIPv4)
                    {
                        if(!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr) &&
                           (includeLoopback || !IN6_IS_ADDR_LOOPBACK(&addr.saIn6.sin6_addr)))
                        {
                            result.push_back(addr);
                            if(singleAddressPerInterface)
                            {
                                break; // One address is enough for each interface.
                            }
                        }
                    }
                }
            }
        }

        free(adapter_addresses);
    }
#elif defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
    struct ifaddrs* ifap;
    if(::getifaddrs(&ifap) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    struct ifaddrs* curr = ifap;
    set<string> interfaces;
    while(curr != 0)
    {
        if(curr->ifa_addr && (includeLoopback || !(curr->ifa_flags & IFF_LOOPBACK)))
        {
            if(curr->ifa_addr->sa_family == AF_INET && protocol != EnableIPv6)
            {
                Address addr;
                memcpy(&addr.saStorage, curr->ifa_addr, sizeof(sockaddr_in));
                if(addr.saIn.sin_addr.s_addr != 0)
                {
                    if(!singleAddressPerInterface || interfaces.find(curr->ifa_name) == interfaces.end())
                    {
                        result.push_back(addr);
                        interfaces.insert(curr->ifa_name);
                    }
                }
            }
            else if(curr->ifa_addr->sa_family == AF_INET6 && protocol != EnableIPv4)
            {
                Address addr;
                memcpy(&addr.saStorage, curr->ifa_addr, sizeof(sockaddr_in6));
                if(!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
                {
                    if(!singleAddressPerInterface || interfaces.find(curr->ifa_name) == interfaces.end())
                    {
                        result.push_back(addr);
                        interfaces.insert(curr->ifa_name);
                    }
                }
            }
        }

        curr = curr->ifa_next;
    }

    ::freeifaddrs(ifap);
#else
    for(int i = 0; i < 2; i++)
    {
        if((i == 0 && protocol == EnableIPv6) || (i == 1 && protocol == EnableIPv4))
        {
            continue;
        }
        SOCKET fd = createSocketImpl(false, i == 0 ? AF_INET : AF_INET6);

#ifdef _AIX
        int cmd = CSIOCGIFCONF;
#else
        int cmd = SIOCGIFCONF;
#endif
        struct ifconf ifc;
        int numaddrs = 10;
        int old_ifc_len = 0;

        //
        // Need to call ioctl multiple times since we do not know up front
        // how many addresses there will be, and thus how large a buffer we need.
        // We keep increasing the buffer size until subsequent calls return
        // the same length, meaning we have all the addresses.
        //
        while(true)
        {
            int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
            ifc.ifc_len = bufsize;
            ifc.ifc_buf = (char*)malloc(bufsize);

            int rs = ioctl(fd, cmd, &ifc);
            if(rs == SOCKET_ERROR)
            {
                free(ifc.ifc_buf);
                closeSocketNoThrow(fd);
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
            else if(ifc.ifc_len == old_ifc_len)
            {
                //
                // Returned same length twice in a row, finished.
                //
                break;
            }
            else
            {
                old_ifc_len = ifc.ifc_len;
            }

            numaddrs += 10;
            free(ifc.ifc_buf);
        }
        closeSocket(fd);

        numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
        struct ifreq* ifr = ifc.ifc_req;
        set<string> interfaces;
        for(int i = 0; i < numaddrs; ++i)
        {
            if(!(ifr[i].ifr_flags & IFF_LOOPBACK)) // Don't include loopback interface addresses
            {
                //
                // On Solaris the above Loopback check does not always work so we double
                // check the address below. Solaris also returns duplicate entries that need
                // to be filtered out.
                //
                if(ifr[i].ifr_addr.sa_family == AF_INET && protocol != EnableIPv6)
                {
                    Address addr;
                    memcpy(&addr.saStorage, &ifr[i].ifr_addr, sizeof(sockaddr_in));
                    if(addr.saIn.sin_addr.s_addr != 0 &&
                       (includeLoopback || addr.saIn.sin_addr.s_addr != htonl(INADDR_LOOPBACK)))
                    {
                        if(!singleAddressPerInterface || interfaces.find(ifr[i].ifr_name) == interfaces.end())
                        {
                            result.push_back(addr);
                            interfaces.insert(ifr[i].ifr_name);
                        }
                    }
                }
                else if(ifr[i].ifr_addr.sa_family == AF_INET6 && protocol != EnableIPv4)
                {
                    Address addr;
                    memcpy(&addr.saStorage, &ifr[i].ifr_addr, sizeof(sockaddr_in6));
                    if(!IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr) &&
                       (includeLoopback || !IN6_IS_ADDR_LOOPBACK(&addr.saIn6.sin6_addr)))
                    {
                        if(!singleAddressPerInterface || interfaces.find(ifr[i].ifr_name) == interfaces.end())
                        {
                            result.push_back(addr);
                            interfaces.insert(ifr[i].ifr_name);
                        }
                    }
                }
            }
        }
        free(ifc.ifc_buf);
    }
#endif

    //
    // Remove potential duplicates from the result.
    //
    set<Address, AddressCompare> seen;
    vector<Address> tmp;
    tmp.swap(result);
    for(vector<Address>::const_iterator p = tmp.begin(); p != tmp.end(); ++p)
    {
        if(seen.find(*p) == seen.end())
        {
            result.push_back(*p);
            seen.insert(*p);
        }
    }
    return result;
}

bool
isLinklocal(const Address& addr)
{
    if(addr.saStorage.ss_family == AF_INET6)
    {
        return IN6_IS_ADDR_LINKLOCAL(&addr.saIn6.sin6_addr);
    }
    else if(addr.saStorage.ss_family == AF_INET)
    {
        // Check for 169.254.X.X in network order
        return (addr.saIn.sin_addr.s_addr & 0xFF) == 169 && ((addr.saIn.sin_addr.s_addr & 0xFF00)>>8) == 254;
    }
    return false;
}

bool
isWildcard(const string& host, ProtocolSupport protocol, bool& ipv4)
{
    Address addr = getAddressForServer(host, 0, protocol, true, false);
    if(addr.saStorage.ss_family == AF_INET)
    {
        if(addr.saIn.sin_addr.s_addr == INADDR_ANY)
        {
            ipv4 = true;
            return true;
        }
    }
    else if(addr.saStorage.ss_family == AF_INET6)
    {
        if(IN6_IS_ADDR_UNSPECIFIED(&addr.saIn6.sin6_addr))
        {
            ipv4 = false;
            return true;
        }
    }
    return false;
}

int
getInterfaceIndex(const string& intf)
{
    if(intf.empty())
    {
        return 0;
    }

    string name;
    bool isAddr;
    in6_addr addr;
    string::size_type pos = intf.find("%");
    if(pos != string::npos)
    {
        //
        // If it's a link-local address, use the zone indice.
        //
        isAddr = false;
        name = intf.substr(pos + 1);
    }
    else
    {
        //
        // Then check if it's an IPv6 address. If it's an address we'll
        // look for the interface index by address.
        //
        isAddr = inet_pton(AF_INET6, intf.c_str(), &addr) > 0;
        name = intf;
    }

    //
    // Check if index
    //
    int index = -1;
    istringstream p(name);
    if((p >> index) && p.eof())
    {
        return index;
    }

#ifdef _WIN32
    IP_ADAPTER_ADDRESSES addrs;
    ULONG buflen = 0;
    if(::GetAdaptersAddresses(AF_INET6, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_ADDRESSES paddrs;
        char* buf = new char[buflen];
        paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
        if(::GetAdaptersAddresses(AF_INET6, 0, 0, paddrs, &buflen) == NO_ERROR)
        {
            while(paddrs)
            {
                if(isAddr)
                {
                    PIP_ADAPTER_UNICAST_ADDRESS ipAddr = paddrs->FirstUnicastAddress;
                    while(ipAddr)
                    {
                        if(ipAddr->Address.lpSockaddr->sa_family == AF_INET6)
                        {
                            struct sockaddr_in6* ipv6Addr =
                                reinterpret_cast<struct sockaddr_in6*>(ipAddr->Address.lpSockaddr);
                            if(memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                            {
                                break;
                            }
                        }
                        ipAddr = ipAddr->Next;
                    }
                    if(ipAddr)
                    {
                        index = paddrs->Ipv6IfIndex;
                        break;
                    }
                }
                else
                {
                    //
                    // Don't need to pass a wide string converter as the wide string
                    // come from Windows API.
                    //
                    if(wstringToString(paddrs->FriendlyName, getProcessStringConverter()) == name)
                    {
                        index = paddrs->Ipv6IfIndex;
                        break;
                    }
                }
                paddrs = paddrs->Next;
            }
        }
        delete[] buf;
    }
    if(index < 0) // interface not found
    {
        throw Ice::SocketException(__FILE__, __LINE__, WSAEINVAL);
    }
#elif !defined(__hpux)

    //
    // Look for an interface with a matching IP address
    //
    if(isAddr)
    {
#  if defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
        struct ifaddrs* ifap;
        if(::getifaddrs(&ifap) != SOCKET_ERROR)
        {
            struct ifaddrs* curr = ifap;
            while(curr != 0)
            {
                if(curr->ifa_addr && curr->ifa_addr->sa_family == AF_INET6)
                {
                    struct sockaddr_in6* ipv6Addr = reinterpret_cast<struct sockaddr_in6*>(curr->ifa_addr);
                    if(memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                    {
                        index = if_nametoindex(curr->ifa_name);
                        break;
                    }
                }
                curr = curr->ifa_next;
            }
            ::freeifaddrs(ifap);
        }
#  else
        SOCKET fd = createSocketImpl(false, AF_INET6);
#    ifdef _AIX
        int cmd = CSIOCGIFCONF;
#    else
        int cmd = SIOCGIFCONF;
#    endif
        struct ifconf ifc;
        int numaddrs = 10;
        int old_ifc_len = 0;

        //
        // Need to call ioctl multiple times since we do not know up front
        // how many addresses there will be, and thus how large a buffer we need.
        // We keep increasing the buffer size until subsequent calls return
        // the same length, meaning we have all the addresses.
        //
        while(true)
        {
            int bufsize = numaddrs * static_cast<int>(sizeof(struct ifreq));
            ifc.ifc_len = bufsize;
            ifc.ifc_buf = (char*)malloc(bufsize);

            int rs = ioctl(fd, cmd, &ifc);
            if(rs == SOCKET_ERROR)
            {
                free(ifc.ifc_buf);
                ifc.ifc_buf = 0;
                break;
            }
            else if(ifc.ifc_len == old_ifc_len)
            {
                //
                // Returned same length twice in a row, finished.
                //
                break;
            }
            else
            {
                old_ifc_len = ifc.ifc_len;
            }
            numaddrs += 10;
            free(ifc.ifc_buf);
        }
        closeSocketNoThrow(fd);

        if(ifc.ifc_buf)
        {
            numaddrs = ifc.ifc_len / static_cast<int>(sizeof(struct ifreq));
            struct ifreq* ifr = ifc.ifc_req;
            for(int i = 0; i < numaddrs; ++i)
            {
                if(ifr[i].ifr_addr.sa_family == AF_INET6)
                {
                    struct sockaddr_in6* ipv6Addr = reinterpret_cast<struct sockaddr_in6*>(&ifr[i].ifr_addr);
                    if(memcmp(&addr, &ipv6Addr->sin6_addr, sizeof(in6_addr)) == 0)
                    {
                        index = if_nametoindex(ifr[i].ifr_name);
                        break;
                    }
                }
            }
            free(ifc.ifc_buf);
        }
#  endif
    }
    else // Look for an interface with the given name.
    {
        index = if_nametoindex(name.c_str());
    }
    if(index <= 0)
    {
        // index == 0 if if_nametoindex returned 0, < 0 if name wasn't found
        throw Ice::SocketException(__FILE__, __LINE__, index == 0 ? getSocketErrno() : ENXIO);
    }
#endif

    return index;
}

struct in_addr
getInterfaceAddress(const string& name)
{
    struct in_addr addr;
    addr.s_addr = INADDR_ANY;
    if(name.empty())
    {
        return addr;
    }

    if(inet_pton(AF_INET, name.c_str(), &addr) > 0)
    {
        return addr;
    }

#ifdef _WIN32
    IP_ADAPTER_ADDRESSES addrs;
    ULONG buflen = 0;
    if(::GetAdaptersAddresses(AF_INET, 0, 0, &addrs, &buflen) == ERROR_BUFFER_OVERFLOW)
    {
        PIP_ADAPTER_ADDRESSES paddrs;
        char* buf = new char[buflen];
        paddrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buf);
        if(::GetAdaptersAddresses(AF_INET, 0, 0, paddrs, &buflen) == NO_ERROR)
        {
            while(paddrs)
            {
                //
                // Don't need to pass a wide string converter as the wide string come
                // from Windows API.
                //
                if(wstringToString(paddrs->FriendlyName, getProcessStringConverter()) == name)
                {
                    struct sockaddr_in addrin;
                    memcpy(&addrin, paddrs->FirstUnicastAddress->Address.lpSockaddr,
                           paddrs->FirstUnicastAddress->Address.iSockaddrLength);
                    delete[] buf;
                    return addrin.sin_addr;
                }
                paddrs = paddrs->Next;
            }
        }
        delete[] buf;
    }
    throw Ice::SocketException(__FILE__, __LINE__, WSAEINVAL);
#else
    ifreq if_address;
    strcpy(if_address.ifr_name, name.c_str());

    SOCKET fd = createSocketImpl(false, AF_INET);
    int rc = ioctl(fd, SIOCGIFADDR, &if_address);
    closeSocketNoThrow(fd);
    if(rc == SOCKET_ERROR)
    {
        throw Ice::SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    return reinterpret_cast<struct sockaddr_in*>(&if_address.ifr_addr)->sin_addr;
#endif
}

int
getAddressStorageSize(const Address& addr)
{
    int size = 0;
    if(addr.saStorage.ss_family == AF_INET)
    {
        size = sizeof(sockaddr_in);
    }
    else if(addr.saStorage.ss_family == AF_INET6)
    {
        size = sizeof(sockaddr_in6);
    }
    return size;
}

vector<Address>
getLoopbackAddresses(ProtocolSupport protocol, int port = 0)
{
    vector<Address> result;

    Address addr;
    memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if(protocol != EnableIPv4)
    {
        addr.saIn6.sin6_family = AF_INET6;
        addr.saIn6.sin6_port = htons(port);
        addr.saIn6.sin6_addr = in6addr_loopback;
        result.push_back(addr);
    }
    if(protocol != EnableIPv6)
    {
        addr.saIn.sin_family = AF_INET;
        addr.saIn.sin_port = htons(port);
        addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        result.push_back(addr);
    }
    return result;
}

#endif // #ifndef ICE_OS_UWP

}

ReadyCallback::~ReadyCallback()
{
    // Out of line to avoid weak vtable
}

NativeInfo::~NativeInfo()
{
    // Out of line to avoid weak vtable
}

void
NativeInfo::setReadyCallback(const ReadyCallbackPtr& callback)
{
    _readyCallback = callback;
}

#ifdef ICE_USE_IOCP

IceInternal::AsyncInfo::AsyncInfo(SocketOperation s)
{
    ZeroMemory(this, sizeof(AsyncInfo));
    status = s;
}

void
IceInternal::NativeInfo::initialize(HANDLE handle, ULONG_PTR key)
{
    _handle = handle;
    _key = key;
}

void
IceInternal::NativeInfo::completed(SocketOperation operation)
{
    if(!PostQueuedCompletionStatus(_handle, 0, _key, getAsyncInfo(operation)))
    {
        throw Ice::SocketException(__FILE__, __LINE__, GetLastError());
    }
}

#elif defined(ICE_OS_UWP)

void
IceInternal::NativeInfo::queueAction(SocketOperation op, IAsyncAction^ action, bool connect)
{
    AsyncInfo* asyncInfo = getAsyncInfo(op);
    if(checkIfErrorOrCompleted(op, action, action->Status, connect))
    {
        asyncInfo->count = 0;
        asyncInfo->error = ERROR_SUCCESS;
    }
    else
    {
        action->Completed = ref new AsyncActionCompletedHandler(
            [=] (IAsyncAction^ info, Windows::Foundation::AsyncStatus status)
            {
                //
                // COMPILERFIX with v141 using operator!= and operator== inside
                // a lambda callback triggers a compiler bug, we move the code to
                // a seperate private method to workaround the issue.
                //
                this->queueActionCompleted(op, asyncInfo, info, status);
            });
        asyncInfo->operation = action;
    }
}

void
IceInternal::NativeInfo::queueActionCompleted(SocketOperation op, AsyncInfo* asyncInfo, IAsyncAction^ info,
                                              Windows::Foundation::AsyncStatus status)
{
    if(status != Windows::Foundation::AsyncStatus::Completed)
    {
        asyncInfo->error = info->ErrorCode.Value;
    }
    else
    {
        asyncInfo->error = ERROR_SUCCESS;
    }
    asyncInfo->count = 0;
    completed(op);
}

void
IceInternal::NativeInfo::queueOperation(SocketOperation op, IAsyncOperation<unsigned int>^ operation)
{
    AsyncInfo* info = getAsyncInfo(op);
    Windows::Foundation::AsyncStatus status = operation->Status;
    if(status == Windows::Foundation::AsyncStatus::Completed)
    {
        //
        // NOTE: it's important to modify the count _before_ calling the completion handler
        // since this might not always be called with the connection mutex but from a Windows
        // thread pool thread if we chained multiple Async calls (GetGetOutputStreamAsync and
        // StoreAsync for example, see the UDPTransceiver implementation). So we can't modify
        // the AsyncInfo structure after calling the completed callback.
        //
        info->count = static_cast<int>(operation->GetResults());
        info->error = ERROR_SUCCESS;
        _completedHandler(op);
        return;
    }
    else if(!checkIfErrorOrCompleted(op, operation, status))
    {
        if(!info->completedHandler)
        {
            info->completedHandler = ref new AsyncOperationCompletedHandler<unsigned int>(
                [=] (IAsyncOperation<unsigned int>^ operation, Windows::Foundation::AsyncStatus status)
                {
                    //
                    // COMPILERFIX with v141 using operator!= and operator== inside
                    // a lambda callback triggers a compiler bug, we move the code to
                    // a seperate private method to workaround the issue.
                    //
                    this->queueOperationCompleted(op, info, operation, status);
                });
        }
        operation->Completed = info->completedHandler;
        info->operation = operation;
    }
}

void
IceInternal::NativeInfo::queueOperationCompleted(SocketOperation op, AsyncInfo* info,
                                                 IAsyncOperation<unsigned int>^ operation,
                                                 Windows::Foundation::AsyncStatus status)
{
    if(status != Windows::Foundation::AsyncStatus::Completed)
    {
        info->count = 0;
        info->error = operation->ErrorCode.Value;
    }
    else
    {
        info->count = static_cast<int>(operation->GetResults());
        info->error = ERROR_SUCCESS;
    }
    completed(op);
}

void
IceInternal::NativeInfo::setCompletedHandler(SocketOperationCompletedHandler^ handler)
{
    _completedHandler = handler;
}

void
IceInternal::NativeInfo::completed(SocketOperation operation)
{
    assert(_completedHandler);
    _completedHandler(operation);
}

bool
IceInternal::NativeInfo::checkIfErrorOrCompleted(SocketOperation op, IAsyncInfo^ info, Windows::Foundation::AsyncStatus status, bool connect)
{
    //
    // NOTE: It's important to only check for info->Status once as it
    // might change during the checks below (the Status can be changed
    // by the Windows thread pool concurrently).
    //
    // We consider that a canceled async status is the same as an
    // error. A canceled async status can occur if there's a timeout
    // and the socket is closed.
    //
    if(status == Windows::Foundation::AsyncStatus::Completed)
    {
        _completedHandler(op);
        return true;
    }
    else if (status == Windows::Foundation::AsyncStatus::Started)
    {
        return false;
    }
    else
    {
        if(connect) // Connect
        {
            checkConnectErrorCode(__FILE__, __LINE__, info->ErrorCode.Value);
        }
        else
        {
            checkErrorCode(__FILE__, __LINE__, info->ErrorCode.Value);
        }
        return true; // Prevent compiler warning.
    }
}

#else

void
IceInternal::NativeInfo::setNewFd(SOCKET fd)
{
    assert(_fd == INVALID_SOCKET); // This can only be called once, when the current socket isn't set yet.
    _newFd = fd;
}

bool
IceInternal::NativeInfo::newFd()
{
    if(_newFd == INVALID_SOCKET)
    {
        return false;
    }
    assert(_fd == INVALID_SOCKET);
    swap(_fd, _newFd);
    return true;
}

#endif

bool
IceInternal::noMoreFds(int error)
{
#if defined(ICE_OS_UWP)
    return error == (int)SocketErrorStatus::TooManyOpenFiles;
#elif defined(_WIN32)
    return error == WSAEMFILE;
#else
    return error == EMFILE || error == ENFILE;
#endif
}

#if defined(ICE_OS_UWP)
string
IceInternal::errorToStringDNS(int)
{
    return "Host not found";
}
#else
string
IceInternal::errorToStringDNS(int error)
{
#  if defined(_WIN32)
    return IceUtilInternal::errorToString(error);
#  else
    return gai_strerror(error);
#  endif
}
#endif

#ifdef ICE_OS_UWP
vector<Address>
IceInternal::getAddresses(const string& host, int port, ProtocolSupport, Ice::EndpointSelectionType, bool, bool)
{
    try
    {
        vector<Address> result;
        Address addr;
        if(host.empty())
        {
            addr.host = ref new HostName("localhost");
        }
        else
        {
            //
            // Don't need to pass a wide string converter as the wide string is passed
            // to Windows API.
            //
            addr.host = ref new HostName(ref new String(
                                             stringToWstring(host,
                                                             getProcessStringConverter()).c_str()));
        }
        stringstream os;
        os << port;
        //
        // Don't need to use any string converter here as the port number use just
        // ASCII characters.
        //
        addr.port = ref new String(stringToWstring(os.str()).c_str());
        result.push_back(addr);
        return result;
    }
    catch(Platform::Exception^ pex)
    {
        throw DNSException(__FILE__, __LINE__, (int)SocketError::GetStatus(pex->HResult), host);
    }

}
#else
vector<Address>
IceInternal::getAddresses(const string& host, int port, ProtocolSupport protocol, Ice::EndpointSelectionType selType,
                          bool preferIPv6, bool canBlock)
{
    vector<Address> result;

    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if(host.empty())
    {
        result = getLoopbackAddresses(protocol, port);
        sortAddresses(result, protocol, selType, preferIPv6);
        return result;
    }

    Address addr;
    memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

    struct addrinfo* info = 0;
    int retry = 5;

    struct addrinfo hints = {};
    if(protocol == EnableIPv4)
    {
        hints.ai_family = PF_INET;
    }
    else if(protocol == EnableIPv6)
    {
        hints.ai_family = PF_INET6;
    }
    else
    {
        hints.ai_family = PF_UNSPEC;
    }

    if(!canBlock)
    {
        hints.ai_flags = AI_NUMERICHOST;
    }

    int rs = 0;
    do
    {
        rs = getaddrinfo(host.c_str(), 0, &hints, &info);
    }
    while(info == 0 && rs == EAI_AGAIN && --retry >= 0);

    // In theory, getaddrinfo should only return EAI_NONAME if
    // AI_NUMERICHOST is specified and the host name is not a IP
    // address. However on some platforms (e.g. macOS 10.4.x)
    // EAI_NODATA is also returned so we also check for it.
#  ifdef EAI_NODATA
    if(!canBlock && (rs == EAI_NONAME || rs == EAI_NODATA))
#  else
    if(!canBlock && rs == EAI_NONAME)
#  endif
    {
        return result; // Empty result indicates that a canBlock lookup is necessary.
    }
    else if(rs != 0)
    {
        throw DNSException(__FILE__, __LINE__, rs, host);
    }

    for(struct addrinfo* p = info; p != ICE_NULLPTR; p = p->ai_next)
    {
        memcpy(&addr.saStorage, p->ai_addr, p->ai_addrlen);
        if(p->ai_family == PF_INET)
        {
            addr.saIn.sin_port = htons(port);
        }
        else if(p->ai_family == PF_INET6)
        {
            addr.saIn6.sin6_port = htons(port);
        }

        bool found = false;
        for(unsigned int i = 0; i < result.size(); ++i)
        {
            if(compareAddress(result[i], addr) == 0)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            result.push_back(addr);
        }
    }

    freeaddrinfo(info);

    if(result.empty())
    {
        throw DNSException(__FILE__, __LINE__, 0, host);
    }
    sortAddresses(result, protocol, selType, preferIPv6);
    return result;
}
#endif

#ifdef ICE_OS_UWP
ProtocolSupport
IceInternal::getProtocolSupport(const Address&)
{
    // For UWP, there's no distinction between IPv4 and IPv6 adresses.
    return EnableBoth;
}
#else
ProtocolSupport
IceInternal::getProtocolSupport(const Address& addr)
{
    return addr.saStorage.ss_family == AF_INET ? EnableIPv4 : EnableIPv6;
}
#endif

Address
IceInternal::getAddressForServer(const string& host, int port, ProtocolSupport protocol, bool preferIPv6, bool canBlock)
{
    //
    // We don't use getaddrinfo when host is empty as it's not portable (some old Linux
    // versions don't support it).
    //
    if(host.empty())
    {
        Address addr;
#ifdef ICE_OS_UWP
        ostringstream os;
        os << port;
        //
        // Don't need to use any string converter here as the port number use just
        // ASCII characters.
        //
        addr.port = ref new String(stringToWstring(os.str()).c_str());
        addr.host = nullptr; // Equivalent of inaddr_any, see doBind implementation.
#else
        memset(&addr.saStorage, 0, sizeof(sockaddr_storage));
        if(protocol != EnableIPv4)
        {
            addr.saIn6.sin6_family = AF_INET6;
            addr.saIn6.sin6_port = htons(port);
            addr.saIn6.sin6_addr = in6addr_any;
        }
        else
        {
            addr.saIn.sin_family = AF_INET;
            addr.saIn.sin_port = htons(port);
            addr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
        }
#endif
        return addr;
    }
    vector<Address> addrs = getAddresses(host, port, protocol, Ice::ICE_ENUM(EndpointSelectionType, Ordered),
                                         preferIPv6, canBlock);
    return addrs.empty() ? Address() : addrs[0];
}

int
IceInternal::compareAddress(const Address& addr1, const Address& addr2)
{
#ifdef ICE_OS_UWP
    int o = String::CompareOrdinal(addr1.port, addr2.port);
    if(o != 0)
    {
        return o;
    }
    if(addr1.host == addr2.host)
    {
        return 0;
    }
    if(addr1.host == nullptr)
    {
        return -1;
    }
    if(addr2.host == nullptr)
    {
        return 1;
    }
    return String::CompareOrdinal(addr1.host->RawName, addr2.host->RawName);
#else
    if(addr1.saStorage.ss_family < addr2.saStorage.ss_family)
    {
        return -1;
    }
    else if(addr2.saStorage.ss_family < addr1.saStorage.ss_family)
    {
        return 1;
    }

    if(addr1.saStorage.ss_family == AF_INET)
    {
        if(addr1.saIn.sin_port < addr2.saIn.sin_port)
        {
            return -1;
        }
        else if(addr2.saIn.sin_port < addr1.saIn.sin_port)
        {
            return 1;
        }

        if(addr1.saIn.sin_addr.s_addr < addr2.saIn.sin_addr.s_addr)
        {
            return -1;
        }
        else if(addr2.saIn.sin_addr.s_addr < addr1.saIn.sin_addr.s_addr)
        {
            return 1;
        }
    }
    else
    {
        if(addr1.saIn6.sin6_port < addr2.saIn6.sin6_port)
        {
            return -1;
        }
        else if(addr2.saIn6.sin6_port < addr1.saIn6.sin6_port)
        {
            return 1;
        }

        int res = memcmp(&addr1.saIn6.sin6_addr, &addr2.saIn6.sin6_addr, sizeof(in6_addr));
        if(res < 0)
        {
            return -1;
        }
        else if(res > 0)
        {
            return 1;
        }
    }

    return 0;
#endif
}

#ifdef ICE_OS_UWP
bool
IceInternal::isIPv6Supported()
{
    return true;
}
#else
bool
IceInternal::isIPv6Supported()
{
    SOCKET fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(fd == INVALID_SOCKET)
    {
        return false;
    }
    else
    {
        closeSocketNoThrow(fd);
        return true;
    }
}
#endif

#ifdef ICE_OS_UWP
SOCKET
IceInternal::createSocket(bool udp, const Address&)
{
    return createSocketImpl(udp, 0);
}
#else
SOCKET
IceInternal::createSocket(bool udp, const Address& addr)
{
    return createSocketImpl(udp, addr.saStorage.ss_family);
}
#endif

#ifndef ICE_OS_UWP
SOCKET
IceInternal::createServerSocket(bool udp, const Address& addr, ProtocolSupport protocol)
{
    SOCKET fd = createSocket(udp, addr);
    if(addr.saStorage.ss_family == AF_INET6 && protocol != EnableIPv4)
    {
        int flag = protocol == EnableIPv6 ? 1 : 0;
        if(setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
        {
#ifdef _WIN32
            if(getSocketErrno() == WSAENOPROTOOPT)
            {
                return fd; // Windows XP doesn't support IPV6_V6ONLY
            }
#endif
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
    return fd;
}
#else
SOCKET
IceInternal::createServerSocket(bool udp, const Address& addr, ProtocolSupport)
{
    return createSocket(udp, addr);
}
#endif

void
IceInternal::closeSocketNoThrow(SOCKET fd)
{
#if defined(ICE_OS_UWP)
    //
    // NOTE: StreamSocket::Close or DatagramSocket::Close aren't
    // exposed in C++, you have to delete the socket to close
    // it. According some Microsoft samples, this is safe even if
    // there are still references to the object...
    //
    //fd->Close();
    delete fd;
#elif defined(_WIN32)
    int error = WSAGetLastError();
    closesocket(fd);
    WSASetLastError(error);
#else
    int error = errno;
    close(fd);
    errno = error;
#endif
}

void
IceInternal::closeSocket(SOCKET fd)
{
#if defined(ICE_OS_UWP)
    //
    // NOTE: StreamSocket::Close or DatagramSocket::Close aren't
    // exposed in C++, you have to delete the socket to close
    // it. According some Microsoft samples, this is safe even if
    // there are still references to the object...
    //
    //fd->Close();
    delete fd;
#elif defined(_WIN32)
    int error = WSAGetLastError();
    if(closesocket(fd) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    WSASetLastError(error);
#else
    int error = errno;

#  if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
    //
    // FreeBSD returns ECONNRESET if the underlying object was
    // a stream socket that was shut down by the peer before all
    // pending data was delivered.
    //
    if(close(fd) == SOCKET_ERROR && getSocketErrno() != ECONNRESET)
#  else
    if(close(fd) == SOCKET_ERROR)
#  endif
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    errno = error;
#endif
}

string
IceInternal::addrToString(const Address& addr)
{
    ostringstream s;
    s << inetAddrToString(addr) << ':' << getPort(addr);
    return s.str();
}

void
IceInternal::fdToLocalAddress(SOCKET fd, Address& addr)
{
#ifndef ICE_OS_UWP
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
    if(getsockname(fd, &addr.sa, &len) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
#else
    StreamSocket^ stream = dynamic_cast<StreamSocket^>(fd);
    if(stream)
    {
        addr.host = stream->Information->LocalAddress;
        addr.port = stream->Information->LocalPort;
    }
    DatagramSocket^ datagram = dynamic_cast<DatagramSocket^>(fd);
    if(datagram)
    {
        addr.host = datagram->Information->LocalAddress;
        addr.port = datagram->Information->LocalPort;
    }
#endif
}

bool
IceInternal::fdToRemoteAddress(SOCKET fd, Address& addr)
{
#ifndef ICE_OS_UWP
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
    if(getpeername(fd, &addr.sa, &len) == SOCKET_ERROR)
    {
        if(notConnected())
        {
            return false;
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    return true;
#else
    StreamSocket^ stream = dynamic_cast<StreamSocket^>(fd);
    if(stream != nullptr)
    {
        addr.host = stream->Information->RemoteAddress;
        addr.port = stream->Information->RemotePort;
    }
    DatagramSocket^ datagram = dynamic_cast<DatagramSocket^>(fd);
    if(datagram != nullptr)
    {
        addr.host = datagram->Information->RemoteAddress;
        addr.port = datagram->Information->RemotePort;
    }
    return addr.host != nullptr;
#endif
}

std::string
IceInternal::fdToString(SOCKET fd, const NetworkProxyPtr& proxy, const Address& target)
{
    if(fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    ostringstream s;

    Address remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

#ifdef _WIN32
    if(!peerConnected)
    {
        //
        // The local address is only accessible with connected sockets on Windows.
        //
        s << "local address = <not available>";
    }
    else
#endif
    {
        Address localAddr;
        fdToLocalAddress(fd, localAddr);
        s << "local address = " << addrToString(localAddr);
    }

    if(proxy)
    {
        if(!peerConnected)
        {
            remoteAddr = proxy->getAddress();
        }
        s << "\n" + proxy->getName() + " proxy address = " << addrToString(remoteAddr);
        s << "\nremote address = " << addrToString(target);
    }
    else
    {
        if(!peerConnected)
        {
            remoteAddr = target;
        }
        s << "\nremote address = " << addrToString(remoteAddr);
    }

    return s.str();
}

std::string
IceInternal::fdToString(SOCKET fd)
{
    if(fd == INVALID_SOCKET)
    {
        return "<closed>";
    }

    Address localAddr;
    fdToLocalAddress(fd, localAddr);

    Address remoteAddr;
    bool peerConnected = fdToRemoteAddress(fd, remoteAddr);

    return addressesToString(localAddr, remoteAddr, peerConnected);
}

void
IceInternal::fdToAddressAndPort(SOCKET fd, string& localAddress, int& localPort, string& remoteAddress, int& remotePort)
{
    if(fd == INVALID_SOCKET)
    {
        localAddress.clear();
        remoteAddress.clear();
        localPort = -1;
        remotePort = -1;
        return;
    }

    Address localAddr;
    fdToLocalAddress(fd, localAddr);
    addrToAddressAndPort(localAddr, localAddress, localPort);

    Address remoteAddr;
    if(fdToRemoteAddress(fd, remoteAddr))
    {
        addrToAddressAndPort(remoteAddr, remoteAddress, remotePort);
    }
    else
    {
        remoteAddress.clear();
        remotePort = -1;
    }
}

void
IceInternal::addrToAddressAndPort(const Address& addr, string& address, int& port)
{
    address = inetAddrToString(addr);
    port = getPort(addr);
}

std::string
IceInternal::addressesToString(const Address& localAddr, const Address& remoteAddr, bool peerConnected)
{
    ostringstream s;
    s << "local address = " << addrToString(localAddr);
    if(peerConnected)
    {
        s << "\nremote address = " << addrToString(remoteAddr);
    }
    else
    {
        s << "\nremote address = <not connected>";
    }
    return s.str();
}

bool
IceInternal::isAddressValid(const Address& addr)
{
#ifndef ICE_OS_UWP
    return addr.saStorage.ss_family != AF_UNSPEC;
#else
    return addr.host != nullptr || addr.port != nullptr;
#endif
}

#ifdef ICE_OS_UWP
vector<string>
IceInternal::getHostsForEndpointExpand(const string& host, ProtocolSupport protocolSupport, bool includeLoopback)
{
    vector<string> hosts;
    if(host.empty() || host == "0.0.0.0" || host == "::" || host == "0:0:0:0:0:0:0:0")
    {
        for(IIterator<HostName^>^ it = NetworkInformation::GetHostNames()->First(); it->HasCurrent; it->MoveNext())
        {
            HostName^ h = it->Current;
            if(h->IPInformation != nullptr && h->IPInformation->NetworkAdapter != nullptr)
            {
                hosts.push_back(wstringToString(h->CanonicalName->Data(), getProcessStringConverter()));
            }
        }
        if(hosts.empty() || includeLoopback)
        {
            if(protocolSupport != EnableIPv6)
            {
                hosts.push_back("127.0.0.1");
            }
            if(protocolSupport != EnableIPv4)
            {
                hosts.push_back("::1");
            }
        }
    }
    return hosts;
}

vector<string>
IceInternal::getInterfacesForMulticast(const string& intf, ProtocolSupport)
{
    vector<string> interfaces;
    if(intf.empty() || intf == "0.0.0.0" || intf == "::" || intf == "0:0:0:0:0:0:0:0")
    {
        for(IIterator<HostName^>^ it = NetworkInformation::GetHostNames()->First(); it->HasCurrent; it->MoveNext())
        {
            HostName^ h = it->Current;
            if(h->IPInformation != nullptr && h->IPInformation->NetworkAdapter != nullptr)
            {
                string s = wstringToString(h->CanonicalName->Data(), getProcessStringConverter());
                if(find(interfaces.begin(), interfaces.end(), s) == interfaces.end())
                {
                    interfaces.push_back(s);
                }
            }
        }
    }
    if(interfaces.empty())
    {
        interfaces.push_back(intf);
    }
    return interfaces;
}
#else
vector<string>
IceInternal::getHostsForEndpointExpand(const string& host, ProtocolSupport protocolSupport, bool includeLoopback)
{
    vector<string> hosts;
    bool ipv4Wildcard = false;
    if(isWildcard(host, protocolSupport, ipv4Wildcard))
    {
        vector<Address> addrs = getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocolSupport, includeLoopback, false);
        for(vector<Address>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
        {
            //
            // NOTE: We don't publish link-local addresses as in most cases
            //       these are not desired to be published and in some cases
            //       will not work without extra configuration.
            //
            if(!isLinklocal(*p))
            {
                hosts.push_back(inetAddrToString(*p));
            }
        }
        if(hosts.empty())
        {
            // Return loopback if no other local addresses are available.
            addrs = getLoopbackAddresses(protocolSupport);
            for(vector<Address>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
            {
                hosts.push_back(inetAddrToString(*p));
            }
        }
    }
    return hosts; // An empty host list indicates to just use the given host.
}

vector<string>
IceInternal::getInterfacesForMulticast(const string& intf, ProtocolSupport protocolSupport)
{
    vector<string> interfaces;
    bool ipv4Wildcard = false;
    if(isWildcard(intf, protocolSupport, ipv4Wildcard))
    {
        vector<Address> addrs = getLocalAddresses(ipv4Wildcard ? EnableIPv4 : protocolSupport, true, true);
        for(vector<Address>::const_iterator p = addrs.begin(); p != addrs.end(); ++p)
        {
            interfaces.push_back(inetAddrToString(*p)); // We keep link local addresses for multicast
        }
    }
    if(interfaces.empty())
    {
        interfaces.push_back(intf);
    }
    return interfaces;
}
#endif

string
IceInternal::inetAddrToString(const Address& ss)
{
#ifndef ICE_OS_UWP
    int size = getAddressStorageSize(ss);
    if(size == 0)
    {
        return "";
    }

    char namebuf[1024];
    namebuf[0] = '\0';
    getnameinfo(&ss.sa, size, namebuf, static_cast<socklen_t>(sizeof(namebuf)), 0, 0, NI_NUMERICHOST);
    return string(namebuf);
#else
    if(ss.host == nullptr)
    {
        return "";
    }
    else
    {
        //
        // Don't need to pass a wide string converter as the wide string come
        // from Windows API.
        //
        return wstringToString(ss.host->RawName->Data(), getProcessStringConverter());
    }
#endif
}

int
IceInternal::getPort(const Address& addr)
{
#ifndef ICE_OS_UWP
    if(addr.saStorage.ss_family == AF_INET)
    {
        return ntohs(addr.saIn.sin_port);
    }
    else if(addr.saStorage.ss_family == AF_INET6)
    {
        return ntohs(addr.saIn6.sin6_port);
    }
    else
    {
        return -1;
    }
#else
    IceUtil::Int64 port = 0;
    //
    // Don't need to use any string converter here as the port number use just ASCII characters.
    //
    if(addr.port == nullptr || !IceUtilInternal::stringToInt64(wstringToString(addr.port->Data()), port))
    {
        return -1;
    }
    return static_cast<int>(port);
#endif
}

void
IceInternal::setPort(Address& addr, int port)
{
#ifndef ICE_OS_UWP
    if(addr.saStorage.ss_family == AF_INET)
    {
        addr.saIn.sin_port = htons(port);
    }
    else
    {
        assert(addr.saStorage.ss_family == AF_INET6);
        addr.saIn6.sin6_port = htons(port);
    }
#else
    ostringstream os;
    os << port;
    //
    // Don't need to use any string converter here as the port number use just
    // ASCII characters.
    //
    addr.port = ref new String(stringToWstring(os.str()).c_str());
#endif
}

bool
IceInternal::isMulticast(const Address& addr)
{
#ifndef ICE_OS_UWP
    if(addr.saStorage.ss_family == AF_INET)
    {
        return IN_MULTICAST(ntohl(addr.saIn.sin_addr.s_addr));
    }
    else if(addr.saStorage.ss_family == AF_INET6)
    {
        return IN6_IS_ADDR_MULTICAST(&addr.saIn6.sin6_addr);
    }
#else
    if(addr.host == nullptr)
    {
        return false;
    }
    //
    // Don't need to use string converters here, this is just to do a local
    // comparison to find if the address is multicast.
    //
    string host = wstringToString(addr.host->RawName->Data());
    string ip = IceUtilInternal::toUpper(host);
    vector<string> tokens;
    IceUtilInternal::splitString(ip, ".", tokens);
    if(tokens.size() == 4)
    {
        IceUtil::Int64 j;
        if(IceUtilInternal::stringToInt64(tokens[0], j))
        {
            if(j >= 233 && j <= 239)
            {
                return true;
            }
        }
    }
    if(ip.find("::") != string::npos)
    {
        return ip.compare(0, 2, "FF") == 0;
    }
#endif
    return false;
}

void
IceInternal::setTcpBufSize(SOCKET fd, const ProtocolInstancePtr& instance)
{
    assert(fd != INVALID_SOCKET);

    //
    // By default, on Windows we use a 128KB buffer size. On Unix
    // platforms, we use the system defaults.
    //
#ifdef _WIN32
    const int dfltBufSize = 128 * 1024;
#else
    const int dfltBufSize = 0;
#endif
    Int rcvSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.RcvSize", dfltBufSize);
    Int sndSize = instance->properties()->getPropertyAsIntWithDefault("Ice.TCP.SndSize", dfltBufSize);

    setTcpBufSize(fd, rcvSize, sndSize, instance);
}

void
IceInternal::setTcpBufSize(SOCKET fd, int rcvSize, int sndSize, const ProtocolInstancePtr& instance)
{
    assert(fd != INVALID_SOCKET);

    if(rcvSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setRecvBufferSize(fd, rcvSize);
        int size = getRecvBufferSize(fd);
        if(size > 0 && size < rcvSize)
        {
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            BufSizeWarnInfo winfo = instance->getBufSizeWarn(TCPEndpointType);
            if(!winfo.rcvWarn || rcvSize != winfo.rcvSize)
            {
                Ice::Warning out(instance->logger());
                out << "TCP receive buffer size: requested size of " << rcvSize << " adjusted to " << size;
                instance->setRcvBufSizeWarn(TCPEndpointType, rcvSize);
            }
        }
    }

    if(sndSize > 0)
    {
        //
        // Try to set the buffer size. The kernel will silently adjust
        // the size to an acceptable value. Then read the size back to
        // get the size that was actually set.
        //
        setSendBufferSize(fd, sndSize);
        int size = getSendBufferSize(fd);
        if(size > 0 && size < sndSize)
        {
            // Warn if the size that was set is less than the requested size and
            // we have not already warned.
            BufSizeWarnInfo winfo = instance->getBufSizeWarn(TCPEndpointType);
            if(!winfo.sndWarn || sndSize != winfo.sndSize)
            {
                Ice::Warning out(instance->logger());
                out << "TCP send buffer size: requested size of " << sndSize << " adjusted to " << size;
                instance->setSndBufSizeWarn(TCPEndpointType, sndSize);
            }
        }
    }
}

#ifdef ICE_OS_UWP
void
IceInternal::setBlock(SOCKET fd, bool)
{
}
#else
void
IceInternal::setBlock(SOCKET fd, bool block)
{
#ifdef _WIN32
    if(block)
    {
        unsigned long arg = 0;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, WSAGetLastError());
        }
    }
    else
    {
        unsigned long arg = 1;
        if(ioctlsocket(fd, FIONBIO, &arg) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, WSAGetLastError());
        }
    }
#else
    if(block)
    {
        int flags = fcntl(fd, F_GETFL);
        flags &= ~O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }
    else
    {
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, errno);
        }
    }
#endif
}
#endif

void
IceInternal::setSendBufferSize(SOCKET fd, int sz)
{
#ifndef ICE_OS_UWP
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&sz), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
#else
    StreamSocket^ stream = dynamic_cast<StreamSocket^>(fd);
    if(stream != nullptr)
    {
        stream->Control->OutboundBufferSizeInBytes = sz;
    }
#endif
}

int
IceInternal::getSendBufferSize(SOCKET fd)
{
#ifndef ICE_OS_UWP
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&sz), &len) == SOCKET_ERROR ||
       static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    return sz;
#else
    StreamSocket^ stream = dynamic_cast<StreamSocket^>(fd);
    if(stream != nullptr)
    {
        return stream->Control->OutboundBufferSizeInBytes;
    }
    return 0; // Not supported
#endif
}

#ifdef ICE_OS_UWP
void
IceInternal::setRecvBufferSize(SOCKET, int)
{
}
#else
void
IceInternal::setRecvBufferSize(SOCKET fd, int sz)
{
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&sz), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

int
IceInternal::getRecvBufferSize(SOCKET fd)
{
#ifndef ICE_OS_UWP
    int sz;
    socklen_t len = sizeof(sz);
    if(getsockopt(fd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&sz), &len) == SOCKET_ERROR ||
       static_cast<unsigned int>(len) != sizeof(sz))
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
    return sz;
#else
    return 0; // Not supported
#endif
}

#ifndef ICE_OS_UWP
void
IceInternal::setMcastGroup(SOCKET fd, const Address& group, const string& intf)
{
    vector<string> interfaces = getInterfacesForMulticast(intf, getProtocolSupport(group));
    set<int> indexes;
    for(vector<string>::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
    {
        int rc = 0;
        if(group.saStorage.ss_family == AF_INET)
        {
            struct ip_mreq mreq;
            mreq.imr_multiaddr = group.saIn.sin_addr;
            mreq.imr_interface = getInterfaceAddress(*p);
            rc = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char*>(&mreq), int(sizeof(mreq)));
        }
        else
        {
            int index = getInterfaceIndex(*p);
            if(indexes.find(index) == indexes.end()) // Don't join twice the same interface (if it has multiple IPs)
            {
                indexes.insert(index);
                struct ipv6_mreq mreq;
                mreq.ipv6mr_multiaddr = group.saIn6.sin6_addr;
                mreq.ipv6mr_interface = index;
                rc = setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, reinterpret_cast<char*>(&mreq), int(sizeof(mreq)));
            }
        }
        if(rc == SOCKET_ERROR)
        {
            closeSocketNoThrow(fd);
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }
}
#else
void
IceInternal::setMcastGroup(SOCKET fd, const Address& group, const string&)
{
    try
    {
        //
        // NOTE: UWP mcast interface is set earlier in doBind.
        //
        safe_cast<DatagramSocket^>(fd)->JoinMulticastGroup(group.host);

        //
        // BUGFIX DatagramSocket will not recive any messages from a multicast group if the
        // messages originate in the same host until the socket is used to send at least one
        // message. We send a valiate connection message that the peers will ignore to workaround
        // the issue.
        //
        auto out = IceInternal::runSync(safe_cast<DatagramSocket^>(fd)->GetOutputStreamAsync(group.host, group.port));
        auto writer = ref new DataWriter(out);

        OutputStream os;
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(validateConnectionMsg);
        os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
        os.write(headerSize); // Message size.
        os.i = os.b.begin();

        writer->WriteBytes(ref new Array<unsigned char>(&*os.i, static_cast<unsigned int>(headerSize)));

        IceInternal::runSync(writer->StoreAsync());
    }
    catch(Platform::Exception^ pex)
    {
        throw SocketException(__FILE__, __LINE__, (int)SocketError::GetStatus(pex->HResult));
    }
}
#endif

#ifdef ICE_OS_UWP
void
IceInternal::setMcastInterface(SOCKET, const string&, const Address&)
{
}
#else
void
IceInternal::setMcastInterface(SOCKET fd, const string& intf, const Address& addr)
{
    int rc;
    if(addr.saStorage.ss_family == AF_INET)
    {
        struct in_addr iface = getInterfaceAddress(intf);
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char*>(&iface), int(sizeof(iface)));
    }
    else
    {
        int interfaceNum = getInterfaceIndex(intf);
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, reinterpret_cast<char*>(&interfaceNum), int(sizeof(int)));
    }
    if(rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

#ifdef ICE_OS_UWP
void
IceInternal::setMcastTtl(SOCKET, int, const Address&)
{
}
#else
void
IceInternal::setMcastTtl(SOCKET fd, int ttl, const Address& addr)
{
    int rc;
    if(addr.saStorage.ss_family == AF_INET)
    {
        rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<char*>(&ttl), int(sizeof(int)));
    }
    else
    {
        rc = setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, reinterpret_cast<char*>(&ttl), int(sizeof(int)));
    }
    if(rc == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

#ifdef ICE_OS_UWP
void
IceInternal::setReuseAddress(SOCKET, bool)
{
}
#else
void
IceInternal::setReuseAddress(SOCKET fd, bool reuse)
{
    int flag = reuse ? 1 : 0;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flag), int(sizeof(int))) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

#ifdef ICE_OS_UWP
namespace
{

void
checkResultAndWait(IAsyncAction^ action)
{
    auto status = action->Status;
    switch(status)
    {
        case Windows::Foundation::AsyncStatus::Started:
        {
            promise<HRESULT> p;
            action->Completed = ref new AsyncActionCompletedHandler(
                [&p] (IAsyncAction^ action, Windows::Foundation::AsyncStatus status)
                    {
                        p.set_value(status != Windows::Foundation::AsyncStatus::Completed ? action->ErrorCode.Value : 0);
                    });

            HRESULT result = p.get_future().get();
            if(result)
            {
                checkErrorCode(__FILE__, __LINE__, result);
            }
            break;
        }
        case Windows::Foundation::AsyncStatus::Error:
        {
            checkErrorCode(__FILE__, __LINE__, action->ErrorCode.Value);
            break;
        }
        default:
        {
            break;
        }
    }
}

}
#endif

#ifdef ICE_OS_UWP
Address
IceInternal::doBind(SOCKET fd, const Address& addr, const string& intf)
{
    Address local;
    try
    {
        StreamSocketListener^ listener = dynamic_cast<StreamSocketListener^>(fd);
        if(listener != nullptr)
        {
            if(addr.host == nullptr) // inaddr_any
            {
                checkResultAndWait(listener->BindServiceNameAsync(addr.port));
            }
            else
            {
                checkResultAndWait(listener->BindEndpointAsync(addr.host, addr.port));
            }
            local.host = addr.host;
            local.port = listener->Information->LocalPort;
        }

        DatagramSocket^ datagram = dynamic_cast<DatagramSocket^>(fd);
        if(datagram != nullptr)
        {
            if(addr.host == nullptr) // inaddr_any
            {
                NetworkAdapter^ adapter;
                if(!intf.empty())
                {
                    auto s = ref new String(Ice::stringToWstring(intf).c_str());
                    auto profiles = NetworkInformation::GetConnectionProfiles();
                    for(auto i = profiles->First(); adapter == nullptr && i->HasCurrent; i->MoveNext())
                    {
                        auto names = i->Current->GetNetworkNames();
                        for(auto j = names->First(); adapter == nullptr && j->HasCurrent; j->MoveNext())
                        {
                            if(j->Current->Equals(s))
                            {
                                adapter = i->Current->NetworkAdapter;
                            }
                        }
                    }
                }

                if(adapter)
                {
                    checkResultAndWait(datagram->BindServiceNameAsync(addr.port, adapter));
                }
                else
                {
                    checkResultAndWait(datagram->BindServiceNameAsync(addr.port));
                }
            }
            else
            {
                checkResultAndWait(datagram->BindEndpointAsync(addr.host, addr.port));
            }
            local.host = datagram->Information->LocalAddress;
            local.port = datagram->Information->LocalPort;
        }
    }
    catch(const Ice::SocketException&)
    {
        closeSocketNoThrow(fd);
        throw;
    }
    return local;
}
#else
Address
IceInternal::doBind(SOCKET fd, const Address& addr, const string&)
{
    int size = getAddressStorageSize(addr);
    assert(size != 0);

    if(::bind(fd, &addr.sa, size) == SOCKET_ERROR)
    {
        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    Address local;
    socklen_t len = static_cast<socklen_t>(sizeof(sockaddr_storage));
#  ifdef NDEBUG
    getsockname(fd, &local.sa, &len);
#  else
    int ret = getsockname(fd, &local.sa, &len);
    assert(ret != SOCKET_ERROR);
#  endif
    return local;
}
#endif

#ifndef ICE_OS_UWP

Address
IceInternal::getNumericAddress(const std::string& address)
{
    vector<Address> addrs = getAddresses(address, 0, EnableBoth, Ice::ICE_ENUM(EndpointSelectionType, Ordered), false,
                                         false);
    if(addrs.empty())
    {
        return Address();
    }
    else
    {
        return addrs[0];
    }
}

int
IceInternal::getSocketErrno()
{
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool
IceInternal::interrupted()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAEINTR;
#else
#   ifdef EPROTO
    return errno == EINTR || errno == EPROTO;
#   else
    return errno == EINTR;
#   endif
#endif
}

bool
IceInternal::acceptInterrupted()
{
    if(interrupted())
    {
        return true;
    }

#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNABORTED ||
           error == WSAECONNRESET ||
           error == WSAETIMEDOUT;
#else
    return errno == ECONNABORTED ||
           errno == ECONNRESET ||
           errno == ETIMEDOUT;
#endif
}

bool
IceInternal::noBuffers()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAENOBUFS ||
           error == WSAEFAULT;
#else
    return errno == ENOBUFS;
#endif
}

bool
IceInternal::wouldBlock()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

bool
IceInternal::connectFailed()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED ||
           error == WSAETIMEDOUT ||
           error == WSAENETUNREACH ||
           error == WSAEHOSTUNREACH ||
           error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAECONNABORTED ||
           error == ERROR_SEM_TIMEOUT ||
           error == ERROR_NETNAME_DELETED;
#else
    return errno == ECONNREFUSED ||
           errno == ETIMEDOUT ||
           errno == ENETUNREACH ||
           errno == EHOSTUNREACH ||
           errno == ECONNRESET ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED;
#endif
}

bool
IceInternal::connectionRefused()
{
#if defined(_WIN32)
    int error = WSAGetLastError();
    return error == WSAECONNREFUSED || error == ERROR_CONNECTION_REFUSED;
#else
    return errno == ECONNREFUSED;
#endif
}

bool
IceInternal::connectionLost()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAECONNRESET ||
           error == WSAESHUTDOWN ||
           error == WSAENOTCONN ||
#   ifdef ICE_USE_IOCP
           error == ERROR_NETNAME_DELETED ||
#   endif
           error == WSAECONNABORTED;
#else
    return errno == ECONNRESET ||
           errno == ENOTCONN ||
           errno == ESHUTDOWN ||
           errno == ECONNABORTED ||
           errno == EPIPE;
#endif
}

bool
IceInternal::connectInProgress()
{
#ifdef _WIN32
    int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK || error == WSA_IO_PENDING || error == ERROR_IO_PENDING;
#else
    return errno == EINPROGRESS;
#endif
}

bool
IceInternal::notConnected()
{
#ifdef _WIN32
    return WSAGetLastError() == WSAENOTCONN;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    return errno == ENOTCONN || errno == EINVAL;
#else
    return errno == ENOTCONN;
#endif
}

bool
IceInternal::recvTruncated()
{
#ifdef _WIN32
    int err = WSAGetLastError();
    return  err == WSAEMSGSIZE || err == ERROR_MORE_DATA;
#else
    // We don't get an error under Linux if a datagram is truncated.
    return false;
#endif
}

void
IceInternal::doListen(SOCKET fd, int backlog)
{
repeatListen:
    if(::listen(fd, backlog) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatListen;
        }

        closeSocketNoThrow(fd);
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}

bool
IceInternal::doConnect(SOCKET fd, const Address& addr, const Address& sourceAddr)
{
    if(isAddressValid(sourceAddr))
    {
        doBind(fd, sourceAddr);
    }

repeatConnect:
    int size = getAddressStorageSize(addr);
    assert(size != 0);

    if(::connect(fd, &addr.sa, size) == SOCKET_ERROR)
    {
        if(interrupted())
        {
            goto repeatConnect;
        }

        if(connectInProgress())
        {
            return false;
        }

        closeSocketNoThrow(fd);
        if(connectionRefused())
        {
            throw ConnectionRefusedException(__FILE__, __LINE__, getSocketErrno());
        }
        else if(connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    Address localAddr;
    try
    {
        fdToLocalAddress(fd, localAddr);
        if(compareAddress(addr, localAddr) == 0)
        {
            throw ConnectionRefusedException(__FILE__, __LINE__, 0); // No appropriate errno
        }
    }
    catch(const LocalException&)
    {
        closeSocketNoThrow(fd);
        throw;
    }
#endif
    return true;
}

void
IceInternal::doFinishConnect(SOCKET fd)
{
    //
    // Note: we don't close the socket if there's an exception. It's the responsability
    // of the caller to do so.
    //

    //
    // Strange windows bug: The following call to Sleep() is
    // necessary, otherwise no error is reported through
    // getsockopt.
    //
#if defined(_WIN32)
    Sleep(0);
#endif

    int val;
    socklen_t len = static_cast<socklen_t>(sizeof(int));
    if(getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    if(val > 0)
    {
#if defined(_WIN32)
        WSASetLastError(val);
#else
        errno = val;
#endif
        if(connectionRefused())
        {
            throw ConnectionRefusedException(__FILE__, __LINE__, getSocketErrno());
        }
        else if(connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

#if defined(__linux)
    //
    // Prevent self connect (self connect happens on Linux when a client tries to connect to
    // a server which was just deactivated if the client socket re-uses the same ephemeral
    // port as the server).
    //
    Address localAddr;
    fdToLocalAddress(fd, localAddr);
    Address remoteAddr;
    if(fdToRemoteAddress(fd, remoteAddr) && compareAddress(remoteAddr, localAddr) == 0)
    {
        throw ConnectionRefusedException(__FILE__, __LINE__, 0); // No appropriate errno
    }
#endif
}

SOCKET
IceInternal::doAccept(SOCKET fd)
{
#ifdef _WIN32
    SOCKET ret;
#else
    int ret;
#endif

repeatAccept:
    if((ret = ::accept(fd, 0, 0)) == INVALID_SOCKET)
    {
        if(acceptInterrupted())
        {
            goto repeatAccept;
        }

        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    setTcpNoDelay(ret);
    setKeepAlive(ret);
    return ret;
}

void
IceInternal::createPipe(SOCKET fds[2])
{
#ifdef _WIN32

    SOCKET fd = createSocketImpl(false, AF_INET);
    setBlock(fd, true);

    Address addr;
    memset(&addr.saStorage, 0, sizeof(sockaddr_storage));

    addr.saIn.sin_family = AF_INET;
    addr.saIn.sin_port = htons(0);
    addr.saIn.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    addr = doBind(fd, addr);
    doListen(fd, 1);

    try
    {
        fds[0] = createSocketImpl(false, AF_INET);
    }
    catch(...)
    {
        ::closesocket(fd);
        throw;
    }

    try
    {
        setBlock(fds[0], true);
#  ifndef NDEBUG
        bool connected = doConnect(fds[0], addr, Address());
        assert(connected);
#  else
        doConnect(fds[0], addr, Address());
#  endif
    }
    catch(...)
    {
        // fds[0] is closed by doConnect
        ::closesocket(fd);
        throw;
    }

    try
    {
        fds[1] = doAccept(fd);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        ::closesocket(fd);
        throw;
    }

    ::closesocket(fd);

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        ::closesocket(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#else

    if(::pipe(fds) != 0)
    {
        throw SyscallException(__FILE__, __LINE__, getSocketErrno());
    }

    try
    {
        setBlock(fds[0], true);
    }
    catch(...)
    {
        // fds[0] is closed by setBlock
        closeSocketNoThrow(fds[1]);
        throw;
    }

    try
    {
        setBlock(fds[1], true);
    }
    catch(...)
    {
        closeSocketNoThrow(fds[0]);
        // fds[1] is closed by setBlock
        throw;
    }

#endif
}

#else // ICE_OS_UWP

void
IceInternal::checkConnectErrorCode(const char* file, int line, HRESULT herr)
{
    if(herr == E_ACCESSDENIED)
    {
        throw SocketException(file, line, static_cast<int>(herr));
    }
    SocketErrorStatus error = SocketError::GetStatus(herr);
    if(error == SocketErrorStatus::ConnectionRefused)
    {
        throw ConnectionRefusedException(file, line, static_cast<int>(error));
    }
    else if(error == SocketErrorStatus::NetworkDroppedConnectionOnReset ||
            error == SocketErrorStatus::ConnectionTimedOut ||
            error == SocketErrorStatus::NetworkIsUnreachable ||
            error == SocketErrorStatus::UnreachableHost ||
            error == SocketErrorStatus::ConnectionResetByPeer ||
            error == SocketErrorStatus::SoftwareCausedConnectionAbort)
    {
        throw ConnectFailedException(file, line, static_cast<int>(error));
    }
    else if(error == SocketErrorStatus::HostNotFound)
    {
        throw DNSException(file, line, static_cast<int>(error), "");
    }
    else
    {
        throw SocketException(file, line, static_cast<int>(error));
    }
}

void
IceInternal::checkErrorCode(const char* file, int line, HRESULT herr)
{
    if(herr == E_ACCESSDENIED)
    {
        throw SocketException(file, line, static_cast<int>(herr));
    }
    SocketErrorStatus error = SocketError::GetStatus(herr);
    if(error == SocketErrorStatus::NetworkDroppedConnectionOnReset ||
       error == SocketErrorStatus::SoftwareCausedConnectionAbort ||
       error == SocketErrorStatus::ConnectionResetByPeer)
    {
        throw ConnectionLostException(file, line, static_cast<int>(error));
    }
    else if(error == SocketErrorStatus::HostNotFound)
    {
        throw DNSException(file, line, static_cast<int>(error), "");
    }
    else
    {
        throw SocketException(file, line, static_cast<int>(error));
    }
}

//
// UWP impose some restriction on operations that block when run from
// STA thread and throws concurrency::invalid_operation. We cannot
// directly call task::get or task::way, this helper method is used to
// workaround this limitation.
//
void
IceInternal::runSync(Windows::Foundation::IAsyncAction^ action)
{
    std::promise<void> p;

    concurrency::create_task(action).then(
        [&p](concurrency::task<void> t)
        {
            try
            {
                t.get();
                p.set_value();
            }
            catch(...)
            {
                p.set_exception(std::current_exception());
            }
        },
        concurrency::task_continuation_context::use_arbitrary());

    return p.get_future().get();
}

#endif

#if defined(ICE_USE_IOCP)
void
IceInternal::doConnectAsync(SOCKET fd, const Address& addr, const Address& sourceAddr, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //
    Address bindAddr;
    if(isAddressValid(sourceAddr))
    {
        bindAddr = sourceAddr;
    }
    else
    {
        memset(&bindAddr.saStorage, 0, sizeof(sockaddr_storage));
        if(addr.saStorage.ss_family == AF_INET)
        {
            bindAddr.saIn.sin_family = AF_INET;
            bindAddr.saIn.sin_port = htons(0);
            bindAddr.saIn.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else if(addr.saStorage.ss_family == AF_INET6)
        {
            bindAddr.saIn6.sin6_family = AF_INET6;
            bindAddr.saIn6.sin6_port = htons(0);
            bindAddr.saIn6.sin6_addr = in6addr_any;
        }
    }

    int size = getAddressStorageSize(bindAddr);
    assert(size != 0);

    if(::bind(fd, &bindAddr.sa, size) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    LPFN_CONNECTEX ConnectEx = ICE_NULLPTR; // a pointer to the 'ConnectEx()' function
    GUID GuidConnectEx = WSAID_CONNECTEX; // The Guid
    DWORD dwBytes;
    if(WSAIoctl(fd,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                &GuidConnectEx,
                sizeof(GuidConnectEx),
                &ConnectEx,
                sizeof(ConnectEx),
                &dwBytes,
                ICE_NULLPTR,
                ICE_NULLPTR) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }

    if(!ConnectEx(fd, &addr.sa, size, 0, 0, 0, &info))
    {
        if(!connectInProgress())
        {
            if(connectionRefused())
            {
                throw ConnectionRefusedException(__FILE__, __LINE__, getSocketErrno());
            }
            else if(connectFailed())
            {
                throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
            }
            else
            {
                throw SocketException(__FILE__, __LINE__, getSocketErrno());
            }
        }
    }
}

void
IceInternal::doFinishConnectAsync(SOCKET fd, AsyncInfo& info)
{
    //
    // NOTE: It's the caller's responsability to close the socket upon
    // failure to connect. The socket isn't closed by this method.
    //

    if(info.error != ERROR_SUCCESS)
    {
        WSASetLastError(info.error);
        if(connectionRefused())
        {
            throw ConnectionRefusedException(__FILE__, __LINE__, getSocketErrno());
        }
        else if(connectFailed())
        {
            throw ConnectFailedException(__FILE__, __LINE__, getSocketErrno());
        }
        else
        {
            throw SocketException(__FILE__, __LINE__, getSocketErrno());
        }
    }

    if(setsockopt(fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, ICE_NULLPTR, 0) == SOCKET_ERROR)
    {
        throw SocketException(__FILE__, __LINE__, getSocketErrno());
    }
}
#endif

bool
IceInternal::isIpAddress(const string& name)
{
#ifdef ICE_OS_UWP
     HostName^ hostname = ref new HostName(ref new String(stringToWstring(name,
                                                          getProcessStringConverter()).c_str()));
     return hostname->Type == HostNameType::Ipv4 || hostname->Type == HostNameType::Ipv6;
#else
    in_addr addr;
    in6_addr addr6;

    return inet_pton(AF_INET, name.c_str(), &addr) > 0 || inet_pton(AF_INET6, name.c_str(), &addr6) > 0;
#endif
}
