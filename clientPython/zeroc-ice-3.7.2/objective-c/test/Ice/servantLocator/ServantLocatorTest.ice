//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

["objc:prefix:TestServantLocator"]
module Test
{

exception TestIntfUserException
{
}

exception TestImpossibleException
{
}

interface TestIntf
{
    void requestFailedException();
    void unknownUserException();
    void unknownLocalException();
    void unknownException();
    void localException();
    void userException();

    void unknownExceptionWithServantException();

    string impossibleException(bool throw) throws TestImpossibleException;
    string intfUserException(bool throw) throws TestIntfUserException, TestImpossibleException;

    void asyncResponse() throws TestIntfUserException, TestImpossibleException;
    void asyncException() throws TestIntfUserException, TestImpossibleException;

    void shutdown();
}

local class Cookie
{
    ["cpp:const"] string message();
}

interface TestActivation
{
    void activateServantLocator(bool activate);
}

}
