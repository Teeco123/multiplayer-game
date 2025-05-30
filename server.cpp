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

    // Store client info
    ClientInfo client;
    client.ip = clientIP;
    client.port = clientPort;
    client.socket = clientSocket;
    ClientHandler::getInstance().CreateClient(client, clientSocket);

    {
      std::lock_guard lock(MutexHandler::getInstance().consoleMutex);
      printf("Client connected - %s:%d \n", clientIP, clientPort);
    }

    // Create new thread for a client and detach it
    threads.push_back(std::thread(&ClientHandler::HandleClient,
                                  &ClientHandler::getInstance(), clientSocket,
                                  clientAddr));
    threads.back().detach();
  }

  socket.Close();
  exit(EXIT_SUCCESS);
}
