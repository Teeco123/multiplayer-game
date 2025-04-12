#include "../include/socket.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

void SocketHandler::Create() {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    printf("Socket failed to start\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket started\n");
  };
}

void SocketHandler::SetOpt() {
  int opt = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
      0) {
    printf("Setsockopt failed: %s", strerror(errno));
    close(serverSocket);
    exit(EXIT_FAILURE);
  }
}

void SocketHandler::Bind(int port) {
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    printf("Bind failed\n");
    close(serverSocket);
    exit(EXIT_FAILURE);
  } else {
    printf("Binded successfully\n");
  }
}

void SocketHandler::Listen() {
  if (listen(serverSocket, 3) == -1) {
    printf("Listen failed\n");
    close(serverSocket);
    exit(EXIT_FAILURE);
  } else {
    printf("Listening successfully\n");
  }
}

void SocketHandler::Accept() {
  clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                        (socklen_t *)&addrLen);

  if (clientSocket == -1) {
    printf("Accept failed %s\n", strerror(errno));
  } else {
    printf("New client is connecting\n");
  }
}

void SocketHandler::Close() { close(serverSocket); }
