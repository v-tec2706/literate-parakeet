//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace proxy
    {
        public sealed class MyDerivedClassI : Test.MyDerivedClassDisp_
        {
            public MyDerivedClassI()
            {
            }

            public override Ice.ObjectPrx echo(Ice.ObjectPrx obj, Ice.Current c)
            {
                return obj;
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public override Dictionary<string, string> getContext(Ice.Current current)
            {
                return _ctx;
            }

            public override bool ice_isA(string s, Ice.Current current)
            {
                _ctx = current.ctx;
                return base.ice_isA(s, current);
            }

            private Dictionary<string, string> _ctx;
        }
    }
}
