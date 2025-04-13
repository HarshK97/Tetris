#include <chrono>
#include <iostream>
#include <ncurses.h>
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
int score = 0;
vector<vector<int>> grid(HEIGHT, vector<int>(WIDTH));

bool canMoveLeft(Block &block) {
  for (auto [dy, dx] : block.shape) {
    int newY = block.y + dy;
    int newX = block.x + dx - 1;
    if (newX < 0 || newY >= HEIGHT || grid[newY][newX] != 0) {
      return false;
    }
  }
  return true;
}

bool canMoveRight(Block &block) {
  for (auto [dy, dx] : block.shape) {
    int newY = block.y + dy;
    int newX = block.x + dx + 1;
    if (newX > WIDTH || newY >= HEIGHT || grid[newY][newX] != 0) {
      return false;
    }
  }
  return true;
}

bool canMoveDown(Block &block) {
  for (auto [dy, dx] : block.shape) {
    int newY = block.y + dy + 1;
    int newX = block.x + dx;
    if (newY >= HEIGHT || grid[newY][newX] != 0) {
      return false;
    }
  }
  return true;
}

void rotateBlock(Block &block) {
  vector<pair<int, int>> rotatedShape;

  for (auto [dy, dx] : block.shape) {
    rotatedShape.push_back({-dx, dy});
  }

  bool validRotation = true;
  for (auto [dy, dx] : block.shape) {
    int newY = block.y + dy;
    int newX = block.x + dx;
    if (newY >= HEIGHT || newX < 0 || newX >= WIDTH || grid[newY][newX] != 0) {
      validRotation = false;
      break;
    }
  }

  if (validRotation) {
    block.shape = rotatedShape;
  }
}

void clearFullLines() {
  for (int i = HEIGHT - 1; i >= 0; --i) {
    bool full = true;
    for (int j = 0; j < WIDTH; ++j) {
      if (grid[i][j] == 0) {
        full = false;
        break;
      }
    }

    if (full) {
      score += 100;
      for (int row = i; row > 0; --row) {
        for (int col = 0; col < WIDTH; ++col) {
          grid[row][col] = grid[row - 1][col];
        }
      }

      for (int col = 0; col < WIDTH; ++col) {
        grid[0][col] = 0;
      }

      i++;
    }
  }
}

int main() {
  initscr();
  start_color();
  use_default_colors();
  noecho();
  cbreak();
  curs_set(0);
  nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);

  init_pair(1, COLOR_WHITE, -1);
  init_pair(2, COLOR_CYAN, -1);

  Block current = getRandomBlock();
  Block next = getRandomBlock();

  while (true) {
    vector<vector<int>> tempGrid = grid;

    int ch = getch();

    if (ch == KEY_LEFT) {
      if (canMoveLeft(current))
        current.x--;
    } else if (ch == KEY_RIGHT) {
      if (canMoveRight(current))
        current.x++;
    } else if (ch == KEY_DOWN) {
      if (canMoveDown(current))
        current.y++;
    } else if (ch == KEY_UP) {
      rotateBlock(current);
    }

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

    clearFullLines();

    clear();

    mvprintw(0, WIDTH / 2 - 5, "Score: %d", score);

    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        if (tempGrid[i][j] == 0) {
          attron(COLOR_PAIR(1));
          mvprintw(i + 1, j * 2, ". ");
          attroff(COLOR_PAIR(1));
        } else {
          attron(COLOR_PAIR(2));
          mvprintw(i + 1, j * 2, "# ");
          attroff(COLOR_PAIR(2));
        }
      }
    }

    refresh();

    this_thread::sleep_for(chrono::milliseconds(500));
  }

  return 0;
}
