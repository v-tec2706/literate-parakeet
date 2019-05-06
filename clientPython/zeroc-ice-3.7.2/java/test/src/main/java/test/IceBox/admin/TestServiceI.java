//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceBox.admin;

public class TestServiceI implements com.zeroc.IceBox.Service
{
    public TestServiceI(com.zeroc.Ice.Communicator serviceManagerCommunicator)
    {
        TestFacetI facet = new TestFacetI();

        //
        // Install a custom admin facet.
        //
        serviceManagerCommunicator.addAdminFacet(facet, "TestFacet");

        //
        // The TestFacetI servant also implements java.util.function.Consumer<java.util.Map<String, String>>.
        // Set the callback on the admin facet.
        //
        com.zeroc.Ice.Object propFacet =
            serviceManagerCommunicator.findAdminFacet("IceBox.Service.TestService.Properties");
        if(propFacet != null)
        {
            com.zeroc.Ice.NativePropertiesAdmin admin = (com.zeroc.Ice.NativePropertiesAdmin)propFacet;
            admin.addUpdateCallback(facet);
        }
    }

    @Override
    public void start(String name, com.zeroc.Ice.Communicator communicator, String[] args)
    {
    }

    @Override
    public void stop()
    {
    }
}
