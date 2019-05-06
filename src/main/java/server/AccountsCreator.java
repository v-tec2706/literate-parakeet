package server;

import Demo.*;
import com.sun.org.apache.xerces.internal.impl.xpath.regex.Match;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;

import java.util.HashMap;
import java.util.Map;
import java.lang.Math;

public class AccountsCreator implements Demo.AccountsCreator {

    public final int PREMIUM_MIN_LIMIT = 1000;
    Map<String, CreatedAccount> createdAccounts = new HashMap<String, CreatedAccount>();

    @Override
    public CreatedAccount createAccount(String firstName, String lastName, String PESEL, Money declaredIncome, Current current) {

        Money initDefaultBalance = new Money();
        initDefaultBalance.currency = declaredIncome.currency;
        initDefaultBalance.value = (int) (Math.random() * 1000);

        String cat = isPremium(declaredIncome.value) ? "premium" : "standard";

        if(isPremium(declaredIncome.value)) {
            PremiumAccount account = new PremiumAccount(firstName, lastName, PESEL, initDefaultBalance);
            ObjectPrx proxy = current.adapter.add(account, new Identity(PESEL,cat));
            CreatedAccount newAccount;
            newAccount = new CreatedAccount(PremiumAccountPrx.uncheckedCast(proxy),
                    account.key, isPremium(declaredIncome.value));
            createdAccounts.put(PESEL,newAccount);
            return newAccount;
        }
        else {
            Account account = new Account(firstName, lastName, PESEL, initDefaultBalance);
            ObjectPrx proxy = current.adapter.add(account, new Identity(PESEL,cat));
            CreatedAccount newAccount;
            newAccount = new CreatedAccount(AccountPrx.uncheckedCast(proxy),
                    account.key, isPremium(declaredIncome.value));
            createdAccounts.put(PESEL,newAccount);
            return newAccount;
        }

//        ObjectPrx proxy = current.adapter.add(account, new Identity(PESEL,cat));
//        CreatedAccount newAccount;
//        if(isPremium(declaredIncome.value)) {
//             newAccount = new CreatedAccount(PremiumAccountPrx.uncheckedCast(proxy),
//                    account.key, isPremium(declaredIncome.value));
//
//        }
//        else {
//             newAccount = new CreatedAccount(AccountPrx.uncheckedCast(proxy),
//                    account.key, isPremium(declaredIncome.value));
//        }
//        createdAccounts.put(PESEL,newAccount);
//        return newAccount;


    }

    public CreatedAccount getAccount(String PESEL, Current current){

        return createdAccounts.get(PESEL);
    }

    boolean isPremium(int declaredIncome){
        return declaredIncome >= PREMIUM_MIN_LIMIT;
    }
}
