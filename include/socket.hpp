#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>

class SocketHandler {
public:
  void Create();
  void SetOpt();
  void Bind(int port);
  void Listen();
  void Accept();

private:
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  int addrLen = sizeof(struct sockaddr_in);
};

#endif
