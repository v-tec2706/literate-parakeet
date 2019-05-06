#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("--all -I. Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty('Ice.Warn.Dispatch', '0')
        with self.initialize(properties=properties) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
