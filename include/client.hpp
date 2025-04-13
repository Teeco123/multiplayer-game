#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <string>
#include <unordered_map>

struct ClientInfo {
  int socket;
  std::string ip;
  int port;
};

class ClientHandler {
public:
  static ClientHandler &getInstance();

  void ListClients();
  void KickClient(std::string clientIP);
  void CreateClient(ClientInfo client, std::string clientIP);
  bool IsIpConnected(std::string clientIP);
  void HandleClient(std::string clientIP, sockaddr_in clientAddr);
  void HandleMessage(std::string clientIP, char *message);
  void HandleDisconnect(std::string clientIP);

  std::unordered_map<std::string, ClientInfo> getClientsMap() const {
    return clients;
  }

private:
  ClientHandler() {}
  std::unordered_map<std::string, ClientInfo> clients;
};

#endif
