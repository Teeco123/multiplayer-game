#include "../include/mutex.hpp"

MutexHandler &MutexHandler::getInstance() {
  static MutexHandler instance;
  return instance;
}
