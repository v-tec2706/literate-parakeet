package server;

import Demo.AccountPrx;
import Demo.CreatedAccount;
import Demo.Currency;
import Demo.Money;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;

import java.util.HashMap;
import java.util.Map;

public class AccountsCreator implements Demo.AccountsCreator {

    public final int PREMIUM_MIN_LIMIT = 1000;
    Map<String, CreatedAccount> createdAccounts = new HashMap<String, CreatedAccount>();

    @Override
    public CreatedAccount createAccount(String firstName, String lastName, String PESEL, Money declaredIncome, Current current) {

        Money initDefaultBalance = new Money();
        initDefaultBalance.currency = Currency.PLN;
        initDefaultBalance.value = 0;

        String cat = isPremium(declaredIncome.value) ? "premium" : "standard";

        //CreatedAccount account = new CreatedAccount();
        Account account;
        if(isPremium(declaredIncome.value))
            account = new PremiumAccount(firstName,lastName,PESEL,initDefaultBalance);
        else account = new Account(firstName, lastName, PESEL, initDefaultBalance);

        ObjectPrx proxy = current.adapter.add(account, new Identity(PESEL,cat));
        CreatedAccount newAccount = new CreatedAccount(AccountPrx.uncheckedCast(proxy),
                account.key,isPremium(declaredIncome.value));

        createdAccounts.put(PESEL,newAccount);
        return newAccount;


    }

    public CreatedAccount getAccount(String PESEL, Current current){

        return createdAccounts.get(PESEL);
    }

    boolean isPremium(int declaredIncome){
        return declaredIncome >= PREMIUM_MIN_LIMIT;
    }
}
