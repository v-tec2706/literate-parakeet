//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Time.h>
#include <Ice/LoggerI.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#include <Ice/LocalException.h>
#include <IceUtil/FileUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceUtilInternal;

namespace
{

IceUtil::Mutex* outputMutex = 0;

class Init
{
public:

    Init()
    {
        outputMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete outputMutex;
        outputMutex = 0;
    }
};

Init init;

//
// Timeout in milliseconds after which rename will be attempted
// in case of failures renaming files. That is set to 5 minutes.
//
const IceUtil::Time retryTimeout = IceUtil::Time::seconds(5 * 60);

}

Ice::LoggerI::LoggerI(const string& prefix, const string& file,
                      bool convert, size_t sizeMax) :
    _prefix(prefix),
    _convert(convert),
    _converter(getProcessStringConverter()),
    _sizeMax(sizeMax)
{
    if(!prefix.empty())
    {
        _formattedPrefix = prefix + ": ";
    }

    if(!file.empty())
    {
        _file = file;
        _out.open(IceUtilInternal::streamFilename(file).c_str(), fstream::out | fstream::app);
        if(!_out.is_open())
        {
            throw InitializationException(__FILE__, __LINE__, "FileLogger: cannot open " + _file);
        }

        if(_sizeMax > 0)
        {
            _out.seekp(0, _out.end);
        }
    }
}

Ice::LoggerI::~LoggerI()
{
    if(_out.is_open())
    {
        _out.close();
    }
}

void
Ice::LoggerI::print(const string& message)
{
    write(message, false);
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    string s = "-- " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message;

    write(s, true);
}

void
Ice::LoggerI::warning(const string& message)
{
    write("-! " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix + "warning: " + message, true);
}

void
Ice::LoggerI::error(const string& message)
{
    write("!! " + IceUtil::Time::now().toDateTime() + " " + _formattedPrefix + "error: " + message, true);
}

string
Ice::LoggerI::getPrefix()
{
    return _prefix;
}

LoggerPtr
Ice::LoggerI::cloneWithPrefix(const std::string& prefix)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(outputMutex); // for _sizeMax
    return ICE_MAKE_SHARED(LoggerI, prefix, _file, _convert, _sizeMax);
}

void
Ice::LoggerI::write(const string& message, bool indent)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(outputMutex);

    string s = message;

    if(indent)
    {
        string::size_type idx = 0;
        while((idx = s.find("\n", idx)) != string::npos)
        {
            s.insert(idx + 1, "   ");
            ++idx;
        }
    }

    if(_out.is_open())
    {
        if(_sizeMax > 0)
        {
            //
            // If file size + message size exceeds max size we archive the log file,
            // but we do not archive empty files or truncate messages.
            //
            size_t sz = static_cast<size_t>(_out.tellp());
            if(sz > 0 && sz + message.size() >= _sizeMax && _nextRetry <= IceUtil::Time::now())
            {
                string basename = _file;
                string ext;

                size_t i = basename.rfind(".");
                if(i != string::npos && i + 1 < basename.size())
                {
                    ext = basename.substr(i + 1);
                    basename = basename.substr(0, i);
                }
                _out.close();

                int id = 0;
                string archive;
                string date = IceUtil::Time::now().toString("%Y%m%d-%H%M%S");
                while(true)
                {
                    ostringstream oss;
                    oss << basename << "-" << date;
                    if(id > 0)
                    {
                        oss << "-" << id;
                    }
                    if(!ext.empty())
                    {
                        oss << "." << ext;
                    }
                    if(IceUtilInternal::fileExists(oss.str()))
                    {
                        id++;
                        continue;
                    }
                    archive = oss.str();
                    break;
                }

                int err = IceUtilInternal::rename(_file, archive);

                _out.open(IceUtilInternal::streamFilename(_file).c_str(), fstream::out | fstream::app);

                if(err)
                {
                    _nextRetry = IceUtil::Time::now() + retryTimeout;

                    //
                    // We temporarily set the maximum size to 0 to ensure there isn't more rename attempts
                    // in the nested error call.
                    //
                    size_t sizeMax = _sizeMax;
                    _sizeMax = 0;
                    sync.release();
                    error("FileLogger: cannot rename `" + _file + "'\n" + IceUtilInternal::lastErrorToString());
                    sync.acquire();
                    _sizeMax = sizeMax;
                }
                else
                {
                    _nextRetry = IceUtil::Time();
                }

                if(!_out.is_open())
                {
                    sync.release();
                    error("FileLogger: cannot open `" + _file + "':\nlog messages will be sent to stderr");
                    write(message, indent);
                    return;
                }            }
        }
        _out << s << endl;
    }
    else
    {
#if defined(ICE_OS_UWP)
        OutputDebugString(stringToWstring(s).c_str());
#elif defined(_WIN32)
        //
        // Convert the message from the native narrow string encoding to the console
        // code page encoding for printing. If the _convert member is set to false
        // we don't do any conversion.
        //
        if(!_convert)
        {
            //
            // Use fprintf_s to avoid encoding conversion when stderr is connected
            // to Windows console. When _convert is set to false we always output
            // UTF-8 encoded messages.
            //
            fprintf_s(stderr, "%s\n", nativeToUTF8(s, _converter).c_str());
            fflush(stderr);
        }
        else
        {
            consoleErr << s << endl;
        }
#else
        cerr << s << endl;
#endif
    }
}
