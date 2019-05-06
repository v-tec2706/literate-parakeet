//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const test = require("TestHelper").TestHelper.test;

    class AMDThrowerI extends Test.Thrower
    {
        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        supportsUndeclaredExceptions(current)
        {
            return true;
        }

        supportsAssertException(current)
        {
            return false;
        }

        throwAasA(a, current)
        {
            throw new Test.A(a);
        }

        throwAorDasAorD(a, current)
        {
            if(a > 0)
            {
                throw new Test.A(a);
            }
            else
            {
                throw new Test.D(a);
            }
        }

        throwBasA(a, b, current)
        {
            return this.throwBasB(a, b, current);
        }

        throwBasB(a, b, current)
        {
            throw new Test.B(a, b);
        }

        throwCasA(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        throwCasB(a, b, c, current)
        {
            return this.throwCasC(a, b, c, current);
        }

        throwCasC(a, b, c, current)
        {
            throw new Test.C(a, b, c);
        }

        throwUndeclaredA(a, current)
        {
            throw new Test.A(a);
        }

        throwUndeclaredB(a, b, current)
        {
            throw new Test.B(a, b);
        }

        throwUndeclaredC(a, b, c, current)
        {
            throw new Test.C(a, b, c);
        }

        throwLocalException(current)
        {
            throw new Ice.TimeoutException();
        }

        throwLocalExceptionIdempotent(current)
        {
            throw new Ice.TimeoutException();
        }

        throwNonIceException(current)
        {
            throw new Error();
        }

        throwAssertException(current)
        {
            test(false);
        }

        throwMemoryLimitException(seq, current)
        {
            return new Uint8Array(1024 * 20); // 20KB is over the configured 10KB message size max.
        }

        throwAfterResponse(current)
        {
        }

        throwAfterException(current)
        {
            throw new Test.A();
        }
    }
    exports.AMDThrowerI = AMDThrowerI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
