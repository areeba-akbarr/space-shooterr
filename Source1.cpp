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


//functions used
void LoadScoreFile();
void SaveScoreFile();
int KeepInBounds(int value, int min, int max);
void LoadAllTextures();
void UnloadAllTextures();
void InitializeGame();
void SetupUfos(int rows, int cols);
void SetupWalls();
void SetupSparks();
void UpdateEverything(float frameTime);
void AdvanceLevel();
void CheckIfLevelWon();
void MoveShip(float frameTime);
void FireShot(Rectangle sourceBox, bool isUfo, float offsetX);
void FireTripleShot();
void MoveUfos(float frameTime);
void UfoShooting(float frameTime);
void MoveShots(float frameTime);
void CheckHits();
void UpdateSparks(float frameTime);
void DrawGameElements();
void DrawSparks();
void DrawTheMenu();
void DrawHowToPlay();
void DrawEndScreen();
void DrawPauseScreen();
void DrawLevelUpScreen();


int KeepInBounds(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

//loading images 
void LoadAllTextures() {
    shipTexture = LoadTexture("player_texture.png");
    ufoTexture = LoadTexture("enemy_texture.png");
    playerShotTexture = LoadTexture("player_bullet.png");
    ufoShotTexture = LoadTexture("enemy_bullet.png");
   
}

// cleans up the memory used by the images when the game closes.
void UnloadAllTextures() {
    UnloadTexture(shipTexture);
    UnloadTexture(ufoTexture);
    UnloadTexture(playerShotTexture);
    UnloadTexture(ufoShotTexture);
}


// main function
int main(void) {

    // 1. Setup
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Space Shooter - Survivors");
    SetTargetFPS(60); 
    srand(static_cast<unsigned int>(time(NULL)));

    // loading
    LoadScoreFile();
    LoadAllTextures();
    SetupSparks();
    InitializeGame();

    // 2.Game Loop
    while (!WindowShouldClose()) {
        float frameTime = GetFrameTime(); 

        UpdateSparks(frameTime); //background starry

        
        switch (gameStatus) {
        case INTRO_MENU:
            if (IsKeyPressed(KEY_ENTER)) {
                InitializeGame();
                gameStatus = IN_GAME;
            }
            else if (IsKeyPressed(KEY_I)) {
                gameStatus = HOW_TO_PLAY;
            }
            break;
        case HOW_TO_PLAY:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                gameStatus = INTRO_MENU;
            }
            break;
        case IN_GAME:
            UpdateEverything(frameTime);
            if (IsKeyPressed(KEY_P)) {
                gameStatus = PAUSED_GAME;
            }
            break;
        case PAUSED_GAME:
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) {
                gameStatus = IN_GAME;
            }
            break;
        case LEVEL_UP: 
            levelTransitionTimer += frameTime; 

            
            if (levelTransitionTimer >= FADE_TIME && !levelResetExecuted) {
                AdvanceLevel();
                levelResetExecuted = true;
            }

            
            if (levelTransitionTimer >= TOTAL_TRANSITION_TIME) {
                gameStatus = IN_GAME;
                levelTransitionTimer = 0.0f;
            }
            break;
        case END_SCREEN:
            
            if (thePlayer.playerScore > highScore) {
                highScore = thePlayer.playerScore;
                SaveScoreFile();
            }
            if (IsKeyPressed(KEY_ENTER)) {
                gameStatus = INTRO_MENU;
            }
            break;
        }

        // 3. drawing phase
        BeginDrawing();
        ClearBackground(BLACK); 

        DrawSparks(); //stars

        
        switch (gameStatus) {
        case INTRO_MENU:
            DrawTheMenu();
            break;
        case HOW_TO_PLAY:
            DrawHowToPlay();
            break;
        case IN_GAME:
        case PAUSED_GAME:
            DrawGameElements(); // ships, enemies, and UI.
            if (gameStatus == PAUSED_GAME)
                DrawPauseScreen();
            break;
        case END_SCREEN:
            DrawGameElements(); 
            DrawEndScreen();   
            break;
        case LEVEL_UP:
            DrawGameElements(); 

            float alpha = 0.0f; .

            if (levelTransitionTimer < FADE_TIME) {
                alpha = levelTransitionTimer / FADE_TIME;
            }
            else if (levelTransitionTimer < FADE_TIME + HOLD_TIME) {
                alpha = 1.0f;
            }
            else {
                alpha = 1.0f - (levelTransitionTimer - (FADE_TIME + HOLD_TIME)) / FADE_TIME;
            }

            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, alpha));

            if (alpha > 0.95f) {
                DrawLevelUpScreen();
            }
            break;
        }

        EndDrawing(); //display the frame
    }

    // 4. cleanup
    UnloadAllTextures();
    CloseWindow();
    return 0; // everything ran successfully
}

//game logic functions

//starry background
void SetupSparks() {
    for (int i = 0; i < MAX_SPARKS; i++) {
        allSparks[i].pos = (Vector2){ static_cast<float>(rand() % SCREEN_WIDTH), static_cast<float>(rand() % SCREEN_HEIGHT) };
        allSparks[i].velocity = (Vector2){ 0.0f, 0.1f + static_cast<float>(rand() % 10) / 10.0f };
        allSparks[i].size = static_cast<float>(rand() % 2) + 1.0f;
        allSparks[i].sparkColor = WHITE;
        allSparks[i].isVisible = true;
    }
}

// resets beofre every level
void InitializeGame() {
    currentLevel = 1;
    gridRows = 2;
    gridCols = 5;
    ufoMoveTimer = 0.0f;
    ufoMoveDirection = 1.0f;
    timeSinceLastUfoShot = 0.0f;
    levelTransitionTimer = 0.0f;
    levelResetExecuted = false;

    thePlayer.hitBox = { SCREEN_WIDTH / 2.0f - SHIP_W / 2.0f,
                       static_cast<float>(SCREEN_HEIGHT) - SHIP_H - 30,
                       static_cast<float>(SHIP_W), static_cast<float>(SHIP_H) };
    thePlayer.livesLeft = 3;
    thePlayer.playerScore = 0;
    thePlayer.fireCooldown = 0.0f;
    thePlayer.tripleShotCooldown = 0.0f;

    for (int i = 0; i < MAX_SHOTS; i++) {
        allShots[i].isActive = false;
    }

    SetupWalls(); 
    SetupUfos(gridRows, gridCols); 
}


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

// defence walls
void SetupWalls() {
    float wallWidth = 100;
    float wallHeight = 15;
    // the gap between walls
    float spacing = (static_cast<float>(SCREEN_WIDTH) - (NUM_WALLS * wallWidth)) / (NUM_WALLS + 1);
    float startY = static_cast<float>(SCREEN_HEIGHT) - 200 + (90.0f / 2.0f) - (wallHeight / 2.0f);
    for (int i = 0; i < NUM_WALLS; i++) {
        allWalls[i].hitPoints = 4; 
        allWalls[i].hitBox = {
            spacing + static_cast<float>(i * (wallWidth + spacing)),
            startY,
            wallWidth,
            wallHeight
        };
    }
}


void UpdateEverything(float frameTime) {
    MoveShip(frameTime);
    thePlayer.fireCooldown -= frameTime;
    thePlayer.tripleShotCooldown -= frameTime;

    // handle normal shooting input
    if ((IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && thePlayer.fireCooldown <= 0) {
        FireShot(thePlayer.hitBox, false, 0.0f);
        thePlayer.fireCooldown = SHIP_FIRE_DELAY; // reset timers
    }

    // handle special triple shot input
    if (IsKeyPressed(KEY_B) && thePlayer.tripleShotCooldown <= 0) {
        FireTripleShot();
    }

    UfoShooting(frameTime);
    MoveUfos(frameTime);
    MoveShots(frameTime);
    CheckHits(); //collisions checker
    CheckIfLevelWon();

    //game over condition checker
    if (thePlayer.livesLeft <= 0) {
        gameStatus = END_SCREEN;
    }
}

// checks if player defeated all enemies in current wave
void CheckIfLevelWon() {
    if (currentUfosAlive <= 0) {
        // We won! Start the fading transition.
        levelTransitionTimer = 0.0f;
        levelResetExecuted = false;
        gameStatus = LEVEL_UP;
    }
}

// prepares all game objects fornext level 
void AdvanceLevel() {

    // update score and level
    int previousLevel = currentLevel;
    currentLevel++; 
    thePlayer.playerScore += 500 * previousLevel; 

    // next wave harder
    gridRows = KeepInBounds(gridRows + 1, 1, 5);
    gridCols = KeepInBounds(gridCols + 1, 1, 10);

    // resets game state
    thePlayer.fireCooldown = 0.0f;
    thePlayer.tripleShotCooldown = 0.0f;
    for (int i = 0; i < MAX_SHOTS; i++) {
        allShots[i].isActive = false;
    }
    SetupWalls();
    SetupUfos(gridRows, gridCols); 

    // resets timing.
    ufoMoveTimer = 0.0f;
    ufoMoveDirection = 1.0f;
    timeSinceLastUfoShot = 0.0f;

    // extra life
    if (currentLevel % 3 == 0) {
        thePlayer.livesLeft++;
    }
}

//physics
// moves the player ship left or right based on input
void MoveShip(float frameTime) {
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        thePlayer.hitBox.x -= thePlayer.speed;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        thePlayer.hitBox.x += thePlayer.speed;
    }

    // clamp the ship's horizontal position
    thePlayer.hitBox.x = static_cast<float>(KeepInBounds(static_cast<int>(thePlayer.hitBox.x),
        0, SCREEN_WIDTH - static_cast<int>(thePlayer.hitBox.width)));
}

// finds empty slot launches a single bullet
void FireShot(Rectangle sourceBox, bool isUfo, float offsetX) {
    
    for (int i = 0; i < MAX_SHOTS; i++) {
        if (!allShots[i].isActive) {
            allShots[i].isActive = true;
            allShots[i].firedByUfo = isUfo;
            allShots[i].hitBox.width = SHOT_W;
            allShots[i].hitBox.height = SHOT_H;

            if (isUfo) {
                // enemy moving down and faster
                allShots[i].speedVal = 8.0f + static_cast<float>(currentLevel - 1) * 0.5f;
                allShots[i].hitBox.y = sourceBox.y + sourceBox.height + 5;
            }
            else {
                //players shots are moving up and faster
                allShots[i].speedVal = 15.0f;
                allShots[i].hitBox.y = sourceBox.y - allShots[i].hitBox.height;
            }

            allShots[i].hitBox.x = sourceBox.x + sourceBox.width / 2 - allShots[i].hitBox.width / 2 + offsetX;
            break;
        }
    }
}


void FireTripleShot() {
    thePlayer.tripleShotCooldown = TRIPLE_SHOT_DELAY;
    FireShot(thePlayer.hitBox, false, -20.0f); //left
    FireShot(thePlayer.hitBox, false, 0.0f);   //centre
    FireShot(thePlayer.hitBox, false, 20.0f);  //right
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


