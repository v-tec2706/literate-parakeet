//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/ConsoleUtil.h>

#include <Slice/Preprocessor.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include "Gen.h"

using namespace std;
using namespace Slice;
using namespace IceUtilInternal;

namespace
{

IceUtil::Mutex* globalMutex = 0;
bool interrupted = false;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

void
interruptedCallback(int /*signal*/)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

    interrupted = true;
}

void
usage(const string& n)
{
    consoleErr << "Usage: " << n << " [options] slice-files...\n";
    consoleErr <<
        "Options:\n"
        "-h, --help               Show this message.\n"
        "-v, --version            Display the Ice version.\n"
        "-DNAME                   Define NAME as 1.\n"
        "-DNAME=DEF               Define NAME as DEF.\n"
        "-UNAME                   Remove any definition for NAME.\n"
        "-IDIR                    Put DIR in the include file search path.\n"
        "-E                       Print preprocessor output on stdout.\n"
        "--output-dir DIR         Create files in the directory DIR.\n"
        "-d, --debug              Print debug messages.\n"
        "--depend                 Generate Makefile dependencies.\n"
        "--depend-xml             Generate dependencies in XML format.\n"
        "--depend-file FILE       Write dependencies to FILE instead of standard output.\n"
        "--validate               Validate command line options.\n"
        "--header-ext EXT         Use EXT instead of the default `h' extension.\n"
        "--source-ext EXT         Use EXT instead of the default `cpp' extension.\n"
        "--add-header HDR[,GUARD] Add #include for HDR (with guard GUARD) to generated source file.\n"
        "--include-dir DIR        Use DIR as the header include directory in source files.\n"
        "--impl-c++11             Generate sample implementations for C++11 mapping.\n"
        "--impl-c++98             Generate sample implementations for C++98 mapping.\n"
        "--checksum               Generate checksums for Slice definitions.\n"
        "--dll-export SYMBOL      Use SYMBOL for DLL exports\n"
        "                         deprecated: use instead [[\"cpp:dll-export:SYMBOL\"]] metadata.\n"
        "--ice                    Allow reserved Ice prefix in Slice identifiers\n"
        "                         deprecated: use instead [[\"ice-prefix\"]] metadata.\n"
        "--underscore             Allow underscores in Slice identifiers\n"
        "                         deprecated: use instead [[\"underscore\"]] metadata.\n"
        ;
}

int
compile(const vector<string>& argv)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("", "validate");
    opts.addOpt("", "header-ext", IceUtilInternal::Options::NeedArg, "h");
    opts.addOpt("", "source-ext", IceUtilInternal::Options::NeedArg, "cpp");
    opts.addOpt("", "add-header", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("D", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("U", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("I", "", IceUtilInternal::Options::NeedArg, "", IceUtilInternal::Options::Repeat);
    opts.addOpt("E");
    opts.addOpt("", "include-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "output-dir", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "dll-export", IceUtilInternal::Options::NeedArg);
    opts.addOpt("", "impl-c++98");
    opts.addOpt("", "impl-c++11");
    opts.addOpt("", "depend");
    opts.addOpt("", "depend-xml");
    opts.addOpt("", "depend-file", IceUtilInternal::Options::NeedArg, "");
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "underscore");
    opts.addOpt("", "checksum");

    bool validate = find(argv.begin(), argv.end(), "--validate") != argv.end();
    vector<string> args;
    try
    {
        args = opts.parse(argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        consoleErr << argv[0] << ": " << e.reason << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if(opts.isSet("version"))
    {
        consoleErr << ICE_STRING_VERSION << endl;
        return EXIT_SUCCESS;
    }

    string headerExtension = opts.optArg("header-ext");
    string sourceExtension = opts.optArg("source-ext");

    vector<string> extraHeaders = opts.argVec("add-header");

    vector<string> cppArgs;
    vector<string> optargs = opts.argVec("D");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-D" + *i);
    }

    optargs = opts.argVec("U");
    for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
    {
        cppArgs.push_back("-U" + *i);
    }

    vector<string> includePaths;
    includePaths = opts.argVec("I");
    for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
    {
        cppArgs.push_back("-I" + Preprocessor::normalizeIncludePath(*i));
    }

    bool preprocess = opts.isSet("E");

    string include = opts.optArg("include-dir");

    string output = opts.optArg("output-dir");

    string dllExport = opts.optArg("dll-export");

    bool implCpp98 = opts.isSet("impl-c++98");

    bool implCpp11 = opts.isSet("impl-c++11");

    bool depend = opts.isSet("depend");

    bool dependxml = opts.isSet("depend-xml");

    string dependFile = opts.optArg("depend-file");

    bool debug = opts.isSet("debug");

    bool ice = opts.isSet("ice");

    bool underscore = opts.isSet("underscore");

    bool checksum = opts.isSet("checksum");

    if(args.empty())
    {
        consoleErr << argv[0] << ": error: no input file" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(depend && dependxml)
    {
        consoleErr << argv[0] << ": error: cannot specify both --depend and --depend-xml" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(implCpp98 && implCpp11)
    {
        consoleErr << argv[0] << ": error: cannot specify both --impl-c++98 and --impl-c++11" << endl;
        if(!validate)
        {
            usage(argv[0]);
        }
        return EXIT_FAILURE;
    }

    if(validate)
    {
        return EXIT_SUCCESS;
    }

    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrlCHandler;
    ctrlCHandler.setCallback(interruptedCallback);

    ostringstream os;
    if(dependxml)
    {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dependencies>" << endl;
    }

    for(vector<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        //
        // Ignore duplicates.
        //
        vector<string>::iterator p = find(args.begin(), args.end(), *i);
        if(p != i)
        {
            continue;
        }

        if(depend || dependxml)
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(false, "-D__SLICE2CPP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            UnitPtr u = Unit::createUnit(false, false, ice, underscore);
            int parseStatus = u->parse(*i, cppHandle, debug);

            string ext = headerExtension;
            static const string headerExtPrefix = "cpp:header-ext:";
            DefinitionContextPtr dc = u->findDefinitionContext(u->topLevelFile());
            assert(dc);
            string meta = dc->findMetaData(headerExtPrefix);
            if(meta.size() > headerExtPrefix.size())
            {
                ext = meta.substr(headerExtPrefix.size());
            }

            u->destroy();

            if(parseStatus == EXIT_FAILURE)
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->printMakefileDependencies(os, depend ? Preprocessor::CPlusPlus : Preprocessor::SliceXML,
                                                  includePaths, "-D__SLICE2CPP__", sourceExtension, ext))
            {
                return EXIT_FAILURE;
            }

            if(!icecpp->close())
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            PreprocessorPtr icecpp = Preprocessor::create(argv[0], *i, cppArgs);
            FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2CPP__");

            if(cppHandle == 0)
            {
                return EXIT_FAILURE;
            }

            if(preprocess)
            {
                char buf[4096];
                while(fgets(buf, static_cast<int>(sizeof(buf)), cppHandle) != ICE_NULLPTR)
                {
                    if(fputs(buf, stdout) == EOF)
                    {
                        return EXIT_FAILURE;
                    }
                }
                if(!icecpp->close())
                {
                    return EXIT_FAILURE;
                }
            }
            else
            {
                UnitPtr u = Unit::createUnit(false, false, ice, underscore);
                int parseStatus = u->parse(*i, cppHandle, debug);

                if(!icecpp->close())
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }

                if(parseStatus == EXIT_FAILURE)
                {
                    status = EXIT_FAILURE;
                }
                else
                {
                    try
                    {
                        Gen gen(icecpp->getBaseName(), headerExtension, sourceExtension, extraHeaders, include,
                                includePaths, dllExport, output, implCpp98, implCpp11, checksum, ice);
                        gen.generate(u);
                    }
                    catch(const Slice::FileException& ex)
                    {
                        // If a file could not be created, then
                        // cleanup any created files.
                        FileTracker::instance()->cleanup();
                        u->destroy();
                        consoleErr << argv[0] << ": error: " << ex.reason() << endl;
                        return EXIT_FAILURE;
                    }
                }

                u->destroy();
            }
        }

        {
            IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(globalMutex);

            if(interrupted)
            {
                FileTracker::instance()->cleanup();
                return EXIT_FAILURE;
            }
        }
    }

    if(dependxml)
    {
        os << "</dependencies>\n";
    }

    if(depend || dependxml)
    {
        writeDependencies(os.str(), dependFile);
    }

    return status;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    vector<string> args = Slice::argvToArgs(argc, argv);
    try
    {
        return compile(args);
    }
    catch(const std::exception& ex)
    {
        consoleErr << args[0] << ": error:" << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(...)
    {
        consoleErr << args[0] << ": error:" << "unknown exception" << endl;
        return EXIT_FAILURE;
    }
}
