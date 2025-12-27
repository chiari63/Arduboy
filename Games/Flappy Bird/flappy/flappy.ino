#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BIRD_X 30
#define BIRD_WIDTH 8
#define BIRD_HEIGHT 8
#define GRAVITY 0.5
#define FLAP_STRENGTH -5

float birdY = SCREEN_HEIGHT / 2;
float birdVelocity = 0;

#define PIPE_WIDTH 15
int gapY;
int pipeX = SCREEN_WIDTH;

bool gameOver = false;
bool gameStarted = false;
int score = 0;
bool passedPipe = false;

#define BUZZER_PIN 11

int difficulty = 1; // 0: Easy, 1: Medium, 2: Hard
int pipeSpeed = 2;
int gapHeight = 30;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(2, INPUT_PULLUP);  // Botão para mover para baixo (Down)
  pinMode(3, INPUT_PULLUP);  // Botão para mover para cima (Up)
  pinMode(BUZZER_PIN, OUTPUT);
  display.clearDisplay();

  showDifficultyMenu();
}

void loop() {
  if (!gameStarted) {
    if (digitalRead(3) == LOW) {
      startGame();
    } else if (digitalRead(2) == LOW) {
      changeDifficulty();
    }
    return;
  }

  if (!gameOver) {
    playGame();
  } else {
    showGameOverScreen();
  }
}

void showDifficultyMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.print("Flappy Bird");

  display.setTextSize(1);
  display.setCursor(5, 30);
  display.print("Difficulty: ");
  if (difficulty == 0) display.print("Easy");
  else if (difficulty == 1) display.print("Medium");
  else if (difficulty == 2) display.print("Hard");

  display.setCursor(5, 45);
  display.print("UP: Start | LEFT: Change");

  display.display();
}

void changeDifficulty() {
  difficulty = (difficulty + 1) % 3; // Alterna entre 0, 1, 2 (Easy, Medium, Hard)
  showDifficultyMenu();
}

void startGame() {
  gameStarted = true;
  birdY = SCREEN_HEIGHT / 2;
  birdVelocity = 0;
  score = 0;
  pipeSpeed = (difficulty == 0) ? 2 : (difficulty == 1) ? 3 : 4;
  gapHeight = (difficulty == 0) ? 40 : (difficulty == 1) ? 30 : 20;
  pipeX = SCREEN_WIDTH;
  gapY = random(10, SCREEN_HEIGHT - gapHeight - 10);
  passedPipe = false;
  gameOver = false;
  display.clearDisplay();
}

void playGame() {
  birdVelocity += GRAVITY;
  birdY += birdVelocity;

  if (digitalRead(3) == LOW) {
    birdVelocity = FLAP_STRENGTH;
    tone(BUZZER_PIN, 1000, 100);
  }

  pipeX -= pipeSpeed;
  if (pipeX < -PIPE_WIDTH) {
    pipeX = SCREEN_WIDTH;
    gapY = random(10, SCREEN_HEIGHT - gapHeight - 10);
    passedPipe = false;
  }

  if (pipeX < BIRD_X + BIRD_WIDTH && pipeX + PIPE_WIDTH > BIRD_X) {
    if (birdY < gapY || birdY + BIRD_HEIGHT > gapY + gapHeight) {
      gameOver = true;
      tone(BUZZER_PIN, 200, 500);
    }
  }

  if (birdY > SCREEN_HEIGHT - BIRD_HEIGHT || birdY < 0) {
    gameOver = true;
    tone(BUZZER_PIN, 200, 500);
  }

  if (!passedPipe && pipeX + PIPE_WIDTH < BIRD_X) {
    score++;
    passedPipe = true;
    tone(BUZZER_PIN, 1500, 100);
  }

  display.clearDisplay();
  display.fillRect(BIRD_X, (int)birdY, BIRD_WIDTH, BIRD_HEIGHT, SSD1306_WHITE);
  display.fillRect(pipeX, 0, PIPE_WIDTH, gapY, SSD1306_WHITE);
  display.fillRect(pipeX, gapY + gapHeight, PIPE_WIDTH, SCREEN_HEIGHT - gapY - gapHeight, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.print("Score: ");
  display.print(score);

  display.display();
}

void showGameOverScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.print("Game Over");

  display.setTextSize(1);
  display.setCursor(10, 40);
  display.print("Press UP to restart");

  display.setCursor(10, 55);
  display.print("Final Score: ");
  display.print(score);

  display.display();

  if (digitalRead(3) == LOW) {
    gameStarted = false;
    showDifficultyMenu();
  }
}
