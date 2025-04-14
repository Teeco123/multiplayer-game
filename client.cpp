#include "include/client.hpp"
#include "raylib.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void receive_updates(int sock, int id) {
  PositionPacket positionData;

  while (true) {
    int bytesRead = recv(sock, &positionData, sizeof(PositionPacket), 0);

    if (bytesRead <= 0) {
      printf("Server disconnected\n");
      break;
    }

    if (bytesRead == sizeof(PositionPacket)) {
      // Skip updates about our own position (optional)
      if (positionData.id != id) {
        printf("Player %d moved to (%f, %f)\n", positionData.id, positionData.x,
               positionData.y);
      }
    }
  }
}

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight,
             "raylib [core] example - keyboard input");

  srand((unsigned)time(NULL));
  int randNumb = rand();
  PositionPacket ballPosition = {(float)screenWidth / 2,
                                 (float)screenHeight / 2, randNumb};

  SetTargetFPS(60);

  // Networking
  struct sockaddr_in serv_addr;

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    printf("Socket failed to start: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  } else {
    printf("Socket started\n");
  };

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);

  if (inet_pton(AF_INET, "192.168.0.115", &serv_addr.sin_addr) <= 0) {
    printf("Invalid address\n");
    return -1;
  } else {
    printf("Address found\n");
  }

  // Connect to server
  if (connect(serverSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
      0) {
    printf("Connection Failed\n");
    return -1;
  } else {
    printf("Connected\n");
  }

  std::thread receive_thread(receive_updates, serverSocket, randNumb);

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_RIGHT))
      ballPosition.x += 4.0f;
    if (IsKeyDown(KEY_LEFT))
      ballPosition.x -= 4.0f;
    if (IsKeyDown(KEY_UP))
      ballPosition.y -= 4.0f;
    if (IsKeyDown(KEY_DOWN))
      ballPosition.y += 4.0f;

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);

    DrawCircleV({ballPosition.x, ballPosition.y}, 50, MAROON);

    EndDrawing();

    send(serverSocket, &ballPosition, sizeof(PositionPacket), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  close(serverSocket);
  CloseWindow(); // Close window and OpenGL context

  return 0;
}
