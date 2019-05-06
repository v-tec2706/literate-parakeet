package server;

import Demo.Currency;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import java.util.HashMap;
import java.util.Map;

public class Server {
    StockExchange stockExchangeThread;
    public void start(String[] args) {

        this.stockExchangeThread = new StockExchange();
        this.stockExchangeThread.start();

        int status = 0;
        Communicator communicator = null;

        try {
            // 1. Inicjalizacja ICE - utworzenie communicatora
            communicator = Util.initialize(args);

            // METODA 2 (niepolecana, dopuszczalna testowo): Konfiguracja adaptera Adapter1 jest
            // w kodzie ródłowym
            ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Adapter1",
                    "tcp -h localhost -p 10001:udp -h localhost -p 10001");

            AccountsCreator factory = new AccountsCreator();
            adapter.add(factory, new Identity("accountfactory", "accfac"));

            // 5. Aktywacja adaptera i przejcie w pętlę przetwarzania żšdań
            adapter.activate();

            System.out.println("Entering event processing loop...");

            communicator.waitForShutdown();

        } catch (Exception e) {
            System.err.println(e);
            status = 1;
        }
        if (communicator != null) {
            // Clean up
            //
            try {
                communicator.destroy();
            } catch (Exception e) {
                System.err.println(e);
                status = 1;
            }
        }
        System.exit(status);
    }

    public static class StockExchange extends Thread {

        public CurrencyService stockService;
        public static Map<Currency, Double> availableCurrencies = new HashMap<Currency, Double>(5);


        public StockExchange() {
            stockService = new CurrencyService("localhost", 50051);
        }

        public static void setAvailableCurrencies() {
            availableCurrencies.put(Currency.EUR,0.0);
            availableCurrencies.put(Currency.USD,0.0);
        }

        public void run() {
            System.out.println("Curriences server is running! ");
            setAvailableCurrencies();
            while(true) {
                synchronized(availableCurrencies) {
                    for(Map.Entry<Currency, Double> entry: availableCurrencies.entrySet()){
                        int c = entry.getKey().value() + 1;
                        Double val = entry.getValue();
                        stock.currencyData data = stock.currencyData.newBuilder().setCurr(stock.currency.valueOf(c)).setValue(val).build();
                        stock.stockRequest currencies = stock.stockRequest.newBuilder().addCurrencies(data).build();
                        Double result = stockService.runStockExchangeService(currencies);
                        availableCurrencies.put(entry.getKey(), result);
                    }
                    try {
                        Thread.sleep(5000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                availableCurrencies.forEach((x,y)-> System.out.print(x.toString()+" : "+y.toString()+" "));
                System.out.println("");
            }
        }
    }

    public static void main(String[] args) {

        Server app = new Server();
        app.start(args);

    }
}