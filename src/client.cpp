#include "../include/client.hpp"
#include <mutex>
#include <unistd.h>

ClientHandler &ClientHandler::getInstance() {
  static ClientHandler instance;
  return instance;
}

void ClientHandler::ListClients() {
  std::lock_guard lock(clientMutex);

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
  std::lock_guard lock(clientMutex);

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
  std::lock_guard lock(clientMutex);

  clients[clientSocket] = client;

  std::string ipString(client.ip);
  connectedIPs.insert(ipString);
}

bool ClientHandler::IsIpConnected(std::string &ip) {
  std::lock_guard lock(clientMutex);
  return connectedIPs.find(ip) != connectedIPs.end();
}
