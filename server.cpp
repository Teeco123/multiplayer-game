#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

// macOS/Unix socket headers
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "include/socket.hpp"

#define PORT 8080

// Client tracking structure
struct ClientInfo {
  std::string ip;
  int port;
};

std::mutex consoleMutex;
std::mutex clientsMutex;

std::map<int, ClientInfo> clients;
std::set<std::string> connectedIPs;

bool isIPConnected(const std::string &ip) {
  std::lock_guard lock(clientsMutex);
  return connectedIPs.find(ip) != connectedIPs.end();
}

void listConnectedClients() {
  std::lock_guard lock(clientsMutex);

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

void kickClient(int socket) {
  std::lock_guard lock(clientsMutex);
  auto client = clients.find(socket);
  if (client != clients.end()) {
    printf("Kicking client %d (%s:%d).\n", socket, client->second.ip.c_str(),
           client->second.port);
    close(socket);
  } else {
    printf("No client with socket ID %d found.\n", socket);
  }
}

void helpCommand() {
  printf("clients\n");
  printf("kick <socket_id>\n");
  printf("help\n");
}

void HandleClient(int clientSocket, sockaddr_in clientAddr) {
  char buffer[1024];

  bool running = true;
  while (running) {
    // Message buffer
    memset(buffer, 0, 1024);

    // Client IP and PORT stringified
    char clientIP[INET_ADDRSTRLEN];
    int clientPort = ntohs(clientAddr.sin_port);
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

    // Check if server is receiving bytes from client else close connection
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      std::lock_guard lock(consoleMutex);
      printf("Client disconnected - %s:%d\n", clientIP, clientPort);
      running = false;
      break;
    }

    // Print client message
    {
      std::lock_guard lock(consoleMutex);
      printf("%s:%d - %s", clientIP, clientPort, buffer);
    }
  }

  // Cleanup
  {
    std::lock_guard lock(clientsMutex);
    clients.erase(clientSocket);
  }

  close(clientSocket);
}

int main() {
  SocketHandler socket;
  // Address structs and length
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  int addrLen = sizeof(struct sockaddr_in);
  std::vector<std::thread> threads;

  socket.Create();
  socket.SetOpt();
  socket.Bind(8080);
  socket.Listen();

  printf("-------------------------------------\n");
  printf("Server started at port - %d\n", PORT);
  printf("-------------------------------------\n");

  // Start a thread to handle server console commands
  std::thread consoleThread([&]() {
    std::string commandLine;
    while (true) {
      std::getline(std::cin, commandLine);
      if (commandLine.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
        continue;
      }

      std::istringstream iss(commandLine);
      std::string command;
      iss >> command;

      // help
      if (command == "help") {
        helpCommand();
        // kick <socket_id>
      } else if (command == "kick") {
        int socket;
        if (iss >> socket) {
          kickClient(socket);
        } else {
          printf("Usage: kick <socket_id>\n");
        }
        // clients
      } else if (command == "clients") {
        listConnectedClients();
      } else {
        printf("Unknown command. Type 'help' for help.\n");
      }
    }
  });
  consoleThread.detach();

  while (true) {

    socket.Accept();
    // Get ip and port of client
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);
    std::string ipString(clientIP);

    if (isIPConnected(ipString)) {
      std::lock_guard lock(consoleMutex);
      printf("Connection rejected - %s:%d (IP already connected)\n", clientIP,
             clientPort);
      const char *message = "Connection rejected: You are already connected "
                            "from this IP address.\n";
      send(clientSocket, message, strlen(message), 0);
      close(clientSocket);
      continue;
    }

    {
      std::lock_guard lock(consoleMutex);
      printf("Client connected - %s:%d \n", clientIP, clientPort);
    }

    // Store client info
    {
      std::lock_guard lock(clientsMutex);
      ClientInfo client;
      client.ip = clientIP;
      client.port = clientPort;
      clients[clientSocket] = client;
      connectedIPs.insert(ipString);
    }

    // Create new thread for a client and detach it
    threads.push_back(std::thread(HandleClient, clientSocket, clientAddr));
    threads.back().detach();
  }

  close(serverSocket);
  exit(EXIT_SUCCESS);
}
