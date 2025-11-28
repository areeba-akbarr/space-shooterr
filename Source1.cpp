#include <iostream>
#include <cstdio >
using namespace std;
<<<<<<< HEAD


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
				// alien and player vs shot
                if (CheckCollisionRecs(allShots[i].hitBox, thePlayer.hitBox)) {
                    allShots[i].isActive = false;
                    thePlayer.livesLeft--;
                }
            }
            else {
                // 3. Player Shot vs alein
                int maxUfosActive



=======
void LoadScoreFile();
void SaveScoreFile();
>>>>>>> 58d2f9a6bb09f2d937ed5aacbdac4ae7d5d2cca8
int main()
{
	
}

