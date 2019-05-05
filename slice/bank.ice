module Demo
{

    enum Currency { PLN, USD, EUR}

    struct Money{
            Currency currency;
            int value;
        }


    interface Account{
        idempotent Money getAccountStatus(string s);
    }

    interface PremiumAccount{
        void getLoan(Currency currency, int value, int periodLength);
    }

    struct CreatedAccount{
        Account* account;
        string pesel;
        bool isPremium;
    }


    interface AccountsCreator{
        CreatedAccount createAccount(string firstName, string lastName,string pesel,Money declaredIncome);
    }
}

