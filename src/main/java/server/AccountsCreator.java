package server;

import Demo.AccountPrx;
import Demo.CreatedAccount;
import Demo.Currency;
import Demo.Money;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;

public class AccountsCreator implements Demo.AccountsCreator {

    public final int PREMIUM_MIN_LIMIT = 1000;

    @Override
    public CreatedAccount createAccount(String firstName, String lastName, String pesel, Money declaredIncome, Current current) {

        Money initDefaultBalance = new Money();
        initDefaultBalance.currency = Currency.PLN;
        initDefaultBalance.value = 0;

        String cat = isPremium(declaredIncome.value) ? "premium" : "standard";

        //CreatedAccount account = new CreatedAccount();
        Account account;
        if(isPremium(declaredIncome.value))
            account = new PremiumAccount(firstName,lastName,pesel,initDefaultBalance);
        else account = new Account(firstName, lastName, pesel, initDefaultBalance);

        ObjectPrx proxy = current.adapter.add(account, new Identity(pesel,cat));
        return new CreatedAccount(AccountPrx.uncheckedCast(proxy),
                 account.key,isPremium(declaredIncome.value));

    }

    boolean isPremium(int declaredIncome){
        return declaredIncome >= PREMIUM_MIN_LIMIT;
    }
}
