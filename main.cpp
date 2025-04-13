#include <chrono>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

struct Block {
  vector<pair<int, int>> shape;
  int x, y;
};

Block T = {{{0, 1}, {1, 0}, {1, 1}, {1, 2}}, 4, 0};

Block I = {{{0, 0}, {1, 0}, {2, 0}, {3, 0}}, 4, 0};

Block O = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}, 4, 0};

Block L = {{{0, 0}, {1, 0}, {2, 0}, {2, 1}}, 4, 0};

Block J = {{{0, 1}, {1, 1}, {2, 0}, {2, 1}}, 4, 0};

Block S = {{{0, 1}, {0, 2}, {1, 0}, {1, 1}}, 4, 0};

int main() {
  int HEIGHT = 20;
  int WIDTH = 10;
  vector<vector<int>> grid(HEIGHT, vector<int>(WIDTH));
  Block current = T;
  while (true) {
    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        grid[i][j] = 0;
      }
    }

    bool canMove = true;
    for (auto [dy, dx] : current.shape) {
      int newY = current.y + dy + 1;
      int newX = current.x + dx;
      if (newY >= HEIGHT) {
        canMove = false;
        break;
      }
    }

    if (!canMove) {
      for (auto [dy, dx] : current.shape) {
        grid[current.y + dy][current.x + dx] = 1;
      }
      break;
    }

    current.y += 1;

    for (auto [dy, dx] : current.shape) {
      grid[current.y + dy][current.x + dx] = 1;
    }

    cout << u8"\033[2J\033[1;1H";
    for (int i = 0; i < grid.size(); i++) {
      for (int j = 0; j < grid[i].size(); j++) {
        cout << (grid[i][j] == 0 ? "." : "#") << " ";
      }
      cout << "\n";
    }

    this_thread::sleep_for(
        chrono::milliseconds(500)); // 500 ms delay (can be adjusted)
  }
  return 0;
}
