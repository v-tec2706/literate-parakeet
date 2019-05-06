//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/Config.h>
#include <IceSSL/UWPTransceiverI.h>
#include <IceSSL/Instance.h>
#include <IceSSL/UWPEngine.h>
#include <IceSSL/ConnectionInfo.h>
#include <Ice/Logger.h>
#include <ppltasks.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Cryptography::Certificates;

namespace
{

std::string
validationResultToString(ChainValidationResult result)
{
    switch (result)
    {
        case ChainValidationResult::Success:
        {
            return "The certificate chain was verified.";
        }
        case ChainValidationResult::Untrusted:
        {
            return "A certificate in the chain is not trusted.";
        }
        case ChainValidationResult::Revoked:
        {
            return "A certificate in the chain has been revoked.";
        }
        case ChainValidationResult::Expired:
        {
            return "A certificate in the chain has expired.";
        }
        case ChainValidationResult::IncompleteChain:
        {
            return "The certificate chain is missing one or more certificates.";
        }
        case ChainValidationResult::InvalidSignature:
        {
            return "The signature of a certificate in the chain cannot be verified.";
        }
        case ChainValidationResult::WrongUsage:
        {
            return "A certificate in the chain is being used for a purpose other than one specified by its CA.";
        }
        case ChainValidationResult::InvalidName:
        {
            return "A certificate in the chain has a name that is not valid. The name is either not included in "
                   "the permitted list or is explicitly excluded.";
        }
        case ChainValidationResult::InvalidCertificateAuthorityPolicy:
        {
            return "A certificate in the chain has a policy that is not valid.";
        }
        case ChainValidationResult::BasicConstraintsError:
        {
            return "The basic constraint extension of a certificate in the chain has not been observed.";
        }
        case ChainValidationResult::UnknownCriticalExtension:
        {
            return "A certificate in the chain contains an unknown extension that is marked \"critical\".";
        }
        case ChainValidationResult::RevocationInformationMissing:
        {
            return "No installed or registered DLL was found to verify revocation.";
        }
        case ChainValidationResult::RevocationFailure:
        {
            return "Unable to connect to the revocation server.";
        }
        case ChainValidationResult::OtherErrors:
        {
            return "An unexpected error occurred while validating the certificate chain.";
        }
        default:
        {
            assert(false);
            return "";
        }
    }
}

}

IceInternal::NativeInfoPtr
UWP::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
UWP::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if(!_connected)
    {
        IceInternal::SocketOperation status = _delegate->initialize(readBuffer, writeBuffer);
        if(status != IceInternal::SocketOperationNone)
        {
            return status;
        }
        _connected = true;

        //
        // Continue connecting, this will call startWrite/finishWrite to upgrade the stream
        // to SSL.
        //
        return IceInternal::SocketOperationConnect;
    }
    else if(!_upgraded)
    {
        _upgraded = true;
        try
        {
            auto fd = safe_cast<StreamSocket^>(_delegate->getNativeInfo()->fd());
            if(fd->Information->ServerCertificate)
            {
                //
                // Build the certificate chain
                //
                auto params = ref new ChainBuildingParameters();
                params->AuthorityInformationAccessEnabled = false;
                params->CurrentTimeValidationEnabled = true;
                params->NetworkRetrievalEnabled = false;
                params->RevocationCheckEnabled = false;

                //
                // BUGFIX: It is currently not possible to set ExclusiveTrustRoots programatically
                // it is causing a read access exception see:https://goo.gl/B6OaNx
                //
                //if(_engine->ca())
                //{
                // params->ExclusiveTrustRoots->Append(_engine->ca()->getCert());
                //}
                try
                {
                    _chain = create_task(fd->Information->ServerCertificate->BuildChainAsync(
                                         fd->Information->ServerIntermediateCertificates, params)).get();
                }
                catch(Platform::Exception^ ex)
                {
                    throw SyscallException(__FILE__, __LINE__, ex->HResult);
                }

                ChainValidationResult result = _chain->Validate();
                if(result != ChainValidationResult::Success)
                {
                    if(_engine->getVerifyPeer() == 0)
                    {
                        if(_instance->traceLevel() >= 1)
                        {
                            _instance->logger()->trace(_instance->traceCategory(),
                                "IceSSL: ignoring certificate verification failure\n" +
                                validationResultToString(result));
                        }
                    }
                    else
                    {
                        throw SecurityException(__FILE__, __LINE__,
                            "IceSSL: certificate validation error:\n" + validationResultToString(result));
                    }
                }
                else
                {
                    _verified = true;
                }
            }
            else if((!_incoming && _engine->getVerifyPeer() > 0) || (_incoming && _engine->getVerifyPeer() == 2))
            {
                //
                // Clients require server certificate if VerifyPeer > 0 and servers require client
                // certificate if VerifyPeer == 2
                //
                throw SecurityException(__FILE__, __LINE__, "IceSSL: certificate required");
            }

            if(_chain)
            {
                auto certs = _chain->GetCertificates(true);
                for(auto iter = certs->First(); iter->HasCurrent; iter->MoveNext())
                {
                    auto cert = UWP::Certificate::create(iter->Current);
                    _certs.push_back(cert);
                }
            }

            _engine->verifyPeer(_host, dynamic_pointer_cast<IceSSL::ConnectionInfo>(getInfo()), toString());
        }
        catch(Platform::Exception^ ex)
        {
            ostringstream os;
            os << "IceSSL: certificate verification failure:\n" << wstringToString(ex->Message->Data());
            throw SecurityException(__FILE__, __LINE__, os.str());
        }
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
UWP::TransceiverI::closing(bool initiator, const Ice::LocalException& ex)
{
    return _delegate->closing(initiator, ex);
}

void
UWP::TransceiverI::close()
{
    _delegate->close();
}

IceInternal::SocketOperation
UWP::TransceiverI::write(IceInternal::Buffer& buf)
{
    return _delegate->write(buf);
}

IceInternal::SocketOperation
UWP::TransceiverI::read(IceInternal::Buffer& buf)
{
    return _delegate->read(buf);
}

bool
UWP::TransceiverI::startWrite(IceInternal::Buffer& buf)
{
    if(_connected && !_upgraded)
    {
        StreamSocket^ stream = safe_cast<StreamSocket^>(_delegate->getNativeInfo()->fd());
        HostName^ host = ref new HostName(ref new String(IceUtil::stringToWstring(_host).c_str()));

        //
        // We ignore SSL Certificate errors at this point, the certificate chain will be validated
        // when the chain is constructed in Transceiver::initialize
        //
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::Expired);
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::IncompleteChain);
        //
        // Check if we need to enable host name verification
        //
        if(!_engine->getCheckCertName() || _host.empty() || _engine->getVerifyPeer() == 0)
        {
            stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::InvalidName);
        }
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::RevocationFailure);
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::RevocationInformationMissing);
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::Untrusted);
        stream->Control->IgnorableServerCertificateErrors->Append(ChainValidationResult::WrongUsage);

        if(_engine->certificate())
        {
            stream->Control->ClientCertificate = _engine->certificate()->getCert();
        }

        try
        {
            Windows::Foundation::IAsyncAction^ action = stream->UpgradeToSslAsync(SocketProtectionLevel::Tls12, host);
            getNativeInfo()->queueAction(IceInternal::SocketOperationWrite, action);
        }
        catch(Platform::Exception^ ex)
        {
            IceInternal::checkErrorCode(__FILE__, __LINE__, ex->HResult);
        }
        return true;
    }
    return _delegate->startWrite(buf);
}

void
UWP::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if(_connected && !_upgraded)
    {
        IceInternal::AsyncInfo* asyncInfo = getNativeInfo()->getAsyncInfo(IceInternal::SocketOperationWrite);
        if(asyncInfo->error != ERROR_SUCCESS)
        {
            if(CERT_E_CN_NO_MATCH == asyncInfo->error)
            {
                ostringstream ostr;
                ostr << "IceSSL: certificate validation failure: "
                     << (IceInternal::isIpAddress(_host) ? "IP address mismatch" : "Hostname mismatch");
                string msg = ostr.str();
                if(_engine->securityTraceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(), msg);
                }

                throw SecurityException(__FILE__, __LINE__, msg);
            }
            else
            {
                IceInternal::checkErrorCode(__FILE__, __LINE__, asyncInfo->error);
            }
        }
        return;
    }
    _delegate->finishWrite(buf);
}

void
UWP::TransceiverI::startRead(IceInternal::Buffer& buf)
{
    _delegate->startRead(buf);
}

void
UWP::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    _delegate->finishRead(buf);
}

string
UWP::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
UWP::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
UWP::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
UWP::TransceiverI::getInfo() const
{
    ConnectionInfoPtr info = ICE_MAKE_SHARED(ConnectionInfo);
    info->verified = _verified;
    info->adapterName = _adapterName;
    info->incoming = _incoming;
    info->underlying = _delegate->getInfo();
    info->certs = _certs;
    return info;
}

void
UWP::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
UWP::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

UWP::TransceiverI::TransceiverI(const InstancePtr& instance,
                                const IceInternal::TransceiverPtr& delegate,
                                const string& hostOrAdapterName,
                                bool incoming) :
    _instance(instance),
    _engine(UWP::SSLEnginePtr::dynamicCast(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _delegate(delegate),
    _connected(false),
    _upgraded(false),
    _verified(false)
{
}

UWP::TransceiverI::~TransceiverI()
{
}
