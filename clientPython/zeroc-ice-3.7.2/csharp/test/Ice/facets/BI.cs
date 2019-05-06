//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace facets
    {
        public sealed class BI : Test.BDisp_
        {
            public BI()
            {
            }

            public override string callA(Ice.Current current)
            {
                return "A";
            }

            public override string callB(Ice.Current current)
            {
                return "B";
            }
        }
    }
}
