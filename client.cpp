#include "raylib.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight,
             "raylib [core] example - keyboard input");

  Vector2 ballPosition = {(float)screenWidth / 2, (float)screenHeight / 2};

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

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_RIGHT))
      ballPosition.x += 2.0f;
    if (IsKeyDown(KEY_LEFT))
      ballPosition.x -= 2.0f;
    if (IsKeyDown(KEY_UP))
      ballPosition.y -= 2.0f;
    if (IsKeyDown(KEY_DOWN))
      ballPosition.y += 2.0f;

    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("move the ball with arrow keys", 10, 10, 20, DARKGRAY);

    DrawCircleV(ballPosition, 50, MAROON);

    EndDrawing();
  }

  CloseWindow(); // Close window and OpenGL context

  return 0;
}
