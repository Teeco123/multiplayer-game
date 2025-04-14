#include "../include/console.hpp"
#include "../include/client.hpp"
#include <iostream>
#include <thread>

void ConsoleHandler::CreateThread() {
  std::thread consoleThread([this]() {
    std::string commandLine;
    while (true) {
      std::getline(std::cin, commandLine);
      ExecuteCommand(commandLine);
    }
  });
  consoleThread.detach();

  printf("Console ready. Type 'help' for available commands.\n");
}

void ConsoleHandler::ExecuteCommand(std::string &commandLine) {
  // Skip empty commands
  if (commandLine.find_first_not_of(" \t\n\r\f\v") == std::string::npos) {
    return;
  }

  std::istringstream iss(commandLine);
  std::string command;
  iss >> command;

  // Handle commands
  if (command == "help") {
    HelpCommand();
  } else if (command == "kick") {
    int socket;
    if (iss >> socket) {
      ClientHandler::getInstance().KickClient(socket);
    } else {
      printf("Usage: kick <socket_id>\n");
    }
  } else if (command == "clients") {
    ClientHandler::getInstance().ListClients();
  } else {
    printf("Unknown command. Type 'help' for help.\n");
  }
}

void ConsoleHandler::HelpCommand() {
  printf("Available commands:\n");
  printf("  clients\n");
  printf("  kick\n");
  printf("  help\n");
}
