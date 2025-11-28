#include <iostream>
using namespace std;


//function for alien grid setup

void SetupUfos(int rows, int cols) {
	10 currentUfosAlive = 0;
	11 for (int i = 0; i < MAX_UFOS; i++) {
		12 allUfos[i].isAlive = false;
		13
	}
	14// we will calculate how many aliens to aluve or spawn
		15 int maxUfosToSpawn = rows * cols;
	16 if (maxUfosToSpawn > MAX_UFOS) maxUfosToSpawn = MAX_UFOS;
	17
		18 for (int r = 0; r < rows; r++) {
		19 for (int c = 0; c < cols; c++) {
			20 int i = r * cols + c;
			21 if (i >= maxUfosToSpawn) break;
			22
				23 allUfos[i].isAlive = true;
			24 allUfos[i].fireTimer = static_cast <float>(rand() % 500) / 100.0f + 2.0f;
			25
				26 float gridWidth = static_cast <float>(cols * (UFO_W + 40) - 40); //setting up grid width
			27 float startX = (static_cast <float>(SCREEN_WIDTH) - gridWidth) / 2 +

				static_cast <float>(c * (UFO_W + 40));

			28 float startY = 50 + static_cast <float>(r * (UFO_H + 20));
			29
				30 allUfos[i].hitBox = { startX , startY , static_cast <float>(UFO_W), static_cast <

				float>(UFO_H) };
			31 currentUfosAlive++;
			32
		}
		33
	}
	34
}

int main()
{
	cout << "hello world ";
	return 0;
}