#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>

class SocketHandler {
public:
  void Create();
  void SetOpt();
  void Bind(int port);
  void Listen();
  void Accept();

  void Close();

  int getServerSocket() const { return serverSocket; }
  int getClientSocket() const { return clientSocket; }
  sockaddr_in getServerAddr() const { return serverAddr; }
  sockaddr_in getClientAddr() const { return clientAddr; }

private:
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  int addrLen = sizeof(struct sockaddr_in);
};

#endif
