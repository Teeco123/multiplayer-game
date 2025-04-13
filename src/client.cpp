#include "../include/client.hpp"
#include "../include/mutex.hpp"
#include <arpa/inet.h>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

ClientHandler &ClientHandler::getInstance() {
  static ClientHandler instance;
  return instance;
}

void ClientHandler::ListClients() {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);

  printf("Total: %lu clients\n", clients.size());
  printf("\n");

  if (clients.empty()) {
    printf("No clients connected.\n");
  } else {
    for (const auto &client : clients) {
      printf("Client %d: %s:%d\n", client.second.socket,
             client.second.ip.c_str(), client.second.port);
    }
  }
}

void ClientHandler::KickClient(std::string ip) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);

  auto client = clients.find(ip);
  if (client != clients.end()) {
    printf("Kicking client %d (%s:%d).\n", client->second.socket,
           client->second.ip.c_str(), client->second.port);
    close(client->second.socket);
  } else {
    printf("No client with IP %s found.\n", ip.c_str());
  }
}

void ClientHandler::CreateClient(ClientInfo client, std::string clientIP) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);

  clients[clientIP] = client;
}

bool ClientHandler::IsIpConnected(std::string ip) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);
  return clients.find(ip) != clients.end();
}

void ClientHandler::HandleMessage(const char *clientIP, int clientPort,
                                  const char *message) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
  printf("%s:%d - %s", clientIP, clientPort, message);
}

void ClientHandler::HandleDisconnect(std::string ip) {

  auto client = clients.find(ip);
  // Print disconnect message
  {
    std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
    printf("Client disconnected - %s\n", ip.c_str());
  }

  // Remove client from clients map
  {
    std::lock_guard lock(MutexHandler::getInstance().clientMutex);
    clients.erase(ip);
  }

  close(client->second.socket);
}

void ClientHandler::HandleClient(int clientSocket, sockaddr_in clientAddr) {
  char buffer[1024];
  bool running = true;

  // Client IP and PORT stringified
  char clientIP[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
  int clientPort = ntohs(clientAddr.sin_port);

  while (running) {
    // Clear message buffer
    memset(buffer, 0, 1024);

    // Check if server is receiving bytes from client
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      running = false;
      HandleDisconnect(clientIP);
      break;
    }

    // Process received message
    HandleMessage(clientIP, clientPort, buffer);
  }
}
