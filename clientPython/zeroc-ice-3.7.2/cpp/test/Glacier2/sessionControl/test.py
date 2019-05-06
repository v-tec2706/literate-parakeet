# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# Note: we limit the send buffer size with Ice.TCP.SndSize, the
# test relies on send() blocking
#
routerProps = lambda process, current : {
    'Glacier2.SessionManager' : 'SessionManager:{0}'.format(current.getTestEndpoint(0)),
    'Glacier2.PermissionsVerifier' : 'Glacier2/NullPermissionsVerifier',
}

Glacier2TestSuite(__name__, testcases=[ClientServerTestCase(servers=[Glacier2Router(props=routerProps), Server()])])
