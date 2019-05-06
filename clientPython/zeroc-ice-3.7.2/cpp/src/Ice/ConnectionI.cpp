//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <Ice/ConnectionI.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/ACM.h>
#include <Ice/ObjectAdapterI.h> // For getThreadPool() and getServantManager().
#include <Ice/EndpointI.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Incoming.h>
#include <Ice/LocalException.h>
#include <Ice/RequestHandler.h> // For RetryException
#include <Ice/ReferenceFactory.h> // For createProxy().
#include <Ice/ProxyFactory.h> // For createProxy().
#include <Ice/BatchRequestQueue.h>

#ifdef ICE_HAS_BZIP2
#  include <bzlib.h>
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

#ifndef ICE_CPP11_MAPPING
Ice::LocalObject* Ice::upCast(ConnectionI* p) { return p; }
#endif

namespace
{

const ::std::string flushBatchRequests_name = "flushBatchRequests";

class TimeoutCallback : public IceUtil::TimerTask
{
public:

    TimeoutCallback(Ice::ConnectionI* connection) : _connection(connection)
    {
    }

    void
    runTimerTask()
    {
        _connection->timedOut();
    }

private:

    Ice::ConnectionI* _connection;
};

class DispatchCall : public DispatchWorkItem
{
public:

    DispatchCall(const ConnectionIPtr& connection, const ConnectionI::StartCallbackPtr& startCB,
                 const vector<ConnectionI::OutgoingMessage>& sentCBs, Byte compress, Int requestId,
                 Int invokeNum, const ServantManagerPtr& servantManager, const ObjectAdapterPtr& adapter,
                 const OutgoingAsyncBasePtr& outAsync, const ICE_DELEGATE(HeartbeatCallback)& heartbeatCallback,
                 InputStream& stream) :
        DispatchWorkItem(connection),
        _connection(connection),
        _startCB(startCB),
        _sentCBs(sentCBs),
        _compress(compress),
        _requestId(requestId),
        _invokeNum(invokeNum),
        _servantManager(servantManager),
        _adapter(adapter),
        _outAsync(outAsync),
        _heartbeatCallback(heartbeatCallback),
        _stream(stream.instance(), currentProtocolEncoding)
    {
        _stream.swap(stream);
    }

    virtual void
    run()
    {
        _connection->dispatch(_startCB, _sentCBs, _compress, _requestId, _invokeNum, _servantManager, _adapter,
                              _outAsync, _heartbeatCallback, _stream);
    }

private:

    const ConnectionIPtr _connection;
    const ConnectionI::StartCallbackPtr _startCB;
    const vector<ConnectionI::OutgoingMessage> _sentCBs;
    const Byte _compress;
    const Int _requestId;
    const Int _invokeNum;
    const ServantManagerPtr _servantManager;
    const ObjectAdapterPtr _adapter;
    const OutgoingAsyncBasePtr _outAsync;
    const ICE_DELEGATE(HeartbeatCallback) _heartbeatCallback;
    InputStream _stream;
};

class FinishCall : public DispatchWorkItem
{
public:

    FinishCall(const Ice::ConnectionIPtr& connection, bool close) :
        DispatchWorkItem(connection), _connection(connection), _close(close)
    {
    }

    virtual void
    run()
    {
        _connection->finish(_close);
    }

private:

    const ConnectionIPtr _connection;
    const bool _close;
};

//
// Class for handling Ice::Connection::begin_flushBatchRequests
//
class ConnectionFlushBatchAsync : public OutgoingAsyncBase
{
public:

    ConnectionFlushBatchAsync(const Ice::ConnectionIPtr&, const InstancePtr&);

    virtual Ice::ConnectionPtr getConnection() const;

    void invoke(const std::string&, Ice::CompressBatch);

private:

    const Ice::ConnectionIPtr _connection;
};
typedef IceUtil::Handle<ConnectionFlushBatchAsync> ConnectionFlushBatchAsyncPtr;

ConnectionState connectionStateMap[] = {
    ICE_ENUM(ConnectionState, ConnectionStateValidating),   // StateNotInitialized
    ICE_ENUM(ConnectionState, ConnectionStateValidating),   // StateNotValidated
    ICE_ENUM(ConnectionState, ConnectionStateActive),       // StateActive
    ICE_ENUM(ConnectionState, ConnectionStateHolding),      // StateHolding
    ICE_ENUM(ConnectionState, ConnectionStateClosing),      // StateClosing
    ICE_ENUM(ConnectionState, ConnectionStateClosing),      // StateClosingPending
    ICE_ENUM(ConnectionState, ConnectionStateClosed),       // StateClosed
    ICE_ENUM(ConnectionState, ConnectionStateClosed),       // StateFinished
};

}

ConnectionFlushBatchAsync::ConnectionFlushBatchAsync(const ConnectionIPtr& connection, const InstancePtr& instance) :
    OutgoingAsyncBase(instance), _connection(connection)
{
}

ConnectionPtr
ConnectionFlushBatchAsync::getConnection() const
{
    return _connection;
}

void
ConnectionFlushBatchAsync::invoke(const string& operation, Ice::CompressBatch compressBatch)
{
    _observer.attach(_instance.get(), operation);
    try
    {
        AsyncStatus status;
        bool compress;
        int batchRequestNum = _connection->getBatchRequestQueue()->swap(&_os, compress);
        if(batchRequestNum == 0)
        {
            status = AsyncStatusSent;
            if(sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
        }
        else
        {
            if(compressBatch == ICE_SCOPED_ENUM(CompressBatch, Yes))
            {
                compress = true;
            }
            else if(compressBatch == ICE_SCOPED_ENUM(CompressBatch, No))
            {
                compress = false;
            }
            status = _connection->sendAsyncRequest(ICE_SHARED_FROM_THIS, compress, false, batchRequestNum);
        }

        if(status & AsyncStatusSent)
        {
            _sentSynchronously = true;
            if(status & AsyncStatusInvokeSentCallback)
            {
                invokeSent();
            }
        }
    }
    catch(const RetryException& ex)
    {
        if(exception(*ex.get()))
        {
            invokeExceptionAsync();
        }
    }
    catch(const Exception& ex)
    {
        if(exception(ex))
        {
            invokeExceptionAsync();
        }
    }
}

Ice::ConnectionI::Observer::Observer() : _readStreamPos(0), _writeStreamPos(0)
{
}

void
Ice::ConnectionI::Observer::startRead(const Buffer& buf)
{
    if(_readStreamPos)
    {
        assert(!buf.b.empty());
        _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    }
    _readStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishRead(const Buffer& buf)
{
    if(_readStreamPos == 0)
    {
        return;
    }
    assert(buf.i >= _readStreamPos);
    _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    _readStreamPos = 0;
}

void
Ice::ConnectionI::Observer::startWrite(const Buffer& buf)
{
    if(_writeStreamPos)
    {
        assert(!buf.b.empty());
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishWrite(const Buffer& buf)
{
    if(_writeStreamPos == 0)
    {
        return;
    }
    if(buf.i > _writeStreamPos)
    {
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = 0;
}

void
Ice::ConnectionI::Observer::attach(const Ice::Instrumentation::ConnectionObserverPtr& observer)
{
    ObserverHelperT<Ice::Instrumentation::ConnectionObserver>::attach(observer);
    if(!observer)
    {
        _writeStreamPos = 0;
        _readStreamPos = 0;
    }
}

void
Ice::ConnectionI::OutgoingMessage::adopt(OutputStream* str)
{
    if(adopted)
    {
        if(str)
        {
            delete stream;
            stream = 0;
            adopted = false;
        }
        else
        {
            return; // Stream is already adopted.
        }
    }
    else if(!str)
    {
        if(outAsync)
        {
            return; // Adopting request stream is not necessary.
        }
        else
        {
            str = stream; // Adopt this stream
            stream = 0;
        }
    }

    assert(str);
    stream = new OutputStream(str->instance(), currentProtocolEncoding);
    stream->swap(*str);
    adopted = true;
}

void
Ice::ConnectionI::OutgoingMessage::canceled(bool adoptStream)
{
    assert(outAsync); // Only requests can timeout.
    outAsync = 0;
    if(adoptStream)
    {
        adopt(0); // Adopt the request stream
    }
    else
    {
        assert(!adopted);
    }
}

bool
Ice::ConnectionI::OutgoingMessage::sent()
{
    if(adopted)
    {
        delete stream;
    }
    stream = 0;

    if(outAsync)
    {
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
        invokeSent = outAsync->sent();
        return invokeSent || receivedReply;
#else
        return outAsync->sent();
#endif
    }
    return false;
}

void
Ice::ConnectionI::OutgoingMessage::completed(const Ice::LocalException& ex)
{
    if(outAsync)
    {
        if(outAsync->exception(ex))
        {
            outAsync->invokeException();
        }
    }

    if(adopted)
    {
        delete stream;
    }
    stream = 0;
}

void
Ice::ConnectionI::start(const StartCallbackPtr& callback)
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state >= StateClosed) // The connection might already be closed if the communicator was destroyed.
        {
            assert(_exception);
            _exception->ice_throw();
        }

        if(!initialize() || !validate())
        {
            if(callback)
            {
                _startCallback = callback;
                return;
            }

            //
            // Wait for the connection to be validated.
            //
            while(_state <= StateNotValidated)
            {
                wait();
            }

            if(_state >= StateClosing)
            {
                assert(_exception);
                _exception->ice_throw();
            }
        }

        //
        // We start out in holding state.
        //
        setState(StateHolding);
    }
    catch(const Ice::LocalException& ex)
    {
        exception(ex);
        if(callback)
        {
            callback->connectionStartFailed(ICE_SHARED_FROM_THIS, ex);
            return;
        }
        else
        {
            waitUntilFinished();
            throw;
        }
    }

    if(callback)
    {
        callback->connectionStartCompleted(ICE_SHARED_FROM_THIS);
    }
}

void
Ice::ConnectionI::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }
    if(_acmLastActivity != IceUtil::Time())
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }
    setState(StateActive);
}

void
Ice::ConnectionI::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }

    setState(StateHolding);
}

void
Ice::ConnectionI::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    switch(reason)
    {
        case ObjectAdapterDeactivated:
        {
            setState(StateClosing, ObjectAdapterDeactivatedException(__FILE__, __LINE__));
            break;
        }

        case CommunicatorDestroyed:
        {
            setState(StateClosing, CommunicatorDestroyedException(__FILE__, __LINE__));
            break;
        }
    }
}

void
Ice::ConnectionI::close(ConnectionClose mode) ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(mode == ICE_SCOPED_ENUM(ConnectionClose, Forcefully))
    {
        setState(StateClosed, ConnectionManuallyClosedException(__FILE__, __LINE__, false));
    }
    else if(mode == ICE_SCOPED_ENUM(ConnectionClose, Gracefully))
    {
        setState(StateClosing, ConnectionManuallyClosedException(__FILE__, __LINE__, true));
    }
    else
    {
        assert(mode == ICE_SCOPED_ENUM(ConnectionClose, GracefullyWithWait));

        //
        // Wait until all outstanding requests have been completed.
        //
        while(!_asyncRequests.empty())
        {
            wait();
        }

        setState(StateClosing, ConnectionManuallyClosedException(__FILE__, __LINE__, true));
    }
}

bool
Ice::ConnectionI::isActiveOrHolding() const
{
    //
    // We can not use trylock here, otherwise the outgoing connection
    // factory might return destroyed (closing or closed) connections,
    // resulting in connection retry exhaustion.
    //
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    return _state > StateNotValidated && _state < StateClosing;
}

bool
Ice::ConnectionI::isFinished() const
{
    //
    // We can use trylock here, because as long as there are still
    // threads operating in this connection object, connection
    // destruction is considered as not yet finished.
    //
    IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);

    if(!sync.acquired())
    {
        return false;
    }

    if(_state != StateFinished || _dispatchCount != 0)
    {
        return false;
    }

    assert(_state == StateFinished);
    return true;
}

void
Ice::ConnectionI::throwException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception)
    {
        assert(_state >= StateClosing);
        _exception->ice_throw();
    }
}

void
Ice::ConnectionI::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
        wait();
    }
}

void
Ice::ConnectionI::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // We wait indefinitely until the connection is finished and all
    // outstanding requests are completed. Otherwise we couldn't
    // guarantee that there are no outstanding calls when deactivate()
    // is called on the servant locators.
    //
    while(_state < StateFinished || _dispatchCount > 0)
    {
        wait();
    }

    assert(_state == StateFinished);

    //
    // Clear the OA. See bug 1673 for the details of why this is necessary.
    //
    _adapter = 0;
}

void
Ice::ConnectionI::updateObserver()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state < StateNotValidated || _state > StateClosed)
    {
        return;
    }

    assert(_instance->initializationData().observer);

    ConnectionObserverPtr o = _instance->initializationData().observer->getConnectionObserver(initConnectionInfo(),
                                                                                              _endpoint,
                                                                                              toConnectionState(_state),
                                                                                              _observer.get());
    _observer.attach(o);
}

void
Ice::ConnectionI::monitor(const IceUtil::Time& now, const ACMConfig& acm)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state != StateActive)
    {
        return;
    }
    assert(acm.timeout != IceUtil::Time());

    //
    // We send a heartbeat if there was no activity in the last
    // (timeout / 4) period. Sending a heartbeat sooner than really
    // needed is safer to ensure that the receiver will receive the
    // heartbeat in time. Sending the heartbeat if there was no
    // activity in the last (timeout / 2) period isn't enough since
    // monitor() is called only every (timeout / 2) period.
    //
    // Note that this doesn't imply that we are sending 4 heartbeats
    // per timeout period because the monitor() method is still only
    // called every (timeout / 2) period.
    //
    if(acm.heartbeat == ICE_ENUM(ACMHeartbeat, HeartbeatAlways) ||
       (acm.heartbeat != ICE_ENUM(ACMHeartbeat, HeartbeatOff) &&
        _writeStream.b.empty() && now >= (_acmLastActivity + acm.timeout / 4)))
    {
        if(acm.heartbeat != ICE_ENUM(ACMHeartbeat, HeartbeatOnDispatch) || _dispatchCount > 0)
        {
            sendHeartbeatNow();
        }
    }

    if(static_cast<Int>(_readStream.b.size()) > headerSize || !_writeStream.b.empty())
    {
        //
        // If writing or reading, nothing to do, the connection
        // timeout will kick-in if writes or reads don't progress.
        // This check is necessary because the actitivy timer is
        // only set when a message is fully read/written.
        //
        return;
    }

    if(acm.close != ICE_ENUM(ACMClose, CloseOff) && now >= (_acmLastActivity + acm.timeout))
    {
        if(acm.close == ICE_ENUM(ACMClose, CloseOnIdleForceful) ||
           (acm.close != ICE_ENUM(ACMClose, CloseOnIdle) && !_asyncRequests.empty()))
        {
            //
            // Close the connection if we didn't receive a heartbeat in
            // the last period.
            //
            setState(StateClosed, ConnectionTimeoutException(__FILE__, __LINE__));
        }
        else if(acm.close != ICE_ENUM(ACMClose, CloseOnInvocation) &&
                _dispatchCount == 0 && _batchRequestQueue->isEmpty() && _asyncRequests.empty())
        {
            //
            // The connection is idle, close it.
            //
            setState(StateClosing, ConnectionTimeoutException(__FILE__, __LINE__));
        }
    }
}

AsyncStatus
Ice::ConnectionI::sendAsyncRequest(const OutgoingAsyncBasePtr& out, bool compress, bool response, int batchRequestNum)
{
    OutputStream* os = out->getOs();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    //
    // If the exception is closed before we even have a chance
    // to send our request, we always try to send the request
    // again.
    //
    if(_exception)
    {
        throw RetryException(*_exception);
    }
    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    //
    // Ensure the message isn't bigger than what we can send with the
    // transport.
    //
    _transceiver->checkSendSize(*os);

    //
    // Notify the request that it's cancelable with this connection.
    // This will throw if the request is canceled.
    //
    out->cancelable(ICE_SHARED_FROM_THIS);
    Int requestId = 0;
    if(response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if(requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }
    else if(batchRequestNum > 0)
    {
        const Byte* p = reinterpret_cast<const Byte*>(&batchRequestNum);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }

    out->attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

    AsyncStatus status = AsyncStatusQueued;
    try
    {
        OutgoingMessage message(out, os, compress, requestId);
        status = sendMessage(message);
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception);
        _exception->ice_throw();
    }

    if(response)
    {
        //
        // Add to the async requests map.
        //
        _asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(),
                                                   pair<const Int, OutgoingAsyncBasePtr>(requestId, out));
    }
    return status;
}

BatchRequestQueuePtr
Ice::ConnectionI::getBatchRequestQueue() const
{
    return _batchRequestQueue;
}

#ifdef ICE_CPP11_MAPPING
std::function<void()>
Ice::ConnectionI::flushBatchRequestsAsync(CompressBatch compress,
                                          ::std::function<void(::std::exception_ptr)> ex,
                                          ::std::function<void(bool)> sent)
{
    class ConnectionFlushBatchLambda : public ConnectionFlushBatchAsync, public LambdaInvoke
    {
    public:

        ConnectionFlushBatchLambda(std::shared_ptr<Ice::ConnectionI>&& connection,
                                   const InstancePtr& instance,
                                   std::function<void(std::exception_ptr)> ex,
                                   std::function<void(bool)> sent) :
            ConnectionFlushBatchAsync(connection, instance), LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<ConnectionFlushBatchLambda>(ICE_SHARED_FROM_THIS, _instance, ex, sent);
    outAsync->invoke(flushBatchRequests_name, compress);
    return [outAsync]() { outAsync->cancel(); };
}
#else
void
Ice::ConnectionI::flushBatchRequests(CompressBatch compress)
{
    end_flushBatchRequests(begin_flushBatchRequests(compress));
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(CompressBatch compress)
{
    return _iceI_begin_flushBatchRequests(compress, dummyCallback, 0);
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(CompressBatch compress,
                                           const CallbackPtr& cb,
                                           const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(compress, cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(CompressBatch compress,
                                           const Callback_Connection_flushBatchRequestsPtr& cb,
                                           const LocalObjectPtr& cookie)
{
    return _iceI_begin_flushBatchRequests(compress, cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::_iceI_begin_flushBatchRequests(CompressBatch compress,
                                                 const CallbackBasePtr& cb,
                                                 const LocalObjectPtr& cookie)
{
    class ConnectionFlushBatchAsyncWithCallback : public ConnectionFlushBatchAsync, public CallbackCompletion
    {
    public:

        ConnectionFlushBatchAsyncWithCallback(const Ice::ConnectionIPtr& connection,
                                              const Ice::CommunicatorPtr& communicator,
                                              const InstancePtr& instance,
                                              const CallbackBasePtr& callback,
                                              const Ice::LocalObjectPtr& cookie) :
            ConnectionFlushBatchAsync(connection, instance),
            CallbackCompletion(callback, cookie),
            _communicator(communicator),
            _connection(connection)
        {
            _cookie = cookie;
        }

        virtual Ice::CommunicatorPtr getCommunicator() const
        {
            return _communicator;
        }

        virtual Ice::ConnectionPtr getConnection() const
        {
            return _connection;
        }

        virtual const std::string&
        getOperation() const
        {
            return flushBatchRequests_name;
        }

    private:

        Ice::CommunicatorPtr _communicator;
        Ice::ConnectionPtr _connection;
    };

    ConnectionFlushBatchAsyncPtr result = new ConnectionFlushBatchAsyncWithCallback(this,
                                                                                    _communicator,
                                                                                    _instance,
                                                                                    cb,
                                                                                    cookie);
    result->invoke(flushBatchRequests_name, compress);
    return result;
}

void
Ice::ConnectionI::end_flushBatchRequests(const AsyncResultPtr& r)
{
    AsyncResult::_check(r, this, flushBatchRequests_name);
    r->_waitForResponse();
}
#endif

namespace
{

const ::std::string heartbeat_name = "heartbeat";

class HeartbeatAsync : public OutgoingAsyncBase
{
public:

    HeartbeatAsync(const ConnectionIPtr& connection,
                   const CommunicatorPtr& communicator,
                   const InstancePtr& instance) :
        OutgoingAsyncBase(instance),
        _communicator(communicator),
        _connection(connection)
    {
    }

    virtual CommunicatorPtr getCommunicator() const
    {
        return _communicator;
    }

    virtual ConnectionPtr getConnection() const
    {
        return _connection;
    }

    virtual const string& getOperation() const
    {
        return heartbeat_name;
    }

    void invoke()
    {
        _observer.attach(_instance.get(), heartbeat_name);
        try
        {
            _os.write(magic[0]);
            _os.write(magic[1]);
            _os.write(magic[2]);
            _os.write(magic[3]);
            _os.write(currentProtocol);
            _os.write(currentProtocolEncoding);
            _os.write(validateConnectionMsg);
            _os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
            _os.write(headerSize); // Message size.
            _os.i = _os.b.begin();

            AsyncStatus status = _connection->sendAsyncRequest(ICE_SHARED_FROM_THIS, false, false, 0);
            if(status & AsyncStatusSent)
            {
                _sentSynchronously = true;
                if(status & AsyncStatusInvokeSentCallback)
                {
                    invokeSent();
                }
            }
        }
        catch(const RetryException& ex)
        {
            if(exception(*ex.get()))
            {
                invokeExceptionAsync();
            }
        }
        catch(const Exception& ex)
        {
            if(exception(ex))
            {
                invokeExceptionAsync();
            }
        }
    }

private:

    CommunicatorPtr _communicator;
    ConnectionIPtr _connection;
};
typedef IceUtil::Handle<HeartbeatAsync> HeartbeatAsyncPtr;

}

#ifdef ICE_CPP11_MAPPING
void
Ice::ConnectionI::heartbeat()
{
    Connection::heartbeatAsync().get();
}

std::function<void()>
Ice::ConnectionI::heartbeatAsync(::std::function<void(::std::exception_ptr)> ex, ::std::function<void(bool)> sent)
{
    class HeartbeatLambda : public HeartbeatAsync, public LambdaInvoke
    {
    public:

        HeartbeatLambda(std::shared_ptr<Ice::ConnectionI>&& connection,
                        std::shared_ptr<Ice::Communicator>& communicator,
                        const InstancePtr& instance,
                        std::function<void(std::exception_ptr)> ex,
                        std::function<void(bool)> sent) :
            HeartbeatAsync(connection, communicator, instance), LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<HeartbeatLambda>(ICE_SHARED_FROM_THIS, _communicator, _instance, ex, sent);
    outAsync->invoke();
    return [outAsync]() { outAsync->cancel(); };
}
#else
void
Ice::ConnectionI::heartbeat()
{
    end_heartbeat(begin_heartbeat());
}

AsyncResultPtr
Ice::ConnectionI::begin_heartbeat()
{
    return _iceI_begin_heartbeat(dummyCallback, 0);
}

AsyncResultPtr
Ice::ConnectionI::begin_heartbeat(const CallbackPtr& cb, const LocalObjectPtr& cookie)
{
    return _iceI_begin_heartbeat(cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::begin_heartbeat(const Callback_Connection_heartbeatPtr& cb, const LocalObjectPtr& cookie)
{
    return _iceI_begin_heartbeat(cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::_iceI_begin_heartbeat(const CallbackBasePtr& cb, const LocalObjectPtr& cookie)
{
    class HeartbeatCallback : public HeartbeatAsync, public CallbackCompletion
    {
    public:

        HeartbeatCallback(const ConnectionIPtr& connection,
                          const CommunicatorPtr& communicator,
                          const InstancePtr& instance,
                          const CallbackBasePtr& callback,
                          const LocalObjectPtr& cookie) :
            HeartbeatAsync(connection, communicator, instance),
            CallbackCompletion(callback, cookie)
        {
            _cookie = cookie;
        }
    };

    HeartbeatAsyncPtr result = new HeartbeatCallback(this, _communicator, _instance, cb, cookie);
    result->invoke();
    return result;
}

void
Ice::ConnectionI::end_heartbeat(const AsyncResultPtr& r)
{
    AsyncResult::_check(r, this, heartbeat_name);
    r->_waitForResponse();
}
#endif

void
Ice::ConnectionI::setHeartbeatCallback(ICE_IN(ICE_DELEGATE(HeartbeatCallback)) callback)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        return;
    }
    _heartbeatCallback = callback;
}

void
Ice::ConnectionI::setCloseCallback(ICE_IN(ICE_DELEGATE(CloseCallback)) callback)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        if(callback)
        {
            class CallbackWorkItem : public DispatchWorkItem
            {
            public:

                CallbackWorkItem(const ConnectionIPtr& connection, ICE_IN(ICE_DELEGATE(CloseCallback)) callback) :
                    _connection(connection),
#ifdef ICE_CPP11_MAPPING
                    _callback(move(callback))
#else
                    _callback(callback)
#endif
                {
                }

                virtual void run()
                {
                    _connection->closeCallback(_callback);
                }

            private:

                const ConnectionIPtr _connection;
                const ICE_DELEGATE(CloseCallback) _callback;
            };
#ifdef ICE_CPP11_MAPPING
            _threadPool->dispatch(new CallbackWorkItem(ICE_SHARED_FROM_THIS, move(callback)));
#else
            _threadPool->dispatch(new CallbackWorkItem(ICE_SHARED_FROM_THIS, callback));
#endif
        }
    }
    else
    {
        _closeCallback = callback;
    }
}

void
Ice::ConnectionI::closeCallback(const ICE_DELEGATE(CloseCallback)& callback)
{
    try
    {
#ifdef ICE_CPP11_MAPPING
        callback(ICE_SHARED_FROM_THIS);
#else
        callback->closed(ICE_SHARED_FROM_THIS);
#endif
    }
    catch(const std::exception& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\n" << ex << '\n' << _desc;
    }
    catch(...)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\nunknown c++ exception" << '\n' << _desc;
    }
}

void
Ice::ConnectionI::setACM(const IceUtil::Optional<int>& timeout,
                         const IceUtil::Optional<Ice::ACMClose>& close,
                         const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(timeout && *timeout < 0)
    {
#ifdef ICE_CPP11_MAPPING
        throw invalid_argument("invalid negative ACM timeout value");
#else
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "invalid negative ACM timeout value");
#endif
    }
    if(!_monitor || _state >= StateClosed)
    {
        return;
    }

    if(_state == StateActive)
    {
        _monitor->remove(ICE_SHARED_FROM_THIS);
    }
    _monitor = _monitor->acm(timeout, close, heartbeat);

    if(_monitor->getACM().timeout <= 0)
    {
        _acmLastActivity = IceUtil::Time(); // Disable the recording of last activity.
    }
    else if(_acmLastActivity == IceUtil::Time() && _state == StateActive)
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }

    if(_state == StateActive)
    {
        _monitor->add(ICE_SHARED_FROM_THIS);
    }
}

ACM
Ice::ConnectionI::getACM() ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    ACM acm;
    acm.timeout = 0;
    acm.close = ICE_ENUM(ACMClose, CloseOff);
    acm.heartbeat = ICE_ENUM(ACMHeartbeat, HeartbeatOff);
    return _monitor ? _monitor->getACM() : acm;
}

void
Ice::ConnectionI::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const LocalException& ex)
{
    //
    // NOTE: This isn't called from a thread pool thread.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        return; // The request has already been or will be shortly notified of the failure.
    }

    for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        if(o->outAsync.get() == outAsync.get())
        {
            if(o->requestId)
            {
                if(_asyncRequestsHint != _asyncRequests.end() &&
                   _asyncRequestsHint->second == ICE_DYNAMIC_CAST(OutgoingAsync, outAsync))
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                }
                else
                {
                    _asyncRequests.erase(o->requestId);
                }
            }

            if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
            {
                setState(StateClosed, ex);
            }
            else
            {
                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                if(o == _sendStreams.begin())
                {
                    o->canceled(true); // true = adopt the stream
                }
                else
                {
                    o->canceled(false);
                    _sendStreams.erase(o);
                }
                if(outAsync->exception(ex))
                {
                    outAsync->invokeExceptionAsync();
                }
            }
            return;
        }
    }

    if(ICE_DYNAMIC_CAST(OutgoingAsync, outAsync))
    {
        if(_asyncRequestsHint != _asyncRequests.end())
        {
            if(_asyncRequestsHint->second == outAsync)
            {
                if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                    if(outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
                    }
                }
                return;
            }
        }

        for(map<Int, OutgoingAsyncBasePtr>::iterator p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
        {
            if(p->second.get() == outAsync.get())
            {
                if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    assert(p != _asyncRequestsHint);
                    _asyncRequests.erase(p);
                    if(outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
                    }
                }
                return;
            }
        }
    }
}

void
Ice::ConnectionI::sendResponse(Int, OutputStream* os, Byte compressFlag, bool /*amd*/)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }

        if(_state >= StateClosed)
        {
            assert(_exception);
            _exception->ice_throw();
        }

        OutgoingMessage message(os, compressFlag > 0);
        sendMessage(message);

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }

        return;
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

void
Ice::ConnectionI::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }

        if(_state >= StateClosed)
        {
            assert(_exception);
            _exception->ice_throw();
        }

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

bool
Ice::ConnectionI::systemException(Int, const SystemException&, bool /*amd*/)
{
    return false; // System exceptions aren't marshalled.
}

void
Ice::ConnectionI::invokeException(Ice::Int, const LocalException& ex, int invokeNum, bool /*amd*/)
{
    //
    // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
    // called in case of a fatal exception we decrement _dispatchCount here.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);

    if(invokeNum > 0)
    {
        assert(_dispatchCount >= invokeNum);
        _dispatchCount -= invokeNum;
        if(_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }
    }
}

EndpointIPtr
Ice::ConnectionI::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

ConnectorPtr
Ice::ConnectionI::connector() const
{
    return _connector; // No mutex protection necessary, _connector is immutable.
}

void
Ice::ConnectionI::setAdapter(const ObjectAdapterPtr& adapter)
{
    if(adapter)
    {
        // Go through the adapter to set the adapter and servant manager on this connection
        // to ensure the object adapter is still active.
        dynamic_cast<ObjectAdapterI*>(adapter.get())->setAdapterOnConnection(ICE_SHARED_FROM_THIS);
    }
    else
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state <= StateNotValidated || _state >= StateClosing)
        {
            return;
        }

        _adapter = 0;
        _servantManager = 0;
    }

    //
    // We never change the thread pool with which we were initially
    // registered, even if we add or remove an object adapter.
    //
}

ObjectAdapterPtr
Ice::ConnectionI::getAdapter() const ICE_NOEXCEPT
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _adapter;
}

EndpointPtr
Ice::ConnectionI::getEndpoint() const ICE_NOEXCEPT
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

ObjectPrxPtr
Ice::ConnectionI::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    return _instance->proxyFactory()->referenceToProxy(
        _instance->referenceFactory()->create(ident, ICE_SHARED_FROM_CONST_THIS(ConnectionI)));
}

void
Ice::ConnectionI::setAdapterAndServantManager(const ObjectAdapterPtr& adapter,
                                              const IceInternal::ServantManagerPtr& servantManager)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated || _state >= StateClosing)
    {
        return;
    }
    assert(adapter); // Called by ObjectAdapterI::setAdapterOnConnection
    _adapter = adapter;
    _servantManager = servantManager;
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
bool
Ice::ConnectionI::startAsync(SocketOperation operation)
{
    if(_state >= StateClosed)
    {
        return false;
    }

    try
    {
        if(operation & SocketOperationWrite)
        {
            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if(_transceiver->startWrite(_writeStream) && !_sendStreams.empty())
            {
                // The whole message is written, assume it's sent now for at-most-once semantics.
                _sendStreams.front().isSent = true;
            }
        }
        else if(operation & SocketOperationRead)
        {
            if(_observer && !_readHeader)
            {
                _observer.startRead(_readStream);
            }

            _transceiver->startRead(_readStream);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        return false;
    }
    return true;
}

bool
Ice::ConnectionI::finishAsync(SocketOperation operation)
{
    try
    {
        if(operation & SocketOperationWrite)
        {
            Buffer::Container::iterator start = _writeStream.i;
            _transceiver->finishWrite(_writeStream);
            if(_instance->traceLevels()->network >= 3 && _writeStream.i != start)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "sent " << (_writeStream.i - start);
                if(!_endpoint->datagram())
                {
                    out << " of " << (_writeStream.b.end() - start);
                }
                out << " bytes via " << _endpoint->protocol() << "\n" << toString();
            }

            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else if(operation & SocketOperationRead)
        {
            Buffer::Container::iterator start = _readStream.i;
            _transceiver->finishRead(_readStream);
            if(_instance->traceLevels()->network >= 3 && _readStream.i != start)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "received ";
                if(_endpoint->datagram())
                {
                    out << _readStream.b.size();
                }
                else
                {
                    out << (_readStream.i - start) << " of " << (_readStream.b.end() - start);
                }
                out << " bytes via " << _endpoint->protocol() << "\n" << toString();
            }

            if(_observer && !_readHeader)
            {
                _observer.finishRead(_readStream);
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
    }
    return _state < StateClosed;
}
#endif

void
Ice::ConnectionI::message(ThreadPoolCurrent& current)
{
    StartCallbackPtr startCB;
    vector<OutgoingMessage> sentCBs;
    Byte compress = 0;
    Int requestId = 0;
    Int invokeNum = 0;
    ServantManagerPtr servantManager;
    ObjectAdapterPtr adapter;
    OutgoingAsyncBasePtr outAsync;
    ICE_DELEGATE(HeartbeatCallback) heartbeatCallback;
    int dispatchCount = 0;

    ThreadPoolMessage<ConnectionI> msg(current, *this);
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        ThreadPoolMessage<ConnectionI>::IOScope io(msg);
        if(!io)
        {
            return;
        }

        if(_state >= StateClosed)
        {
            return;
        }

        SocketOperation readyOp = current.operation;
        try
        {
            unscheduleTimeout(current.operation);

            SocketOperation writeOp = SocketOperationNone;
            SocketOperation readOp = SocketOperationNone;
            if(readyOp & SocketOperationWrite)
            {
                if(_observer)
                {
                    _observer.startWrite(_writeStream);
                }
                writeOp = write(_writeStream);
                if(_observer && !(writeOp & SocketOperationWrite))
                {
                    _observer.finishWrite(_writeStream);
                }
            }

            while(readyOp & SocketOperationRead)
            {
                if(_observer && !_readHeader)
                {
                    _observer.startRead(_readStream);
                }

                readOp = read(_readStream);
                if(readOp & SocketOperationRead)
                {
                    break;
                }
                if(_observer && !_readHeader)
                {
                    assert(_readStream.i == _readStream.b.end());
                    _observer.finishRead(_readStream);
                }

                if(_readHeader) // Read header if necessary.
                {
                    _readHeader = false;

                    if(_observer)
                    {
                        _observer->receivedBytes(static_cast<int>(headerSize));
                    }

                    ptrdiff_t pos = _readStream.i - _readStream.b.begin();
                    if(pos < headerSize)
                    {
                        //
                        // This situation is possible for small UDP packets.
                        //
                        throw IllegalMessageSizeException(__FILE__, __LINE__);
                    }

                    _readStream.i = _readStream.b.begin();
                    const Byte* m;
                    _readStream.readBlob(m, static_cast<Int>(sizeof(magic)));
                    if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
                    {
                        throw BadMagicException(__FILE__, __LINE__, "", Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
                    }
                    ProtocolVersion pv;
                    _readStream.read(pv);
                    checkSupportedProtocol(pv);
                    EncodingVersion ev;
                    _readStream.read(ev);
                    checkSupportedProtocolEncoding(ev);

                    Byte messageType;
                    _readStream.read(messageType);
                    Byte compressByte;
                    _readStream.read(compressByte);
                    Int size;
                    _readStream.read(size);
                    if(size < headerSize)
                    {
                        throw IllegalMessageSizeException(__FILE__, __LINE__);
                    }
                    if(size > static_cast<Int>(_messageSizeMax))
                    {
                        Ex::throwMemoryLimitException(__FILE__, __LINE__, size, _messageSizeMax);
                    }
                    if(size > static_cast<Int>(_readStream.b.size()))
                    {
                        _readStream.b.resize(size);
                    }
                    _readStream.i = _readStream.b.begin() + pos;
                }

                if(_readStream.i != _readStream.b.end())
                {
                    if(_endpoint->datagram())
                    {
                        throw DatagramLimitException(__FILE__, __LINE__); // The message was truncated.
                    }
                    continue;
                }
                break;
            }

            SocketOperation newOp = static_cast<SocketOperation>(readOp | writeOp);
            readyOp = static_cast<SocketOperation>(readyOp & ~newOp);
            assert(readyOp || newOp);

            if(_state <= StateNotValidated)
            {
                if(newOp)
                {
                    //
                    // Wait for all the transceiver conditions to be
                    // satisfied before continuing.
                    //
                    scheduleTimeout(newOp);
                    _threadPool->update(ICE_SHARED_FROM_THIS, current.operation, newOp);
                    return;
                }

                if(_state == StateNotInitialized && !initialize(current.operation))
                {
                    return;
                }

                if(_state <= StateNotValidated && !validate(current.operation))
                {
                    return;
                }

                _threadPool->unregister(ICE_SHARED_FROM_THIS, current.operation);

                //
                // We start out in holding state.
                //
                setState(StateHolding);
                if(_startCallback)
                {
                    swap(_startCallback, startCB);
                    if(startCB)
                    {
                        ++dispatchCount;
                    }
                }
            }
            else
            {
                assert(_state <= StateClosingPending);

                //
                // We parse messages first, if we receive a close
                // connection message we won't send more messages.
                //
                if(readyOp & SocketOperationRead)
                {
                    newOp = static_cast<SocketOperation>(newOp | parseMessage(current.stream,
                                                                              invokeNum,
                                                                              requestId,
                                                                              compress,
                                                                              servantManager,
                                                                              adapter,
                                                                              outAsync,
                                                                              heartbeatCallback,
                                                                              dispatchCount));
                }

                if(readyOp & SocketOperationWrite)
                {
                    newOp = static_cast<SocketOperation>(newOp | sendNextMessage(sentCBs));
                    if(!sentCBs.empty())
                    {
                        ++dispatchCount;
                    }
                }

                if(_state < StateClosed)
                {
                    scheduleTimeout(newOp);
                    _threadPool->update(ICE_SHARED_FROM_THIS, current.operation, newOp);
                }
            }

            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }

            if(dispatchCount == 0)
            {
                return; // Nothing to dispatch we're done!
            }

            _dispatchCount += dispatchCount;
            io.completed();
        }
        catch(const DatagramLimitException&) // Expected.
        {
            if(_warnUdp)
            {
                Warning out(_instance->initializationData().logger);
                out << "maximum datagram size of " << _readStream.i - _readStream.b.begin() << " exceeded";
            }
            _readStream.resize(headerSize);
            _readStream.i = _readStream.b.begin();
            _readHeader = true;
            return;
        }
        catch(const SocketException& ex)
        {
            setState(StateClosed, ex);
            return;
        }
        catch(const LocalException& ex)
        {
            if(_endpoint->datagram())
            {
                if(_warn)
                {
                    Warning out(_instance->initializationData().logger);
                    out << "datagram connection exception:\n" << ex << '\n' << _desc;
                }
                _readStream.resize(headerSize);
                _readStream.i = _readStream.b.begin();
                _readHeader = true;
            }
            else
            {
                setState(StateClosed, ex);
            }
            return;
        }
    }

    if(!_dispatcher) // Optimization, call dispatch() directly if there's no dispatcher.
    {
        dispatch(startCB, sentCBs, compress, requestId, invokeNum, servantManager, adapter, outAsync, heartbeatCallback,
                 current.stream);
    }
    else
    {
        _threadPool->dispatchFromThisThread(new DispatchCall(ICE_SHARED_FROM_THIS, startCB, sentCBs, compress, requestId,
                                                             invokeNum, servantManager, adapter, outAsync,
                                                             heartbeatCallback, current.stream));

    }
}

void
ConnectionI::dispatch(const StartCallbackPtr& startCB, const vector<OutgoingMessage>& sentCBs,
                      Byte compress, Int requestId, Int invokeNum, const ServantManagerPtr& servantManager,
                      const ObjectAdapterPtr& adapter, const OutgoingAsyncBasePtr& outAsync,
                      const ICE_DELEGATE(HeartbeatCallback)& heartbeatCallback, InputStream& stream)
{
    int dispatchedCount = 0;

    //
    // Notify the factory that the connection establishment and
    // validation has completed.
    //
    if(startCB)
    {
        startCB->connectionStartCompleted(ICE_SHARED_FROM_THIS);
        ++dispatchedCount;
    }

    //
    // Notify AMI calls that the message was sent.
    //
    if(!sentCBs.empty())
    {
        for(vector<OutgoingMessage>::const_iterator p = sentCBs.begin(); p != sentCBs.end(); ++p)
        {
#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
            if(p->invokeSent)
            {
                p->outAsync->invokeSent();
            }
            if(p->receivedReply)
            {
                OutgoingAsyncPtr o = ICE_DYNAMIC_CAST(OutgoingAsync, p->outAsync);
                if(o->response())
                {
                    o->invokeResponse();
                }
            }
#else
            p->outAsync->invokeSent();
#endif
        }
        ++dispatchedCount;
    }

    //
    // Asynchronous replies must be handled outside the thread
    // synchronization, so that nested calls are possible.
    //
    if(outAsync)
    {
        outAsync->invokeResponse();
        ++dispatchedCount;
    }

    if(heartbeatCallback)
    {
        try
        {
#ifdef ICE_CPP11_MAPPING
            heartbeatCallback(ICE_SHARED_FROM_THIS);
#else
            heartbeatCallback->heartbeat(ICE_SHARED_FROM_THIS);
#endif
        }
        catch(const std::exception& ex)
        {
            Error out(_instance->initializationData().logger);
            out << "connection callback exception:\n" << ex << '\n' << _desc;
        }
        catch(...)
        {
            Error out(_instance->initializationData().logger);
            out << "connection callback exception:\nunknown c++ exception" << '\n' << _desc;
        }
        ++dispatchedCount;
    }

    //
    // Method invocation (or multiple invocations for batch messages)
    // must be done outside the thread synchronization, so that nested
    // calls are possible.
    //
    if(invokeNum)
    {
        invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);

        //
        // Don't increase count, the dispatch count is
        // decreased when the incoming reply is sent.
        //
    }

    //
    // Decrease dispatch count.
    //
    if(dispatchedCount > 0)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _dispatchCount -= dispatchedCount;
        if(_dispatchCount == 0)
        {
            //
            // Only initiate shutdown if not already done. It might
            // have already been done if the sent callback or AMI
            // callback was dispatched when the connection was already
            // in the closing state.
            //
            if(_state == StateClosing)
            {
                try
                {
                    initiateShutdown();
                }
                catch(const LocalException& ex)
                {
                    setState(StateClosed, ex);
                }
            }
            else if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }
    }
}

void
Ice::ConnectionI::finished(ThreadPoolCurrent& current, bool close)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(_state == StateClosed);
        unscheduleTimeout(static_cast<SocketOperation>(SocketOperationRead | SocketOperationWrite));
    }

    //
    // If there are no callbacks to call, we don't call ioCompleted() since we're not going
    // to call code that will potentially block (this avoids promoting a new leader and
    // unecessary thread creation, especially if this is called on shutdown).
    //
    if(!_startCallback && _sendStreams.empty() && _asyncRequests.empty() && !_closeCallback && !_heartbeatCallback)
    {
        finish(close);
        return;
    }

    current.ioCompleted();
    if(!_dispatcher) // Optimization, call finish() directly if there's no dispatcher.
    {
        finish(close);
    }
    else
    {
        _threadPool->dispatchFromThisThread(new FinishCall(ICE_SHARED_FROM_THIS, close));
    }
}

void
Ice::ConnectionI::finish(bool close)
{
    if(!_initialized)
    {
        if(_instance->traceLevels()->network >= 2)
        {
            string verb = _connector ? "establish" : "accept";
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);

            out << "failed to " << verb << " " << _endpoint->protocol() << " connection\n" << toString()
                << "\n" << *_exception;
        }
    }
    else
    {
        if(_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "closed " << _endpoint->protocol() << " connection\n" << toString();

            if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionManuallyClosedException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get())))
            {
                out << "\n" << *_exception;
            }
        }
    }

    if(close)
    {
        try
        {
            _transceiver->close();
        }
        catch(const Ice::LocalException& ex)
        {
            Error out(_logger);
            out << "unexpected connection exception:\n" << ex << '\n' << _desc;
        }
    }

    if(_startCallback)
    {
        assert(_exception);

        _startCallback->connectionStartFailed(ICE_SHARED_FROM_THIS, *_exception);
        _startCallback = 0;
    }

    if(!_sendStreams.empty())
    {
        if(!_writeStream.b.empty())
        {
            //
            // Return the stream to the outgoing call. This is important for
            // retriable AMI calls which are not marshalled again.
            //
            OutgoingMessage* message = &_sendStreams.front();
            _writeStream.swap(*message->stream);

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
            //
            // The current message might be sent but not yet removed from _sendStreams. If
            // the response has been received in the meantime, we remove the message from
            // _sendStreams to not call finished on a message which is already done.
            //
            if(message->isSent || message->receivedReply)
            {
                if(message->sent() && message->invokeSent)
                {
                    message->outAsync->invokeSent();
                }
                if(message->receivedReply)
                {
                    OutgoingAsyncPtr outAsync = ICE_DYNAMIC_CAST(OutgoingAsync, message->outAsync);
                    if(outAsync->response())
                    {
                        outAsync->invokeResponse();
                    }
                }
                _sendStreams.pop_front();
            }
#endif
        }

        for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
        {
            o->completed(*_exception);
            if(o->requestId) // Make sure finished isn't called twice.
            {
                _asyncRequests.erase(o->requestId);
            }
        }

        _sendStreams.clear();
    }

    for(map<Int, OutgoingAsyncBasePtr>::const_iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
        if(q->second->exception(*_exception))
        {
            q->second->invokeException();
        }
    }

    _asyncRequests.clear();

    //
    // Don't wait to be reaped to reclaim memory allocated by read/write streams.
    //
    _writeStream.clear();
    _writeStream.b.clear();
    _readStream.clear();
    _readStream.b.clear();

    if(_closeCallback)
    {
        closeCallback(_closeCallback);
        _closeCallback = ICE_NULLPTR;
    }

    _heartbeatCallback = ICE_NULLPTR;

    //
    // This must be done last as this will cause waitUntilFinished() to return (and communicator
    // objects such as the timer might be destroyed too).
    //
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        setState(StateFinished);

        if(_dispatchCount == 0)
        {
            reap();
        }
    }
}

string
Ice::ConnectionI::toString() const ICE_NOEXCEPT
{
    return _desc; // No mutex lock, _desc is immutable.
}

NativeInfoPtr
Ice::ConnectionI::getNativeInfo()
{
    return _transceiver->getNativeInfo();
}

void
Ice::ConnectionI::timedOut()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        setState(StateClosed, ConnectTimeoutException(__FILE__, __LINE__));
    }
    else if(_state < StateClosing)
    {
        setState(StateClosed, TimeoutException(__FILE__, __LINE__));
    }
    else if(_state < StateClosed)
    {
        setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
    }
}

string
Ice::ConnectionI::type() const ICE_NOEXCEPT
{
    return _type; // No mutex lock, _type is immutable.
}

Ice::Int
Ice::ConnectionI::timeout() const ICE_NOEXCEPT
{
    return _endpoint->timeout(); // No mutex lock, _endpoint is immutable.
}

ConnectionInfoPtr
Ice::ConnectionI::getInfo() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        _exception->ice_throw();
    }
    return initConnectionInfo();
}

void
Ice::ConnectionI::setBufferSize(Ice::Int rcvSize, Ice::Int sndSize)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        _exception->ice_throw();
    }
    _transceiver->setBufferSize(rcvSize, sndSize);
    _info = 0; // Invalidate the cached connection info
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

Ice::ConnectionI::ConnectionI(const CommunicatorPtr& communicator,
                              const InstancePtr& instance,
                              const ACMMonitorPtr& monitor,
                              const TransceiverPtr& transceiver,
                              const ConnectorPtr& connector,
                              const EndpointIPtr& endpoint,
                              const ObjectAdapterIPtr& adapter) :
    _communicator(communicator),
    _instance(instance),
    _monitor(monitor),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->protocol()),
    _connector(connector),
    _endpoint(endpoint),
    _adapter(adapter),
    _dispatcher(_instance->initializationData().dispatcher), // Cached for better performance.
    _logger(_instance->initializationData().logger), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _timer(_instance->timer()), // Cached for better performance.
    _writeTimeout(new TimeoutCallback(this)),
    _writeTimeoutScheduled(false),
    _readTimeout(new TimeoutCallback(this)),
    _readTimeoutScheduled(false),
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _warnUdp(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0),
    _compressionLevel(1),
    _nextRequestId(1),
    _asyncRequestsHint(_asyncRequests.end()),
    _messageSizeMax(adapter ? adapter->messageSizeMax() : _instance->messageSizeMax()),
    _batchRequestQueue(new BatchRequestQueue(instance, endpoint->datagram())),
    _readStream(_instance.get(), Ice::currentProtocolEncoding),
    _readHeader(false),
    _writeStream(_instance.get(), Ice::currentProtocolEncoding),
    _dispatchCount(0),
    _state(StateNotInitialized),
    _shutdownInitiated(false),
    _initialized(false),
    _validated(false)
{
    const Ice::PropertiesPtr& properties = _instance->initializationData().properties;

    int& compressionLevel = const_cast<int&>(_compressionLevel);
    compressionLevel = properties->getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
    if(compressionLevel < 1)
    {
        compressionLevel = 1;
    }
    else if(compressionLevel > 9)
    {
        compressionLevel = 9;
    }

    if(adapter)
    {
        _servantManager = adapter->getServantManager();
    }

    if(_monitor && _monitor->getACM().timeout > 0)
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }
}

Ice::ConnectionIPtr
Ice::ConnectionI::create(const CommunicatorPtr& communicator,
                         const InstancePtr& instance,
                         const ACMMonitorPtr& monitor,
                         const TransceiverPtr& transceiver,
                         const ConnectorPtr& connector,
                         const EndpointIPtr& endpoint,
                         const ObjectAdapterIPtr& adapter)
{
    Ice::ConnectionIPtr conn(new ConnectionI(communicator, instance, monitor, transceiver, connector,
                                             endpoint, adapter));
    if(adapter)
    {
        const_cast<ThreadPoolPtr&>(conn->_threadPool) = adapter->getThreadPool();
    }
    else
    {
        const_cast<ThreadPoolPtr&>(conn->_threadPool) = conn->_instance->clientThreadPool();
    }
    conn->_threadPool->initialize(conn);
    return conn;
}

Ice::ConnectionI::~ConnectionI()
{
    assert(!_startCallback);
    assert(!_closeCallback);
    assert(!_heartbeatCallback);
    assert(_state == StateFinished);
    assert(_dispatchCount == 0);
    assert(_sendStreams.empty());
    assert(_asyncRequests.empty());
}

void
Ice::ConnectionI::setState(State state, const LocalException& ex)
{
    //
    // If setState() is called with an exception, then only closed and
    // closing states are permissible.
    //
    assert(state >= StateClosing);

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    if(!_exception)
    {
        //
        // If we are in closed state, an exception must be set.
        //
        assert(_state != StateClosed);
        ICE_SET_EXCEPTION_FROM_CLONE(_exception, ex.ice_clone());
        //
        // We don't warn if we are not validated.
        //
        if(_warn && _validated)
        {
            //
            // Don't warn about certain expected exceptions.
            //
            if(!(dynamic_cast<const CloseConnectionException*>(&ex) ||
                 dynamic_cast<const ConnectionManuallyClosedException*>(&ex) ||
                 dynamic_cast<const ConnectionTimeoutException*>(&ex) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(&ex) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(&ex) ||
                 (dynamic_cast<const ConnectionLostException*>(&ex) && _state >= StateClosing)))
            {
                Warning out(_logger);
                out << "connection exception:\n" << ex << '\n' << _desc;
            }
        }
    }

    //
    // We must set the new state before we notify requests of any
    // exceptions. Otherwise new requests may retry on a connection
    // that is not yet marked as closed or closing.
    //
    setState(state);
}

void
Ice::ConnectionI::setState(State state)
{
    //
    // We don't want to send close connection messages if the endpoint
    // only supports oneway transmission from client to server.
    //
    if(_endpoint->datagram() && state == StateClosing)
    {
        state = StateClosed;
    }

    //
    // Skip graceful shutdown if we are destroyed before validation.
    //
    if(_state <= StateNotValidated && state == StateClosing)
    {
        state = StateClosed;
    }

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    try
    {
        switch(state)
        {
            case StateNotInitialized:
            {
                assert(false);
                break;
            }

            case StateNotValidated:
            {
                if(_state != StateNotInitialized)
                {
                    assert(_state == StateClosed);
                    return;
                }
                break;
            }

            case StateActive:
            {
                //
                // Can only switch from holding or not validated to
                // active.
                //
                if(_state != StateHolding && _state != StateNotValidated)
                {
                    return;
                }
                _threadPool->_register(ICE_SHARED_FROM_THIS, SocketOperationRead);
                break;
            }

            case StateHolding:
            {
                //
                // Can only switch from active or not validated to
                // holding.
                //
                if(_state != StateActive && _state != StateNotValidated)
                {
                    return;
                }
                if(_state == StateActive)
                {
                    _threadPool->unregister(ICE_SHARED_FROM_THIS, SocketOperationRead);
                }
                break;
            }

            case StateClosing:
            case StateClosingPending:
            {
                //
                // Can't change back from closing pending.
                //
                if(_state >= StateClosingPending)
                {
                    return;
                }
                break;
            }

            case StateClosed:
            {
                if(_state == StateFinished)
                {
                    return;
                }

                _batchRequestQueue->destroy(*_exception);

                //
                // Don't need to close now for connections so only close the transceiver
                // if the selector request it.
                //
                if(_threadPool->finish(ICE_SHARED_FROM_THIS, false))
                {
                    _transceiver->close();
                }
                break;
            }

            case StateFinished:
            {
                assert(_state == StateClosed);
                _communicator = 0;
                break;
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Error out(_logger);
        out << "unexpected connection exception:\n" << ex << '\n' << _desc;
    }

    //
    // We only register with the connection monitor if our new state
    // is StateActive. Otherwise we unregister with the connection
    // monitor, but only if we were registered before, i.e., if our
    // old state was StateActive.
    //
    if(_monitor)
    {
        if(state == StateActive)
        {
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            _monitor->add(ICE_SHARED_FROM_THIS);
        }
        else if(_state == StateActive)
        {
            _monitor->remove(ICE_SHARED_FROM_THIS);
        }
    }

    if(_instance->initializationData().observer)
    {
        ConnectionState oldState = toConnectionState(_state);
        ConnectionState newState = toConnectionState(state);
        if(oldState != newState)
        {
            _observer.attach(_instance->initializationData().observer->getConnectionObserver(initConnectionInfo(),
                                                                                             _endpoint,
                                                                                             newState,
                                                                                             _observer.get()));
        }
        if(_observer && state == StateClosed && _exception)
        {
            if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionManuallyClosedException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
                 (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state >= StateClosing)))
            {
                _observer->failed(_exception->ice_id());
            }
        }
    }
    _state = state;

    notifyAll();

    if(_state == StateClosing && _dispatchCount == 0)
    {
        try
        {
            initiateShutdown();
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
        }
    }
}

void
Ice::ConnectionI::initiateShutdown()
{
    assert(_state == StateClosing && _dispatchCount == 0);

    if(_shutdownInitiated)
    {
        return;
    }
    _shutdownInitiated = true;

    if(!_endpoint->datagram())
    {
        //
        // Before we shut down, we send a close connection message.
        //
        OutputStream os(_instance.get(), Ice::currentProtocolEncoding);
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(closeConnectionMsg);
        os.write(static_cast<Byte>(1)); // compression status: compression supported but not used.
        os.write(headerSize); // Message size.

        OutgoingMessage message(&os, false);
        if(sendMessage(message) & AsyncStatusSent)
        {
            setState(StateClosingPending);

            //
            // Notify the transceiver of the graceful connection closure.
            //
            SocketOperation op = _transceiver->closing(true, *_exception);
            if(op)
            {
                scheduleTimeout(op);
                _threadPool->_register(ICE_SHARED_FROM_THIS, op);
            }
        }
    }
}

void
Ice::ConnectionI::sendHeartbeatNow()
{
    assert(_state == StateActive);

    if(!_endpoint->datagram())
    {
        OutputStream os(_instance.get(), Ice::currentProtocolEncoding);
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
        try
        {
            OutgoingMessage message(&os, false);
            sendMessage(message);
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
            assert(_exception);
        }
    }
}

bool
Ice::ConnectionI::initialize(SocketOperation operation)
{
    SocketOperation s = _transceiver->initialize(_readStream, _writeStream);
    if(s != SocketOperationNone)
    {
        scheduleTimeout(s);
        _threadPool->update(ICE_SHARED_FROM_THIS, operation, s);
        return false;
    }

    //
    // Update the connection description once the transceiver is initialized.
    //
    const_cast<string&>(_desc) = _transceiver->toString();
    _initialized = true;
    setState(StateNotValidated);
    return true;
}

bool
Ice::ConnectionI::validate(SocketOperation operation)
{
    if(!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
        if(_adapter) // The server side has the active role for connection validation.
        {
            if(_writeStream.b.empty())
            {
                _writeStream.write(magic[0]);
                _writeStream.write(magic[1]);
                _writeStream.write(magic[2]);
                _writeStream.write(magic[3]);
                _writeStream.write(currentProtocol);
                _writeStream.write(currentProtocolEncoding);
                _writeStream.write(validateConnectionMsg);
                _writeStream.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
                _writeStream.write(headerSize); // Message size.
                _writeStream.i = _writeStream.b.begin();
                traceSend(_writeStream, _logger, _traceLevels);
            }

            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if(_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if(op)
                {
                    scheduleTimeout(op);
                    _threadPool->update(ICE_SHARED_FROM_THIS, operation, op);
                    return false;
                }
            }

            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else // The client side has the passive role for connection validation.
        {
            if(_readStream.b.empty())
            {
                _readStream.b.resize(headerSize);
                _readStream.i = _readStream.b.begin();
            }

            if(_observer)
            {
                _observer.startRead(_readStream);
            }

            if(_readStream.i != _readStream.b.end())
            {
                SocketOperation op = read(_readStream);
                if(op)
                {
                    scheduleTimeout(op);
                    _threadPool->update(ICE_SHARED_FROM_THIS, operation, op);
                    return false;
                }
            }

            if(_observer)
            {
                _observer.finishRead(_readStream);
            }

            assert(_readStream.i == _readStream.b.end());
            _readStream.i = _readStream.b.begin();
            Byte m[4];
            _readStream.read(m[0]);
            _readStream.read(m[1]);
            _readStream.read(m[2]);
            _readStream.read(m[3]);
            if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
            {
                throw BadMagicException(__FILE__, __LINE__, "", Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
            }
            ProtocolVersion pv;
            _readStream.read(pv);
            checkSupportedProtocol(pv);
            EncodingVersion ev;
            _readStream.read(ev);
            checkSupportedProtocolEncoding(ev);
            Byte messageType;
            _readStream.read(messageType);
            if(messageType != validateConnectionMsg)
            {
                throw ConnectionNotValidatedException(__FILE__, __LINE__);
            }
            Byte compress;
            _readStream.read(compress); // Ignore compression status for validate connection.
            Int size;
            _readStream.read(size);
            if(size != headerSize)
            {
                throw IllegalMessageSizeException(__FILE__, __LINE__);
            }
            traceRecv(_readStream, _logger, _traceLevels);

            _validated = true;
        }
    }

    _writeStream.resize(0);
    _writeStream.i = _writeStream.b.begin();

    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    if(_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        if(_endpoint->datagram())
        {
            out << "starting to " << (_connector ? "send" : "receive") << " " << _endpoint->protocol() << " messages\n";
            out << _transceiver->toDetailedString();
        }
        else
        {
            out << (_connector ? "established" : "accepted") << " " << _endpoint->protocol() << " connection\n";
            out << toString();
        }
    }

    return true;
}

SocketOperation
Ice::ConnectionI::sendNextMessage(vector<OutgoingMessage>& callbacks)
{
    if(_sendStreams.empty())
    {
        return SocketOperationNone;
    }
    else if(_state == StateClosingPending && _writeStream.i == _writeStream.b.begin())
    {
        // Message wasn't sent, empty the _writeStream, we're not going to send more data.
        OutgoingMessage* message = &_sendStreams.front();
        _writeStream.swap(*message->stream);
        return SocketOperationNone;
    }

    assert(!_writeStream.b.empty() && _writeStream.i == _writeStream.b.end());
    try
    {
        while(true)
        {
            //
            // Notify the message that it was sent.
            //
            OutgoingMessage* message = &_sendStreams.front();
            if(message->stream)
            {
                _writeStream.swap(*message->stream);
                if(message->sent())
                {
                    callbacks.push_back(*message);
                }
            }
            _sendStreams.pop_front();

            //
            // If there's nothing left to send, we're done.
            //
            if(_sendStreams.empty())
            {
                break;
            }

            //
            // If we are in the closed state or if the close is
            // pending, don't continue sending.
            //
            // This can occur if parseMessage (called before
            // sendNextMessage by message()) closes the connection.
            //
            if(_state >= StateClosingPending)
            {
                return SocketOperationNone;
            }

            //
            // Otherwise, prepare the next message stream for writing.
            //
            message = &_sendStreams.front();
            assert(!message->stream->i);
#ifdef ICE_HAS_BZIP2
            if(message->compress && message->stream->b.size() >= 100) // Only compress messages > 100 bytes.
            {
                //
                // Message compressed. Request compressed response, if any.
                //
                message->stream->b[9] = 2;

                //
                // Do compression.
                //
                OutputStream stream(_instance.get(), Ice::currentProtocolEncoding);
                doCompress(*message->stream, stream);

                traceSend(*message->stream, _logger, _traceLevels);

                message->adopt(&stream); // Adopt the compressed stream.
                message->stream->i = message->stream->b.begin();
            }
            else
            {
#endif
                if(message->compress)
                {
                    //
                    // Message not compressed. Request compressed response, if any.
                    //
                    message->stream->b[9] = 1;
                }

                //
                // No compression, just fill in the message size.
                //
                Int sz = static_cast<Int>(message->stream->b.size());
                const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
                reverse_copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#else
                copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#endif
                message->stream->i = message->stream->b.begin();
                traceSend(*message->stream, _logger, _traceLevels);

#ifdef ICE_HAS_BZIP2
            }
#endif
            _writeStream.swap(*message->stream);

            //
            // Send the message.
            //
            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }
            assert(_writeStream.i);
            if(_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if(op)
                {
                    return op;
                }
            }
            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }

        //
        // If all the messages were sent and we are in the closing state, we schedule
        // the close timeout to wait for the peer to close the connection.
        //
        if(_state == StateClosing && _shutdownInitiated)
        {
            setState(StateClosingPending);
            SocketOperation op = _transceiver->closing(true, *_exception);
            if(op)
            {
                return op;
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
    }
    return SocketOperationNone;
}

AsyncStatus
Ice::ConnectionI::sendMessage(OutgoingMessage& message)
{
    assert(_state < StateClosed);

    message.stream->i = 0; // Reset the message stream iterator before starting sending the message.

    if(!_sendStreams.empty())
    {
        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0);
        return AsyncStatusQueued;
    }

    //
    // Attempt to send the message without blocking. If the send blocks, we register
    // the connection with the selector thread.
    //

    message.stream->i = message.stream->b.begin();
    SocketOperation op;
#ifdef ICE_HAS_BZIP2
    if(message.compress && message.stream->b.size() >= 100) // Only compress messages larger than 100 bytes.
    {
        //
        // Message compressed. Request compressed response, if any.
        //
        message.stream->b[9] = 2;

        //
        // Do compression.
        //
        OutputStream stream(_instance.get(), Ice::currentProtocolEncoding);
        doCompress(*message.stream, stream);
        stream.i = stream.b.begin();

        traceSend(*message.stream, _logger, _traceLevels);

        //
        // Send the message without blocking.
        //
        if(_observer)
        {
            _observer.startWrite(stream);
        }
        op = write(stream);
        if(!op)
        {
            if(_observer)
            {
                _observer.finishWrite(stream);
            }

            AsyncStatus status = AsyncStatusSent;
            if(message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(&stream);
    }
    else
    {
#endif
        if(message.compress)
        {
            //
            // Message not compressed. Request compressed response, if any.
            //
            message.stream->b[9] = 1;
        }

        //
        // No compression, just fill in the message size.
        //
        Int sz = static_cast<Int>(message.stream->b.size());
        const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#else
        copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#endif
        message.stream->i = message.stream->b.begin();

        traceSend(*message.stream, _logger, _traceLevels);

        //
        // Send the message without blocking.
        //
        if(_observer)
        {
            _observer.startWrite(*message.stream);
        }
        op = write(*message.stream);
        if(!op)
        {
            if(_observer)
            {
                _observer.finishWrite(*message.stream);
            }
            AsyncStatus status = AsyncStatusSent;
            if(message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0); // Adopt the stream.
#ifdef ICE_HAS_BZIP2
    }
#endif

    _writeStream.swap(*_sendStreams.back().stream);
    scheduleTimeout(op);
    _threadPool->_register(ICE_SHARED_FROM_THIS, op);
    return AsyncStatusQueued;
}

#ifdef ICE_HAS_BZIP2
static string
getBZ2Error(int bzError)
{
    if(bzError == BZ_RUN_OK)
    {
        return ": BZ_RUN_OK";
    }
    else if(bzError == BZ_FLUSH_OK)
    {
        return ": BZ_FLUSH_OK";
    }
    else if(bzError == BZ_FINISH_OK)
    {
        return ": BZ_FINISH_OK";
    }
    else if(bzError == BZ_STREAM_END)
    {
        return ": BZ_STREAM_END";
    }
    else if(bzError == BZ_CONFIG_ERROR)
    {
        return ": BZ_CONFIG_ERROR";
    }
    else if(bzError == BZ_SEQUENCE_ERROR)
    {
        return ": BZ_SEQUENCE_ERROR";
    }
    else if(bzError == BZ_PARAM_ERROR)
    {
        return ": BZ_PARAM_ERROR";
    }
    else if(bzError == BZ_MEM_ERROR)
    {
        return ": BZ_MEM_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR)
    {
        return ": BZ_DATA_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR_MAGIC)
    {
        return ": BZ_DATA_ERROR_MAGIC";
    }
    else if(bzError == BZ_IO_ERROR)
    {
        return ": BZ_IO_ERROR";
    }
    else if(bzError == BZ_UNEXPECTED_EOF)
    {
        return ": BZ_UNEXPECTED_EOF";
    }
    else if(bzError == BZ_OUTBUFF_FULL)
    {
        return ": BZ_OUTBUFF_FULL";
    }
    else
    {
        return "";
    }
}

void
Ice::ConnectionI::doCompress(OutputStream& uncompressed, OutputStream& compressed)
{
    const Byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);
    int bzError = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
                                           &compressedLen,
                                           reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
                                           uncompressedLen,
                                           _compressionLevel, 0, 0);
    if(bzError != BZ_OK)
    {
        throw CompressionException(__FILE__, __LINE__, "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError));
    }
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);

    //
    // Write the size of the compressed stream into the header of the
    // uncompressed stream. Since the header will be copied, this size
    // will also be in the header of the compressed stream.
    //
    Int compressedSize = static_cast<Int>(compressed.b.size());
    p = reinterpret_cast<const Byte*>(&compressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#else
    copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#endif

    //
    // Add the size of the uncompressed stream before the message body
    // of the compressed stream.
    //
    Int uncompressedSize = static_cast<Int>(uncompressed.b.size());
    p = reinterpret_cast<const Byte*>(&uncompressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#endif

    //
    // Copy the header from the uncompressed stream to the compressed one.
    //
    copy(uncompressed.b.begin(), uncompressed.b.begin() + headerSize, compressed.b.begin());
}

void
Ice::ConnectionI::doUncompress(InputStream& compressed, InputStream& uncompressed)
{
    Int uncompressedSize;
    compressed.i = compressed.b.begin() + headerSize;
    compressed.read(uncompressedSize);
    if(uncompressedSize <= headerSize)
    {
        throw IllegalMessageSizeException(__FILE__, __LINE__);
    }

    if(uncompressedSize > static_cast<Int>(_messageSizeMax))
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, uncompressedSize, _messageSizeMax);
    }
    uncompressed.resize(uncompressedSize);

    unsigned int uncompressedLen = uncompressedSize - headerSize;
    unsigned int compressedLen = static_cast<unsigned int>(compressed.b.size() - headerSize - sizeof(Int));
    int bzError = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
                                             &uncompressedLen,
                                             reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
                                             compressedLen,
                                             0, 0);
    if(bzError != BZ_OK)
    {
        throw CompressionException(__FILE__, __LINE__, "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError));
    }

    copy(compressed.b.begin(), compressed.b.begin() + headerSize, uncompressed.b.begin());
}
#endif

SocketOperation
Ice::ConnectionI::parseMessage(InputStream& stream, Int& invokeNum, Int& requestId, Byte& compress,
                               ServantManagerPtr& servantManager, ObjectAdapterPtr& adapter,
                               OutgoingAsyncBasePtr& outAsync, ICE_DELEGATE(HeartbeatCallback)& heartbeatCallback,
                               int& dispatchCount)
{
    assert(_state > StateNotValidated && _state < StateClosed);

    _readStream.swap(stream);
    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    assert(stream.i == stream.b.end());

    //
    // Connection is validated on first message. This is only used by
    // setState() to check wether or not we can print a connection
    // warning (a client might close the connection forcefully if the
    // connection isn't validated).
    //
    _validated = true;

    try
    {
        //
        // We don't need to check magic and version here. This has
        // already been done by the ThreadPool, which provides us
        // with the stream.
        //
        assert(stream.i == stream.b.end());
        stream.i = stream.b.begin() + 8;
        Byte messageType;
        stream.read(messageType);
        stream.read(compress);

        if(compress == 2)
        {
#ifdef ICE_HAS_BZIP2
            InputStream ustream(_instance.get(), Ice::currentProtocolEncoding);
            doUncompress(stream, ustream);
            stream.b.swap(ustream.b);
#else
            throw FeatureNotSupportedException(__FILE__, __LINE__, "Cannot uncompress compressed message");
#endif
        }
        stream.i = stream.b.begin() + headerSize;

        switch(messageType)
        {
            case closeConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_endpoint->datagram())
                {
                    if(_warn)
                    {
                        Warning out(_logger);
                        out << "ignoring close connection message for datagram connection:\n" << _desc;
                    }
                }
                else
                {
                    setState(StateClosingPending, CloseConnectionException(__FILE__, __LINE__));

                    //
                    // Notify the transceiver of the graceful connection closure.
                    //
                    SocketOperation op = _transceiver->closing(false, *_exception);
                    if(op)
                    {
                        return op;
                    }
                    setState(StateClosed);
                }
                break;
            }

            case requestMsg:
            {
                if(_state >= StateClosing)
                {
                    trace("received request during closing\n(ignored by server, client will retry)", stream, _logger,
                          _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(requestId);
                    invokeNum = 1;
                    servantManager = _servantManager;
                    adapter = _adapter;
                    ++dispatchCount;
                }
                break;
            }

            case requestBatchMsg:
            {
                if(_state >= StateClosing)
                {
                    trace("received batch request during closing\n(ignored by server, client will retry)", stream,
                          _logger, _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(invokeNum);
                    if(invokeNum < 0)
                    {
                        invokeNum = 0;
                        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
                    }
                    servantManager = _servantManager;
                    adapter = _adapter;
                    dispatchCount += invokeNum;
                }
                break;
            }

            case replyMsg:
            {
                traceRecv(stream, _logger, _traceLevels);

                stream.read(requestId);

                map<Int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.end();

                if(_asyncRequestsHint != _asyncRequests.end())
                {
                    if(_asyncRequestsHint->first == requestId)
                    {
                        q = _asyncRequestsHint;
                    }
                }

                if(q == _asyncRequests.end())
                {
                    q = _asyncRequests.find(requestId);
                }

                if(q != _asyncRequests.end())
                {
                    outAsync = q->second;

                    if(q == _asyncRequestsHint)
                    {
                        _asyncRequests.erase(q++);
                        _asyncRequestsHint = q;
                    }
                    else
                    {
                        _asyncRequests.erase(q);
                    }

                    stream.swap(*outAsync->getIs());

#if defined(ICE_USE_IOCP) || defined(ICE_OS_UWP)
                    //
                    // If we just received the reply of a request which isn't acknowledge as
                    // sent yet, we queue the reply instead of processing it right away. It
                    // will be processed once the write callback is invoked for the message.
                    //
                    OutgoingMessage* message = _sendStreams.empty() ? 0 : &_sendStreams.front();
                    if(message && message->outAsync.get() == outAsync.get())
                    {
                        message->receivedReply = true;
                        outAsync = 0;
                    }
                    else if(outAsync->response())
                    {
                        ++dispatchCount;
                    }
                    else
                    {
                        outAsync = 0;
                    }
#else
                    if(outAsync->response())
                    {
                        ++dispatchCount;
                    }
                    else
                    {
                        outAsync = 0;
                    }
#endif
                    notifyAll(); // Notify threads blocked in close(false)
                }

                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_heartbeatCallback)
                {
                    heartbeatCallback = _heartbeatCallback;
                    ++dispatchCount;
                }
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
                throw UnknownMessageException(__FILE__, __LINE__);
            }
        }
    }
    catch(const LocalException& ex)
    {
        if(_endpoint->datagram())
        {
            if(_warn)
            {
                Warning out(_logger);
                out << "datagram connection exception:\n" << ex << '\n' << _desc;
            }
        }
        else
        {
            setState(StateClosed, ex);
        }
    }

    return _state == StateHolding ? SocketOperationNone : SocketOperationRead;
}

void
Ice::ConnectionI::invokeAll(InputStream& stream, Int invokeNum, Int requestId, Byte compress,
                            const ServantManagerPtr& servantManager, const ObjectAdapterPtr& adapter)
{
    //
    // Note: In contrast to other private or protected methods, this
    // operation must be called *without* the mutex locked.
    //

    try
    {
        while(invokeNum > 0)
        {
            //
            // Prepare the invocation.
            //
            bool response = !_endpoint->datagram() && requestId != 0;
            assert(!response || invokeNum == 1);

            Incoming in(_instance.get(), this, this, adapter, response, compress, requestId);

            //
            // Dispatch the invocation.
            //
            in.invoke(servantManager, &stream);

            --invokeNum;
        }

        stream.clear();
    }
    catch(const LocalException& ex)
    {
        invokeException(requestId, ex, invokeNum, false);  // Fatal invocation exception
    }
}

void
Ice::ConnectionI::scheduleTimeout(SocketOperation status)
{
    int timeout;
    if(_state < StateActive)
    {
        DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
        if(defaultsAndOverrides->overrideConnectTimeout)
        {
            timeout = defaultsAndOverrides->overrideConnectTimeoutValue;
        }
        else
        {
            timeout = _endpoint->timeout();
        }
    }
    else if(_state < StateClosingPending)
    {
        if(_readHeader) // No timeout for reading the header.
        {
            status = static_cast<SocketOperation>(status & ~SocketOperationRead);
        }
        timeout = _endpoint->timeout();
    }
    else
    {
        DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
        if(defaultsAndOverrides->overrideCloseTimeout)
        {
            timeout = defaultsAndOverrides->overrideCloseTimeoutValue;
        }
        else
        {
            timeout = _endpoint->timeout();
        }
    }

    if(timeout < 0)
    {
        return;
    }

    try
    {
        if(status & IceInternal::SocketOperationRead)
        {
            if(_readTimeoutScheduled)
            {
                _timer->cancel(_readTimeout);
            }
            _timer->schedule(_readTimeout, IceUtil::Time::milliSeconds(timeout));
            _readTimeoutScheduled = true;
        }
        if(status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect))
        {
            if(_writeTimeoutScheduled)
            {
                _timer->cancel(_writeTimeout);
            }
            _timer->schedule(_writeTimeout, IceUtil::Time::milliSeconds(timeout));
            _writeTimeoutScheduled = true;
        }
    }
    catch(const IceUtil::Exception&)
    {
        assert(false);
    }
}

void
Ice::ConnectionI::unscheduleTimeout(SocketOperation status)
{
    if((status & IceInternal::SocketOperationRead) && _readTimeoutScheduled)
    {
        _timer->cancel(_readTimeout);
        _readTimeoutScheduled = false;
    }
    if((status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect)) &&
       _writeTimeoutScheduled)
    {
        _timer->cancel(_writeTimeout);
        _writeTimeoutScheduled = false;
    }
}

Ice::ConnectionInfoPtr
Ice::ConnectionI::initConnectionInfo() const
{
    if(_state > StateNotInitialized && _info) // Update the connection information until it's initialized
    {
        return _info;
    }

    try
    {
        _info = _transceiver->getInfo();
    }
    catch(const Ice::LocalException&)
    {
        _info = ICE_MAKE_SHARED(ConnectionInfo);
    }

    Ice::ConnectionInfoPtr info = _info;
    while(info)
    {
        info->connectionId = _endpoint->connectionId();
        info->incoming = _connector == 0;
        info->adapterName = _adapter ? _adapter->getName() : string();
        info = info->underlying;
    }
    return _info;
}

ConnectionState
ConnectionI::toConnectionState(State state) const
{
    return connectionStateMap[static_cast<int>(state)];
}

SocketOperation
ConnectionI::read(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _transceiver->read(buf);
    if(_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "received ";
        if(_endpoint->datagram())
        {
            out << buf.b.size();
        }
        else
        {
            out << (buf.i - start) << " of " << (buf.b.end() - start);
        }
        out << " bytes via " << _endpoint->protocol() << "\n" << toString();
    }
    return op;
}

SocketOperation
ConnectionI::write(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _transceiver->write(buf);
    if(_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "sent " << (buf.i - start);
        if(!_endpoint->datagram())
        {
            out << " of " << (buf.b.end() - start);
        }
        out << " bytes via " << _endpoint->protocol() << "\n" << toString();
    }
    return op;
}

void
ConnectionI::reap()
{
    if(_monitor)
    {
        _monitor->reap(ICE_SHARED_FROM_THIS);
    }
    if(_observer)
    {
        _observer.detach();
    }
}
