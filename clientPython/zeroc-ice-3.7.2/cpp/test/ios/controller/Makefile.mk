#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_type                 = xcode
$(project)_platforms            = iphonesimulator iphoneos
$(project)_configs              = xcodesdk cpp11-xcodesdk
$(project)_dependencies         = $(tests)
$(project)_project              = C++ Test Controller.xcodeproj
$(project)_scheme               = $(if $(filter cpp11-xcodesdk,$2),C++11 Test Controller,C++98 Test Controller)
$(project)_configuration        = $(if $(filter $(OPTIMIZE),yes),Release,Debug)

projects += $(project)
tests:: $(project)
