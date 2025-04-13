#include "../include/client.hpp"
#include "../include/mutex.hpp"
#include <arpa/inet.h>
#include <mutex>
#include <netinet/in.h>
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
    for (const auto &pair : clients) {
      printf("Client %d: %s:%d\n", pair.first, pair.second.ip.c_str(),
             pair.second.port);
    }
  }
}

void ClientHandler::KickClient(int socket) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);

  auto client = clients.find(socket);
  if (client != clients.end()) {
    printf("Kicking client %d (%s:%d).\n", socket, client->second.ip.c_str(),
           client->second.port);
    close(socket);
  } else {
    printf("No client with socket ID %d found.\n", socket);
  }
}

void ClientHandler::CreateClient(ClientInfo client, int clientSocket) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);

  clients[clientSocket] = client;

  std::string ipString(client.ip);
  connectedIPs.insert(ipString);
}

bool ClientHandler::IsIpConnected(std::string &ip) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);
  return connectedIPs.find(ip) != connectedIPs.end();
}

void ClientHandler::HandleMessage(const char *clientIP, int clientPort,
                                  const char *message) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
  printf("%s:%d - %s", clientIP, clientPort, message);
}

void ClientHandler::HandleDisconnect(int clientSocket, const char *clientIP,
                                     int clientPort) {
  // Print disconnect message
  {
    std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
    printf("Client disconnected - %s:%d\n", clientIP, clientPort);
  }

  // Remove client from clients map
  {
    std::lock_guard lock(MutexHandler::getInstance().clientMutex);
    clients.erase(clientSocket);

    std::string ipString(clientIP);
    connectedIPs.erase(ipString);
  }

  close(clientSocket);
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
      HandleDisconnect(clientSocket, clientIP, clientPort);
      break;
    }

    // Process received message
    HandleMessage(clientIP, clientPort, buffer);
  }
}
