//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_CONFIGURATION_H
#define TEST_CONFIGURATION_H

#include <IceUtil/IceUtil.h>
#include <Ice/UniquePtr.h>

#include <Ice/LocalException.h>
#include <Ice/Selector.h>

#ifndef TEST_API
#   if defined(ICE_STATIC_LIBS)
#       define TEST_API /**/
#   elif defined(TEST_API_EXPORTS)
#       define TEST_API ICE_DECLSPEC_EXPORT
#   else
#       define TEST_API ICE_DECLSPEC_IMPORT
#   endif
#endif

class Configuration;
typedef IceUtil::Handle<Configuration> ConfigurationPtr;

class TEST_API Configuration : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    Configuration();
    virtual ~Configuration();

    void connectorsException(Ice::LocalException*);
    void checkConnectorsException();

    void connectException(Ice::LocalException*);
    void checkConnectException();

    void initializeSocketOperation(IceInternal::SocketOperation);
    void initializeException(Ice::LocalException*);
    IceInternal::SocketOperation initializeSocketOperation();
    void checkInitializeException();

    void readReady(bool);
    void readException(Ice::LocalException*);
    bool readReady();
    void checkReadException();

    void writeReady(bool);
    void writeException(Ice::LocalException*);
    bool writeReady();
    void checkWriteException();

    void buffered(bool);
    bool buffered();

    static Configuration* getInstance();

private:

    IceInternal::UniquePtr<Ice::LocalException> _connectorsException;
    IceInternal::UniquePtr<Ice::LocalException> _connectException;
    IceInternal::SocketOperation _initializeSocketOperation;
    int _initializeResetCount;
    IceInternal::UniquePtr<Ice::LocalException> _initializeException;
    int _readReadyCount;
    IceInternal::UniquePtr<Ice::LocalException> _readException;
    int _writeReadyCount;
    IceInternal::UniquePtr<Ice::LocalException> _writeException;
    bool _buffered;

    static Configuration* _instance;
};

#endif
