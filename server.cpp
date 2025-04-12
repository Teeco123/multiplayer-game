#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define PORT 8080

std::mutex consoleMutex;

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

  // Cleanup client socket
  close(clientSocket);
}

int main() {
  std::vector<std::thread> threads;

  // Address structs and length
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  int addrLen = sizeof(struct sockaddr_in);

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

    {
      std::lock_guard lock(consoleMutex);
      printf("Client connected - %s:%d \n", clientIP, clientPort);
    }

    // Create new thread for a client and detach it
    threads.push_back(std::thread(HandleClient, clientSocket, clientAddr));
    threads.back().detach();
  }

  exit(EXIT_SUCCESS);
}
