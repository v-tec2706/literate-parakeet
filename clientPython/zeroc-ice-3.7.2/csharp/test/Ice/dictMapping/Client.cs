//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace dictMapping
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                using(var communicator = initialize(ref args))
                {
                    var output = getWriter();
                    var myClass = AllTests.allTests(this, false);
                    output.Write("shutting down server... ");
                    output.Flush();
                    myClass.shutdown();
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
