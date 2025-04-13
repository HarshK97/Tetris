#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

using namespace std;

const int HEIGHT = 20;
const int WIDTH = 10;
int score = 0;
bool game_over = false;

struct Block {
  vector<pair<int, int>> shape;
  int x, y;
};

vector<vector<int>> grid(HEIGHT, vector<int>(WIDTH));
Block current, next_block;

Block getRandomBlock() {
  vector<vector<pair<int, int>>> shapes = {
      {{0, 1}, {1, 0}, {1, 1}, {1, 2}}, // T
      {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, // I
      {{0, 0}, {0, 1}, {1, 0}, {1, 1}}, // O
      {{0, 0}, {1, 0}, {2, 0}, {2, 1}}, // L
      {{0, 1}, {1, 1}, {2, 0}, {2, 1}}, // J
      {{0, 1}, {0, 2}, {1, 0}, {1, 1}}, // S
      {{0, 0}, {0, 1}, {1, 1}, {1, 2}}  // Z
  };

  Block b;
  b.x = 4;
  b.y = 0;
  b.shape = shapes[rand() % shapes.size()];
  return b;
}

bool canMoveLeft(Block &b) {
  for (auto &[dy, dx] : b.shape) {
    int x = b.x + dx - 1;
    int y = b.y + dy;
    if (x < 0 || y >= HEIGHT || grid[y][x])
      return false;
  }
  return true;
}

bool canMoveRight(Block &b) {
  for (auto &[dy, dx] : b.shape) {
    int x = b.x + dx + 1;
    int y = b.y + dy;
    if (x >= WIDTH || y >= HEIGHT || grid[y][x])
      return false;
  }
  return true;
}

bool canMoveDown(Block &b) {
  for (auto &[dy, dx] : b.shape) {
    int x = b.x + dx;
    int y = b.y + dy + 1;
    if (y >= HEIGHT || grid[y][x])
      return false;
  }
  return true;
}

void rotateBlock(Block &b) {
  vector<pair<int, int>> rotated;
  for (auto &[y, x] : b.shape)
    rotated.emplace_back(x, -y);

  for (auto &[y, x] : rotated) {
    int nx = b.x + x;
    int ny = b.y + y;
    if (nx < 0 || nx >= WIDTH || ny >= HEIGHT || grid[ny][nx])
      return;
  }
  b.shape = rotated;
}

void clearFullLines() {
  for (int y = HEIGHT - 1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < WIDTH; x++)
      if (!grid[y][x]) {
        full = false;
        break;
      }

    if (full) {
      score += 100;
      for (int ny = y; ny > 0; ny--)
        grid[ny] = grid[ny - 1];
      grid[0].assign(WIDTH, 0);
      y++;
    }
  }
}

void drawBlock(Block &b, float r, float g, float fb) {
  glColor3f(r, g, fb);
  for (auto &[dy, dx] : b.shape) {
    float x = b.x + dx;
    float y = b.y + dy;
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 1, y);
    glVertex2f(x + 1, y + 1);
    glVertex2f(x, y + 1);
    glEnd();
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  // Draw grid
  glColor3f(0.2f, 0.2f, 0.2f);
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (grid[y][x]) {
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + 1, y);
        glVertex2f(x + 1, y + 1);
        glVertex2f(x, y + 1);
        glEnd();
      }
    }
  }

  // Draw current block
  drawBlock(current, 0.0f, 1.0f, 1.0f);

  // Draw grid lines
  glColor3f(0.5f, 0.5f, 0.5f);
  glBegin(GL_LINES);
  for (int i = 0; i <= WIDTH; i++) {
    glVertex2f(i, 0);
    glVertex2f(i, HEIGHT);
  }
  for (int i = 0; i <= HEIGHT; i++) {
    glVertex2f(0, i);
    glVertex2f(WIDTH, i);
  }
  glEnd();

  // Draw score
  glColor3f(1, 1, 1);
  glRasterPos2f(1, HEIGHT + 1);
  string s = "Score: " + to_string(score);
  for (char c : s)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);

  if (game_over) {
    glColor3f(1, 0, 0);
    glRasterPos2f(WIDTH / 2 - 4, HEIGHT / 2);
    string msg = "GAME OVER";
    for (char c : msg)
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
  }

  glutSwapBuffers();
}

void specialKeys(int key, int, int) {
  if (game_over)
    return;

  switch (key) {
  case GLUT_KEY_LEFT:
    if (canMoveLeft(current))
      current.x--;
    break;
  case GLUT_KEY_RIGHT:
    if (canMoveRight(current))
      current.x++;
    break;
  case GLUT_KEY_DOWN:
    if (canMoveDown(current))
      current.y++;
    break;
  case GLUT_KEY_UP:
    rotateBlock(current);
    break;
  }
  glutPostRedisplay();
}

void update(int) {
  if (!game_over) {
    if (canMoveDown(current)) {
      current.y++;
    } else {
      // Lock block
      for (auto &[dy, dx] : current.shape) {
        int y = current.y + dy;
        int x = current.x + dx;
        if (y >= 0)
          grid[y][x] = 1;
      }

      clearFullLines();

      current = next_block;
      next_block = getRandomBlock();
      if (!canMoveDown(current))
        game_over = true;
    }
    glutPostRedisplay();
    glutTimerFunc(500, update, 0);
  }
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float aspect = (float)WIDTH / (HEIGHT + 2);
  if ((float)w / h > aspect) {
    int new_w = h * aspect;
    glViewport((w - new_w) / 2, 0, new_w, h);
  } else {
    int new_h = w / aspect;
    glViewport(0, (h - new_h) / 2, w, new_h);
  }
  glOrtho(0, WIDTH, HEIGHT + 2, 0, -1, 1);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(400, 800);
  glutCreateWindow("Tetris");
  srand(time(0));

  current = getRandomBlock();
  next_block = getRandomBlock();

  glClearColor(0, 0, 0, 1);
  glutDisplayFunc(display);
  glutSpecialFunc(specialKeys);
  glutTimerFunc(500, update, 0);
  glutReshapeFunc(reshape);

  glutMainLoop();
  return 0;
}
