#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

testcases = [
    ClientServerTestCase(),
    ClientServerTestCase("client/server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/exceptions", "serveramd"):
    testcases += [
        ClientAMDServerTestCase(),
        ClientAMDServerTestCase("client/amd server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
    ]

TestSuite(__name__, testcases, options = { "serialize" : [False] })
