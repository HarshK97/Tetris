#include <GL/freeglut_std.h>
#include <iostream>

#include <GL/gl.h>
#include <GL/glut.h>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <float.h>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using namespace std;

// Genetic Algorithm Parameters
const int POPULATION_SIZE = 50;
const int GENERATIONS = 100;
const double MUTATION_RATE = 0.1;

// Heuristic weights: aggregate height, completed lines, holes and bumpiness
struct Genome {
  vector<double> weights;
  double fitness;
};

// Global Variable for genetic algorithm
vector<Genome> population;
int current_genome = 0;
int generation = 0;
bool ai_enabled = true;

// Tetris Game Logic
class Block {
public:
  vector<pair<int, int>> shape;
  int x, y;

  Block() : x(4), y(0) {}
};

void drawQuad(int x, int y, float r, float g, float b) {
  glColor3f(r, g, b);
  glBegin(GL_QUADS);
  glVertex2f(x, y);
  glVertex2f(x + 1, y);
  glVertex2f(x + 1, y + 1);
  glVertex2f(x, y + 1);
  glEnd();
}
void drawText(float x, float y, const string &text) {
  glRasterPos2f(x, y);
  for (char c : text) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }
}
void drawNextBlockPreview(const Block &b, float offsetX, float offsetY) {
  glColor3f(0.3f, 0.7f, 0.3f);
  for (auto &[x, y] : b.shape) {
    drawQuad(offsetX + x, offsetY + y, 0.3f, 0.7f, 0.3f);
  }
}

Block currentBlock;
Block nextBlock;
int score = 0;
bool game_over = false;

class Game {
public:
  static const int HEIGHT = 20;
  static const int WIDTH = 10;
  static vector<vector<int>> grid;

  static void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw grid blocks
    glColor3f(0.2f, 0.2f, 0.2f);
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        if (grid[y][x]) {
          drawQuad(x, y, 0.2f, 0.2f, 0.2f);
        }
      }
    }

    for (auto &[y, x] : currentBlock.shape) {
      drawQuad(currentBlock.x + x, currentBlock.y + y, 0.8f, 0.1f, 0.1f);
    }

    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(Game::WIDTH, 0);
    glVertex2f(Game::WIDTH + 6, 0);
    glVertex2f(Game::WIDTH + 6, Game::HEIGHT);
    glVertex2f(Game::WIDTH, Game::HEIGHT);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); // White text
    if (game_over) {
      glColor3f(1.0f, 0.0f, 0.0f); // Red color
      drawText(Game::WIDTH + 0.5, Game::HEIGHT - 4, "GAME OVER");
      drawText(Game::WIDTH + 0.5, Game::HEIGHT - 6, "Press R to restart");
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    // drawText(Game::WIDTH + 0.5, Game::HEIGHT - 8,
    //          "AI: " + string(ai_enabled ? "ON" : "OFF"));
    drawText(Game::WIDTH + 0.5, Game::HEIGHT - 10,
             "Gen: " + to_string(generation));
    drawText(Game::WIDTH + 0.5, Game::HEIGHT - 11,
             "Player: " + to_string(current_genome));

    drawText(Game::WIDTH + 0.5, Game::HEIGHT - 2, "Score: " + to_string(score));
    drawText(Game::WIDTH + 0.5, 2, "Next:");
    drawNextBlockPreview(nextBlock, Game::WIDTH + 1.5, 3);
    glutSwapBuffers();
  }
};

vector<vector<int>> Game::grid(Game::HEIGHT, vector<int>(Game::WIDTH));

// Heuristic function to evaulate board static
double evaluateBoard(const vector<vector<int>> &grid, const Genome &g) {
  int aggregateHeight = 0;
  int completedLines = 0;
  int holes = 0;
  int bumpiness = 0;

  // Calculate aggregate height amd holes
  vector<int> heights(Game::WIDTH, 0);
  for (int x = 0; x < Game::WIDTH; x++) {
    for (int y = 0; y < Game::HEIGHT; y++) {
      if (grid[y][x]) {
        if (heights[x] == 0) {
          heights[x] = Game::HEIGHT - y;
        }
      } else if (heights[x] > 0) {
        holes++;
      }
    }
  }

  // Caluclate bumpiness
  for (int x = 0; x < Game::WIDTH - 1; x++) {
    bumpiness += abs(heights[x] - heights[x + 1]);
  }

  // Calculate complete lines
  for (int y = 0; y < Game::HEIGHT; y++) {
    bool full = true;
    for (int x = 0; x < Game::WIDTH; x++) {
      if (grid[y][x]) {
        full = false;
        break;
      }
    }
    if (full) {
      completedLines++;
    }
  }

  return g.weights[0] * aggregateHeight + g.weights[1] * completedLines +
         g.weights[2] * holes + g.weights[3] * bumpiness;
}

// Genetic algorithm baby
void initializePopulation() {
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<> dis(-1, 1);

  for (int i = 0; i < POPULATION_SIZE; i++) {
    Genome genome;
    genome.weights = {dis(gen), dis(gen), dis(gen), dis(gen)};
    genome.fitness = 0;
    population.push_back(genome);
  }
}

Block getRandomBlock();
void rotateBlock(Block &b, const vector<vector<int>> &grid);
bool isColliding(const Block &block);
int clearFullLines();

void evaluatePopulation() {
  for (auto &genome : population) {
    // Initalize a fresh game state
    vector<vector<int>> simulatedGrid = Game::grid;
    Block simulatedBlock = getRandomBlock();
    int simulatedScore = 0;

    // Simulating the game
    while (true) {
      double bestFitness = -1e9;
      Block bestBlock = simulatedBlock;

      // Trying all rotations and positions
      for (int rotation = 0; rotation < 4; rotation++) {
        Block testBlock = simulatedBlock;

        // Apply rotation
        rotateBlock(testBlock, simulatedGrid);

        // Test all vaild horizontal positions
        for (int x = -Game::WIDTH; x <= Game::WIDTH; x++) {
          testBlock.x = simulatedBlock.x + x;

          // Check if the block placement is vaild
          if (!isColliding(testBlock)) {
            double fitness = evaluateBoard(simulatedGrid, genome);
            if (fitness > bestFitness) {
              bestFitness = fitness;
              bestBlock = testBlock;
            }
          }
        }
      }

      // Place the best block on the grid
      for (auto &[dy, dx] : bestBlock.shape) {
        simulatedGrid[bestBlock.y + dy][bestBlock.x + dx] = 1;
      }

      simulatedScore += clearFullLines();

      // Get next block
      simulatedBlock = getRandomBlock();

      // Check for game over
      if (isColliding(simulatedBlock)) {
        break;
      }
    }

    genome.fitness = simulatedScore;
  }
}

void saveBestChromosomeToFile(const Genome &bestGenome, int generation) {
  std::ofstream file("best_chromosomes.txt",
                     std::ios::app); // Open file in append mode
  if (file.is_open()) {
    file << "Generation: " << generation << "\n";
    file << "Fitness: " << bestGenome.fitness << "\n";
    file << "Weights: ";
    for (double weight : bestGenome.weights) {
      file << weight << " ";
    }
    file << "\n\n"; // Add a blank line for readability
    file.close();   // Close the file
  } else {
    std::cerr << "Error: Unable to open file for writing.\n";
  }
}

void selectNewPopulation() {
  // Sort population by fitness
  sort(population.begin(), population.end(),
       [](const Genome &a, const Genome &b) { return a.fitness > b.fitness; });

  // Save the best chromosome of this generation to a file
  saveBestChromosomeToFile(population[0], generation);

  // Top Half population and make have babies lol
  vector<Genome> newPopulation;
  for (int i = 0; i < POPULATION_SIZE / 2; i++) {
    newPopulation.push_back(population[i]);
  }

  // Breeding them and mutating their babies
  random_device rd;
  mt19937 gen(rd());
  uniform_real_distribution<> dis(0, 1);

  for (int i = 0; i < POPULATION_SIZE / 2; i++) {
    Genome father = population[i];
    Genome mother = population[(i + 1) % (POPULATION_SIZE / 2)];

    Genome child;
    for (size_t j = 0; j < father.weights.size(); j++) {
      if (dis(gen) < 0.5) {
        child.weights.push_back(father.weights[j]);
      } else {
        child.weights.push_back(mother.weights[j]);
      }

      // Mutating the babies
      if (dis(gen) < MUTATION_RATE) {
        child.weights[j] += dis(gen) * 0.1;
      }
    }

    newPopulation.push_back(child);
  }

  population = newPopulation;
  generation++;
}

void performAiMove() {
  if (game_over || population.empty()) {
    cout << "Cannot perform AI move: "
         << (game_over ? "game over" : "population empty") << endl;
    return;
  }

  double bestFitness = -1e9;
  int bestRotation = 0;
  int bestX = currentBlock.x;

  // Store original block position
  Block originalBlock = currentBlock;

  // Try all rotations
  for (int rotation = 0; rotation < 4; rotation++) {
    // Reset block to original position for each rotation
    Block rotatedBlock = originalBlock;

    // Apply the rotation
    for (int r = 0; r < rotation; r++) {
      rotateBlock(rotatedBlock, Game::grid);
    }

    // Try all horizontal positions
    for (int x = 0; x < Game::WIDTH; x++) {
      // Create a new block for testing
      Block testBlock = rotatedBlock;
      testBlock.x = x; // Set the horizontal position

      // Check if this position is valid
      bool validPosition = true;
      for (auto &[dy, dx] : testBlock.shape) {
        int testX = testBlock.x + dx;
        int testY = testBlock.y + dy;

        if (testX < 0 || testX >= Game::WIDTH || testY < 0 ||
            testY >= Game::HEIGHT || (testY >= 0 && Game::grid[testY][testX])) {
          validPosition = false;
          break;
        }
      }

      if (!validPosition) {
        continue; // Skip invalid positions
      }

      // Drop the block to the bottom
      Block droppedBlock = testBlock;
      while (true) {
        droppedBlock.y++;

        // Check if the new position collides
        bool collides = false;
        for (auto &[dy, dx] : droppedBlock.shape) {
          int testX = droppedBlock.x + dx;
          int testY = droppedBlock.y + dy;

          if (testY >= Game::HEIGHT || Game::grid[testY][testX]) {
            collides = true;
            break;
          }
        }

        if (collides) {
          // Move back up to the last valid position
          droppedBlock.y--;
          break;
        }
      }

      // Create a simulated grid with the dropped block
      vector<vector<int>> simulatedGrid = Game::grid;
      for (auto &[dy, dx] : droppedBlock.shape) {
        int placeX = droppedBlock.x + dx;
        int placeY = droppedBlock.y + dy;

        if (placeY >= 0 && placeY < Game::HEIGHT) {
          simulatedGrid[placeY][placeX] = 1;
        }
      }

      // Evaluate this move
      double fitness = evaluateBoard(simulatedGrid, population[current_genome]);

      // Update best move if this is better
      if (fitness > bestFitness) {
        bestFitness = fitness;
        bestRotation = rotation;
        bestX = x;
      }
    }
  }

  // Reset to original block
  currentBlock = originalBlock;

  // Apply the best rotation
  for (int r = 0; r < bestRotation; r++) {
    rotateBlock(currentBlock, Game::grid);
  }

  // Move to the best X position
  currentBlock.x = bestX;

  // Drop the block to the bottom
  while (true) {
    currentBlock.y++;

    if (isColliding(currentBlock)) {
      currentBlock.y--;
      break;
    }
  }

  // Place the block on the grid
  for (auto &[dy, dx] : currentBlock.shape) {
    int placeX = currentBlock.x + dx;
    int placeY = currentBlock.y + dy;

    if (placeY >= 0 && placeY < Game::HEIGHT && placeX >= 0 &&
        placeX < Game::WIDTH) {
      Game::grid[placeY][placeX] = 1;
    }
  }

  // Clear lines and update score
  clearFullLines();

  // Get next block
  currentBlock = nextBlock;
  nextBlock = getRandomBlock();

  // Check for game over
  if (isColliding(currentBlock)) {
    game_over = true;
  }
}

bool canMoveLeft(Block &b);
bool canMoveRight(Block &b);
bool canMoveDown(Block &b);

void toggleAi() {
  ai_enabled = !ai_enabled;
  cout << "AI is now " << (ai_enabled ? "enabled" : "disabled") << endl;
}

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
  b.shape = shapes[rand() % shapes.size()];
  return b;
}

void rotateBlock(Block &b, const vector<vector<int>> &grid) {
  vector<pair<int, int>> rotated;
  for (auto &[x, y] : b.shape)
    rotated.emplace_back(-y, x);

  // Wall kick tests
  vector<pair<int, int>> kicks = {{0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  for (auto &[dx, dy] : kicks) {
    bool valid = true;
    for (auto &[x, y] : rotated) {
      int nx = b.x + x + dx;
      int ny = b.y + y + dy;
      if (nx < 0 || nx >= Game::WIDTH || ny < 0 || ny >= Game::HEIGHT ||
          grid[ny][nx]) {
        valid = false;
        break;
      }
    }
    if (valid) {
      b.x += dx;
      b.y += dy;
      b.shape = rotated;
      return;
    }
  }
}

bool canMoveLeft(Block &b) {
  for (auto [dy, dx] : b.shape) {
    int x = b.x + dx - 1;
    int y = b.y + dy;
    if (x < 0 || y >= Game::HEIGHT || Game::grid[y][x])
      return false;
  }
  return true;
}
bool canMoveRight(Block &b) {
  for (auto [dy, dx] : b.shape) {
    int x = b.x + dx + 1;
    int y = b.y + dy;
    if (x >= Game::WIDTH || y >= Game::HEIGHT || Game::grid[y][x])
      return false;
  }
  return true;
}
bool canMoveDown(Block &b) {
  for (auto [dy, dx] : b.shape) {
    int x = b.x + dx;
    int y = b.y + dy + 1;
    if (y >= Game::HEIGHT || Game::grid[y][x])
      return false;
  }
  return true;
}

int clearFullLines() {
  int lines_cleared = 0;
  for (int y = Game::HEIGHT - 1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < Game::WIDTH; x++) {
      if (!Game::grid[y][x]) {
        full = false;
        break;
      }
    }
    if (full) {
      lines_cleared++;
      score += 100;
      for (int ny = y; ny > 0; ny--) {
        Game::grid[ny] = Game::grid[ny - 1];
      }
      Game::grid[0].assign(Game::WIDTH, 0);
      y++;
    }
  }
  return lines_cleared;
}
void specialKeys(int key, int, int) {
  if (game_over)
    return;
  switch (key) {
  case GLUT_KEY_LEFT:
    if (canMoveLeft(currentBlock))
      currentBlock.x--;
    break;
  case GLUT_KEY_RIGHT:
    if (canMoveRight(currentBlock))
      currentBlock.x++;
    break;
  case GLUT_KEY_DOWN:
    if (canMoveDown(currentBlock))
      currentBlock.y++;
    break;
  case GLUT_KEY_UP:
    rotateBlock(currentBlock, Game::grid);
    break;
  }
  glutPostRedisplay();
}

bool isColliding(const Block &block) {
  for (auto &[dy, dx] : block.shape) {
    int x = block.x + dx;
    int y = block.y + dy;

    // Check boundaries
    if (x < 0 || x >= Game::WIDTH || y >= Game::HEIGHT)
      return true;

    // Check collision with existing blocks
    if (y >= 0 && Game::grid[y][x])
      return true;
  }
  return false;
}

void advanceToNextGenome();

void update(int) {
  if (game_over) {
    advanceToNextGenome();
    glutTimerFunc(50, update, 0);
    return;
  }

  if (ai_enabled) {
    performAiMove();
  }

  else if (canMoveDown(currentBlock)) {
    currentBlock.y++;
  } else {
    bool placementSuccess = true;

    // Place block on grid with safety checks
    for (auto &[dy, dx] : currentBlock.shape) {
      int y = currentBlock.y + dy;
      int x = currentBlock.x + dx;

      if (y < 0) {
        // Block is above the grid - game over
        game_over = true;
        placementSuccess = false;
        break;
      } else if (y >= Game::HEIGHT || x < 0 || x >= Game::WIDTH) {
        // Out of bounds - should never happen with proper collision detection
        placementSuccess = false;
        break;
      } else {
        Game::grid[y][x] = 1;
      }
    }

    int lines = clearFullLines();

    if (placementSuccess) {
      clearFullLines();

      currentBlock = nextBlock;
      nextBlock = getRandomBlock();

      // Check if new block can be placed
      if (isColliding(currentBlock)) {
        game_over = true;
      }
    }

    if (game_over) {
      glutPostRedisplay();
      return;
    }
  }

  glutPostRedisplay();
  glutTimerFunc(500, update, 0);
}

void keyboard(unsigned char key, int, int) {
  if (game_over && (key == 'r' || key == 'R')) {
    game_over = false;
    Game::grid = vector<vector<int>>(Game::HEIGHT, vector<int>(Game::WIDTH));
    score = 0;
    currentBlock = getRandomBlock();
    nextBlock = getRandomBlock();
    glutPostRedisplay();
    glutTimerFunc(500, update, 0);
  } else if (key == 'a' || key == 'A') {
    toggleAi(); // Toggle AI on/off when 'A' is pressed
  }
}

void restartGame() {
  // Reset game state
  game_over = false;
  Game::grid = vector<vector<int>>(Game::HEIGHT, vector<int>(Game::WIDTH));
  score = 0;
  currentBlock = getRandomBlock();
  nextBlock = getRandomBlock();
}

void advanceToNextGenome() {
  // Save current genome's fitness (score)
  if (current_genome < population.size()) {
    population[current_genome].fitness = score;
    cout << "Genome " << current_genome << " scored: " << score << endl;
  }

  // Move to next genome
  current_genome++;

  // If all genomes evaluated, evolve to next generation
  if (current_genome >= population.size()) {
    cout << "Generation " << generation << " complete. Evolving..." << endl;
    selectNewPopulation(); // This handles the genetic algorithm logic
    current_genome = 0;    // Reset to first genome of new generation

    // Optional: Save or display generation stats
    cout << "Starting Generation " << generation << endl;
  }

  // Display which genome is playing now
  cout << "Now playing: Genome " << current_genome << " of Generation "
       << generation << endl;

  // Restart game for the new genome
  restartGame();
}

int main(int argc, char **argv) {
  srand(time(0));
  currentBlock = getRandomBlock();
  nextBlock = getRandomBlock();

  initializePopulation();

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 1000);
  glutCreateWindow("Tetris");

  glClearColor(0, 0, 0, 1);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, Game::WIDTH + 6, Game::HEIGHT, 0);

  glutDisplayFunc(Game::display);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(500, update, 0);
  glutSpecialFunc(specialKeys);
  glutMainLoop();

  return 0;
}
