#include <iostream>
#include "raylib.h"     // used to include graphics
#include <cstdlib>      // mostly to use rand function
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
    Rectangle hitBox; //location of ship and its size 
    float speed = SHIP_MOVE_SPEED; //current player speed 
    int livesLeft = 3;
    int playerScore = 0;
    float fireCooldown = 0.0f;
    float tripleShotCooldown = 0.0f;
};

//enemy
struct Ufo {
    Rectangle hitBox; //enemy location and size 
    bool isAlive = false;  //enemy slot active?
    float fireTimer = 0.0f;
    const float UFO_FIRE_RATE = 2.0f;
};

struct LaserShot {
    Rectangle hitBox; //bullet location and size 
    bool isActive = false;
    float speedVal = 0.0f;
    bool firedByUfo = false;
};

struct Spark {
    Vector2 pos;//star position in space 
    Vector2 velocity; //star speed and direction 
    float size;
    Color sparkColor;
    bool isVisible = false;
};

struct DefenseWall {
    Rectangle hitBox; //wall location and size 
    int hitPoints = 4;
};

//to switch between where player is in the game 
enum GameStatus {
    INTRO_MENU, HOW_TO_PLAY, IN_GAME, PAUSED_GAME, END_SCREEN, LEVEL_UP
};
//initialising global variables 
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
//saves the current highscore 
void SaveScoreFile() {
    FILE* file = fopen("top_score.txt", "w");
    if (file) {
        fprintf(file, "%d", highScore); //write the no
        fclose(file);
    }
}
//loads highhscore from text file 
void LoadScoreFile() {
    FILE* file = fopen("top_score.txt", "r");
    if (file) {
        //reads one integer from file into the highscore variable 
        if (fscanf(file, "%d", &highScore) != 1) {
            highScore = 0;
        }
        fclose(file);
    }
}

//keep val btw max and min
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
    SetTargetFPS(60); // 60 frames per sec
    srand(static_cast<unsigned int>(time(NULL))); //initializing randomizer

    // loading
    LoadScoreFile();
    LoadAllTextures();
    SetupSparks();
    InitializeGame();

    // 2.Game Loop runs till user closes window
    while (!WindowShouldClose()) {
        float frameTime = GetFrameTime(); //time passed since last screen update

        UpdateSparks(frameTime); //background starry

        //depending on where we are menu,game etc corresponding action takes place
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
        case LEVEL_UP: //for smooth transition between levels
            levelTransitionTimer += frameTime; //increment the timer

            
            if (levelTransitionTimer >= FADE_TIME && !levelResetExecuted) {
                AdvanceLevel();
                levelResetExecuted = true; //set flag so this only runs 
            }

            //transition time up then resume the game 
            if (levelTransitionTimer >= TOTAL_TRANSITION_TIME) {
                gameStatus = IN_GAME;
                levelTransitionTimer = 0.0f;
            }
            break;
        case END_SCREEN:
            //check if current score is the new highscore
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

        //draw content depending on the current state
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
            DrawEndScreen();   //draw the "game over" display
            break;
        case LEVEL_UP:
            DrawGameElements(); //keep old game state visible during the fade out

            float alpha = 0.0f; //opacity of black screen (0 = transparent, 1 = solid)

            if (levelTransitionTimer < FADE_TIME) {
                //screen turns solid
                alpha = levelTransitionTimer / FADE_TIME;
            }
            else if (levelTransitionTimer < FADE_TIME + HOLD_TIME) {
                //screen is solid
                alpha = 1.0f;
            }
            else {
                //screen turns transparent 
                alpha = 1.0f - (levelTransitionTimer - (FADE_TIME + HOLD_TIME)) / FADE_TIME;
            }
            //draw the fading black screen over everything else 
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, alpha));
            //only draw the text when screen is dark enough to read
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
        //give the star a random spot on the screen 
        allSparks[i].pos = (Vector2){ static_cast<float>(rand() % SCREEN_WIDTH), static_cast<float>(rand() % SCREEN_HEIGHT) };
        //slowly move star down the screen 
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
    //reset transition variables 
    levelTransitionTimer = 0.0f;
    levelResetExecuted = false;
    //put the player ship in its starting position 
    thePlayer.hitBox = { SCREEN_WIDTH / 2.0f - SHIP_W / 2.0f,
                       static_cast<float>(SCREEN_HEIGHT) - SHIP_H - 30,
                       static_cast<float>(SHIP_W), static_cast<float>(SHIP_H) };
    thePlayer.livesLeft = 3;
    thePlayer.playerScore = 0;
    thePlayer.fireCooldown = 0.0f;
    thePlayer.tripleShotCooldown = 0.0f;
    //clear all existing bullets 
    for (int i = 0; i < MAX_SHOTS; i++) {
        allShots[i].isActive = false;
    }

    SetupWalls(); //place the defense barriers 
    SetupUfos(gridRows, gridCols); //place enemies 
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

            currentUfosAlive++; //count the new enemies 
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
        // we won! Start the fading transition.
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
    //looks for first inactive shot slot 
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
            //centre the shot with optional offset for triple shot spread
            allShots[i].hitBox.x = sourceBox.x + sourceBox.width / 2 - allShots[i].hitBox.width / 2 + offsetX;
            break;
        }
    }
}

//fire three player shots 
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
    //calculates speed which increases the level
    float currentSpeed = UFO_X_SPEED * (0.8f + static_cast<float>(currentLevel) * 0.2f);

    int maxUfosActive = gridRows * gridCols;
    if (maxUfosActive > MAX_UFOS)
        maxUfosActive = MAX_UFOS;
    //move all active enemies 
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




// drawing

// moving background
void DrawSparks() {
    for (int i = 0; i < MAX_SPARKS; i++) {
        if (allSparks[i].isVisible) {
            DrawCircleV(allSparks[i].pos, allSparks[i].size, allSparks[i].sparkColor);
        }
    }
}

// drawing main objects / elements
void DrawGameElements() {
    int maxUfosActive = gridRows * gridCols;
    if (maxUfosActive > MAX_UFOS) maxUfosActive = MAX_UFOS;

    // draw ufos if alive
    for (int i = 0; i < maxUfosActive; i++) {
        if (allUfos[i].isAlive) {
            
            DrawTexturePro(ufoTexture,
                (Rectangle) {
                0.0f, 0.0f, static_cast<float>(ufoTexture.width), static_cast<float>(ufoTexture.height)
            },
                allUfos[i].hitBox,
                (Vector2) {
                0, 0
            }, 0.0f, WHITE);
        }
    }

    //draw ship
    DrawTexturePro(shipTexture,
        (Rectangle) {
        0.0f, 0.0f, static_cast<float>(shipTexture.width), static_cast<float>(shipTexture.height)
    },
        thePlayer.hitBox,
        (Vector2) {
        0, 0
    }, 0.0f, WHITE);

    // draw shots
    for (int i = 0; i < MAX_SHOTS; i++) {
        if (allShots[i].isActive) {
            Texture2D shotTex = allShots[i].firedByUfo ? ufoShotTexture : playerShotTexture;
            DrawTexturePro(shotTex,
                (Rectangle) {
                0.0f, 0.0f, static_cast<float>(shotTex.width), static_cast<float>(shotTex.height)
            },
                allShots[i].hitBox,
                (Vector2) {
                0, 0
            }, 0.0f, WHITE);
        }
    }

    // draw walls
    for (int i = 0; i < NUM_WALLS; i++) {
        DrawRectangleRec(allWalls[i].hitBox, DARKGRAY);
        DrawRectangleLinesEx(allWalls[i].hitBox, 2, WHITE);
    }

    // draw extra things ui
    DrawText(TextFormat("SCORE: %06i", thePlayer.playerScore), 10, 10, 20, WHITE);
    DrawText(TextFormat("LEVEL: %i", currentLevel), SCREEN_WIDTH / 2 - 50, 10, 20, WHITE);

    const int LIVES_TEXT_SIZE = 20;
    const char* livesText = TextFormat("LIVES: %i", thePlayer.livesLeft);
    int livesTextWidth = MeasureText(livesText, LIVES_TEXT_SIZE);
    DrawText(livesText, SCREEN_WIDTH - livesTextWidth - 10, 10, LIVES_TEXT_SIZE, WHITE);

    // triple shot timer
    Color cdColor = thePlayer.tripleShotCooldown <= 0.0f ? LIME : RED;
    DrawText(TextFormat("TRIPLE SHOT CD: %.1f", thePlayer.tripleShotCooldown > 0.0f ? thePlayer.tripleShotCooldown : 0.0f), 10, 40, 20, cdColor);
}

//main title
void DrawTheMenu() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(DARKBLUE, 0.8f));
    DrawText("SPACE SHOOTER: VIRUS DEFENDER", SCREEN_WIDTH / 2 - MeasureText("SPACE SHOOTER: VIRUS DEFENDER", 60) / 2, 100, 60, WHITE);

    DrawText(TextFormat("HIGH SCORE: %06i", highScore), SCREEN_WIDTH / 2 - MeasureText("HIGH SCORE: 000000", 30) / 2, 250, 30, GOLD);

    DrawText("Press ENTER to START", SCREEN_WIDTH / 2 - MeasureText("Press ENTER to START", 30) / 2, 350, 30, GREEN);
    DrawText("Press I for INSTRUCTIONS", SCREEN_WIDTH / 2 - MeasureText("Press I for INSTRUCTIONS", 30) / 2, 400, 30, SKYBLUE);
}

//instructions
void DrawHowToPlay() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(DARKBLUE, 0.9f));
    DrawText("INSTRUCTIONS", SCREEN_WIDTH / 2 - MeasureText("INSTRUCTIONS", 50) / 2, 50, 50, WHITE);
    DrawText("Use LEFT/RIGHT (or A/D) to move your ship.", 50, 150, 30, LIGHTGRAY);
    DrawText("Press SPACE (or Left Click) to fire bullets.", 50, 200, 30, LIGHTGRAY);
    DrawText("Press B for a powerful TRIPLE SHOT.", 50, 250, 30, YELLOW);
    DrawText("Destroy all viruses to advance to the next level.", 50, 300, 30, LIGHTGRAY);
    DrawText("The permanent BARRIERS STOP ALL BULLETS and protect the player.", 50, 350, 30, LIGHTGRAY);
    DrawText("You get an extra life every 3 levels.", 50, 400, 30, LIGHTGRAY);
    DrawText("Press ESCAPE to return to Menu", 50, SCREEN_HEIGHT - 50, 30, RED);
}

//gameover
void DrawEndScreen() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(RED, 0.9f));
    DrawText("GAME OVER!", SCREEN_WIDTH / 2 - MeasureText("GAME OVER!", 80) / 2, 200, 80, WHITE);
    DrawText(TextFormat("FINAL SCORE: %06i", thePlayer.playerScore), SCREEN_WIDTH / 2 - MeasureText("FINAL SCORE: 000000", 30) / 2, 360, 30, LIME);
    DrawText(TextFormat("HIGH SCORE: %06i", highScore), SCREEN_WIDTH / 2 - MeasureText("HIGH SCORE: 000000", 30) / 2, 410, 30, GOLD);
    DrawText("Press ENTER to return to Menu", SCREEN_WIDTH / 2 - MeasureText("Press ENTER to return to Menu", 30) / 2, 550, 30, YELLOW);
}

//puase
void DrawPauseScreen() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ColorAlpha(BLACK, 0.5f));
    DrawText("PAUSED", SCREEN_WIDTH / 2 - MeasureText("PAUSED", 80) / 2, 200, 80, WHITE);
    DrawText("Press P or ENTER to CONTINUE", SCREEN_WIDTH / 2 - MeasureText("Press P or ENTER to CONTINUE", 30) / 2, 350, 30, GREEN);
}

//transition state between two states
void DrawLevelUpScreen() {
    DrawText(TextFormat("LEVEL %i CLEARED!", currentLevel - 1), SCREEN_WIDTH / 2 - MeasureText("LEVEL X CLEARED!", 60) / 2, 200, 60, WHITE);
    DrawText(TextFormat("SCORE: %06i", thePlayer.playerScore), SCREEN_WIDTH / 2 - MeasureText("SCORE: 000000", 40) / 2, 350, 40, GOLD);
    DrawText(TextFormat("GET READY FOR LEVEL %i", currentLevel), SCREEN_WIDTH / 2 - MeasureText("GET READY FOR LEVEL XX", 30) / 2, 450, 30, LIME);
}




