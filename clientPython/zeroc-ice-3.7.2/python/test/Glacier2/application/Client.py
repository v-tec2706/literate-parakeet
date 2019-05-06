#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import sys
import threading
import Ice
import Glacier2

from TestHelper import TestHelper
TestHelper.loadSlice("Callback.ice")
import Test


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


class CallbackReceiverI(Test.CallbackReceiver):

    def __init__(self):
        self._received = False
        self._cond = threading.Condition()

    def callback(self, current):
        with self._cond:
            self._received = True
            self._cond.notify()

    def waitForCallback(self):
        with self._cond:
            while not self._received:
                self._cond.wait()
            self._received = False;


class Application(Glacier2.Application):

    def __init__(self, helper):
        Glacier2.Application.__init__(self)
        self._restart = 0
        self._destroyed = False
        self._receiver = CallbackReceiverI()
        self._helper = helper

    def createSession(self):
        return Glacier2.SessionPrx.uncheckedCast(self.router().createSession("userid", "abc123"))

    def runWithSession(self, args):

        test(self.router());
        test(self.categoryForClient());
        test(self.objectAdapter());

        if self._restart == 0:
            sys.stdout.write("testing Glacier2::Application restart... ")
            sys.stdout.flush()

        base = self.communicator().stringToProxy("callback:{0}".format(
            self._helper.getTestEndpoint(properties=self.communicator().getProperties())));
        callback = Test.CallbackPrx.uncheckedCast(base)
        self._restart += 1
        if self._restart < 5:
            receiver = Test.CallbackReceiverPrx.uncheckedCast(self.addWithUUID(self._receiver))
            callback.initiateCallback(receiver)
            self._receiver.waitForCallback()
            self.restart()

        print("ok")

        sys.stdout.write("testing server shutdown... ")
        callback.shutdown()
        print("ok")

        return 0

    def sessionDestroyed(self):
        self._destroyed = True


class Client(TestHelper):

    def run(self, args):
        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)
        initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:{0}".format(
            self.getTestEndpoint(properties=initData.properties, num=50)))

        app = Application(self)
        status = app.main(sys.argv, initData=initData)
        test(status == 0)
        test(app._restart == 5)
        test(app._destroyed)

        initData.properties.setProperty("Ice.Default.Router", "")
        with self.initialize(initData=initData) as communicator:
            sys.stdout.write("testing stringToProxy for process object... ")
            sys.stdout.flush()
            processBase = communicator.stringToProxy("Glacier2/admin -f Process:{0}".format(
                self.getTestEndpoint(properties=initData.properties, num=51)))
            print("ok")

            sys.stdout.write("testing checked cast for admin object... ")
            sys.stdout.flush()
            process = Ice.ProcessPrx.checkedCast(processBase)
            test(process)
            print("ok")

            sys.stdout.write("testing Glacier2 shutdown... ")
            sys.stdout.flush()
            process.shutdown()
            try:
                process.ice_ping()
                test(False)
            except Ice.LocalException:
                print("ok")
