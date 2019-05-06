#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries := IceLocatorDiscovery

IceLocatorDiscovery_targetdir                   := $(libdir)
IceLocatorDiscovery_dependencies                := Ice
IceLocatorDiscovery_sliceflags                  := --include-dir IceLocatorDiscovery
IceLocatorDiscovery_generated_includedir        := $(project)/generated/IceLocatorDiscovery
IceLocatorDiscovery_cppflags                    := -DICE_LOCATOR_DISCOVERY_API_EXPORTS

projects += $(project)
