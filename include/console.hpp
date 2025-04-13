#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <mutex>

class ConsoleHandler {
public:
  void CreateThread();
  void ExecuteCommand(std::string &commandLine);

  void HelpCommand();

private:
  std::mutex consoleMutex;
  std::string commandLine;
};

#endif
