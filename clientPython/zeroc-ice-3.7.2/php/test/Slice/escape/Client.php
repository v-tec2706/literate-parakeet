<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

$NS = function_exists("Ice\\initialize");
require_once('Ice.php');
require_once('Key.php');
require_once('Clash.php');

if($NS)
{
    $code = <<<EOT
        abstract class and_echo extends _and\_echo {}
EOT;
    eval($code);
}

class echoI extends and_echo
{
    public function _else($a, $b)
    {
    }
}

function allTests($helper)
{
    global $NS;

    echo "testing type names... ";
    flush();
    $a = $NS ? constant("_and\\_array::_as") : constant("and_array::_as");
    $b = $NS ? eval("return new _and\\_xor();") : eval("return new and_xor();");
    test($b->_abstract == 0);
    test($b->_clone == 0);
    test($b->_private == 0);
    test($b->_protected == 0);
    test($b->_public == 0);
    test($b->_this == 0);
    test($b->_throw == 0);
    test($b->_use == 0);
    test($b->_var == 0);
    $communicator = $helper->communicator();
    $p = $communicator->stringToProxy("test:tcp -p 10000");
    $c = $NS ? eval("return _and\\functionPrxHelper::uncheckedCast(\$p);") :
               eval("return and_functionPrxHelper::uncheckedCast(\$p);");
    $d = $NS ? eval("return _and\\diePrxHelper::uncheckedCast(\$p);") :
               eval("return and_diePrxHelper::uncheckedCast(\$p);");
    $e1 = new echoI();
    $f = $NS ? eval("return _and\\enddeclarePrxHelper::uncheckedCast(\$p);") :
               eval("return and_enddeclarePrxHelper::uncheckedCast(\$p);");
    $g = $NS ? eval("return new _and\\_endif();") : eval("return new and_endif();");
    $h = $NS ? eval("return new _and\\_endwhile();") : eval("return new and_endwhile();");
    $i = $NS ? constant("_and\\_or") : constant("and_or");
    $j = $NS ? constant("_and\\_print") : constant("and_print");
    $j = $NS ? constant("_and\\_require_once") : constant("and_require_once");
    echo "ok\n";
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            allTests($this);
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
