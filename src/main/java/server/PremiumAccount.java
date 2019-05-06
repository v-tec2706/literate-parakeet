package server;

import Demo.Currency;
import Demo.Money;
import com.zeroc.Ice.Current;

public class PremiumAccount extends Account implements Demo.PremiumAccount {
    public PremiumAccount(String firstName, String lastName, String pesel, Money balance) {
        super(firstName, lastName, pesel, balance);
    }

    @Override
    public boolean isPremium(Current current){
        return true;
    }

    @Override
    public double getLoan(Currency currency, int value, int periodLength,Current current){

        double cost = Server.StockExchange.availableCurrencies.get(currency);
        return value*cost/periodLength;
    }

}
