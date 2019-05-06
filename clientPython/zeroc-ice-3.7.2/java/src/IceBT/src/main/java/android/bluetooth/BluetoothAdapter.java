//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// This is a placeholder for the Android API. It is not included in the IceBT JAR file.
//

package android.bluetooth;

public final class BluetoothAdapter
{
    public boolean cancelDiscovery()
    {
        return false;
    }

    public boolean isEnabled()
    {
        return false;
    }

    public String getAddress()
    {
        return "";
    }

    public static boolean checkBluetoothAddress(String address)
    {
        return false;
    }

    public static BluetoothAdapter getDefaultAdapter()
    {
        return null;
    }

    public BluetoothDevice getRemoteDevice(byte[] address)
    {
        return null;
    }

    public BluetoothDevice getRemoteDevice(String address)
    {
        return null;
    }

    public BluetoothServerSocket listenUsingInsecureRfcommWithServiceRecord(String name, java.util.UUID uuid)
        throws java.io.IOException
    {
        return null;
    }

    public BluetoothServerSocket listenUsingRfcommWithServiceRecord(String name, java.util.UUID uuid)
        throws java.io.IOException
    {
        return null;
    }
}
