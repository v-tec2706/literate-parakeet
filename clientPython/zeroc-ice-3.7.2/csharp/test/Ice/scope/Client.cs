//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace scope
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.scope.TypeId"};
                initData.properties = createTestProperties(ref args);
                using(var communicator = initialize(initData))
                {
                    var output = getWriter();
                    output.Write("test same Slice type name in different scopes... ");
                    output.Flush();
                    AllTests.allTests(this);
                    output.WriteLine("ok");
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
