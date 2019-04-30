package server;


import Demo.*;
import com.zeroc.Ice.Current;

public class Account implements Demo.Account
{
    String firstName;
    String lastName;
    String pesel;
    Money balance;
    String key;

    public Account(String firstName, String lastName, String pesel, Money balance) {
        this.firstName = firstName;
        this.lastName = lastName;
        this.pesel = pesel;
        this.balance = balance;
        this.key = String.valueOf(Math.random());
    }

    @Override
    public void getAccountStatus(String s, Current current) {
        System.out.println(s);
    }


}