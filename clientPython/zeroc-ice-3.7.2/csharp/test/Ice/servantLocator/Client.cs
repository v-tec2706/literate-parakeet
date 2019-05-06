//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace servantLocator
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.servantLocator.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    var obj = AllTests.allTests(this);
                    obj.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
