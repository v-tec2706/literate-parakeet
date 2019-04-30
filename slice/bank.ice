module Demo
{

    enum Currency { PLN, USD, EUR}

    interface Account{
        void getAccountStatus(string s);
    }

    interface PremiumAccount{
        void getLoan(Currency currency, int value, int periodLength);
    }

    struct CreatedAccount{
        Account* account;
        string pesel;
        bool isPremium;
    }

    struct Money{
        Currency currency;
        int value;
    }

    interface AccountsCreator{
        CreatedAccount createAccount(string firstName, string lastName,int pesel,Money declaredIncome);
    }
}

