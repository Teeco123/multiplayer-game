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

  printf("\n==== CONNECTED CLIENTS ====\n");
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
  printf("==========================\n");
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
  // Address structs and length
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  int addrLen = sizeof(struct sockaddr_in);
  std::vector<std::thread> threads;

  // Create server socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    printf("Socket failed to start\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket started\n");
  };

  // Set reuse addres for socket
  int opt = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    printf("Setsockopt failed: %s", strerror(errno));
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Set addres family and port
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  // Bind server socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    printf("Bind failed\n");
    close(serverSocket);
    exit(EXIT_FAILURE);
  } else {
    printf("Binded successfully\n");
  }

  // Start listening
  if (listen(serverSocket, 3) == -1) {
    printf("Listen failed\n");
    close(serverSocket);
    exit(EXIT_FAILURE);
  } else {
    printf("Listening successfully\n");
  }

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

      if (command == "clients") {
        listConnectedClients();
      } else if (command == "kick") {
        int socket;
        if (iss >> socket) {
          std::lock_guard lock(clientsMutex);
          auto client = clients.find(socket);
          if (client != clients.end()) {
            printf("Kicking client %d (%s:%d).\n", socket,
                   client->second.ip.c_str(), client->second.port);
            close(socket);
          } else {
            printf("No client with socket ID %d found.\n", socket);
          }
        } else {
          printf("Usage: kick <socketId>\n");
        }
      } else {
        printf("Unknown command. Type 'help' for help.\n");
      }
    }
  });
  consoleThread.detach();

  while (true) {
    // Create client socket
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                          (socklen_t *)&addrLen);

    // Accept new connection from client
    if (clientSocket == -1) {
      printf("Accept failed %s\n", strerror(errno));
    } else {
      printf("New client is connecting\n");
    }

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
