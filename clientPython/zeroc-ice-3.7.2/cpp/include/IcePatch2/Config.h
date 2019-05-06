//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PATCH2_CONFIG_H
#define ICE_PATCH2_CONFIG_H

//
// Automatically link with IcePatch2[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_PATCH2) && defined(ICEPATCH2_API_EXPORTS)
#   define ICE_BUILDING_ICE_PATCH2
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_PATCH2)
#   pragma comment(lib, ICE_LIBNAME("IcePatch2"))
#endif

#endif
