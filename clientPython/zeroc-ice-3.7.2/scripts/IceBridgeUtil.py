#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import *

class IceBridge(ProcessFromBinDir, ProcessIsReleaseOnly, Server):

    def __init__(self, *args, **kargs):
        Server.__init__(self, "icebridge", mapping=Mapping.getByName("cpp"), desc="IceBridge", *args, **kargs)

    def getProps(self, current):
        props = Server.getProps(self, current);
        props.update({
            'Ice.Warn.Dispatch' : 0,
            'IceBridge.Target.Endpoints' : current.getTestEndpoint(0) + ":" + current.getTestEndpoint(0, "udp"),
            'IceBridge.Source.Endpoints' : current.getTestEndpoint(1) + ":" + current.getTestEndpoint(1, "udp"),
            'Ice.Admin.Endpoints' : current.getTestEndpoint(2, "tcp"),
            'Ice.Admin.InstanceName' : "IceBridge",

        })
        del props['Ice.ThreadPool.Server.Size']
        del props['Ice.ThreadPool.Server.SizeMax']
        del props['Ice.ThreadPool.Server.SizeWarn']
        return props
