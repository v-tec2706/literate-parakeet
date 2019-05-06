package server;

import java.util.concurrent.TimeUnit;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;

public class CurrencyService {

    private ManagedChannel channel;
    private stock.StockExchangeGrpc.StockExchangeBlockingStub blockingStub;
    public CurrencyService(String host, int port) {
        this(ManagedChannelBuilder.forAddress(host, port).usePlaintext().build());
    }

    private CurrencyService(ManagedChannel channel){
        this.channel = channel;
        blockingStub = stock.StockExchangeGrpc.newBlockingStub(channel);
    }

    public void shutdown() throws InterruptedException {
        channel.shutdown().awaitTermination(5, TimeUnit.SECONDS);
    }



    public Double runStockExchangeService(stock.stockRequest currencies) {
        stock.stockResponse updatedCurrencies = blockingStub.getUpdatedCurrencies(currencies);
        Double result = updatedCurrencies.getUpdatedCurrencies(0).getValue();
        return result;
    }
}
