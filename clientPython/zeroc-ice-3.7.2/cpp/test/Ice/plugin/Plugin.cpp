//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace std;

namespace
{

class Plugin : public Ice::Plugin
{

public:

    Plugin(const Ice::CommunicatorPtr& communicator) :
         _communicator(communicator),
         _initialized(false),
         _destroyed(false)
    {
    }

    void
    initialize()
    {
        _initialized = true;
    }

    void
    destroy()
    {
        _destroyed = true;
    }

    ~Plugin()
    {
        test(_initialized);
        test(_destroyed);
    }

private:

    const Ice::CommunicatorPtr _communicator;
    Ice::StringSeq _args;
    bool _initialized;
    bool _destroyed;
};

class PluginInitializeFailExeption : public std::exception
{

public:

    PluginInitializeFailExeption() ICE_NOEXCEPT {}
#ifndef ICE_CPP11_COMPILER
    virtual ~PluginInitializeFailExeption() throw() {}
#endif
    virtual const char* what() const ICE_NOEXCEPT { return "PluginInitializeFailExeption"; }
};

class PluginInitializeFail : public Ice::Plugin
{

public:

    PluginInitializeFail(const Ice::CommunicatorPtr& communicator) :
         _communicator(communicator)
    {
    }

    void
    initialize()
    {
        throw PluginInitializeFailExeption();
    }

    void
    destroy()
    {
        test(false);
    }

private:

    const Ice::CommunicatorPtr _communicator;
};

class BasePlugin;
ICE_DEFINE_PTR(BasePluginPtr, BasePlugin);

class BasePlugin : public Ice::Plugin
{

public:

    BasePlugin(const Ice::CommunicatorPtr& communicator) :
         _communicator(communicator),
         _initialized(false),
         _destroyed(false)
    {
    }

    bool
    isInitialized() const
    {
        return _initialized;
    }

    bool
    isDestroyed() const
    {
        return _destroyed;
    }

protected:

    const Ice::CommunicatorPtr _communicator;
    bool _initialized;
    bool _destroyed;
    BasePluginPtr _other;
};

class PluginOne : public BasePlugin
{

public:

    PluginOne(const Ice::CommunicatorPtr& communicator) :
        BasePlugin(communicator)
    {
    }

    void
    initialize()
    {
        _other = ICE_DYNAMIC_CAST(BasePlugin, _communicator->getPluginManager()->getPlugin("PluginTwo"));
        test(!_other->isInitialized());
        _initialized = true;
    }

    void
    destroy()
    {
        _destroyed = true;
        test(_other->isDestroyed());
        _other = 0;
    }
};

class PluginTwo : public BasePlugin
{

public:

    PluginTwo(const Ice::CommunicatorPtr& communicator) :
         BasePlugin(communicator)
    {
    }

    void
    initialize()
    {
        _initialized = true;
        _other = ICE_DYNAMIC_CAST(BasePlugin, _communicator->getPluginManager()->getPlugin("PluginOne"));
        test(_other->isInitialized());
    }

    void
    destroy()
    {
        _destroyed = true;
        test(!_other->isDestroyed());
        _other = 0;
    }
};

class PluginThree : public BasePlugin
{

public:

    PluginThree(const Ice::CommunicatorPtr& communicator) :
         BasePlugin(communicator)
    {
    }

    void
    initialize()
    {
        _initialized = true;
        _other = ICE_DYNAMIC_CAST(BasePlugin, _communicator->getPluginManager()->getPlugin("PluginTwo"));
        test(_other->isInitialized());
    }

    void
    destroy()
    {
        _destroyed = true;
        test(!_other->isDestroyed());
        _other = 0;
    }
};

class BasePluginFail;
ICE_DEFINE_PTR(BasePluginFailPtr, BasePluginFail);

class BasePluginFail : public Ice::Plugin
{

public:

    BasePluginFail(const Ice::CommunicatorPtr& communicator) :
         _communicator(communicator),
         _initialized(false),
         _destroyed(false)
    {
    }

    bool
    isInitialized() const
    {
        return _initialized;
    }

    bool
    isDestroyed() const
    {
        return _destroyed;
    }

protected:

    const Ice::CommunicatorPtr _communicator;
    bool _initialized;
    bool _destroyed;
    BasePluginFailPtr _one;
    BasePluginFailPtr _two;
    BasePluginFailPtr _three;
};

class PluginOneFail : public BasePluginFail
{

public:

    PluginOneFail(const Ice::CommunicatorPtr& communicator) :
        BasePluginFail(communicator)
    {
    }

    void
    initialize()
    {
        _two = ICE_DYNAMIC_CAST(BasePluginFail, _communicator->getPluginManager()->getPlugin("PluginTwoFail"));
        test(!_two->isInitialized());
        _three = ICE_DYNAMIC_CAST(BasePluginFail, _communicator->getPluginManager()->getPlugin("PluginThreeFail"));
        test(!_three->isInitialized());
        _initialized = true;
    }

    void
    destroy()
    {
        test(_two->isDestroyed());
        //
        // Not destroyed because initialize fails.
        //
        test(!_three->isDestroyed());
        _destroyed = true;
        _two = 0;
        _three = 0;
    }

    ~PluginOneFail()
    {
        test(_initialized);
        test(_destroyed);
    }
};

class PluginTwoFail : public BasePluginFail
{

public:

    PluginTwoFail(const Ice::CommunicatorPtr& communicator) :
        BasePluginFail(communicator)
    {
    }

    void
    initialize()
    {
        _initialized = true;
        _one = ICE_DYNAMIC_CAST(BasePluginFail, _communicator->getPluginManager()->getPlugin("PluginOneFail"));
        test(_one->isInitialized());
        _three = ICE_DYNAMIC_CAST(BasePluginFail, _communicator->getPluginManager()->getPlugin("PluginThreeFail"));
        test(!_three->isInitialized());
    }

    void
    destroy()
    {
        _destroyed = true;
        test(!_one->isDestroyed());
        _one = 0;
        _three = 0;
    }

    ~PluginTwoFail()
    {
        test(_initialized);
        test(_destroyed);
    }
};

class PluginThreeFail : public BasePluginFail
{

public:

    PluginThreeFail(const Ice::CommunicatorPtr& communicator) :
         BasePluginFail(communicator)
    {
    }

    void
    initialize()
    {
        throw PluginInitializeFailExeption();
    }

    void
    destroy()
    {
        test(false);
    }

    ~PluginThreeFail()
    {
        test(!_initialized);
        test(!_destroyed);
    }
};

}

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPlugin(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new Plugin(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginWithArgs(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq& args)
{
    test(args.size() == 3);
    test(args[0] == "C:\\Program Files\\");
    test(args[1] == "--DatabasePath");
    test(args[2] == "C:\\Program Files\\Application\\db");
    return new Plugin(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginInitializeFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginInitializeFail(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginOne(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginOne(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginTwo(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginTwo(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginThree(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginThree(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginOneFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginOneFail(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginTwoFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginTwoFail(communicator);
}

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createPluginThreeFail(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginThreeFail(communicator);
}

}
