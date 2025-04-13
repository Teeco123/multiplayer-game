#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <map>
#include <mutex>

struct ClientInfo {
  std::string ip;
  int port;
};

class ConsoleHandler {
public:
  void CreateThread();
  void ExecuteCommand(std::string &commandLine);

  void HelpCommand();
  void ClientsCommand(const std::map<int, ClientInfo> &clients);

private:
  std::mutex consoleMutex;
  std::string commandLine;
  std::map<int, ClientInfo> clients;
};

#endif
