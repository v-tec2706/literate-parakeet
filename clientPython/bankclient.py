import sys
import Ice
import Demo



Ice.loadSlice('../slice/bank.ice')


def registerToBank():
    print("-----------------------------------------------")
    print("---- Register to bank ----")
    print("---- Put your firstname,lastname,PESEL and monthly income (currency, value): ----")


    userData = input()
    userDataSplitted = userData.split(" ")
    declaredIncome = Demo.Money(getCurrency(userDataSplitted[3]), int(userDataSplitted[4]))
    createdAccount = accountsCreator.createAccount(userDataSplitted[0], userDataSplitted[1], userDataSplitted[2], declaredIncome)
    print("---------------------------------------------------------")
    print("---- Your account is created. Client key number is:  ----")
    print(createdAccount.key)
    print("---------------------------------------------------------")


def accountStatus():
    print("-----------------------------------------------")
    print("---- Check account status: ----")
    print("---- PESEL and user key:   ----")

    # creator = Demo.AccountsCreatorPrx.checkedCast(account_creator)
    userData = input()
    userDataSplitted = userData.split(" ")
    accountProxy = accountsCreator.getAccount(userDataSplitted[0]).account
    # account = result.account
    # print(account)

    print("-----------------------------------------------")
    print("---- Your account status: ----")

    account_balance = accountProxy.getAccountStatus()
    print(account_balance)

    print("-----------------------------------------------")


def getLoan():
    print("-----------------------------------------------")
    print("---- To get loan enter PESEL, currency, value and length of loan period: ----")

    #  clientProxy = Demo.AccountsCreatorPrx.checkedCast(account_creator)
    userData = input()
    userDataSplitted = userData.split(" ")
    result = accountsCreator.getAccount(userDataSplitted[0])
    #result = accountProxy.getAccount(userDataSplitted[0])
    accountProxy = result.account

    print(accountProxy.isPremium())
    if accountProxy.isPremium():
        premiumAccountProxy = Demo.PremiumAccountPrx.checkedCast(accountProxy)
        cost = premiumAccountProxy.getLoan(getCurrency(userDataSplitted[1]), int(userDataSplitted[2]), int(userDataSplitted[3]))
        print("Cost of loan is: ", cost)
    else:
        print("You are not allowed o get loan, your account is not premium")

    print("-----------------------------------------------")


def getCurrency(currency):
    if currency.upper() == "PLN":
        return Demo.Currency.PLN
    elif currency.upper() == "EUR":
        return Demo.Currency.EUR
    elif currency.upper() == "USD":
        return Demo.Currency.USD


with Ice.initialize(sys.argv) as communicator:
    base = communicator.stringToProxy("accfac/accountfactory:tcp -h localhost -p 10001:udp -h localhost -p 10001")
    accountsCreator = Demo.AccountsCreatorPrx.checkedCast(base)

    choice = -1
    while choice != 0:
        print("-----------------------------------------------")
        print("---- Avaliable operations:        ----")
        print("---- * 1 * create account         ----")
        print("---- * 2 * log in to an account   ----")
        print("---- * 3 * get loan               ----")
        print("---- * 0 * exit                   ----")
        print("-----------------------------------------------")
        choice = int(input())
        if choice == 1:
            registerToBank()
        if choice == 2:
            accountStatus()
        if choice == 3:
            getLoan()

