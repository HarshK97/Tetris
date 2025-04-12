#include <iostream>
#include <utility>
#include <vector>

using namespace std;

struct Block {
  vector<pair<int, int>> shape;
  int x, y;
};

Block T = {{{0, 1}, {1, 0}, {1, 1}, {1, 2}}, 3, 0};

Block I = {{{0, 0}, {1, 0}, {2, 0}, {3, 0}}, 3, 0};

Block O = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}, 3, 0};

Block L = {{{0, 0}, {1, 0}, {2, 0}, {2, 1}}, 3, 0};

Block J = {{{0, 1}, {1, 1}, {2, 0}, {2, 1}}, 3, 0};

Block S = {{{0, 1}, {0, 2}, {1, 0}, {1, 1}}, 3, 0};

int main() {
  vector<vector<int>> grid(20, vector<int>(10));
  for (int i = 0; i < grid.size(); i++) {
    for (int j = 0; j < grid[i].size(); j++) {
      grid[i][j] = 0;
    }
  }
  for (int i = 0; i < grid.size(); i++) {
    for (int j = 0; j < grid[i].size(); j++) {
      cout << grid[i][j] << " ";
    }
    cout << "\n";
  }
  return 0;
}
