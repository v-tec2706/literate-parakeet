//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Threading;
using Test;

public class AllTests : Test.AllTests
{
    public class Progress : IProgress<bool>
    {
        public Progress()
        {
        }

        public bool getResult()
        {
            return _sentSynchronously;
        }

        public void Report(bool sentSynchronously)
        {
            _sentSynchronously = sentSynchronously;
        }

        bool _sentSynchronously = false;
    }

    private class Callback
    {
        internal Callback(System.IO.TextWriter output)
        {
            _called = false;
            _output = output;
        }

        public void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }
                _called = false;
            }
        }

        public void response()
        {
            test(Dispatcher.isDispatcherThread());
            called();
        }

        public void exception(Ice.Exception ex)
        {
            if(!(ex is Ice.NoEndpointException))
            {
                _output.WriteLine(ex.ToString());
                test(false);
            }
            test(Dispatcher.isDispatcherThread());
            called();
        }

        public void payload()
        {
            test(Dispatcher.isDispatcherThread());
        }

        public void ignoreEx(Ice.Exception ex)
        {
            if(!(ex is Ice.CommunicatorDestroyedException))
            {
                _output.WriteLine(ex.ToString());
                test(false);
            }
        }

        public void sent(bool sentSynchronously)
        {
            test(sentSynchronously || Dispatcher.isDispatcherThread());
        }

        public void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
        private System.IO.TextWriter _output;
    }

    public static void allTests(Test.TestHelper helper)
    {
        var output = helper.getWriter();
        Ice.Communicator communicator = helper.communicator();
        string sref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        output.Write("testing dispatcher... ");
        output.Flush();
        {
            p.op();

            Callback cb = new Callback(output);
            p.begin_op().whenCompleted(cb.response, cb.exception);
            cb.check();

            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op().whenCompleted(cb.exception);
            cb.check();

            //
            // Expect InvocationTimeoutException.
            //
            {
                Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(10));
                to.begin_sleep(500).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception ex) => {
                        test(ex is Ice.InvocationTimeoutException);
                        test(Dispatcher.isDispatcherThread());
                        cb.called();
                    });
                cb.check();
            }

            testController.holdAdapter();
            Test.Callback_TestIntf_opWithPayload resp = cb.payload;
            Ice.ExceptionCallback excb = cb.ignoreEx;
            Ice.SentCallback scb = cb.sent;

            byte[] seq = new byte[10 * 1024];
            (new System.Random()).NextBytes(seq);
            Ice.AsyncResult r;
            while((r = p.begin_opWithPayload(seq).whenCompleted(resp, excb).whenSent(scb)).sentSynchronously());
            testController.resumeAdapter();
            r.waitForCompleted();
        }
        output.WriteLine("ok");

        output.Write("testing dispatcher with continuations... ");
        output.Flush();
        {
            p.op();

            Callback cb = new Callback(output);
            System.Action<Task> continuation = (Task previous) =>
            {
                try
                {
                    previous.Wait();
                    cb.response();
                }
                catch(System.AggregateException ex)
                {
                    cb.exception((Ice.Exception)ex.InnerException);
                }
            };
            // We use sleepAsync instead of opAsync to ensure the response isn't received before
            // we setup the continuation
            var t = p.sleepAsync(500).ContinueWith(continuation, TaskContinuationOptions.ExecuteSynchronously);
            t.Wait();
            cb.check();

            var i = (TestIntfPrx)p.ice_adapterId("dummy");

            //
            // sleepAsync doesn't help here as the test will fail with Ice.NoEndpointException and sleepAsync
            // will not be called.
            //
            //i.sleepAsync(500).ContinueWith(continuation, TaskContinuationOptions.ExecuteSynchronously).Wait();
            //cb.check();

            //
            // Expect InvocationTimeoutException.
            //
            {
                Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(20));
                to.sleepAsync(500).ContinueWith(
                    previous =>
                    {
                        try
                        {
                            previous.Wait();
                            test(false);
                        }
                        catch(System.AggregateException ex)
                        {
                            test(ex.InnerException is Ice.InvocationTimeoutException);
                            test(Dispatcher.isDispatcherThread());
                        }
                    }, TaskContinuationOptions.ExecuteSynchronously).Wait();
            }

            //
            // Repeat using the proxy scheduler in this case we don't need to call sleepAsync, continuations
            // are waranted to run with the dispatcher even if not executed synchronously.
            //

            t = p.opAsync().ContinueWith(continuation, p.ice_scheduler());
            t.Wait();
            cb.check();

            i.opAsync().ContinueWith(continuation, i.ice_scheduler()).Wait();
            cb.check();

            //
            // Expect InvocationTimeoutException.
            //
            {
                Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(10));
                to.sleepAsync(500).ContinueWith(
                    previous =>
                    {
                        try
                        {
                            previous.Wait();
                            test(false);
                        }
                        catch(System.AggregateException ex)
                        {
                            test(ex.InnerException is Ice.InvocationTimeoutException);
                            test(Dispatcher.isDispatcherThread());
                        }
                    }, p.ice_scheduler()).Wait();
            }

            //
            // Hold adapter to ensure the invocations don't complete synchronously
            // Also disable collocation optimization on p
            //
            testController.holdAdapter();
            var p2 = Test.TestIntfPrxHelper.uncheckedCast(p.ice_collocationOptimized(false));
            System.Action<Task> continuation2 = (Task previous) =>
            {
                test(Dispatcher.isDispatcherThread());
                try
                {
                    previous.Wait();
                }
                catch(System.AggregateException ex)
                {
                    test(ex.InnerException is Ice.CommunicatorDestroyedException);
                }
            };

            byte[] seq = new byte[10 * 1024];
            (new System.Random()).NextBytes(seq);
            Progress sentSynchronously;
            do
            {
                sentSynchronously = new Progress();
                t = p2.opWithPayloadAsync(seq, progress: sentSynchronously).ContinueWith(
                    continuation2,
                    TaskContinuationOptions.ExecuteSynchronously);
            }
            while(sentSynchronously.getResult());
            testController.resumeAdapter();
            t.Wait();
        }
        output.WriteLine("ok");

        output.Write("testing dispatcher with async/await... ");
        output.Flush();
        {
            TaskCompletionSource<object> t = new TaskCompletionSource<object>();
            p.opAsync().ContinueWith(async previous => // Execute the code below from the Ice client thread pool
            {
                try
                {
                    await p.opAsync();
                    test(Dispatcher.isDispatcherThread());

                    try
                    {
                        TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
                        await i.opAsync();
                        test(false);
                    }
                    catch(Exception)
                    {
                        test(Dispatcher.isDispatcherThread());
                    }

                    Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(10));
                    try
                    {
                        await to.sleepAsync(500);
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                        test(Dispatcher.isDispatcherThread());
                    }
                    t.SetResult(null);
                }
                catch(Exception ex)
                {
                    t.SetException(ex);
                }
            }, p.ice_scheduler());

            t.Task.Wait();
        }
        output.WriteLine("ok");

        p.shutdown();
    }
}
