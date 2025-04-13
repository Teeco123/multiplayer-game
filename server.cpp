#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

// macOS/Unix socket headers
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "include/client.hpp"
#include "include/console.hpp"
#include "include/socket.hpp"

#define PORT 8080

// Client tracking structure

std::mutex consoleMutex;
std::mutex clientsMutex;

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
  }

  close(clientSocket);
}

int main() {
  SocketHandler socket;
  ConsoleHandler console;

  std::vector<std::thread> threads;

  socket.Create();
  socket.SetOpt();
  socket.Bind(8080);
  socket.Listen();

  printf("-------------------------------------\n");
  printf("Server started at port - %d\n", PORT);
  printf("-------------------------------------\n");

  console.CreateThread();

  while (true) {

    socket.Accept();

    auto clientAddr = socket.getClientAddr();
    auto clientSocket = socket.getClientSocket();

    // Get ip and port of client
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);
    std::string ipStr(clientIP);

    if (ClientHandler::getInstance().IsIpConnected(ipStr)) {
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
    ClientInfo client;
    client.ip = clientIP;
    client.port = clientPort;
    ClientHandler::getInstance().CreateClient(client, clientSocket);

    // Create new thread for a client and detach it
    threads.push_back(std::thread(HandleClient, clientSocket, clientAddr));
    threads.back().detach();
  }

  socket.Close();
  exit(EXIT_SUCCESS);
}
