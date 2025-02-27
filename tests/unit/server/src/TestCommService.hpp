#ifndef TEST_CAPIOCOMMUNICATIONSERVICE_HPP
#define TEST_CAPIOCOMMUNICATIONSERVICE_HPP

#include "../server/communication-service/CapioCommunicationService.hpp"
#include <gtest/gtest.h>
#include <thread>

constexpr char TEST_MESSAGE[] = "hello world how is it going?";
constexpr capio_off64_t BUFFER_SIZES = 1024;
constexpr int REQUIRED_CONNECTIONS = 2; // Numero di connessioni richieste prima che il coordinatore inizi a inviare messaggi
/*
TEST(CapioCommServiceTest, TestBroadcast) {
    // Get the hostname of the current machine
    char ownHostname[HOST_NAME_MAX] = {0};
    gethostname(ownHostname, HOST_NAME_MAX);
    char coordinator[HOST_NAME_MAX] = "fd-coordinator";

    CapioCommunicationService backend("TCP", "1234", 300);
    capio_off64_t size_revc, offset;

    std::vector<std::string> connections;

    // Aspetta che ci siano abbastanza connessioni aperte
    while (true) {
        connections = backend.get_open_connections();
        if (connections.size() >= REQUIRED_CONNECTIONS) {
            break;  // Esci quando sono state raggiunte abbastanza connessioni
        }
        std::cout << "Attesa di connessioni: " << connections.size() << " / " << REQUIRED_CONNECTIONS << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Aspetta un po' prima di controllare di nuovo
    }

    if (std::string(ownHostname) == "fd-coordinator") {
        for (int size = 1024; size <= 16 * 1024 * 1024; size *= 2) {
            char* buff = new char[size];
            char* buff1 = new char[size];
            std::cout << "[SERVER] Testing with buffer size: " << size << " bytes" << std::endl;
            memcpy(buff, TEST_MESSAGE, strlen(TEST_MESSAGE));

            for (const auto &i : connections) {
                std::cout << "[SERVER] Invio a: " << i << std::endl;
                backend.send(i, buff, size, "./test", 0);
                std::cout << "[SERVER] Messaggio inviato a: " << i << " con lunghezza: " << size << std::endl;

                std::cout << "[SERVER] Attendo risposta da: " << i << std::endl;
                size_revc = size;
                backend.recive(buff1, &size_revc, &offset);
                std::cout << "[SERVER] Risposta ricevuta da " << i
                          << " con lunghezza: " << size_revc
                          << " Contenuto: " << std::string(buff1, size_revc) << std::endl;
                std::cout << "[SERVER] Ricevuto messaggio da " << i
          << " con lunghezza: " << size_revc
          << " Contenuto: " << std::string(buff1, size_revc) << std::endl;

            }

            delete[] buff;
            delete[] buff1;
        }
    }else {
        // Questo è un client, aspetta il messaggio e invia indietro i dati
        for (int size = 1024; size <= 16 * 1024 * 1024; size *= 2) {
            char* buffrec = new char[size];
            std::cout << "[CLIENT] In attesa del messaggio con buffer size: " << size << std::endl;

            backend.recive(buffrec, &size_revc, &offset);
            std::cout << "[CLIENT] Ricevuto messaggio da " << coordinator
                      << " con lunghezza: " << size_revc
                      << " Contenuto: " << std::string(buffrec, size_revc) << std::endl;


            backend.send(coordinator, buffrec, size_revc, "./test", 0);

            std::cout << "[CLIENT] Messaggio inviato indietro a " << coordinator << std::endl;

            delete[] buffrec;
        }
    }
}


#endif // TEST_CAPIOCOMMUNICATIONSERVICE_HPP*/

TEST(CapioCommServiceTest, TestPingPong) {
    // pare il il primo utente che fara da server

    gethostname(node_name.data(), HOST_NAME_MAX);
    CapioCommunicationService backend("TCP", "1234", 300);
    capio_off64_t size_revc, offset;

    std::vector<std::string> connections;

    do {
        connections = backend.get_open_connections();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } while (connections.empty());

    char ownHostname[HOST_NAME_MAX] = {0};
    gethostname(ownHostname, HOST_NAME_MAX);
    for (size_t size = 128 * 1024 * 1024; size <= 8L * 1024 * 1024 * 1024; size *= 2) { // 128MB -> 16GB
    //for (int size = 1024; size <= 16 * 1024 * 1024; size *= 2) {// 1KB, 2KB, 4KB, ..., 16MB
                char* buff = new char[size];
                char* buff1 = new char[size];
                char* buffrec = new char[size];
        std::cout << "Testing with buffer size: " << size << " bytes" << std::endl;
            for (const auto &i : connections) {

            memcpy(buff, TEST_MESSAGE, strlen(TEST_MESSAGE));

            if (i.compare(ownHostname) < 0) {
                // Inizio del tempo di invio
                auto start_time = std::chrono::high_resolution_clock::now();

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Sending ping to: " << i << std::endl;
                backend.send(i, buff, size, "./test", 0);
                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Sending ping to: " << i << std::endl;
                // Inizio del tempo di ricezione (solo dopo il send)
                backend.recive(buff1, &size_revc, &offset);
                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Sending ping to: " << i << std::endl;
                // Fine del tempo di ricezione
                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> total_duration = end_time - start_time;

                // Calcolare la banda considerando sia il messaggio inviato che ricevuto
                //double bandwidth = ((size * 2.0) / (1024.0 * 1024.0 * 1024.0)) / total_duration.count(); // GB/s
                std::cout << "ci ho messo" <<  total_duration.count() << "secondi a mandare " << (size * 2) / (1024.0 * 1024.0) << "MBytes" << std::endl;
                //std::cout << "Banda del mandante: " << bandwidth << " GB/s" << std::endl;
                double bandwidth = ((size * 2) / (1024.0 * 1024.0)) / total_duration.count(); // MB/s
                std::cout << "Banda del mandante  " << bandwidth << " MB/s" << std::endl;

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Received ping response from : " << i << std::endl;

            } else {
                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND :Listening for ping from: " << i
                          << std::endl;

                backend.recive(buffrec, &size_revc, &offset);

                // Inizio del tempo di invio risposta
              //  auto start_time = std::chrono::high_resolution_clock::now();

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << " SECOD: Received ping from: " << i << std::endl;

                backend.send(i, buffrec, size_revc, "./test", 0);

                // Fine del tempo di invio risposta
             /*   auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> send_duration = end_time - start_time;
                double bandwidth = ((size * 2.0) / (1024.0 * 1024.0 * 1024.0)) / send_duration.count(); // GB/s
                std::cout << "Banda del ricevente: " << bandwidth << " GB/s" << std::endl;*/

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND: Sent ping response to: " << i << std::endl;
            }
        }
        delete[] buff;
        delete[] buff1;
        delete[] buffrec;


    }
}

#endif // TEST_CAPIOCOMMUNICATIONSERVICE_HPP
