#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice('Test.ice')
import Test
import Inner
import Ice


class I1(Test.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class I2(Test.Inner.Inner2.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class I3(Test.Inner.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class I4(Inner.Test.Inner2.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(I1(), Ice.stringToIdentity("i1"))
            adapter.add(I2(), Ice.stringToIdentity("i2"))
            adapter.add(I3(), Ice.stringToIdentity("i3"))
            adapter.add(I4(), Ice.stringToIdentity("i4"))
            adapter.activate()
            communicator.waitForShutdown()
