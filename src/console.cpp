#include "../include/console.hpp"
#include <iostream>
#include <map>
#include <mutex>
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
  } else if (command == "clients") {
    ClientsCommand(clients);
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

void ConsoleHandler::ClientsCommand(const std::map<int, ClientInfo> &clients) {
  std::lock_guard lock(consoleMutex);

  printf("Total: %lu clients\n", clients.size());
  printf("\n");

  if (clients.empty()) {
    printf("No clients connected.\n");
  } else {
    for (const auto &pair : clients) {
      printf("Client %d: %s:%d\n", pair.first, pair.second.ip.c_str(),
             pair.second.port);
    }
  }
}
