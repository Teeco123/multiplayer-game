#include <thread>
#include <vector>

// macOS/Unix socket headers
#include <arpa/inet.h>
#include <unistd.h>

#include "include/client.hpp"
#include "include/console.hpp"
#include "include/mutex.hpp"
#include "include/socket.hpp"

#define PORT 8080

std::mutex consoleMutex;
std::mutex clientsMutex;

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
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Connection rejected - %s:%d (IP already connected)\n", clientIP,
             clientPort);
      const char *message = "Connection rejected: You are already connected "
                            "from this IP address.\n";
      send(clientSocket, message, strlen(message), 0);
      close(clientSocket);
      continue;
    }

    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Client connected - %s:%d \n", clientIP, clientPort);
    }

    // Store client info
    ClientInfo client;
    client.ip = clientIP;
    client.port = clientPort;
    ClientHandler::getInstance().CreateClient(client, clientSocket);

    // Create new thread for a client and detach it
    threads.push_back(std::thread(&ClientHandler::HandleClient,
                                  &ClientHandler::getInstance(), clientSocket,
                                  clientAddr));
    threads.back().detach();
  }

  socket.Close();
  exit(EXIT_SUCCESS);
}
