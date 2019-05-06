# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
#
# Ice version 3.7.2
#
# <auto-generated>
#
# Generated from file `bank.ice'
#
# Warning: do not edit this file.
#
# </auto-generated>
#

from sys import version_info as _version_info_
import Ice, IcePy

# Start of module Demo
_M_Demo = Ice.openModule('Demo')
__name__ = 'Demo'

if 'Currency' not in _M_Demo.__dict__:
    _M_Demo.Currency = Ice.createTempClass()
    class Currency(Ice.EnumBase):

        def __init__(self, _n, _v):
            Ice.EnumBase.__init__(self, _n, _v)

        def valueOf(self, _n):
            if _n in self._enumerators:
                return self._enumerators[_n]
            return None
        valueOf = classmethod(valueOf)

    Currency.PLN = Currency("PLN", 0)
    Currency.USD = Currency("USD", 1)
    Currency.EUR = Currency("EUR", 2)
    Currency._enumerators = { 0:Currency.PLN, 1:Currency.USD, 2:Currency.EUR }

    _M_Demo._t_Currency = IcePy.defineEnum('::Demo::Currency', Currency, (), Currency._enumerators)

    _M_Demo.Currency = Currency
    del Currency

if 'Money' not in _M_Demo.__dict__:
    _M_Demo.Money = Ice.createTempClass()
    class Money(object):
        def __init__(self, currency=_M_Demo.Currency.PLN, value=0):
            self.currency = currency
            self.value = value

        def __hash__(self):
            _h = 0
            _h = 5 * _h + Ice.getHash(self.currency)
            _h = 5 * _h + Ice.getHash(self.value)
            return _h % 0x7fffffff

        def __compare(self, other):
            if other is None:
                return 1
            elif not isinstance(other, _M_Demo.Money):
                return NotImplemented
            else:
                if self.currency is None or other.currency is None:
                    if self.currency != other.currency:
                        return (-1 if self.currency is None else 1)
                else:
                    if self.currency < other.currency:
                        return -1
                    elif self.currency > other.currency:
                        return 1
                if self.value is None or other.value is None:
                    if self.value != other.value:
                        return (-1 if self.value is None else 1)
                else:
                    if self.value < other.value:
                        return -1
                    elif self.value > other.value:
                        return 1
                return 0

        def __lt__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r < 0

        def __le__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r <= 0

        def __gt__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r > 0

        def __ge__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r >= 0

        def __eq__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r == 0

        def __ne__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r != 0

        def __str__(self):
            return IcePy.stringify(self, _M_Demo._t_Money)

        __repr__ = __str__

    _M_Demo._t_Money = IcePy.defineStruct('::Demo::Money', Money, (), (
        ('currency', (), _M_Demo._t_Currency),
        ('value', (), IcePy._t_int)
    ))

    _M_Demo.Money = Money
    del Money

_M_Demo._t_Account = IcePy.defineValue('::Demo::Account', Ice.Value, -1, (), False, True, None, ())

if 'AccountPrx' not in _M_Demo.__dict__:
    _M_Demo.AccountPrx = Ice.createTempClass()
    class AccountPrx(Ice.ObjectPrx):

        def getAccountStatus(self, context=None):
            return _M_Demo.Account._op_getAccountStatus.invoke(self, ((), context))

        def getAccountStatusAsync(self, context=None):
            return _M_Demo.Account._op_getAccountStatus.invokeAsync(self, ((), context))

        def begin_getAccountStatus(self, _response=None, _ex=None, _sent=None, context=None):
            return _M_Demo.Account._op_getAccountStatus.begin(self, ((), _response, _ex, _sent, context))

        def end_getAccountStatus(self, _r):
            return _M_Demo.Account._op_getAccountStatus.end(self, _r)

        def isPremium(self, context=None):
            return _M_Demo.Account._op_isPremium.invoke(self, ((), context))

        def isPremiumAsync(self, context=None):
            return _M_Demo.Account._op_isPremium.invokeAsync(self, ((), context))

        def begin_isPremium(self, _response=None, _ex=None, _sent=None, context=None):
            return _M_Demo.Account._op_isPremium.begin(self, ((), _response, _ex, _sent, context))

        def end_isPremium(self, _r):
            return _M_Demo.Account._op_isPremium.end(self, _r)

        @staticmethod
        def checkedCast(proxy, facetOrContext=None, context=None):
            return _M_Demo.AccountPrx.ice_checkedCast(proxy, '::Demo::Account', facetOrContext, context)

        @staticmethod
        def uncheckedCast(proxy, facet=None):
            return _M_Demo.AccountPrx.ice_uncheckedCast(proxy, facet)

        @staticmethod
        def ice_staticId():
            return '::Demo::Account'
    _M_Demo._t_AccountPrx = IcePy.defineProxy('::Demo::Account', AccountPrx)

    _M_Demo.AccountPrx = AccountPrx
    del AccountPrx

    _M_Demo.Account = Ice.createTempClass()
    class Account(Ice.Object):

        def ice_ids(self, current=None):
            return ('::Demo::Account', '::Ice::Object')

        def ice_id(self, current=None):
            return '::Demo::Account'

        @staticmethod
        def ice_staticId():
            return '::Demo::Account'

        def getAccountStatus(self, current=None):
            raise NotImplementedError("servant method 'getAccountStatus' not implemented")

        def isPremium(self, current=None):
            raise NotImplementedError("servant method 'isPremium' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Demo._t_AccountDisp)

        __repr__ = __str__

    _M_Demo._t_AccountDisp = IcePy.defineClass('::Demo::Account', Account, (), None, ())
    Account._ice_type = _M_Demo._t_AccountDisp

    Account._op_getAccountStatus = IcePy.Operation('getAccountStatus', Ice.OperationMode.Idempotent, Ice.OperationMode.Idempotent, False, None, (), (), (), ((), _M_Demo._t_Money, False, 0), ())
    Account._op_isPremium = IcePy.Operation('isPremium', Ice.OperationMode.Normal, Ice.OperationMode.Normal, False, None, (), (), (), ((), IcePy._t_bool, False, 0), ())

    _M_Demo.Account = Account
    del Account

_M_Demo._t_PremiumAccount = IcePy.defineValue('::Demo::PremiumAccount', Ice.Value, -1, (), False, True, None, ())

if 'PremiumAccountPrx' not in _M_Demo.__dict__:
    _M_Demo.PremiumAccountPrx = Ice.createTempClass()
    class PremiumAccountPrx(_M_Demo.AccountPrx):

        def getLoan(self, currency, value, periodLength, context=None):
            return _M_Demo.PremiumAccount._op_getLoan.invoke(self, ((currency, value, periodLength), context))

        def getLoanAsync(self, currency, value, periodLength, context=None):
            return _M_Demo.PremiumAccount._op_getLoan.invokeAsync(self, ((currency, value, periodLength), context))

        def begin_getLoan(self, currency, value, periodLength, _response=None, _ex=None, _sent=None, context=None):
            return _M_Demo.PremiumAccount._op_getLoan.begin(self, ((currency, value, periodLength), _response, _ex, _sent, context))

        def end_getLoan(self, _r):
            return _M_Demo.PremiumAccount._op_getLoan.end(self, _r)

        @staticmethod
        def checkedCast(proxy, facetOrContext=None, context=None):
            return _M_Demo.PremiumAccountPrx.ice_checkedCast(proxy, '::Demo::PremiumAccount', facetOrContext, context)

        @staticmethod
        def uncheckedCast(proxy, facet=None):
            return _M_Demo.PremiumAccountPrx.ice_uncheckedCast(proxy, facet)

        @staticmethod
        def ice_staticId():
            return '::Demo::PremiumAccount'
    _M_Demo._t_PremiumAccountPrx = IcePy.defineProxy('::Demo::PremiumAccount', PremiumAccountPrx)

    _M_Demo.PremiumAccountPrx = PremiumAccountPrx
    del PremiumAccountPrx

    _M_Demo.PremiumAccount = Ice.createTempClass()
    class PremiumAccount(_M_Demo.Account):

        def ice_ids(self, current=None):
            return ('::Demo::Account', '::Demo::PremiumAccount', '::Ice::Object')

        def ice_id(self, current=None):
            return '::Demo::PremiumAccount'

        @staticmethod
        def ice_staticId():
            return '::Demo::PremiumAccount'

        def getLoan(self, currency, value, periodLength, current=None):
            raise NotImplementedError("servant method 'getLoan' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Demo._t_PremiumAccountDisp)

        __repr__ = __str__

    _M_Demo._t_PremiumAccountDisp = IcePy.defineClass('::Demo::PremiumAccount', PremiumAccount, (), None, (_M_Demo._t_AccountDisp,))
    PremiumAccount._ice_type = _M_Demo._t_PremiumAccountDisp

    PremiumAccount._op_getLoan = IcePy.Operation('getLoan', Ice.OperationMode.Normal, Ice.OperationMode.Normal, False, None, (), (((), _M_Demo._t_Currency, False, 0), ((), IcePy._t_int, False, 0), ((), IcePy._t_int, False, 0)), (), ((), IcePy._t_double, False, 0), ())

    _M_Demo.PremiumAccount = PremiumAccount
    del PremiumAccount

if 'CreatedAccount' not in _M_Demo.__dict__:
    _M_Demo.CreatedAccount = Ice.createTempClass()
    class CreatedAccount(object):
        def __init__(self, account=None, key='', isPremium=False):
            self.account = account
            self.key = key
            self.isPremium = isPremium

        def __eq__(self, other):
            if other is None:
                return False
            elif not isinstance(other, _M_Demo.CreatedAccount):
                return NotImplemented
            else:
                if self.account != other.account:
                    return False
                if self.key != other.key:
                    return False
                if self.isPremium != other.isPremium:
                    return False
                return True

        def __ne__(self, other):
            return not self.__eq__(other)

        def __str__(self):
            return IcePy.stringify(self, _M_Demo._t_CreatedAccount)

        __repr__ = __str__

    _M_Demo._t_CreatedAccount = IcePy.defineStruct('::Demo::CreatedAccount', CreatedAccount, (), (
        ('account', (), _M_Demo._t_AccountPrx),
        ('key', (), IcePy._t_string),
        ('isPremium', (), IcePy._t_bool)
    ))

    _M_Demo.CreatedAccount = CreatedAccount
    del CreatedAccount

_M_Demo._t_AccountsCreator = IcePy.defineValue('::Demo::AccountsCreator', Ice.Value, -1, (), False, True, None, ())

if 'AccountsCreatorPrx' not in _M_Demo.__dict__:
    _M_Demo.AccountsCreatorPrx = Ice.createTempClass()
    class AccountsCreatorPrx(Ice.ObjectPrx):

        def createAccount(self, firstName, lastName, pesel, declaredIncome, context=None):
            return _M_Demo.AccountsCreator._op_createAccount.invoke(self, ((firstName, lastName, pesel, declaredIncome), context))

        def createAccountAsync(self, firstName, lastName, pesel, declaredIncome, context=None):
            return _M_Demo.AccountsCreator._op_createAccount.invokeAsync(self, ((firstName, lastName, pesel, declaredIncome), context))

        def begin_createAccount(self, firstName, lastName, pesel, declaredIncome, _response=None, _ex=None, _sent=None, context=None):
            return _M_Demo.AccountsCreator._op_createAccount.begin(self, ((firstName, lastName, pesel, declaredIncome), _response, _ex, _sent, context))

        def end_createAccount(self, _r):
            return _M_Demo.AccountsCreator._op_createAccount.end(self, _r)

        def getAccount(self, pesel, context=None):
            return _M_Demo.AccountsCreator._op_getAccount.invoke(self, ((pesel, ), context))

        def getAccountAsync(self, pesel, context=None):
            return _M_Demo.AccountsCreator._op_getAccount.invokeAsync(self, ((pesel, ), context))

        def begin_getAccount(self, pesel, _response=None, _ex=None, _sent=None, context=None):
            return _M_Demo.AccountsCreator._op_getAccount.begin(self, ((pesel, ), _response, _ex, _sent, context))

        def end_getAccount(self, _r):
            return _M_Demo.AccountsCreator._op_getAccount.end(self, _r)

        @staticmethod
        def checkedCast(proxy, facetOrContext=None, context=None):
            return _M_Demo.AccountsCreatorPrx.ice_checkedCast(proxy, '::Demo::AccountsCreator', facetOrContext, context)

        @staticmethod
        def uncheckedCast(proxy, facet=None):
            return _M_Demo.AccountsCreatorPrx.ice_uncheckedCast(proxy, facet)

        @staticmethod
        def ice_staticId():
            return '::Demo::AccountsCreator'
    _M_Demo._t_AccountsCreatorPrx = IcePy.defineProxy('::Demo::AccountsCreator', AccountsCreatorPrx)

    _M_Demo.AccountsCreatorPrx = AccountsCreatorPrx
    del AccountsCreatorPrx

    _M_Demo.AccountsCreator = Ice.createTempClass()
    class AccountsCreator(Ice.Object):

        def ice_ids(self, current=None):
            return ('::Demo::AccountsCreator', '::Ice::Object')

        def ice_id(self, current=None):
            return '::Demo::AccountsCreator'

        @staticmethod
        def ice_staticId():
            return '::Demo::AccountsCreator'

        def createAccount(self, firstName, lastName, pesel, declaredIncome, current=None):
            raise NotImplementedError("servant method 'createAccount' not implemented")

        def getAccount(self, pesel, current=None):
            raise NotImplementedError("servant method 'getAccount' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Demo._t_AccountsCreatorDisp)

        __repr__ = __str__

    _M_Demo._t_AccountsCreatorDisp = IcePy.defineClass('::Demo::AccountsCreator', AccountsCreator, (), None, ())
    AccountsCreator._ice_type = _M_Demo._t_AccountsCreatorDisp

    AccountsCreator._op_createAccount = IcePy.Operation('createAccount', Ice.OperationMode.Normal, Ice.OperationMode.Normal, False, None, (), (((), IcePy._t_string, False, 0), ((), IcePy._t_string, False, 0), ((), IcePy._t_string, False, 0), ((), _M_Demo._t_Money, False, 0)), (), ((), _M_Demo._t_CreatedAccount, False, 0), ())
    AccountsCreator._op_getAccount = IcePy.Operation('getAccount', Ice.OperationMode.Normal, Ice.OperationMode.Normal, False, None, (), (((), IcePy._t_string, False, 0),), (), ((), _M_Demo._t_CreatedAccount, False, 0), ())

    _M_Demo.AccountsCreator = AccountsCreator
    del AccountsCreator

# End of module Demo
