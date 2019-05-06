//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.testcontroller;

import java.io.*;
import java.util.*;

import Ice.Logger;
import Ice.Communicator;

import android.content.Context;
import android.os.Build;
import android.util.Log;
import android.app.Application;

import Test.Common.ProcessControllerRegistryPrxHelper;
import Test.Common.ProcessControllerRegistryPrx;
import Test.Common.ProcessControllerPrx;
import Test.Common.ProcessControllerPrxHelper;

public class ControllerApp extends Application
{
    private final String TAG = "ControllerApp";
    private ControllerI _controllerI;
    private ControllerActivity _activity;
    private java.util.Map<String, ClassLoader> _classLoaders = new java.util.HashMap<String, ClassLoader>();
    private String _ipv4Address;
    private String _ipv6Address;

    static private class TestSuiteBundle
    {
        @SuppressWarnings("unchecked")
        TestSuiteBundle(String name, ClassLoader loader) throws ClassNotFoundException
        {
            _loader = loader;
            _class = (Class<? extends test.TestHelper>)_loader.loadClass(name);
        }

        test.TestHelper newInstance()
                throws IllegalAccessException, InstantiationException
        {
            if(_class == null)
            {
                return null;
            }
            return _class.newInstance();
        }

        ClassLoader getClassLoader()
        {
            return  _loader;
        }

        private String _name;
        private ClassLoader _loader;
        private Class<? extends test.TestHelper> _class;
    }

    class AndroidLogger implements Ice.Logger
    {
        private final String _prefix;

        AndroidLogger(String prefix)
        {
            _prefix = prefix;
        }

        @Override
        public void print(String message)
        {
            Log.d(TAG, message);
        }

        @Override
        public void trace(String category, String message)
        {
            Log.v(category, message);
        }

        @Override
        public void warning(String message)
        {
            Log.w(TAG, message);
        }

        @Override
        public void error(String message)
        {
            Log.e(TAG, message);
        }

        @Override
        public String getPrefix()
        {
            return _prefix;
        }

        @Override
        public Logger cloneWithPrefix(String s)
        {
            return new AndroidLogger(s);
        }
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        Ice.Util.setProcessLogger(new AndroidLogger(""));
    }

    synchronized public void setIpv4Address(String address)
    {
        _ipv4Address = address;
    }

    synchronized public void setIpv6Address(String address)
    {
        int i = address.indexOf("%");
        _ipv6Address = i == -1 ? address : address.substring(i);
    }

    public List<String> getAddresses(boolean ipv6)
    {
        List<String> addresses = new java.util.ArrayList<String>();
        try
        {
            java.util.Enumeration<java.net.NetworkInterface> ifaces = java.net.NetworkInterface.getNetworkInterfaces();
            while(ifaces.hasMoreElements())
            {
                java.net.NetworkInterface iface = ifaces.nextElement();
                java.util.Enumeration<java.net.InetAddress> addrs = iface.getInetAddresses();
                while (addrs.hasMoreElements())
                {
                    java.net.InetAddress addr = addrs.nextElement();
                    if((ipv6 && addr instanceof java.net.Inet6Address) ||
                       (!ipv6 && !(addr instanceof java.net.Inet6Address)))
                    {
                        addresses.add(addr.getHostAddress());
                    }
                }
            }
        }
        catch(java.net.SocketException ex)
        {
        }
        return addresses;
    }

    public synchronized void startController(ControllerActivity activity, boolean bluetooth)
    {
        _activity = activity;
        if(_controllerI == null)
        {
            _controllerI = new ControllerI(bluetooth);
        }
    }

    public synchronized void println(final String data)
    {
        _activity.runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                synchronized(ControllerApp.this)
                {
                    _activity.println(data);
                }
            }
        });
    }

    public static boolean isEmulator()
    {
        return Build.FINGERPRINT.startsWith("generic") ||
               Build.FINGERPRINT.startsWith("unknown") ||
               Build.MODEL.contains("google_sdk") ||
               Build.MODEL.contains("Emulator") ||
               Build.MODEL.contains("Android SDK built for x86") ||
               Build.MANUFACTURER.contains("Genymotion") ||
               (Build.BRAND.startsWith("generic") && Build.DEVICE.startsWith("generic")) ||
               Build.PRODUCT.equals("google_sdk");
    }

    class ControllerI
    {
        public ControllerI(boolean bluetooth)
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.ThreadPool.Server.SizeMax", "10");
            initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp");
            //initData.properties.setProperty("Ice.Trace.Network", "3");
            //initData.properties.setProperty("Ice.Trace.Protocol", "1");
            initData.properties.setProperty("ControllerAdapter.AdapterId", java.util.UUID.randomUUID().toString());
            initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");
            if(!isEmulator())
            {
                if(bluetooth)
                {
                    initData.properties.setProperty("Ice.Plugin.IceBT", "IceBT.PluginFactory");
                }
                initData.properties.setProperty("Ice.Plugin.IceDiscovery", "IceDiscovery.PluginFactory");
                initData.properties.setProperty("IceDiscovery.DomainId", "TestController");
            }
            _communicator = Ice.Util.initialize(initData);
            Ice.ObjectAdapter adapter = _communicator.createObjectAdapter("ControllerAdapter");
            ProcessControllerPrx processController = ProcessControllerPrxHelper.uncheckedCast(
                    adapter.add(new ProcessControllerI(), Ice.Util.stringToIdentity("AndroidCompat/ProcessController")));
            adapter.activate();
            ProcessControllerRegistryPrx registry;
            if(isEmulator())
            {
                registry = ProcessControllerRegistryPrxHelper.uncheckedCast(
                        _communicator.stringToProxy("Util/ProcessControllerRegistry:tcp -h 10.0.2.2 -p 15001"));
            }
            else
            {
                // Use IceDiscovery to find a process controller registry
                registry = ProcessControllerRegistryPrxHelper.uncheckedCast(
                        _communicator.stringToProxy("Util/ProcessControllerRegistry"));
            }
            registerProcessController(adapter, registry, processController);
            println("AndroidCompat/ProcessController");
        }

        public void
        registerProcessController(final Ice.ObjectAdapter adapter,
                                  final ProcessControllerRegistryPrx registry,
                                  final ProcessControllerPrx processController)
        {
            registry.begin_ice_ping(
                new Ice.Callback()
                {
                    public void completed(Ice.AsyncResult r)
                    {
                        try
                        {
                            registry.end_ice_ping(r);
                            Ice.Connection connection = registry.ice_getCachedConnection();
                            connection.setAdapter(adapter);
                            connection.setACM(new Ice.IntOptional(5),
                                              new Ice.Optional<Ice.ACMClose>(Ice.ACMClose.CloseOff),
                                              new Ice.Optional<Ice.ACMHeartbeat>(Ice.ACMHeartbeat.HeartbeatAlways));

                            connection.setCloseCallback(new Ice.CloseCallback()
                                {
                                    @Override
                                    public void closed(Ice.Connection con)
                                    {
                                        println("connection with process controller registry closed");
                                        while(true)
                                        {
                                            try
                                            {
                                                Thread.sleep(500);
                                                break;
                                            }
                                            catch(InterruptedException e)
                                            {
                                            }
                                        }
                                        registerProcessController(adapter, registry, processController);
                                    }
                                });

                            registry.begin_setProcessController(processController,
                                new Ice.Callback()
                                {
                                    public void completed(Ice.AsyncResult r)
                                    {
                                        try
                                        {
                                            registry.end_setProcessController(r);
                                        }
                                        catch(Ice.Exception ex)
                                        {
                                            handleException(ex, adapter, registry, processController);
                                        }
                                    }
                                });
                        }
                        catch(Ice.Exception ex)
                        {
                            handleException(ex, adapter, registry, processController);
                        }
                    }
                });
        }

        public void handleException(Throwable ex,
                                    final Ice.ObjectAdapter adapter,
                                    final ProcessControllerRegistryPrx registry,
                                    final ProcessControllerPrx processController)
        {
            if(ex instanceof Ice.ConnectFailedException || ex instanceof Ice.TimeoutException)
            {
                while(true)
                {
                    try
                    {
                        Thread.sleep(500);
                        break;
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
                registerProcessController(adapter, registry, processController);
            }
            else
            {
                println(ex.toString());
            }
        }

        public void destroy()
        {
            _communicator.destroy();
        }

        private ProcessControllerRegistryPrx _registry;
        private Ice.Communicator _communicator;
    }

    class ControllerHelperI extends Thread implements test.TestHelper.ControllerHelper
    {
        public ControllerHelperI(TestSuiteBundle bundle, String[] args, String exe)
        {
            _bundle = bundle;
            _args = args;
        }

        public void communicatorInitialized(Communicator communicator)
        {
            if(communicator.getProperties().getProperty("Ice.Plugin.IceSSL").equals("IceSSL.PluginFactory"))
            {
                IceSSL.Plugin plugin = (IceSSL.Plugin)communicator.getPluginManager().getPlugin("IceSSL");
                String keystore = communicator.getProperties().getProperty("IceSSL.Keystore");
                communicator.getProperties().setProperty("IceSSL.Keystore", "");
                java.io.InputStream certs = getResources().openRawResource(
                        keystore.equals("client.bks") ? R.raw.client : R.raw.server);
                plugin.setKeystoreStream(certs);
                plugin.setTruststoreStream(certs);
                communicator.getPluginManager().initializePlugins();
            }
        }

        public void run()
        {
            try
            {
                _helper = _bundle.newInstance();
                _helper.setClassLoader(_bundle.getClassLoader());
                _helper.setControllerHelper(this);
                _helper.setWriter(new Writer()
                    {
                        @Override
                        public void close() throws IOException
                        {
                        }

                        @Override
                        public void flush() throws IOException
                        {
                        }

                        @Override
                        public void write(char[] buf, int offset, int count)
                            throws IOException
                        {
                            _out.append(buf, offset, count);
                        }
                    });

                _helper.run(_args);
                completed(0);
            }
            catch(Exception ex)
            {
                ex.printStackTrace(_helper.getWriter());
                completed(-1);
            }
        }

        public void shutdown()
        {
            if(_helper != null)
            {
                _helper.shutdown();
            }
        }

        public String getOutput()
        {
            return _out.toString();
        }

        synchronized public void serverReady()
        {
            _ready = true;
            notifyAll();
        }

        synchronized private void completed(int status)
        {
            _completed = true;
            _status = status;
            notifyAll();
        }

        synchronized private void waitReady(int timeout)
            throws Test.Common.ProcessFailedException
        {
            long now = IceInternal.Time.currentMonotonicTimeMillis();
            while(!_ready && !_completed)
            {
                try
                {
                    wait(timeout * 1000);
                    if(IceInternal.Time.currentMonotonicTimeMillis() - now > timeout * 1000)
                    {
                        throw new Test.Common.ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            if(_completed && _status != 0)
            {
                throw new Test.Common.ProcessFailedException(_out.toString());
            }
        }

        synchronized private int waitSuccess(int timeout)
            throws Test.Common.ProcessFailedException
        {
            long now = IceInternal.Time.currentMonotonicTimeMillis();
            while(!_completed)
            {
                try
                {
                    wait(timeout * 1000);
                    if(IceInternal.Time.currentMonotonicTimeMillis() - now > timeout * 1000)
                    {
                        throw new Test.Common.ProcessFailedException("timed out waiting for the process to be ready");
                    }
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            return _status;
        }

        private TestSuiteBundle _bundle;
        private String[] _args;
        private String _exe;
        private test.TestHelper _helper;
        private boolean _ready = false;
        private boolean _completed = false;
        private int _status = 0;
        private final StringBuffer _out = new StringBuffer();
    }

    class ProcessControllerI extends Test.Common._ProcessControllerDisp
    {
        public Test.Common.ProcessPrx start(final String testsuite, final String exe, String[] args, Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            println("starting " + testsuite + " " + exe + "... ");
            String className = "test." + testsuite.replace("/", ".") + "." +
                exe.substring(0, 1).toUpperCase(Locale.ROOT) + exe.substring(1);
            try
            {
                TestSuiteBundle bundle = new TestSuiteBundle(className, getClassLoader());
                ControllerHelperI controllerHelper = new ControllerHelperI(bundle, args, exe);
                controllerHelper.start();
                return Test.Common.ProcessPrxHelper.uncheckedCast(
                    current.adapter.addWithUUID(new ProcessI(controllerHelper)));
            }
            catch(ClassNotFoundException ex)
            {
                throw new Test.Common.ProcessFailedException(
                    "testsuite `" + testsuite + "' exe ` " + exe + "' start failed:\n" + ex.toString());
            }
        }

        public String getHost(String protocol, boolean ipv6, Ice.Current current)
        {
            if(isEmulator())
            {
                return  "127.0.0.1";
            }
            else
            {
                synchronized(ControllerApp.this)
                {
                    return ipv6 ? _ipv6Address : _ipv4Address;
                }
            }
        }
    }

    class ProcessI extends Test.Common._ProcessDisp
    {
        public ProcessI(ControllerHelperI controllerHelper)
        {
            _controllerHelper = controllerHelper;
        }

        public void waitReady(int timeout, Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            _controllerHelper.waitReady(timeout);
        }

        public int waitSuccess(int timeout, Ice.Current current)
            throws Test.Common.ProcessFailedException
        {
            return _controllerHelper.waitSuccess(timeout);
        }

        public String terminate(Ice.Current current)
        {
            _controllerHelper.shutdown();
            current.adapter.remove(current.id);
            while(true)
            {
                try
                {
                    _controllerHelper.join();
                    break;
                }
                catch(InterruptedException ex)
                {
                }
            }
            return _controllerHelper.getOutput();
        }

        private ControllerHelperI _controllerHelper;
    }
}
