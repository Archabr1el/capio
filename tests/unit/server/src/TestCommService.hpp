#ifndef TEST_CAPICOMMUNIOCOMMUNICATIONSERVICE_HPP
#define TEST_CAPIOCATIONSERVICE_HPP

#include "../server/communication-service/CapioCommunicationService.hpp"
#include <gtest/gtest.h>
#include <thread>

/*
constexpr char TEST_MESSAGE[] = "hello world how is it going?";
constexpr capio_off64_t BUFFER_SIZES = 1024;
constexpr int REQUIRED_CONNECTIONS = 2; // Numero di connessioni richieste prima che il coordinatore inizi a inviare messaggi



TEST(CapioCommServiceTest, TestPingPong) {

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
    for (size_t size = 128 * 1024 * 1024; size <= 8L * 1024 * 1024 * 1024; size *= 2) { // 128MB -> 8GB  16...tropo
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
                std::cout << "ci ho messo" <<  total_duration.count() << "secondi a mandare due volte " << (size) / (1024.0 * 1024.0) << "MBytes" << std::endl;
                double bandwidth = ((size * 2) / (1024.0 * 1024.0)) / total_duration.count(); // MB/s
                std::cout << "Banda del mandante  " << bandwidth << " MB/s" << std::endl;

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "Received ping response from : " << i << std::endl;

            } else {
                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND :Listening for ping from: " << i
                          << std::endl;

                backend.recive(buffrec, &size_revc, &offset);

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << " SECOD: Received ping from: " << i << std::endl;

                backend.send(i, buffrec, size_revc, "./test", 0);

                std::cout << CAPIO_LOG_SERVER_CLI_LEVEL_INFO << "SECOND: Sent ping response to: " << i << std::endl;
            }
        }
        delete[] buff;
        delete[] buff1;
        delete[] buffrec;


    }
}

#endif // TEST_CAPIOCOMMUNICATIONSERVICE_HPP
*/

#include <chrono>
#include <vector>
#include <iostream>
#define EXPECTED_CONNECTIONS 3// Numero di client attesi numero macchine 4
#include <algorithm>
#include <cstring>
#define SERVER_HOSTNAME "fd-02"

//converti il tempo in un buffer da passare alla send
std::time_t convertToTimeT(const char* timestamp) {
    std::tm tm = {};
    std::istringstream stringa(timestamp);
    stringa >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (stringa.fail()) {
        std::cerr << "[ERROR] Errore nel parsing della data: " << timestamp << std::endl;
        throw std::runtime_error("Errore nel parsing della data");
    }

    std::time_t t = std::mktime(&tm);
    if (t == -1) {
        std::cerr << "[ERROR] std::mktime ha restituito -1 per timestamp: " << timestamp << std::endl;
    }

    return t;
}

void client(CapioCommunicationService& backend) {
    capio_off64_t size_recv, offset;
    //for (size_t size = 128 * 1024 * 1024; size <= 4L * 1024 * 1024 * 1024; size *= 2) { // con 8 connection fai 2L con 4 connection fai 4L con 11 connection fai 1L
    for (int size = 1024; size <= 16 *1024 * 1024; size *= 2) { // 1Kb -> 16Mb
        std::vector<char> buff_rec(size, 0);  // usato per problemi di Segmentatio Falut e poi tenuto nonostante non fosse quesot il problema

        std::cout << "[CLIENT] Waiting for message of size " << size << " bytes..." << std::endl;
        backend.recive(buff_rec.data(), &size_recv, &offset);

        if (size_recv < size) {
            buff_rec[size_recv] = '\0';
        }

        std::cout << "[CLIENT] Received message: " << buff_rec.data() << " (size: " << size_recv << ")" << std::endl;
        backend.send("fd-02", buff_rec.data(), size_recv, "./test", 0);

        auto end_time = std::chrono::high_resolution_clock::now();

        try {
            std::chrono::system_clock::time_point start =
                std::chrono::system_clock::from_time_t(convertToTimeT(buff_rec.data()));
            std::chrono::duration<double> total_duration = end_time - start;
            double bandwidth = ((size / (1024.0 * 1024.0)) / total_duration.count());
            std::cout << "Tempo: " << total_duration.count()
                      << " seconds, Bandwidth: " << bandwidth << " MB/s" << std::endl;
        } catch (const std::exception& e) { //anche questo catch fatto prcedentemete per segmentation falut
            std::cerr << "[ERROR] Exception in time conversion: " << e.what() << std::endl;
        }
    }
}

void fillBufferWithTimestamp(std::vector<char>& buffer) { //Segmentation falut risolto grazie al riempimento
    //del buffer con il tempo convertito in char * ma sopratutto con il riemimento del resto del buffer con '\0'

    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %H:%M:%S");
    std::string timestampStr = oss.str();

    if (timestampStr.size() > buffer.size()) {
        std::cerr << "[ERROR] Buffer too small for timestamp!" << std::endl;
        return;
    }


    std::memset(buffer.data(), 0, buffer.size());  //risoluzione segmentation falut
    std::memcpy(buffer.data(), timestampStr.c_str(), timestampStr.size());
}

void server(CapioCommunicationService& backend, const std::vector<std::string>& connections) {
    capio_off64_t size_recv, offset;
    //for (size_t size = 128 * 1024 * 1024; size <= 4L * 1024 * 1024 * 1024; size *= 2) {  // con 8 connection fai 2L con 4 connection fai 4L con 11 connection fai 1L
    for (int size = 1024; size <= 16*  1024 * 1024; size *= 2) { //1Kb-> 16Mb
        std::vector<char> buff(size, 0);      // Usa std::vector per gestione sicura
        std::vector<char> buff_rec(size, 0);  // Evita il rischio di buffer overflow

        std::cout << "[SERVER] Broadcasting message of size " << size << " bytes" << std::endl;

        for (const auto& client : connections) {
            if (client != SERVER_HOSTNAME) {

                std::cout << "[SERVER] Sending to " << client << std::endl;

                std::vector<char> timestampBuffer(size, 0);

                fillBufferWithTimestamp(timestampBuffer);
                std::cout << "[DEBUG] Sending " << size << " bytes to " << client
          << " - Buffer size: " << timestampBuffer.size() << std::endl;

                backend.send(client, timestampBuffer.data(), size, "./test", 0);

            }
        }

        std::cout << "[SERVER] Waiting for response from " << (connections.size() - 1) << " clients" << std::endl;
        backend.recive(buff_rec.data(), &size_recv, &offset);
        std::cout << "[SERVER] Received response of size " << size_recv << " bytes" << std::endl;
    }
}



TEST(CapioCommServiceTest, TestBroadcast) {
    CapioCommunicationService backend("TCP", "1234", 300);
    std::vector<std::string> connections;
    char ownHostname[HOST_NAME_MAX] = {0};
    gethostname(ownHostname, HOST_NAME_MAX);

    do {
        connections = backend.get_open_connections();
        std::cout << "[WAITING] Connessioni: " << connections.size() << "/" << EXPECTED_CONNECTIONS << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (connections.size() < EXPECTED_CONNECTIONS);

    if (std::string(ownHostname) == SERVER_HOSTNAME) {
        std::cout << "[SERVER] Sono il server " << ownHostname << ", avvio il broadcast!" << std::endl;
        server(backend, connections);
    } else {
        std::cout << "[CLIENT] Sono un client " << ownHostname << ", in attesa del messaggio dal server." << std::endl;
        client(backend);
    }
}

#endif // TEST_CAPIOCOMMUNICATIONSERVICE_HPP