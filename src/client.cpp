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

void ClientHandler::KickClient(int socket) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);

  auto client = clients.find(socket);
  if (client != clients.end()) {
    printf("Kicking client %d (%s:%d).\n", client->second.socket,
           client->second.ip.c_str(), client->second.port);
    close(client->second.socket);
  } else {
    printf("No client with socket %d found.\n", socket);
  }
}

void ClientHandler::CreateClient(ClientInfo client, int socket) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);

  clients[socket] = client;
}

void ClientHandler::HandleMessage(int socket, char *message) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
  auto client = clients.find(socket);
  if (client != clients.end()) {
    printf("%s:%d - %s", client->second.ip.c_str(), client->second.port,
           message);
  } else {
    printf("Error occured while sending message.\n");
  }
}

void ClientHandler::HandleDisconnect(int socket) {

  auto client = clients.find(socket);
  if (client != clients.end()) {
    // Remove client from clients map
    {
      std::lock_guard lock(MutexHandler::getInstance().clientMutex);
      clients.erase(socket);
    }
    // Print disconnect message
    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Client disconnected - %d\n", socket);
    }

    close(client->second.socket);
  } else {
    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Error occured while disconnecting user with socket %d", socket);
    }
  }
}

void ClientHandler::BroadcastPosition(PositionPacket &positionData) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);

  for (const auto &client : clients) {
    send(client.second.socket, &positionData, sizeof(PositionPacket), 0);
  }
}

void ClientHandler::HandleClient(int socket, sockaddr_in clientAddr) {
  char buffer[1024];
  PositionPacket positionData;

  auto client = clients.find(socket);
  while (client != clients.end()) {
    // Clear message buffer
    memset(buffer, 0, 1024);

    int bytes_read =
        recv(client->second.socket, &positionData, sizeof(PositionPacket), 0);

    // Check if server is receiving bytes from client
    int bytesReceived = recv(client->second.socket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      HandleDisconnect(socket);
      break;
    }

    // Process received message
    // HandleMessage(ip, buffer);
    BroadcastPosition(positionData);
  }
}
