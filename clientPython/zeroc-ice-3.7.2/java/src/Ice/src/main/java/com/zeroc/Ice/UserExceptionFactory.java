//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Instantiates user exceptions.
 *
 * @see InputStream#throwException
 **/
public interface UserExceptionFactory
{
    /**
     * Instantiate a user exception with the given Slice type ID (such as <code>::Module::MyException</code>)
     * and throw it. If the implementation does not throw an exception, the Ice run time will fall back
     * to using its default behavior for instantiating the user exception.
     *
     * @param typeId The Slice type ID of the user exception to be created.
     * @throws UserException A user exception instance corresponding to the type ID.
     **/
    void createAndThrow(String typeId)
        throws UserException;
}
