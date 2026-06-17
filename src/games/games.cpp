#include "games.h"
#include "../globals/globals.h"
#include "../display/display.h"
#include "../helpers/helpers.h"
#include "../rgb/rgb.h"
#include "../sd/sdState.h"

enum GameState { STATE_MENU, STATE_PONG, STATE_FLAPPY, STATE_BREAKOUT };
static GameState currentGameState = STATE_MENU;

static MenuState gamesMenuState = {0, 0};
static String gamesItems[] = {"Pong", "Flappy Bird", "Breakout"};
static const int gamesItemCount = 3;

static unsigned long rgbTimer = 0;

static void triggerRgbFlash(uint8_t r, uint8_t g, uint8_t b, unsigned long durationMs) {
    setRgbColor(r, g, b);
    rgbTimer = millis() + durationMs;
}

static void updateRgbTimer() {
    if (rgbTimer > 0 && millis() >= rgbTimer) {
        if (currentGameState == STATE_PONG) {
            setRgbPurple();
        } else if (currentGameState == STATE_FLAPPY) {
            setRgbBlue();
        } else if (currentGameState == STATE_BREAKOUT) {
            setRgbGreen();
        } else {
            setRgbWhite();
        }
        rgbTimer = 0;
    }
}

namespace Pong {
    float paddleX;
    const int paddleY = 59;
    const int paddleW = 20;
    const int paddleH = 3;
    
    // AI Opponent Paddle
    float aiPaddleX;
    const int aiPaddleY = 11;
    const int aiPaddleW = 20;
    const int aiPaddleH = 3;
    float aiErrorOffset;
    
    float ballX, ballY;
    float ballDX, ballDY;
    const float ballSize = 3;
    
    int score;
    int lives;
    bool gameOver;
    unsigned long lastUpdate;
    
    void init() {
        paddleX = (128 - paddleW) / 2.0f;
        aiPaddleX = (128 - aiPaddleW) / 2.0f;
        aiErrorOffset = 0.0f;
        ballX = 64.0f;
        ballY = 20.0f;
        ballDX = 1.3f;
        ballDY = 1.3f;
        score = 0;
        lives = 3;
        gameOver = false;
        lastUpdate = millis();
    }
    
    void update(int rotaryDir, bool buttonPressed) {
        if (gameOver) {
            if (buttonPressed) {
                init();
                setRgbPurple();
            }
            return;
        }
        
        if (rotaryDir != 0) {
            paddleX += rotaryDir * 5.0f;
            if (paddleX < 0) paddleX = 0;
            if (paddleX > 128 - paddleW) paddleX = 128 - paddleW;
        }
        
        unsigned long now = millis();
        if (now - lastUpdate >= 25) {
            lastUpdate = now;
            
            // Move AI paddle smoothly towards the ball with a speed limit and error offset
            float targetX = ballX + ballSize / 2.0f - aiPaddleW / 2.0f + aiErrorOffset;
            float diff = targetX - aiPaddleX;
            
            // Track slower when the ball is moving away, faster when moving towards AI
            float lerpFactor = 0.12f;
            float maxStep = 1.8f;
            if (ballDY > 0) {
                lerpFactor = 0.06f;
                maxStep = 1.0f;
            }
            
            float step = diff * lerpFactor;
            if (step > maxStep) step = maxStep;
            if (step < -maxStep) step = -maxStep;
            
            aiPaddleX += step;
            if (aiPaddleX < 0) aiPaddleX = 0;
            if (aiPaddleX > 128 - aiPaddleW) aiPaddleX = 128 - aiPaddleW;
            
            ballX += ballDX;
            ballY += ballDY;
            
            // Wall bounce (left/right walls only)
            if (ballX <= 0) {
                ballX = 0;
                ballDX = -ballDX;
                aiErrorOffset = random(-6, 7); // Randomize AI tracking error on wall bounce
                triggerRgbFlash(0, 0, 100, 50);
            } else if (ballX >= 128 - ballSize) {
                ballX = 128 - ballSize;
                ballDX = -ballDX;
                aiErrorOffset = random(-6, 7); // Randomize AI tracking error on wall bounce
                triggerRgbFlash(0, 0, 100, 50);
            }
            
            // AI Paddle bounce (top paddle)
            if (ballY <= aiPaddleY + aiPaddleH && ballY >= aiPaddleY) {
                if (ballX + ballSize >= aiPaddleX && ballX <= aiPaddleX + aiPaddleW) {
                    ballY = aiPaddleY + aiPaddleH;
                    ballDY = -ballDY;
                    
                    float hitPoint = (ballX + ballSize / 2.0f) - (aiPaddleX + aiPaddleW / 2.0f);
                    float normalizedHit = hitPoint / (aiPaddleW / 2.0f);
                    ballDX = normalizedHit * 2.0f;
                    
                    ballDY = ballDY * 1.05f;
                    if (ballDY < -3.0f) ballDY = -3.0f;
                    if (ballDY > 3.0f) ballDY = 3.0f;
                    
                    aiErrorOffset = 0.0f; // Reset error offset on bounce
                    triggerRgbFlash(0, 0, 100, 50);
                }
            }
            
            // Player Paddle bounce (bottom paddle)
            if (ballY >= paddleY - ballSize && ballY <= paddleY) {
                if (ballX + ballSize >= paddleX && ballX <= paddleX + paddleW) {
                    ballY = paddleY - ballSize;
                    ballDY = -ballDY;
                    
                    float hitPoint = (ballX + ballSize / 2.0f) - (paddleX + paddleW / 2.0f);
                    float normalizedHit = hitPoint / (paddleW / 2.0f);
                    ballDX = normalizedHit * 2.0f;
                    
                    ballDY = ballDY * 1.05f;
                    if (ballDY < -3.0f) ballDY = -3.0f;
                    if (ballDY > 3.0f) ballDY = 3.0f;
                    
                    aiErrorOffset = random(-6, 7); // Randomize AI tracking error when player hits ball
                    triggerRgbFlash(0, 255, 255, 50); // Flash Cyan when player blocks
                }
            }
            
            // Out of bounds (top side - AI missed)
            if (ballY < 11) {
                score++;
                triggerRgbFlash(0, 255, 0, 150); // Flash Green when player scores
                
                // Serve from AI paddle position
                ballX = aiPaddleX + aiPaddleW / 2.0f;
                ballY = 20.0f;
                ballDX = (random(0, 2) == 0 ? -1.0f : 1.0f);
                ballDY = 1.3f;
                aiErrorOffset = 0.0f;
            }
            
            // Out of bounds (bottom side - Player missed)
            if (ballY > 64) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                    setRgbRed();
                } else {
                    ballX = paddleX + paddleW / 2.0f;
                    ballY = 40.0f;
                    ballDX = 1.0f;
                    ballDY = -1.3f;
                    aiErrorOffset = 0.0f;
                    triggerRgbFlash(255, 0, 0, 200); // Flash Red when player loses a life
                }
            }
        }
    }
    
    void draw() {
        display.clearDisplay();
        
        if (gameOver) {
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(10, 10);
            display.print("GAME OVER");
            display.setTextSize(1);
            display.setCursor(10, 35);
            display.print("Score: ");
            display.print(score);
            display.setCursor(10, 48);
            display.print("Press to restart");
            display.display();
            return;
        }
        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, 2);
        display.print("S:");
        display.print(score);
        display.setCursor(100, 2);
        display.print("L:");
        display.print(lives);
        display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
        
        // Draw Player Paddle
        display.fillRect((int)paddleX, paddleY, paddleW, paddleH, SSD1306_WHITE);
        
        // Draw AI Paddle
        display.fillRect((int)aiPaddleX, aiPaddleY, aiPaddleW, aiPaddleH, SSD1306_WHITE);
        
        // Draw Ball
        display.fillRect((int)ballX, (int)ballY, (int)ballSize, (int)ballSize, SSD1306_WHITE);
        
        display.display();
    }
}

namespace Flappy {
    float birdY;
    float birdVelocity;
    const float gravity = 0.15f;
    const float flapStrength = -2.0f;
    
    // Difficulty settings
    const float pipeSpeed = 1.75f;
    const float pipeSpacing = 52.0f;
    
    struct Pipe {
        float x;
        int gapY;
        bool passed;
    } pipes[2];
    
    int score;
    bool gameOver;
    unsigned long lastUpdate;
    
    void init() {
        birdY = 28.0f;
        birdVelocity = 0.0f;
        score = 0;
        gameOver = false;
        
        pipes[0].x = 128.0f;
        pipes[0].gapY = 12 + random(0, 20);
        pipes[0].passed = false;
        
        pipes[1].x = 128.0f + pipeSpacing;
        pipes[1].gapY = 12 + random(0, 20);
        pipes[1].passed = false;
        
        lastUpdate = millis();
    }
    
    void update(int rotaryDir, bool buttonPressed) {
        if (gameOver) {
            if (buttonPressed) {
                init();
                setRgbBlue();
            }
            return;
        }
        
        if (buttonPressed) {
            birdVelocity = flapStrength;
            triggerRgbFlash(0, 128, 255, 80);
        }
        
        unsigned long now = millis();
        if (now - lastUpdate >= 30) {
            lastUpdate = now;
            
            birdVelocity += gravity;
            birdY += birdVelocity;
            
            if (birdY < 0) {
                birdY = 0;
                birdVelocity = 0;
            }
            
            if (birdY + 4.0f >= 60.0f) {
                birdY = 56.0f;
                gameOver = true;
                setRgbRed();
            }
            
            for (int i = 0; i < 2; i++) {
                pipes[i].x -= pipeSpeed;
                
                if (pipes[i].x < -8.0f) {
                    pipes[i].x = pipes[1 - i].x + pipeSpacing;
                    pipes[i].gapY = 12 + random(0, 20);
                    pipes[i].passed = false;
                }
                
                if (!pipes[i].passed && pipes[i].x < 20.0f) {
                    pipes[i].passed = true;
                    score++;
                    triggerRgbFlash(0, 255, 0, 100);
                }
                
                float birdLeft = 20.0f;
                float birdRight = 24.0f;
                float birdTop = birdY;
                float birdBottom = birdY + 4.0f;
                
                float pipeLeft = pipes[i].x;
                float pipeRight = pipes[i].x + 8.0f;
                float gapTop = pipes[i].gapY;
                float gapBottom = pipes[i].gapY + 22.0f;
                
                if (birdRight > pipeLeft && birdLeft < pipeRight) {
                    if (birdTop < gapTop || birdBottom > gapBottom) {
                        gameOver = true;
                        setRgbRed();
                    }
                }
            }
        }
    }
    
    void draw() {
        display.clearDisplay();
        
        if (gameOver) {
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(10, 10);
            display.print("GAME OVER");
            display.setTextSize(1);
            display.setCursor(10, 35);
            display.print("Score: ");
            display.print(score);
            display.setCursor(10, 48);
            display.print("Press to restart");
            display.display();
            return;
        }
        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, 2);
        display.print("Score: ");
        display.print(score);
        
        // Ground line
        display.drawFastHLine(0, 60, 128, SSD1306_WHITE);
        
        // Bird
        display.fillRect(20, (int)birdY, 4, 4, SSD1306_WHITE);
        
        // Pipes
        for (int i = 0; i < 2; i++) {
            int px = (int)pipes[i].x;
            int gapTop = pipes[i].gapY;
            int gapBottom = pipes[i].gapY + 22;
            
            display.fillRect(px, 0, 8, gapTop, SSD1306_WHITE);
            display.fillRect(px, gapBottom, 8, 60 - gapBottom, SSD1306_WHITE);
        }
        
        display.display();
    }
}

namespace Breakout {
    float paddleX;
    const int paddleY = 59;
    const int paddleW = 20;
    const int paddleH = 3;
    
    float ballX, ballY;
    float ballDX, ballDY;
    const float ballSize = 3;
    
    bool bricks[4][8];
    int bricksLeft;
    int score;
    int lives;
    int level;
    bool gameOver;
    unsigned long lastUpdate;
    
    void generateLevelBricks(int patternIndex) {
        bricksLeft = 0;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 8; c++) {
                bool brickPresent = false;
                switch (patternIndex) {
                    case 0: // Full grid
                        brickPresent = true;
                        break;
                    case 1: // Alternating columns (stripes)
                        brickPresent = (c % 2 == 0);
                        break;
                    case 2: // Alternating rows
                        brickPresent = (r % 2 == 0);
                        break;
                    case 3: // Pyramid / V-shape
                        brickPresent = (c >= r && c < 8 - r);
                        break;
                    case 4: // Checkerboard
                        brickPresent = ((r + c) % 2 == 0);
                        break;
                    case 5: // Outer border
                        brickPresent = (r == 0 || r == 3 || c == 0 || c == 7);
                        break;
                    case 6: // Diamond
                        if (r == 0 || r == 3) brickPresent = (c == 3 || c == 4);
                        else brickPresent = (c >= 2 && c <= 5);
                        break;
                    case 7: // Two pillars
                        brickPresent = (c == 1 || c == 2 || c == 5 || c == 6);
                        break;
                }
                bricks[r][c] = brickPresent;
                if (brickPresent) bricksLeft++;
            }
        }
    }
    
    void init() {
        paddleX = (128 - paddleW) / 2.0f;
        ballX = 64.0f;
        ballY = 40.0f;
        ballDX = 1.0f;
        ballDY = -1.2f;
        score = 0;
        lives = 3;
        level = 1;
        gameOver = false;
        generateLevelBricks(0); // Level 1 is full grid
        lastUpdate = millis();
    }
    
    void update(int rotaryDir, bool buttonPressed) {
        if (gameOver) {
            if (buttonPressed) {
                init();
                setRgbGreen();
            }
            return;
        }
        
        if (rotaryDir != 0) {
            paddleX += rotaryDir * 5.0f;
            if (paddleX < 0) paddleX = 0;
            if (paddleX > 128 - paddleW) paddleX = 128 - paddleW;
        }
        
        unsigned long now = millis();
        if (now - lastUpdate >= 25) {
            lastUpdate = now;
            
            ballX += ballDX;
            ballY += ballDY;
            
            // Wall bounce
            if (ballX <= 0) {
                ballX = 0;
                ballDX = -ballDX;
                triggerRgbFlash(0, 100, 0, 50);
            } else if (ballX >= 128 - ballSize) {
                ballX = 128 - ballSize;
                ballDX = -ballDX;
                triggerRgbFlash(0, 100, 0, 50);
            }
            
            if (ballY <= 11) {
                ballY = 11;
                ballDY = -ballDY;
                triggerRgbFlash(0, 100, 0, 50);
            }
            
            // Paddle bounce
            if (ballY >= paddleY - ballSize && ballY <= paddleY) {
                if (ballX + ballSize >= paddleX && ballX <= paddleX + paddleW) {
                    ballY = paddleY - ballSize;
                    ballDY = -ballDY;
                    
                    float hitPoint = (ballX + ballSize / 2.0f) - (paddleX + paddleW / 2.0f);
                    float normalizedHit = hitPoint / (paddleW / 2.0f);
                    ballDX = normalizedHit * 1.8f;
                    
                    if (ballDX < -1.8f) ballDX = -1.8f;
                    if (ballDX > 1.8f) ballDX = 1.8f;
                    
                    triggerRgbFlash(0, 255, 0, 100);
                }
            }
            
            // Brick collisions
            if (ballY >= 12 && ballY <= 36) {
                int r = (int)(ballY - 12) / 6;
                int c = (int)(ballX - 1) / 16;
                
                if (r >= 0 && r < 4 && c >= 0 && c < 8) {
                    if (bricks[r][c]) {
                        bricks[r][c] = false;
                        bricksLeft--;
                        score += 10;
                        
                        triggerRgbFlash(255, 255, 0, 80);
                        
                        float brickTop = 12 + r * 6;
                        float brickBottom = brickTop + 4;
                        float brickLeft = 1 + c * 16;
                        float brickRight = brickLeft + 14;
                        
                        float overlapX = min(ballX + ballSize - brickLeft, brickRight - ballX);
                        float overlapY = min(ballY + ballSize - brickTop, brickBottom - ballY);
                        
                        if (overlapX < overlapY) {
                            ballDX = -ballDX;
                        } else {
                            ballDY = -ballDY;
                        }
                        
                        if (bricksLeft <= 0) {
                            // Level cleared! Progress to next level automatically
                            level++;
                            triggerRgbFlash(0, 255, 0, 500); // Long green flash for level clear
                            
                            // Select next pattern randomly (0 to 7)
                            int nextPattern = random(0, 8);
                            generateLevelBricks(nextPattern);
                            
                            // Increase difficulty: scale ball speed
                            float speedMultiplier = min(1.6f, 1.0f + (level - 1) * 0.08f);
                            ballX = paddleX + paddleW / 2.0f;
                            ballY = 40.0f;
                            ballDX = (random(0, 2) == 0 ? -1.0f : 1.0f) * 1.0f * speedMultiplier;
                            ballDY = -1.2f * speedMultiplier;
                        }
                    }
                }
            }
            
            // Out of bounds
            if (ballY > 64) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                    setRgbRed();
                } else {
                    ballX = paddleX + paddleW / 2.0f;
                    ballY = 40.0f;
                    float speedMultiplier = min(1.6f, 1.0f + (level - 1) * 0.08f);
                    ballDX = (random(0, 2) == 0 ? -1.0f : 1.0f) * 1.0f * speedMultiplier;
                    ballDY = -1.2f * speedMultiplier;
                    triggerRgbFlash(255, 0, 0, 200);
                }
            }
        }
    }
    
    void draw() {
        display.clearDisplay();
        
        if (gameOver) {
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(10, 10);
            display.print("GAME OVER");
            display.setTextSize(1);
            display.setCursor(10, 35);
            display.print("Score: ");
            display.print(score);
            display.setCursor(10, 48);
            display.print("Press to restart");
            display.display();
            return;
        }
        
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(2, 1);
        display.print("S:");
        display.print(score);
        display.setCursor(52, 1);
        display.print("Lv:");
        display.print(level);
        display.setCursor(100, 1);
        display.print("L:");
        display.print(lives);
        
        display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
        
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 8; c++) {
                if (bricks[r][c]) {
                    int bx = 1 + c * 16;
                    int by = 12 + r * 6;
                    display.fillRect(bx, by, 14, 4, SSD1306_WHITE);
                }
            }
        }
        
        display.fillRect((int)paddleX, paddleY, paddleW, paddleH, SSD1306_WHITE);
        display.fillRect((int)ballX, (int)ballY, (int)ballSize, (int)ballSize, SSD1306_WHITE);
        
        display.display();
    }
}

static unsigned long lastTransitionTime = 0;

void initGames() {
    currentGameState = STATE_MENU;
    gamesMenuState.selectedIndex = 0;
    gamesMenuState.scrollOffset = 0;
    drawMenu("Games Menu", gamesItems, gamesItemCount, gamesMenuState.selectedIndex, gamesMenuState.scrollOffset, selectedScroll);
}

bool handleGamesMode() {
    if (currentGameState == STATE_MENU) {
        int rotaryReadings = readRotary();
        if (rotaryReadings != 0) {
            updateMenuState(gamesMenuState, gamesItemCount, rotaryReadings);
            drawMenu("Games Menu", gamesItems, gamesItemCount, gamesMenuState.selectedIndex, gamesMenuState.scrollOffset, selectedScroll);
        }
        
        swRotary.update();
        if (swRotary.pressed()) {
            if (gamesMenuState.selectedIndex == 0) {
                currentGameState = STATE_PONG;
                backBtnLatched = false;
                Pong::init();
                Pong::draw();
                setRgbPurple();
                updateRgb();
            }
            else if (gamesMenuState.selectedIndex == 1) {
                currentGameState = STATE_FLAPPY;
                backBtnLatched = false;
                Flappy::init();
                Flappy::draw();
                setRgbBlue();
                updateRgb();
            }
            else if (gamesMenuState.selectedIndex == 2) {
                currentGameState = STATE_BREAKOUT;
                backBtnLatched = false;
                Breakout::init();
                Breakout::draw();
                setRgbGreen();
                updateRgb();
            }
        }
        
        backBtn.update();
        if (backBtn.pressed() || backBtnLatched) {
            backBtnLatched = false;
            if (millis() - lastTransitionTime >= 300) {
                lastTransitionTime = millis();
                return true; // Go back to SELECT MODE
            }
        }
    } else {
        // Back button always exits game to menu
        backBtn.update();
        if (backBtn.pressed() || backBtnLatched) {
            backBtnLatched = false;
            if (millis() - lastTransitionTime >= 300) {
                lastTransitionTime = millis();
                setRgbRainbow(false);
                initGames();
            }
            return false;
        }
        
        swRotary.update();
        bool buttonPressed = swRotary.pressed();
        int rotaryDir = readRotary();
        
        if (currentGameState == STATE_PONG) {
            Pong::update(rotaryDir, buttonPressed);
            Pong::draw();
        }
        else if (currentGameState == STATE_FLAPPY) {
            Flappy::update(rotaryDir, buttonPressed);
            Flappy::draw();
        }
        else if (currentGameState == STATE_BREAKOUT) {
            Breakout::update(rotaryDir, buttonPressed);
            Breakout::draw();
        }
    }
    
    updateRgbTimer();
    updateRgb();
    return false;
}
