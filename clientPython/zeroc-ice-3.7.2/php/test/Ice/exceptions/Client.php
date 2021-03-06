<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

require_once('Test.php');

function allTests($helper)
{
    global $NS;

    echo "testing stringToProxy... ";
    flush();
    $ref = sprintf("thrower:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $base = $communicator->stringToProxy($ref);
    test($base != null);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $thrower = $base->ice_checkedCast("::Test::Thrower");
    test($thrower != null);
    test($thrower == $base);
    echo "ok\n";

    echo "catching exact types... ";
    flush();

    try
    {
        $thrower->throwAasA(1);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsA = $NS ? "Test\\A" : "Test_A";
        if(!($ex instanceof $clsA))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(1);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsA = $NS ? "Test\\A" : "Test_A";
        if(!($ex instanceof $clsA))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(-1);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsD = $NS ? "Test\\D" : "Test_D";
        if(!($ex instanceof $clsD))
        {
            throw $ex;
        }
        test($ex->dMem == -1);
    }

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsB = $NS ? "Test\\B" : "Test_B";
        if(!($ex instanceof $clsB))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsC = $NS ? "Test\\C" : "Test_C";
        if(!($ex instanceof $clsC))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    echo "ok\n";

    echo "catching base types... ";
    flush();

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsA = $NS ? "Test\\A" : "Test_A";
        if(!($ex instanceof $clsA))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsB = $NS ? "Test\\B" : "Test_B";
        if(!($ex instanceof $clsB))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    echo "ok\n";

    echo "catching derived types... ";
    flush();

    try
    {
        $thrower->throwBasA(1, 2);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsB = $NS ? "Test\\B" : "Test_B";
        if(!($ex instanceof $clsB))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasA(1, 2, 3);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsC = $NS ? "Test\\C" : "Test_C";
        if(!($ex instanceof $clsC))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    try
    {
        $thrower->throwCasB(1, 2, 3);
        test(false);
    }
    catch(Exception $ex)
    {
        $clsC = $NS ? "Test\\C" : "Test_C";
        if(!($ex instanceof $clsC))
        {
            throw $ex;
        }
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    echo "ok\n";

    if($thrower->supportsUndeclaredExceptions())
    {
        echo "catching unknown user exception... ";
        flush();

        try
        {
            $thrower->throwUndeclaredA(1);
            test(false);
        }
        catch(Exception $ex)
        {
            $uue = $NS ? "Ice\\UnknownUserException" : "Ice_UnknownUserException";
            if(!($ex instanceof $uue))
            {
                throw $ex;
            }
        }

        try
        {
            $thrower->throwUndeclaredB(1, 2);
            test(false);
        }
        catch(Exception $ex)
        {
            $uue = $NS ? "Ice\\UnknownUserException" : "Ice_UnknownUserException";
            if(!($ex instanceof $uue))
            {
                throw $ex;
            }
        }

        try
        {
            $thrower->throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch(Exception $ex)
        {
            $uue = $NS ? "Ice\\UnknownUserException" : "Ice_UnknownUserException";
            if(!($ex instanceof $uue))
            {
                throw $ex;
            }
        }

        echo "ok\n";
    }

    {
        echo "testing memory limit marshal exception...";
        flush();
        try
        {
            $thrower->throwMemoryLimitException(array(0x00));
            test(false);
        }
        catch(Exception $ex)
        {
            $uue = $NS ? "Ice\\MemoryLimitException" : "Ice_MemoryLimitException";
            if(!($ex instanceof $uue))
            {
                throw $ex;
            }
        }

        try
        {
            $thrower->throwMemoryLimitException(array_pad(array(), 20 * 1024, 0x00));
            test(false);
        }
        catch(Exception $ex)
        {
            $uue = $NS ? "Ice\\ConnectionLostException" : "Ice_ConnectionLostException";
            $ule = $NS ? "Ice\\UnknownLocalException" : "Ice_UnknownLocalException";
            if(!($ex instanceof $uue) && !($ex instanceof $ule))
            {
                throw $ex;
            }
        }

        echo "ok\n";
    }

    echo "catching object not exist exception... ";
    flush();

    $stringToIdentity = $NS ? "Ice\\stringToIdentity" : "Ice_stringToIdentity";
    $id = $stringToIdentity("does not exist");
    try
    {
        $thrower2 = $thrower->ice_identity($id)->ice_uncheckedCast("::Test::Thrower");
        $thrower2->throwAasA(1);
        test(false);
    }
    catch(Exception $ex)
    {
        $one = $NS ? "Ice\\ObjectNotExistException" : "Ice_ObjectNotExistException";
        if(!($ex instanceof $one))
        {
            throw $ex;
        }
        test($ex->id == $id);
    }

    echo "ok\n";

    echo "catching facet not exist exception... ";
    flush();

    {
        $thrower2 = $thrower->ice_uncheckedCast("::Test::Thrower", "no such facet");
        try
        {
            $thrower2->ice_ping();
            test(false);
        }
        catch(Exception $ex)
        {
            $fne = $NS ? "Ice\\FacetNotExistException" : "Ice_FacetNotExistException";
            if(!($ex instanceof $fne))
            {
                throw $ex;
            }
            test($ex->facet == "no such facet");
        }
    }

    echo "ok\n";

    echo "catching operation not exist exception... ";
    flush();

    try
    {
        $thrower2 = $thrower->ice_uncheckedCast("::Test::WrongOperation");
        $thrower2->noSuchOperation();
        test(false);
    }
    catch(Exception $ex)
    {
        $one = $NS ? "Ice\\OperationNotExistException" : "Ice_OperationNotExistException";
        if(!($ex instanceof $one))
        {
            throw $ex;
        }
        test($ex->operation = "noSuchOperation");
    }

    echo "ok\n";

    echo "catching unknown local exception... ";
    flush();

    try
    {
        $thrower->throwLocalException();
        test(false);
    }
    catch(Exception $ex)
    {
        $ule = $NS ? "Ice\\UnknownLocalException" : "Ice_UnknownLocalException";
        if(!($ex instanceof $ule))
        {
            throw $ex;
        }
    }

    try
    {
        $thrower->throwLocalExceptionIdempotent();
        test(false);
    }
    catch(Exception $ex)
    {
        $ule = $NS ? "Ice\\UnknownLocalException" : "Ice_UnknownLocalException";
        $one = $NS ? "Ice\\OperationNotExistException" : "Ice_OperationNotExistException";
        if(!($ex instanceof $ule) && !($ex instanceof $one))
        {
            throw $ex;
        }
    }

    echo "ok\n";

    echo "catching unknown non-Ice exception... ";
    flush();

    try
    {
        $thrower->throwNonIceException();
        test(false);
    }
    catch(Exception $ex)
    {
        $ue = $NS ? "Ice\\UnknownException" : "Ice_UnknownException";
        if(!($ex instanceof $ue))
        {
            throw $ex;
        }
    }

    echo "ok\n";

    return $thrower;
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $properties = $this->createTestProperties($args);
            $properties->setProperty("Ice.MessageSizeMax", "10");
            //
            // This property is set by the test suite, howerver we need to override it for this test.
            // Unlike C++, we can not pass $argv into Ice::createProperties, so we just set it after.
            //
            $properties->setProperty("Ice.Warn.Connections", "0");
            $communicator = $this->initialize($properties);
            $proxy= allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
?>
