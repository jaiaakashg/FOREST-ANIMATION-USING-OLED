#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Water animation variables
const int waterHeight = 16;
const int waterSpeed = 2;
int waterOffset = 0;

// Tree animation variables
const int tree1X = 30, tree2X = 90;
const int treeY = 42;
float treeSway1 = 0, treeSway2 = 0;
float tree1Angle = 0, tree2Angle = 0;
unsigned long lastTreeUpdateTime = 0;
const float swayFrequency = 0.003; 
const float maxSwayAngle = PI / 10.0;

// Bird animation variables
int bird1X = -10, bird1Y = 10;
int bird2X = -30, bird2Y = 20;
bool bird1Direction = true, bird2Direction = true;

// Cloud animation variables
const int numClouds = 3;
int cloudX[numClouds] = {10, 60, 110};
int cloudY[numClouds] = {5, 20, 10};
int cloudSpeed[numClouds] = {1, 2, 1};
int cloudDirection[numClouds] = {1, -1, 1};
int cloudSize = 20;
unsigned long lastCloudUpdateTime = 0;


bool isDay = true;
const int dayColor = SSD1306_WHITE;
const int nightColor = SSD1306_BLACK;
const int sunsetThreshold = 50;
unsigned long lastCycleUpdateTime = 0;
const unsigned long dayDuration = 30000; 
const unsigned long nightDuration = 15000; 

void setup() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.display();
}

void drawWater(int offset) {
    for (int i = 0; i < SCREEN_WIDTH; i += 4) {
        int y = waterHeight + offset + sin((i + offset) * 0.1) * 2;
        display.drawLine(i, y, i + 2, y + 2, SSD1306_WHITE);
        display.drawLine(i + 2, y + 2, i + 4, y, SSD1306_WHITE);
    }
}

void drawTree(int x, int y, float swayAngle) {
    // Draw trunk with bark texture
    display.fillRect(x - 3, y, 6, -20, SSD1306_WHITE);

    // Calculate branch positions based on sway angle
    int leftBranchX = x - 5 + int(sin(swayAngle) * 10);
    int rightBranchX = x + 5 + int(sin(swayAngle) * 10);
    int leftBranchY = y - 20 + int(cos(swayAngle) * 10);
    int rightBranchY = y - 20 - int(cos(swayAngle) * 10);

    // Trunk
    display.drawLine(x, y, x, y - 20, SSD1306_WHITE);

    // Left Branch
    display.drawLine(x, y - 20, leftBranchX, leftBranchY, SSD1306_WHITE);

    // Right Branch
    display.drawLine(x, y - 20, rightBranchX, rightBranchY, SSD1306_WHITE);

    // Draw leaves
    display.fillCircle(leftBranchX, leftBranchY, 4, SSD1306_WHITE);
    display.fillCircle(rightBranchX, rightBranchY, 4, SSD1306_WHITE);
}

void drawBird(int x, int y) {
    display.drawTriangle(x, y, x + 5, y - 3, x + 10, y, SSD1306_WHITE);
    display.drawTriangle(x + 10, y, x + 15, y - 3, x + 20, y, SSD1306_WHITE);
}

void drawCloud(int x, int y, int size) {
    display.fillCircle(x, y, size, SSD1306_WHITE);
    display.fillCircle(x + size / 2, y - size / 4, size * 0.8, SSD1306_WHITE);
    display.fillCircle(x - size / 2, y - size / 4, size * 0.8, SSD1306_WHITE);
}

void animateWater() {
    waterOffset += waterSpeed;
    if (waterOffset > 100) {
        waterOffset = 0;
    }
    drawWater(waterOffset);
}

void animateTrees() {
    unsigned long currentTime = millis();
    if (currentTime - lastTreeUpdateTime >= 20) { 
        lastTreeUpdateTime = currentTime;

        float sway1 = sin(currentTime * swayFrequency) * maxSwayAngle;
        float sway2 = cos(currentTime * swayFrequency) * maxSwayAngle;

   
        tree1Angle += (sway1 - tree1Angle) * 0.1;
        tree2Angle += (sway2 - tree2Angle) * 0.1; 

        tree1Angle = constrain(tree1Angle, -maxSwayAngle, maxSwayAngle);
        tree2Angle = constrain(tree2Angle, -maxSwayAngle, maxSwayAngle);

        treeSway1 = tree1Angle;
        treeSway2 = tree2Angle;
    }

    drawTree(tree1X, treeY, treeSway1);
    drawTree(tree2X, treeY, treeSway2);
}

void animateBirds() {
    if (bird1Direction) {
        bird1X += 2;
    } else {
        bird1X -= 2;
    }
    if (bird1X > SCREEN_WIDTH) {
        bird1X = -20;
        bird1Y = random(10, 30);
        bird1Direction = !bird1Direction;
    }
    drawBird(bird1X, bird1Y);

    if (bird2Direction) {
        bird2X += 3;
    } else {
        bird2X -= 3;
    }
    if (bird2X > SCREEN_WIDTH) {
        bird2X = -30;
        bird2Y = random(10, 30);
        bird2Direction = !bird2Direction;
    }
    drawBird(bird2X, bird2Y);
}

void animateClouds() {
    unsigned long currentTime = millis();
    if (currentTime - lastCloudUpdateTime >= 50) { 
        lastCloudUpdateTime = currentTime;

        // Move clouds horizontally
        for (int i = 0; i < numClouds; ++i) {
            cloudX[i] += cloudSpeed[i] * cloudDirection[i];

            if (cloudX[i] > SCREEN_WIDTH + cloudSize) {
                cloudX[i] = -cloudSize;
            } else if (cloudX[i] < -cloudSize) {
                cloudX[i] = SCREEN_WIDTH + cloudSize;
            }

            drawCloud(cloudX[i], cloudY[i], cloudSize);
        }
    }
}

void updateDayNightCycle() {
    unsigned long currentTime = millis();
    if (currentTime - lastCycleUpdateTime >= (isDay ? dayDuration : nightDuration)) {
        lastCycleUpdateTime = currentTime;
        isDay = !isDay;

 
        if (isDay) {
            display.setTextColor(SSD1306_WHITE);
            display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, dayColor); 
        } else {
            display.setTextColor(SSD1306_BLACK);
            display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, nightColor);
        }
    }
}

void loop() {
    display.clearDisplay();

    animateWater();
    animateTrees();
    animateBirds();
    animateClouds();

    updateDayNightCycle();

    display.display();
    delay(20); 
}

 
