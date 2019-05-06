# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class IceGridDistributionTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)

        datadir = os.path.join(current.testsuite.getPath(), "data")

        files = [
          [ "original/rootfile", "rootfile" ],
          [ "original/dir1/file1", "dummy-file1"],
          [ "original/dir1/file2", "dummy-file2"],
          [ "original/dir2/file3", "dummy-file3"],
          [ "updated/rootfile", "rootfile-updated!" ],
          [ "updated/dir1/file2", "dummy-file2-updated!"],
          [ "updated/dir2/file3", "dummy-file3"],
          [ "updated/dir2/file4", "dummy-file4"],
        ]

        current.write("creating IcePatch2 data directory... ")
        current.mkdirs(datadir)

        for [file, content] in files:
            file = os.path.join(datadir, file)
            if not os.path.exists(os.path.dirname(file)):
                os.makedirs(os.path.dirname(file))
            f = open(file, 'w')
            f.write(content)
            f.close()

        IcePatch2Calc(args=[os.path.join(datadir, "original")]).run(current)
        IcePatch2Calc(args=[os.path.join(datadir, "updated")]).run(current)

        current.writeln("ok")

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [ IceGridDistributionTestCase() ], runOnMainThread=True, multihost=False)
