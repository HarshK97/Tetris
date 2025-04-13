#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

struct Block {
  vector<pair<int, int>> shape;
  int x, y;
};

Block getRandomBlock() {
  int x = 4, y = 0;
  vector<vector<pair<int, int>>> shapes = {
      {{0, 1}, {1, 0}, {1, 1}, {1, 2}}, // T
      {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, // I
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}}, // O
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}}, // L
      {{0, 1}, {1, 1}, {2, 0}, {2, 1}}, // J
      {{0, 1}, {0, 2}, {1, 0}, {1, 1}}, // S
      {{0, 0}, {0, 1}, {1, 1}, {1, 2}}  // Z
  };

  static random_device rd;
  static mt19937 gen(rd());
  uniform_int_distribution<> dist(0, 6);
  int idx = dist(gen);
  return Block{shapes[idx], x, y};
}

int HEIGHT = 20;
int WIDTH = 10;
vector<vector<int>> grid(HEIGHT, vector<int>(WIDTH));

int main() {
  Block current = getRandomBlock();
  Block next = getRandomBlock();

  while (true) {
    vector<vector<int>> tempGrid = grid;

    bool canMove = true;
    for (auto [dy, dx] : current.shape) {
      int newY = current.y + dy + 1;
      int newX = current.x + dx;
      if (newY >= HEIGHT || grid[newY][newX] != 0) {
        canMove = false;
        break;
      }
    }

    if (!canMove) {
      for (auto [dy, dx] : current.shape) {
        int y = current.y + dy;
        int x = current.x + dx;
        if (y >= 0 && y < HEIGHT) {
          grid[y][x] = 1;
        }
      }

      bool gameOver = false;
      for (auto [dy, dx] : next.shape) {
        int newY = next.y + dy;
        int newX = next.x + dx;
        if (grid[newY][newX] != 0) {
          gameOver = true;
          break;
        }
      }

      if (gameOver) {
        cout << "\nGame Over!\n";
        break;
      }

      current = next;
      next = getRandomBlock();
      continue;
    }

    current.y += 1;

    for (auto [dy, dx] : current.shape) {
      int y = current.y + dy;
      int x = current.x + dx;
      if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
        tempGrid[y][x] = 1;
      }
    }

    cout << u8"\033[2J\033[1;1H";
    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        cout << (tempGrid[i][j] == 0 ? "." : "#") << " ";
      }
      cout << "\n";
    }

    this_thread::sleep_for(chrono::milliseconds(500));
  }

  return 0;
}
