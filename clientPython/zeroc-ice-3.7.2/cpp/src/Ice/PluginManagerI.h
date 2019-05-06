//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PLUGIN_MANAGER_I_H
#define ICE_PLUGIN_MANAGER_I_H

#include <Ice/Plugin.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/DynamicLibraryF.h>
#include <Ice/BuiltinSequences.h>
#include <IceUtil/Mutex.h>
#include <map>

namespace Ice
{

typedef Ice::Plugin* (*PluginFactory)(const ::Ice::CommunicatorPtr&, const std::string&, const ::Ice::StringSeq&);

class PluginManagerI : public PluginManager, public IceUtil::Mutex
{
public:

    static void registerPluginFactory(const std::string&, PluginFactory, bool);

    virtual void initializePlugins();
    virtual StringSeq getPlugins() ICE_NOEXCEPT;
    virtual PluginPtr getPlugin(const std::string&);
    virtual void addPlugin(const std::string&, const PluginPtr&);
    virtual void destroy() ICE_NOEXCEPT;
    PluginManagerI(const CommunicatorPtr&, const IceInternal::DynamicLibraryListPtr&);

private:

    friend class IceInternal::Instance;

    void loadPlugins(int&, const char*[]);
    void loadPlugin(const std::string&, const std::string&, StringSeq&);

    PluginPtr findPlugin(const std::string&) const;

    CommunicatorPtr _communicator;
    IceInternal::DynamicLibraryListPtr _libraries;

    struct PluginInfo
    {
        std::string name;
        PluginPtr plugin;
    };
    typedef std::vector<PluginInfo> PluginInfoList;

    PluginInfoList _plugins;
    bool _initialized;
    static const char * const _kindOfObject;
};
typedef IceUtil::Handle<PluginManagerI> PluginManagerIPtr;

}

#endif
