#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice, Test, MyObjectI, sys

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator):

    oa = communicator.createObjectAdapterWithEndpoints("MyOA", "tcp -h localhost")
    oa.activate()

    servant = MyObjectI.MyObjectI()

    # Register default servant with category "foo"
    oa.addDefaultServant(servant, "foo")

    # Start test
    sys.stdout.write("testing single category... ")
    sys.stdout.flush()

    r = oa.findDefaultServant("foo")
    test(r == servant)

    r = oa.findDefaultServant("bar")
    test(r == None)

    identity = Ice.Identity()
    identity.category = "foo"

    names = ( "foo", "bar", "x", "y", "abcdefg" )

    for idx in range(0, 5):
        identity.name = names[idx]
        prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))
        prx.ice_ping()
        test(prx.getName() == names[idx])

    identity.name = "ObjectNotExist"
    prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))
    try:
        prx.ice_ping()
        test(False)
    except Ice.ObjectNotExistException:
        # Expected
        pass

    try:
        prx.getName()
        test(False)
    except Ice.ObjectNotExistException:
        # Expected
        pass

    identity.name = "FacetNotExist"
    prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))
    try:
        prx.ice_ping()
        test(False)
    except Ice.FacetNotExistException:
        # Expected
        pass

    try:
        prx.getName()
        test(False)
    except Ice.FacetNotExistException:
        # Expected
        pass

    identity.category = "bar"
    for idx in range(0, 5):
        identity.name = names[idx]
        prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))

        try:
            prx.ice_ping()
            test(False)
        except Ice.ObjectNotExistException:
            # Expected
            pass

        try:
            prx.getName()
            test(False)
        except Ice.ObjectNotExistException:
            # Expected
            pass

    oa.removeDefaultServant("foo")
    identity.category = "foo"
    prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))
    try:
        prx.ice_ping()
    except Ice.ObjectNotExistException:
        # Expected
        pass

    print("ok")

    sys.stdout.write("testing default category... ")
    sys.stdout.flush()

    oa.addDefaultServant(servant, "")

    r = oa.findDefaultServant("bar")
    test(r == None)

    r = oa.findDefaultServant("")
    test(r == servant)

    for idx in range(0, 5):
        identity.name = names[idx]
        prx = Test.MyObjectPrx.uncheckedCast(oa.createProxy(identity))
        prx.ice_ping()
        test(prx.getName() == names[idx])

    print("ok")
