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

    int max = 2000;
    int min = 1000;
    int range = max - min + 1;

    public Account(String firstName, String lastName, String pesel, Money balance) {
        this.firstName = firstName;
        this.lastName = lastName;
        this.pesel = pesel;
        this.balance = balance;
        this.key = String.valueOf((int)(Math.random() * range) + min);
    }

    @Override
    public Money getAccountStatus(Current current) {
        System.out.println("Value "+ balance.value+balance.currency);
        return balance;
    }

    @Override
    public boolean isPremium(Current current){
        return false;
    }
}