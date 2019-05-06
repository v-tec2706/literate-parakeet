# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class IceGridUpdateTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node-1")
        current.mkdirs("db/node-2")

clientProps = lambda process, current: {
    "NodePropertiesOverride" : current.testcase.icegridnode[0].getPropertiesOverride(current),
    "IceBoxExe" : IceBox().getCommandLine(current),
    "IceGridNodeExe" : IceGridNode().getCommandLine(current),
    "ServerDir" : current.getBuildDir("server"),
    "TestDir" : "{testdir}"
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [IceGridUpdateTestCase(application=None, client=IceGridClient(props=clientProps))], multihost=False)
