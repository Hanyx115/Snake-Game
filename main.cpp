#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

const int GRID_WIDTH = 40;
const int GRID_HEIGHT = 30;
const int CELL_SIZE = 20;
const int INITIAL_SNAKE_LENGTH = 3;
const int BASE_TIMER_DELAY = 100;

enum Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { RUNNING, GAME_OVER };

struct Point {
    int x, y;
};

std::vector<Point> snake;
std::vector<Point> walls;
Point apple;

Direction currentDirection = RIGHT;
GameState currentState = RUNNING;
int score = 0;
int level = 1;
int delay = BASE_TIMER_DELAY;
float colorShift = 0.0f;

void drawText(float x, float y, const char* text, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; ++i) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
    }
}

void drawCell(int x, int y, float r, float g, float b) {
    glColor3f(r, g, b);
    glRectf(x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE);
}

bool isOnSnake(int x, int y) {
    for (int i = 0; i < snake.size(); ++i)
        if (snake[i].x == x && snake[i].y == y)
            return true;
    return false;
}

bool isOnWall(int x, int y) {
    for (int i = 0; i < walls.size(); ++i)
        if (walls[i].x == x && walls[i].y == y)
            return true;
    return false;
}

void placeApple() {
    do {
        apple.x = rand() % GRID_WIDTH;
        apple.y = rand() % GRID_HEIGHT;
    } while (isOnSnake(apple.x, apple.y) || isOnWall(apple.x, apple.y));
}

void buildWalls() {
    walls.clear();
    if (level < 2) return;

    for (int i = 5; i < GRID_WIDTH - 5; i += 4) {
        Point p;
        p.x = i;
        p.y = 10 + (level % 4); // change wall pattern each level
        walls.push_back(p);
    }

    for (int j = 5; j < GRID_HEIGHT - 5; j += 4) {
        Point p;
        p.x = 10 + (level % 5);
        p.y = j;
        walls.push_back(p);
    }
}

void resetGame() {
    snake.clear();
    walls.clear();
    currentDirection = RIGHT;
    currentState = RUNNING;
    score = 0;
    level = 1;
    delay = BASE_TIMER_DELAY;

    for (int i = 0; i < INITIAL_SNAKE_LENGTH; ++i) {
        Point p = { GRID_WIDTH / 2 - i, GRID_HEIGHT / 2 };
        snake.push_back(p);
    }

    buildWalls();
    placeApple();
}

bool checkCollision(int x, int y) {
    if (x < 0 || y < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT)
        return true;
    for (int i = 1; i < snake.size(); ++i)
        if (snake[i].x == x && snake[i].y == y)
            return true;
    if (isOnWall(x, y))
        return true;
    return false;
}

void update(int) {
    if (currentState == RUNNING) {
        for (int i = snake.size() - 1; i > 0; --i)
            snake[i] = snake[i - 1];

        switch (currentDirection) {
            case UP:    snake[0].y++; break;
            case DOWN:  snake[0].y--; break;
            case LEFT:  snake[0].x--; break;
            case RIGHT: snake[0].x++; break;
        }

        if (checkCollision(snake[0].x, snake[0].y)) {
            currentState = GAME_OVER;
        }

        if (snake[0].x == apple.x && snake[0].y == apple.y) {
            snake.push_back(snake.back());
            score += 10;
            level = score / 50 + 1;
            delay = BASE_TIMER_DELAY - (level - 1) * 5;
            if (delay < 40) delay = 40;
            buildWalls();
            placeApple();
        }

        colorShift += 0.05f;
        if (colorShift > 2 * 3.14159f)
            colorShift = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(delay, update, 0);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw Apple
    drawCell(apple.x, apple.y, 1.0f, 0.0f, 0.0f);

    // Draw Walls
    for (int i = 0; i < walls.size(); ++i)
        drawCell(walls[i].x, walls[i].y, 0.5f, 0.5f, 0.5f);

    // Draw Snake
    for (int i = 0; i < snake.size(); ++i) {
        if (i == 0) {
            drawCell(snake[i].x, snake[i].y, 0.0f, 1.0f, 0.0f);
        } else {
            float r = 0.5f + 0.5f * sin(colorShift + i * 0.3f);
            float g = 0.5f + 0.5f * sin(colorShift + i * 0.5f + 2.0f);
            float b = 0.5f + 0.5f * sin(colorShift + i * 0.7f + 4.0f);
            drawCell(snake[i].x, snake[i].y, r, g, b);
        }
    }

    // Score + Level
    std::stringstream ss;
    ss << "Score: " << score << "    Level: " << level;
    drawText(10, GRID_HEIGHT * CELL_SIZE - 20, ss.str().c_str(), 1.0f, 1.0f, 1.0f);

    if (currentState == GAME_OVER) {
        drawText(GRID_WIDTH * CELL_SIZE / 2 - 60, GRID_HEIGHT * CELL_SIZE / 2, "GAME OVER", 1.0f, 0.0f, 0.0f);
        drawText(GRID_WIDTH * CELL_SIZE / 2 - 90, GRID_HEIGHT * CELL_SIZE / 2 - 25, "Press F1 to Restart", 1.0f, 1.0f, 1.0f);
    }

    glutSwapBuffers();
}

void handleInput(int key, int, int) {
    if (currentState == GAME_OVER && key == GLUT_KEY_F1) {
        resetGame();
        return;
    }

    if (currentState != RUNNING) return;

    switch (key) {
        case GLUT_KEY_UP:    if (currentDirection != DOWN)  currentDirection = UP;    break;
        case GLUT_KEY_DOWN:  if (currentDirection != UP)    currentDirection = DOWN;  break;
        case GLUT_KEY_LEFT:  if (currentDirection != RIGHT) currentDirection = LEFT;  break;
        case GLUT_KEY_RIGHT: if (currentDirection != LEFT)  currentDirection = RIGHT; break;
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, GRID_WIDTH * CELL_SIZE, 0, GRID_HEIGHT * CELL_SIZE);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    srand((unsigned int)time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE);
    glutCreateWindow("🔥 Snake Game - RGB + Levels + Walls 🔥");

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    resetGame();
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutSpecialFunc(handleInput);
    glutTimerFunc(delay, update, 0);

    glutMainLoop();
    return 0;
}
