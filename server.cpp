#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

int main() {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (serverSocket == -1) {
    printf("Socket failed to start\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket started\n");
  };

  struct sockaddr_in serverAddr, clientAddr;
  char buffer[1024];
  int addrLen = sizeof(struct sockaddr_in);

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(8080);

  // Bind
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    printf("Bind failed\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Bind success\n");
  }

  // Listen
  if (listen(serverSocket, 3) == -1) {
    printf("Listen failed\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Listen success\n");
  }

  // Accept a connection
  int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                            (socklen_t *)&addrLen);
  if (clientSocket == -1) {
    printf("Accept failed\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Client connected\n");
  }

  bool running = true;
  while (running) {
    memset(buffer, 0, 1024);

    int bytesReceived = recv(clientSocket, buffer, 1024, 0);

    if (bytesReceived <= 0) {
      printf("Client disconnected\n");
      break;
    }

    printf("Received: %s", buffer);
  }

  exit(EXIT_SUCCESS);
}
