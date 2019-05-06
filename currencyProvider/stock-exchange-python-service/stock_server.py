from concurrent import futures
import time
import math
from random import randrange, uniform
import logging

import grpc

import sys
sys.path.insert(0, './generated_python')

from stock_pb2 import currency
from stock_pb2 import stockResponse
from stock_pb2_grpc import StockExchangeServicer
from stock_pb2_grpc import add_StockExchangeServicer_to_server


LONGTIME = 60 * 60 * 60
allCurrencies = {currency.Name(1): 1.0, currency.Name(2): 4.27, currency.Name(3): 3.74}

MIN_BOUND = 3.0
MAX_BOUND = 6.0


class CurrencyChangesSimulator(object):

    def __init__(self, bottom, up):
        self.bottom = bottom
        self.up = up


    def simulate(self, stockRequest):
        cur = stockRequest.currencies.curr
        for c in allCurrencies:
            cond = allCurrencies[c] + uniform(self.bottom, self.up)
            if currency.Value(c) == cur and cond > MIN_BOUND and cond < MAX_BOUND:
                allCurrencies[c] = cond
                stockRequest.currencies.value = allCurrencies[c]
                print("Currency = ", currency.Name(cur), " with value = ", stockRequest.currencies.value)
                break

        return stockResponse(updatedCurrencies=stockRequest.currencies)


class StockExchangeServicer(StockExchangeServicer):

    def __init__(self):
        self.currenciesChangesSimulator = CurrencyChangesSimulator(-0.5, 0.5)


    def processPassedCurrency(self, stockRequest):
        self.currenciesChangesSimulator.currencyPrice = stockRequest.currencies
        currencyPrice = self.currenciesChangesSimulator.simulate(stockRequest)
        return currencyPrice

    def getUpdatedCurrencies(self, request, context):
        currencyPrice = self.processPassedCurrency(request)
        return currencyPrice


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    add_StockExchangeServicer_to_server(
        StockExchangeServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    try:
        while True:
            time.sleep(LONGTIME)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
    logging.basicConfig()
    print("Starting server...")
    serve()
