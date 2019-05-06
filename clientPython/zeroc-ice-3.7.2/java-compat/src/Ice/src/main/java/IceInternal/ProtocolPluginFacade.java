//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface ProtocolPluginFacade
{
    //
    // Get the Communicator instance with which this facade is
    // associated.
    //
    Ice.Communicator getCommunicator();

    //
    // Register an EndpointFactory.
    //
    void addEndpointFactory(EndpointFactory factory);

    //
    // Get an EndpointFactory.
    //
    EndpointFactory getEndpointFactory(short type);

    //
    // Look up a Java class by name.
    //
    Class<?> findClass(String className);
}
