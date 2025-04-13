#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <string>

class ConsoleHandler {
public:
  void CreateThread();
  void ExecuteCommand(std::string &commandLine);

  void HelpCommand();

private:
  std::string commandLine;
};

#endif
