#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Default.Locator", "locator:{0}".format(self.getTestEndpoint(properties=properties)))
        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator)
