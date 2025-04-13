#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <mutex>

class MutexHandler {
public:
  static MutexHandler &getInstance();

  std::mutex consoleMutex;
  std::mutex clientMutex;
};

#endif
