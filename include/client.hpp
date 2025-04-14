#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <string>
#include <unordered_map>

struct PositionPacket {
  float x, y;
  int id;
};

struct ClientInfo {
  int socket;
  std::string ip;
  int port;
  PositionPacket position;
};

class ClientHandler {
public:
  static ClientHandler &getInstance();

  void ListClients();
  void KickClient(int socket);
  void CreateClient(ClientInfo client, int socket);
  bool IsIpConnected(int socket);
  void BroadcastPosition(PositionPacket &positionData);
  void HandleClient(int socket, sockaddr_in clientAddr);
  void HandleMessage(int socket, char *message);
  void HandleDisconnect(int socket);

  std::unordered_map<int, ClientInfo> getClientsMap() const { return clients; }

private:
  ClientHandler() {}
  std::unordered_map<int, ClientInfo> clients;
};

#endif
