#include <iostream>
#include "raylib.h"     
#include <cstdlib>      
#include <time.h>       
#include <cmath>        
#include <cstdio>
using namespace std;

//game constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;
const int MAX_UFOS = 50;
const int MAX_SHOTS = 20;
const int MAX_SPARKS = 100;
const int NUM_WALLS = 4;

// dimensions for the images
const int SHIP_W = 80;
const int SHIP_H = 60;
const int UFO_W = 60;
const int UFO_H = 40;
const int SHOT_W = 16;
const int SHOT_H = 32;

// speeds and time
const float BASE_UFO_TIME = 0.05f;
const float UFO_X_SPEED = 1.0f;
const float UFO_Y_DROP = 20.0f;
const float SHIP_MOVE_SPEED = 8.0f;
const float SHIP_FIRE_DELAY = 0.2f;
const float TRIPLE_SHOT_DELAY = 1.5f;


//level transitions 
const float FADE_TIME = 0.5f;
const float HOLD_TIME = 1.5f;
const float TOTAL_TRANSITION_TIME = FADE_TIME * 2 + HOLD_TIME;

//structd

struct GamerShip {
    Rectangle hitBox;
    float speed = SHIP_MOVE_SPEED;
    int livesLeft = 3;
    int playerScore = 0;
    float fireCooldown = 0.0f;
    float tripleShotCooldown = 0.0f;
};

//enemy
struct Ufo {
    Rectangle hitBox;
    bool isAlive = false;
    float fireTimer = 0.0f;
    const float UFO_FIRE_RATE = 2.0f;
};

struct LaserShot {
    Rectangle hitBox;
    bool isActive = false;
    float speedVal = 0.0f;
    bool firedByUfo = false;
};

struct Spark {
    Vector2 pos;
    Vector2 velocity;
    float size;
    Color sparkColor;
    bool isVisible = false;
};

struct DefenseWall {
    Rectangle hitBox;
    int hitPoints = 4;
};


enum GameStatus {
    INTRO_MENU, HOW_TO_PLAY, IN_GAME, PAUSED_GAME, END_SCREEN, LEVEL_UP
};

GameStatus gameStatus = INTRO_MENU;
int currentLevel = 1;
float ufoMoveTimer = 0.0f;
float ufoMoveDirection = 1.0f;
float timeSinceLastUfoShot = 0.0f;
int highScore = 0;
int currentUfosAlive = 10;
int gridRows = 2;
int gridCols = 5;

// Transition state trackers
float levelTransitionTimer = 0.0f;
bool levelResetExecuted = false;


// the images we loaded
Texture2D shipTexture, ufoTexture, playerShotTexture, ufoShotTexture;

// Game Arrays
GamerShip thePlayer;
LaserShot allShots[MAX_SHOTS];
Ufo allUfos[MAX_UFOS];
DefenseWall allWalls[NUM_WALLS];
Spark allSparks[MAX_SPARKS];




void LoadScoreFile();
void SaveScoreFile();
// function for alien grid setup

void SetupUfos(int rows, int cols) {
    currentUfosAlive = 0;

    // reset all UFOs
    for (int i = 0; i < MAX_UFOS; i++) {
        allUfos[i].isAlive = false;
    }

    // calculate how many aliens to spawn
    int maxUfosToSpawn = rows * cols;
    if (maxUfosToSpawn > MAX_UFOS)
        maxUfosToSpawn = MAX_UFOS;

    // create a grid of UFOs
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {

            int i = r * cols + c;
            if (i >= maxUfosToSpawn)
                break;

            allUfos[i].isAlive = true;
            allUfos[i].fireTimer = static_cast<float>(rand() % 500) / 100.0f + 2.0f;

            float gridWidth = static_cast<float>(cols * (UFO_W + 40) - 40); // grid width
            float startX = (static_cast<float>(SCREEN_WIDTH) - gridWidth) / 2 +
                static_cast<float>(c * (UFO_W + 40));

            float startY = 50 + static_cast<float>(r * (UFO_H + 20));

            allUfos[i].hitBox = { startX, startY,
                                  static_cast<float>(UFO_W),
                                  static_cast<float>(UFO_H) };

            currentUfosAlive++;
        }
    }
}


// this is for controlooing the lien up down movement and shifts
void MoveUfos(float frameTime) {
    ufoMoveTimer += frameTime;
    if (ufoMoveTimer < BASE_UFO_TIME / static_cast<float>(currentLevel))
        return;

    ufoMoveTimer = 0.0f;

    bool hitWall = false;
    float currentSpeed = UFO_X_SPEED * (0.8f + static_cast<float>(currentLevel) * 0.2f);

    int maxUfosActive = gridRows * gridCols;
    if (maxUfosActive > MAX_UFOS)
        maxUfosActive = MAX_UFOS;

    for (int i = 0; i < maxUfosActive; i++) {
        if (allUfos[i].isAlive) {

            // Move left or right
            allUfos[i].hitBox.x += currentSpeed * ufoMoveDirection;

            // Check wall collision
            if (allUfos[i].hitBox.x <= 0 ||
                allUfos[i].hitBox.x >= static_cast<float>(SCREEN_WIDTH) - allUfos[i].hitBox.width) {
                hitWall = true;
            }

            // Check if aliens reached near bottom of screen, if yes end screen will show
            if (allUfos[i].hitBox.y + allUfos[i].hitBox.height >=
                static_cast<float>(SCREEN_HEIGHT) - 100) {
                gameStatus = END_SCREEN;
            }
        }
    }

    // If any aliens hits a wall then reverse direction and move down
    if (hitWall) {
        ufoMoveDirection *= -1.0f;

        for (int i = 0; i < maxUfosActive; i++) {
            if (allUfos[i].isAlive) {
                allUfos[i].hitBox.y += UFO_Y_DROP;
            }
        }
    }
}

// function to control when and how aliens should shoot
void UfoShooting(float frameTime) {
    timeSinceLastUfoShot += frameTime;

    const float BASE_UFO_FIRE_INTERVAL = 1.0f;
    float fireInterval = BASE_UFO_FIRE_INTERVAL / (0.5f + static_cast<float>(currentLevel) * 0.5f);

    if (timeSinceLastUfoShot >= fireInterval && currentUfosAlive > 0) {
        timeSinceLastUfoShot = 0.0f;

        int aliveIndices[MAX_UFOS];
        int count = 0;
        for (int i = 0; i < MAX_UFOS; i++) {
            if (allUfos[i].isAlive) {
                aliveIndices[count++] = i;
            }
        }

        if (count > 0) {
            int targetIndex = aliveIndices[rand() % count];
            FireShot(allUfos[targetIndex].hitBox, true, 0.0f);
        }
    }
}


// Updates the position of all active bullets and removes them if they go off-screen.
void MoveShots(float frameTime) {
    for (int i = 0; i < MAX_SHOTS; i++) {
        if (allShots[i].isActive) {
            if (allShots[i].firedByUfo) {
                allShots[i].hitBox.y += allShots[i].speedVal;
            }
            else {
                allShots[i].hitBox.y -= allShots[i].speedVal;
            }

            if (allShots[i].hitBox.y < -allShots[i].hitBox.height ||
                allShots[i].hitBox.y > static_cast<float>(SCREEN_HEIGHT)) {
                allShots[i].isActive = false;
            }
        }
    }
}

// Handles all collision detection between bullets, ships, and walls.
void CheckHits() {
    for (int i = 0; i < MAX_SHOTS; i++) {
        if (allShots[i].isActive) {

            // 1. Check against Defense Walls
            bool shotDestroyedByWall = false;
            for (int w = 0; w < NUM_WALLS; w++) {
                if (CheckCollisionRecs(allShots[i].hitBox, allWalls[w].hitBox)) {
                    allShots[i].isActive = false;
                    shotDestroyedByWall = true;
                    break;
                }
            }
            if (shotDestroyedByWall) continue;

            if (allShots[i].firedByUfo) {
                // 2. alien Shot vs the player shots
                if (CheckCollisionRecs(allShots[i].hitBox, thePlayer.hitBox)) {
                    allShots[i].isActive = false;
                    thePlayer.livesLeft--;
                }
            }
            else {
                // 3. Player Shot vs alien
                int maxUfosActive = gridRows * gridCols;
                if (maxUfosActive > MAX_UFOS) maxUfosActive = MAX_UFOS;

                for (int j = 0; j < maxUfosActive; j++) {
                    if (allUfos[j].isAlive &&
                        CheckCollisionRecs(allShots[i].hitBox, allUfos[j].hitBox)) {
                        allUfos[j].isAlive = false;
                        allShots[i].isActive = false;
                        thePlayer.playerScore += 100;
                        currentUfosAlive--;
                        break;
                    }
                }
            }
        }
    }
}
void SaveScoreFile() {
    FILE* file = fopen("top_score.txt", "w");
    if (file) {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
}
void LoadScoreFile() {
    FILE* file = fopen("top_score.txt", "r");
    if (file) {
        if (fscanf(file, "%d", &highScore) != 1) {
            highScore = 0;
        }
        fclose(file);
    }
}


