<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

class ClientWithProfile extends TestHelper
{
    function run($args)
    {
        global $NS;
        $properties = call_user_func($NS ? "\\Ice\\getProperties" : "Ice_getProperties", "Test");
        test($properties->getPropertyAsInt("Ice.Trace.Protocol") == 1);
        test($properties->getPropertyAsInt("Ice.Trace.Network") == 1);
        test($properties->getPropertyAsInt("Ice.Warn.Connections") == 1);
        test($properties->getProperty("Hello.Proxy") == "hello:tcp -h localhost -p 10000");
    }
}

?>
