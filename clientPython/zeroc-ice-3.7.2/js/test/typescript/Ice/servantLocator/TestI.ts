//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import {Ice} from "ice";
import {Test} from "./generated";

export class TestI extends Test.TestIntf
{
    requestFailedException(current:Ice.Current):void
    {
    }

    unknownUserException(current:Ice.Current):void
    {
    }

    unknownLocalException(current:Ice.Current):void
    {
    }

    unknownException(current:Ice.Current):void
    {
    }

    localException(current:Ice.Current):void
    {
    }

    userException(current:Ice.Current):void
    {
    }

    jsException(current:Ice.Current):void
    {
        throw new Error();
    }

    javaException(current:Ice.Current):void
    {
    }

    unknownExceptionWithServantException(current:Ice.Current):void
    {
        throw new Ice.ObjectNotExistException();
    }

    impossibleException(throwEx:boolean, current:Ice.Current):string
    {
        if(throwEx)
        {
            throw new Test.TestImpossibleException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    intfUserException(throwEx:boolean, current:Ice.Current):string
    {
        if(throwEx)
        {
            throw new Test.TestIntfUserException();
        }

        //
        // Return a value so we can be sure that the stream position
        // is reset correctly if finished() throws.
        //
        return "Hello";
    }

    asyncResponse(current:Ice.Current):void
    {
        throw new Ice.ObjectNotExistException();
    }

    asyncException(current:Ice.Current):void
    {
        throw new Ice.ObjectNotExistException();
    }

    shutdown(current:Ice.Current):void
    {
        current.adapter.deactivate();
    }
}
