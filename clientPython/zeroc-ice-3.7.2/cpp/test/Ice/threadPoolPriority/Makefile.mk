#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs := client server servercustom

$(test)_servercustom_sources    := ServerCustomThreadPool.cpp Test.ice TestI.cpp

tests += $(test)
