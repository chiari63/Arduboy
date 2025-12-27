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
#define GAP_HEIGHT 30
#define PIPE_SPEED 2
int pipeX = SCREEN_WIDTH;
int gapY;

bool gameOver = false;
bool gameStarted = false;
int score = 0;
bool passedPipe = false;

#define BUZZER_PIN 11

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(3, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  display.clearDisplay();

  gapY = random(10, SCREEN_HEIGHT - GAP_HEIGHT - 10);

  // Tela inicial
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 10);
  display.print("Flappy Bird");

  display.setTextSize(1);
  display.setCursor(25, 40);
  display.print("Press UP to start");

  display.display();
}

void loop() {
  if (!gameStarted) {
    if (digitalRead(3) == LOW) {
      gameStarted = true;
      birdY = SCREEN_HEIGHT / 2;
      birdVelocity = 0;
      pipeX = SCREEN_WIDTH;
      score = 0;
      gapY = random(10, SCREEN_HEIGHT - GAP_HEIGHT - 10);
      display.clearDisplay();
    }
    return;
  }

  if (!gameOver) {
    birdVelocity += GRAVITY;
    birdY += birdVelocity;

    if (digitalRead(3) == LOW) {
      birdVelocity = FLAP_STRENGTH;
      tone(BUZZER_PIN, 1000, 100);  // Som do flap
    }

    pipeX -= PIPE_SPEED;
    if (pipeX < -PIPE_WIDTH) {
      pipeX = SCREEN_WIDTH;
      gapY = random(10, SCREEN_HEIGHT - GAP_HEIGHT - 10);
      passedPipe = false;
    }

    if (pipeX < BIRD_X + BIRD_WIDTH && pipeX + PIPE_WIDTH > BIRD_X) {
      if (birdY < gapY || birdY + BIRD_HEIGHT > gapY + GAP_HEIGHT) {
        gameOver = true;
        tone(BUZZER_PIN, 200, 500);  // Som de game over
      }
    }

    if (birdY > SCREEN_HEIGHT - BIRD_HEIGHT || birdY < 0) {
      gameOver = true;
      tone(BUZZER_PIN, 200, 500);  // Som de game over
    }

    if (!passedPipe && pipeX + PIPE_WIDTH < BIRD_X) {
      score++;
      passedPipe = true;
      tone(BUZZER_PIN, 1500, 100);  // Som ao passar pelo cano
    }

    display.clearDisplay();
    display.fillRect(BIRD_X, (int)birdY, BIRD_WIDTH, BIRD_HEIGHT, SSD1306_WHITE);
    display.fillRect(pipeX, 0, PIPE_WIDTH, gapY, SSD1306_WHITE);
    display.fillRect(pipeX, gapY + GAP_HEIGHT, PIPE_WIDTH, SCREEN_HEIGHT - gapY - GAP_HEIGHT, SSD1306_WHITE);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 5);
    display.print("Score: ");
    display.print(score);

    display.display();
  } else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 10);
    display.print("Game Over");

    display.setTextSize(1);
    display.setCursor(15, 40);
    display.print("Press UP to restart");

    display.setCursor(20, 55);
    display.print("Final Score: ");
    display.print(score);

    display.display();

    if (digitalRead(3) == LOW) {
      gameOver = false;
      gameStarted = false;
    }
  }

  delay(30);
}
