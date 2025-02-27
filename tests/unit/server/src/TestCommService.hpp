#ifndef TEST_CAPIOCOMMUNICATIONSERVICE_HPP
#define TEST_CAPIOCOMMUNICATIONSERVICE_HPP

#include "../server/communication-service/CapioCommunicationService.hpp"
#include <gtest/gtest.h>
#include <thread>

constexpr char TEST_MESSAGE[] = "hello world how is it going?";
constexpr capio_off64_t BUFFER_SIZES = 1024;




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
    for (int size = 1024; size <= 16 * 1024 * 1024; size *= 2) {// 1KB, 2KB, 4KB, ..., 16MB
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
                double bandwidth = ((size * 2) / (1024.0 * 1024.0)) / total_duration.count(); // MB/s
                std::cout << "Banda del mandante  " << bandwidth << " MB/s" << std::endl;

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Received ping response from : " << i << std::endl;

            } else {
                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND :Listening for ping from: " << i
                          << std::endl;

                backend.recive(buffrec, &size_revc, &offset);

                // Inizio del tempo di invio risposta
                auto start_time = std::chrono::high_resolution_clock::now();

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << " SECOD: Received ping from: " << i << std::endl;

                backend.send(i, buffrec, size_revc, "./test", 0);

                // Fine del tempo di invio risposta
                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> send_duration = end_time - start_time;
                double bandwidth = (size * 2) / (1024.0 * 1024.0) / send_duration.count(); // MB/s
                std::cout << "SECOND: Banda in invio risposta: " << bandwidth << " MB/s" << std::endl;

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND: Sent ping response to: " << i << std::endl;
            }
        }
        delete[] buff;
        delete[] buff1;
        delete[] buffrec;


    }
}

#endif // TEST_CAPIOCOMMUNICATIONSERVICE_HPP
