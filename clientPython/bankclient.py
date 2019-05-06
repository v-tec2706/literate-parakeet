import sys
import Ice
import Demo
Ice.loadSlice('../slice/bank.ice')




def registerToBank():
    creator = Demo.AccountsCreatorPrx.checkedCast(account_creator)
    print("-----------------------------------------------")
    print("---- Register to bank ----")
    print("---- Put your firstname,lastname,PESEL and monthly income: ----")

    userData = input()
    userDataSplitted = userData.split(" ")

    income = Demo.Money(Demo.Currency.PLN, int(userDataSplitted[3]))
    result = creator.createAccount(userDataSplitted[0], userDataSplitted[1], userDataSplitted[2], income)
    key = result.pesel
    print(result)



def accountStatus():
    print("-----------------------------------------------")
    print("---- Check account status: ----")
    print("---- PESEL and user key:   ----")

    creator = Demo.AccountsCreatorPrx.checkedCast(account_creator)
    userData = input();
    userDataSplitted = userData.split(" ");
    result = creator.getAccount(userDataSplitted[0])
    account = result.account
   # print(account)

    print("-----------------------------------------------")
    print("---- Your account status: ----")


    account_balance = account.getAccountStatus(userDataSplitted[0])
    print(account_balance)

    print("-----------------------------------------------")



with Ice.initialize(sys.argv) as communicator:
    account_creator = communicator.stringToProxy("accfac/accountfactory:tcp -h localhost -p 10001:udp -h localhost -p 10001")
    print(account_creator)

    choice = 1
    while(choice != 0):
        print("-----------------------------------------------")
        print("---- Avaliable operations:        ----")
        print("---- * 1 * create account         ----")
        print("---- * 2 * log in to an account   ----")
        print("---- * 0 * exit                   ----")
        print("-----------------------------------------------")
        choice = int(input())
        if choice == 1:
            registerToBank()
        if choice == 2:
            accountStatus()






#    print(creator)
#    income = Demo.Money(Demo.Currency.PLN,2000)
#    pesel = "96999987987"

