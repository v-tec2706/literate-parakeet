#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice('Test.ice')
import Ice
import Test
import time


class TestI(Test.Hello):
    def sayHello(self, delay, current=None):
        if delay != 0:
            time.sleep(delay / 1000.0)

    def raiseUE(self, current=None):
        raise Test.UE()

    def add(self, s1, s2, current=None):
        return s1 + s2

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we supress
        # this warning.
        #
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
