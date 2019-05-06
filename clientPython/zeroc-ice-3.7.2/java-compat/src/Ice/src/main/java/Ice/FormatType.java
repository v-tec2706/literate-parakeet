//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * This enumeration describes the possible formats for classes and exceptions.
 **/
public enum FormatType
{
    /**
     * Indicates that no preference was specified.
     **/
    DefaultFormat,
    /**
     * A minimal format that eliminates the possibility for slicing unrecognized types.
     **/
    CompactFormat,
    /**
     * Allow slicing and preserve slices for unknown types.
     **/
    SlicedFormat
}
