module Demo
{

    enum Currency {PLN, USD, EUR}

    struct Money{
            Currency currency;
            int value;
        }

    interface Account{
        idempotent Money getAccountStatus();
        bool isPremium();
    }

    interface PremiumAccount extends Account{
        double getLoan(Currency currency, int value, int periodLength);
    }

    struct CreatedAccount{
        Account* account;
        string key;
        bool isPremium;
    }

    interface AccountsCreator{
        CreatedAccount createAccount(string firstName, string lastName,string pesel,Money declaredIncome);
	    CreatedAccount getAccount(string pesel);
    }
}

