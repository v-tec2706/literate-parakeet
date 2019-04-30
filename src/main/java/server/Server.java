package server;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

public class Server {
    public void start(String[] args) {
        int status = 0;
        Communicator communicator = null;

        try {
            // 1. Inicjalizacja ICE - utworzenie communicatora
            communicator = Util.initialize(args);

            // METODA 2 (niepolecana, dopuszczalna testowo): Konfiguracja adaptera Adapter1 jest
            // w kodzie ródłowym
            ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Adapter1",
                    "tcp -h localhost -p 10001:udp -h localhost -p 10001");

//            // 3. Stworzenie serwanta/serwantów
//            AccountI accountServant = new AccountI("somepesel", "w",
//                    new MoneyAmount(3, Currency.PLN));

            AccountsCreator factory = new AccountsCreator();
            adapter.add(factory, new Identity("accountfactory", "accfac"));


//            locator.createAccount("jan", "kowalski", "somepesel", new MoneyAmount(3000,
//                    Currency.PLN));
            // 4. Dodanie wpisów do tablicy ASM
//            adapter.add(accountServant, new Identity("somepesel", "standard"));


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


    public static void main(String[] args) {
        Server app = new Server();
        app.start(args);
    }
}