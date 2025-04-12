#include <iostream>
#include <utility>
#include <vector>

using namespace std;

struct Block {
  vector<pair<int, int>> shape;
  int x, y;
};

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
