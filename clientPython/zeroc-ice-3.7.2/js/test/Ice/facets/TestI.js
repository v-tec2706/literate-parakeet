//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Test = require("Test").Test;

    class DI extends Test.D
    {
        callA(current)
        {
            return "A";
        }

        callB(current)
        {
            return "B";
        }

        callC(current)
        {
            return "C";
        }

        callD(current)
        {
            return "D";
        }
    }

    class EmptyI extends Test.Empty
    {
    }

    class FI extends Test.F
    {
        callE(current)
        {
            return "E";
        }

        callF(current)
        {
            return "F";
        }
    }

    class HI extends Test.H
    {
        callG(current)
        {
            return "G";
        }

        callH(current)
        {
            return "H";
        }

        shutdown(current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    exports.DI = DI;
    exports.EmptyI = EmptyI;
    exports.FI = FI;
    exports.HI = HI;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
