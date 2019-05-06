#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_libraries := $(test)_TestService $(test)_RegistryPlugin

$(test)_TestService_sources             = Service.cpp TestI.cpp Test.ice
$(test)_TestService_dependencies        = IceBox
$(test)_TestService_version             =
$(test)_TestService_soversion           =

$(test)_RegistryPlugin_sources          = RegistryPlugin.cpp
$(test)_RegistryPlugin_dependencies     = IceGrid Glacier2

$(test)_client_dependencies             = IceGrid Glacier2

tests += $(test)
