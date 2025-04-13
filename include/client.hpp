#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <map>
#include <mutex>
#include <netinet/in.h>
#include <set>

struct ClientInfo {
  std::string ip;
  int port;
};

class ClientHandler {
public:
  static ClientHandler &getInstance();

  void ListClients();
  void KickClient(int socket);
  void CreateClient(ClientInfo client, int clientSocket);
  bool IsIpConnected(std::string &ip);
  void HandleClient(int clientSocket, sockaddr_in clientAddr);
  void HandleMessage(const char *clientIP, int clientPort, const char *message);
  void HandleDisconnect(int clientSocket, const char *clientIP, int clientPort);

  std::map<int, ClientInfo> getClientsMap() const { return clients; }

private:
  ClientHandler() {}
  std::mutex clientMutex;
  std::mutex consoleMutex;
  std::map<int, ClientInfo> clients;
  std::set<std::string> connectedIPs;
};

#endif
