package server;


import Demo.*;
import com.zeroc.Ice.Current;

public class Account implements Demo.Account
{
    public void printString(String s, com.zeroc.Ice.Current current)
    {
        System.out.println(s);
    }

    @Override
    public void getAccountStatus(String s, Current current) {
        System.out.println(s);
    }
}