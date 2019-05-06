//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Protocol.h>
#include <Ice/LocalException.h>

namespace IceInternal
{

const Ice::Byte magic[] = { 0x49, 0x63, 0x65, 0x50 };   // 'I', 'c', 'e', 'P'

const Ice::Byte requestHdr[] =
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    protocolEncodingMajor,
    protocolEncodingMinor,
    requestMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (placeholder)
    0, 0, 0, 0 // Request id (placeholder)
};

const Ice::Byte requestBatchHdr[] =
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    protocolEncodingMajor,
    protocolEncodingMinor,
    requestBatchMsg,
    0, // Compression status
    0, 0, 0, 0, // Message size (place holder)
    0, 0, 0, 0  // Number of requests in batch (placeholder)
};

const Ice::Byte replyHdr[] =
{
    magic[0],
    magic[1],
    magic[2],
    magic[3],
    protocolMajor,
    protocolMinor,
    protocolEncodingMajor,
    protocolEncodingMinor,
    replyMsg,
    0, // Compression status
    0, 0, 0, 0 // Message size (placeholder)
};

void
stringToMajorMinor(const std::string& str, Ice::Byte& major, Ice::Byte& minor)
{
    std::string::size_type pos = str.find_first_of(".");
    if(pos == std::string::npos)
    {
        throw Ice::VersionParseException(__FILE__, __LINE__, "malformed version value `" + str + "'");
    }

    std::istringstream majStr(str.substr(0, pos));
    Ice::Int majVersion;
    if(!(majStr >> majVersion) || !majStr.eof())
    {
        throw Ice::VersionParseException(__FILE__, __LINE__, "invalid major version value `" + str + "'");
    }

    std::istringstream minStr(str.substr(pos + 1, std::string::npos));
    Ice::Int minVersion;
    if(!(minStr >> minVersion) || !minStr.eof())
    {
        throw Ice::VersionParseException(__FILE__, __LINE__, "invalid minor version value `" + str + "'");
    }

    if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
    {
        throw Ice::VersionParseException(__FILE__, __LINE__, "range error in version `" + str + "'");
    }

    major = static_cast<Ice::Byte>(majVersion);
    minor = static_cast<Ice::Byte>(minVersion);
}

void
throwUnsupportedProtocolException(const char* f, int l, const Ice::ProtocolVersion& v, const Ice::ProtocolVersion& s)
{
    throw Ice::UnsupportedProtocolException(f, l, "", v, s);
}

void
throwUnsupportedEncodingException(const char* f, int l, const Ice::EncodingVersion& v, const Ice::EncodingVersion& s)
{
    throw Ice::UnsupportedEncodingException(f, l, "", v, s);
}

}

namespace Ice
{

const EncodingVersion currentEncoding = { IceInternal::encodingMajor, IceInternal::encodingMinor };
const ProtocolVersion currentProtocol = { IceInternal::protocolMajor, IceInternal::protocolMinor };
//
// The encoding to use for protocol messages, this version is tied to
// the protocol version.
//

const EncodingVersion currentProtocolEncoding = { IceInternal::protocolEncodingMajor,
                                                  IceInternal::protocolEncodingMinor };

const ProtocolVersion Protocol_1_0 = { 1, 0 };

const EncodingVersion Encoding_1_0 = { 1, 0 };
const EncodingVersion Encoding_1_1 = { 1, 1 };

}
