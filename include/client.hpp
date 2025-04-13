#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <map>
#include <mutex>
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

private:
  ClientHandler() {}
  std::mutex clientMutex;
  std::map<int, ClientInfo> clients;
  std::set<std::string> connectedIPs;
};

#endif
