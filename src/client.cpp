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

void ClientHandler::HandleMessage(std::string ip, char *message) {
  std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
  auto client = clients.find(ip);
  if (client != clients.end()) {
    printf("%s:%d - %s", client->second.ip.c_str(), client->second.port,
           message);
  } else {
    printf("Error occured while sending message.\n");
  }
}

void ClientHandler::HandleDisconnect(std::string ip) {

  auto client = clients.find(ip);
  if (client != clients.end()) {
    // Remove client from clients map
    {
      std::lock_guard lock(MutexHandler::getInstance().clientMutex);
      clients.erase(ip);
    }
    // Print disconnect message
    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Client disconnected - %s\n", ip.c_str());
    }

    close(client->second.socket);
  } else {
    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Error occured while disconnecting user with IP %s", ip.c_str());
    }
  }
}

void ClientHandler::BroadcastPosition(PositionPacket &positionData) {
  std::lock_guard lock(MutexHandler::getInstance().clientMutex);

  send(4, &positionData, sizeof(PositionPacket), 0);
  send(5, &positionData, sizeof(PositionPacket), 0);
}

void ClientHandler::HandleClient(std::string ip, sockaddr_in clientAddr) {
  char buffer[1024];
  PositionPacket positionData;

  auto client = clients.find(ip);
  while (client != clients.end()) {
    // Clear message buffer
    memset(buffer, 0, 1024);

    int bytes_read =
        recv(client->second.socket, &positionData, sizeof(PositionPacket), 0);

    // Check if server is receiving bytes from client
    int bytesReceived = recv(client->second.socket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      HandleDisconnect(ip);
      break;
    }

    // Process received message
    // HandleMessage(ip, buffer);
    BroadcastPosition(positionData);
  }
}
