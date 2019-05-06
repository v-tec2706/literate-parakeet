//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.admin;

class RemoteLoggerI extends Ice._RemoteLoggerDisp
{

    @Override
    public synchronized void init(String prefix, Ice.LogMessage[] logMessages, Ice.Current current)
    {
        _prefix = prefix;
        for(int i = 0; i < logMessages.length; ++i)
        {
            _initMessages.add(logMessages[i]);
        }
        _receivedCalls++;
        notifyAll();
    }

    @Override
    public synchronized void log(Ice.LogMessage logMessage, Ice.Current current)
    {
        _logMessages.add(logMessage);
        _receivedCalls++;
        notifyAll();
    }

    synchronized void checkNextInit(String prefix, Ice.LogMessageType type, String message, String category)
    {
        test(_prefix.equals(prefix));
        test(_initMessages.size() > 0);
        Ice.LogMessage logMessage = _initMessages.pop();
        test(logMessage.type == type);
        test(logMessage.message.equals(message));
        test(logMessage.traceCategory.equals(category));
    }

    synchronized void checkNextLog(Ice.LogMessageType type, String message, String category)
    {
        test(_logMessages.size() > 0);
        Ice.LogMessage logMessage = _logMessages.pop();
        test(logMessage.type == type);
        test(logMessage.message.equals(message));
        test(logMessage.traceCategory.equals(category));
    }

    synchronized void wait(int calls)
    {
        _receivedCalls -= calls;

        while(_receivedCalls < 0)
        {
            try
            {
                wait();
            }
            catch(InterruptedException ex)
            {
                break;
            }
        }
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int _receivedCalls;
    private String _prefix;
    private java.util.Deque<Ice.LogMessage> _initMessages = new java.util.ArrayDeque<Ice.LogMessage>();
    private java.util.Deque<Ice.LogMessage> _logMessages = new java.util.ArrayDeque<Ice.LogMessage>();
}
