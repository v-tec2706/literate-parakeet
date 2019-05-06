#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice('Test.ice')
import sys
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError('test assertion failed')


class Client(TestHelper):

    def run(self, args):
        sys.stdout.write("testing Slice predefined macros... ")
        sys.stdout.flush()

        d = Test.Default()
        test(d.x == 10)
        test(d.y == 10)

        nd = Test.NoDefault()
        test(nd.x != 10)
        test(nd.y != 10)

        c = Test.PythonOnly()
        test(c.lang == "python")
        test(c.version == Ice.intVersion())
        print("ok")
