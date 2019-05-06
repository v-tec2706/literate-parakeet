//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace servantLocator
    {
        public sealed class TestActivationI : Test.TestActivationDisp_
        {
            override public void activateServantLocator(bool activate, Ice.Current current)
            {
                if(activate)
                {
                    current.adapter.addServantLocator(new ServantLocatorI(""), "");
                    current.adapter.addServantLocator(new ServantLocatorI("category"), "category");
                }
                else
                {
                    var locator = current.adapter.removeServantLocator("");
                    locator.deactivate("");
                    locator = current.adapter.removeServantLocator("category");
                    locator.deactivate("category");
                }
            }
        }
    }
}
