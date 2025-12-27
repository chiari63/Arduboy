#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinos dos botões e do buzzer
const int buttonUp = 3;
const int buttonDown = 4;
const int buzzer = 11;
const int buttonPause = 6;

// Raquete
int paddleHeight = 16;
int leftPaddleY = (SCREEN_HEIGHT - paddleHeight) / 2;
int rightPaddleY = (SCREEN_HEIGHT - paddleHeight) / 2;
int paddleWidth = 2;
int paddleSpeed = 2;

// Bola
int ballSize = 2;
int ballX = SCREEN_WIDTH / 2;
int ballY = SCREEN_HEIGHT / 2;
int ballSpeedX = 2;
int ballSpeedY = 2;

// Placar
int leftScore = 0;
int rightScore = 0;
int maxScore = 10;

bool paused = false;

void setup() {
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(buttonPause, INPUT_PULLUP);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
  // Exibir tela de início
  displayStartScreen();
  
  // Esperar até que o botão "up" seja pressionado
  waitForStart();
}

void loop() {

    if (digitalRead(buttonPause) == LOW) {
    delay(300);  // Pequeno delay para evitar múltiplas leituras
    paused = !paused;  // Alterna entre pausado e não pausado
    if (paused) {
      showPauseScreen();
    } else {  
      display.clearDisplay();
      display.display();
    }
  }
  
  if (paused) {
    delay(50);
    return;
  }
  // Movimento da raquete esquerda
  if (digitalRead(buttonUp) == LOW && leftPaddleY > 0) {
    leftPaddleY -= paddleSpeed;
  }
  if (digitalRead(buttonDown) == LOW && leftPaddleY < SCREEN_HEIGHT - paddleHeight) {
    leftPaddleY += paddleSpeed;
  }
  
  // Movimento da raquete direita (IA com comportamento mais humano)
  if (ballY > rightPaddleY + paddleHeight / 2 && rightPaddleY < SCREEN_HEIGHT - paddleHeight) {
    if (random(0, 10) > 2) { // 80% de chance de seguir a bola
        rightPaddleY += paddleSpeed;
    }
  }
  if (ballY < rightPaddleY + paddleHeight / 2 && rightPaddleY > 0) {
    if (random(0, 10) > 2) { // 80% de chance de seguir a bola
        rightPaddleY -= paddleSpeed;
    }
  }
  
  // Movimento da bola
  ballX += ballSpeedX;
  ballY += ballSpeedY;
  
  // Colisão com as paredes superior e inferior
  if (ballY <= 0 || ballY >= SCREEN_HEIGHT - ballSize) {
    ballSpeedY = -ballSpeedY;
    tone(buzzer, 1000, 50); // Som de colisão
  }
  
  // Colisão com a raquete esquerda
  if (ballX <= paddleWidth && ballY >= leftPaddleY && ballY <= leftPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX;
    tone(buzzer, 1000, 50); // Som de colisão
  }
  
  // Colisão com a raquete direita
  if (ballX >= SCREEN_WIDTH - paddleWidth - ballSize && ballY >= rightPaddleY && ballY <= rightPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX;
    tone(buzzer, 1000, 50); // Som de colisão
  }
  
  // Pontuação
  if (ballX < 0) {
    rightScore++;
    resetBall();
  }
  if (ballX > SCREEN_WIDTH) {
    leftScore++;
    resetBall();
  }
  
  // Verifica se alguém ganhou
  if (leftScore >= maxScore) {
    displayGameOverScreen("You Win!");
  } else if (rightScore >= maxScore) {
    displayGameOverScreen("You Lose!");
  }
  
  // Desenha o jogo
  display.clearDisplay();

  // Desenha as bordas da quadra
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

  // Desenha a linha central
  display.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, WHITE);
  
  // Desenha as raquetes
  display.fillRect(1, leftPaddleY, paddleWidth, paddleHeight, WHITE);
  display.fillRect(SCREEN_WIDTH - paddleWidth - 1, rightPaddleY, paddleWidth, paddleHeight, WHITE);

  // Desenha a bola
  display.fillRect(ballX, ballY, ballSize, ballSize, WHITE);

  // Desenha o placar dentro da quadra na parte superior
  display.setTextSize(1); // Tamanho da fonte menor
  display.setTextColor(WHITE);
  
  display.setCursor(SCREEN_WIDTH / 3 - 10, 5); // Posição do placar esquerdo
  display.print(leftScore);

  display.setCursor(SCREEN_WIDTH * 4 / 5 - 10, 5); // Posição do placar direito
  display.print(rightScore);
  
  display.display();
  
  delay(10);
}

void displayStartScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40, 15);
  display.println("PONG");
  
  display.setTextSize(1);
  display.setCursor(15, 40);
  display.println("Press Up to Start");
  
  display.display();
}

void waitForStart() {
  while (digitalRead(buttonUp) == HIGH) {
    // Espera até que o botão "up" seja pressionado
  }
  tone(buzzer, 1000, 100); // Som ao iniciar o jogo
  delay(500); // Pequena pausa antes de iniciar o jogo
}

void displayGameOverScreen(const char* message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(13, 20);
  display.println(message);

  display.setTextSize(1);
  display.setCursor(8, 50);
  display.println("Press Up to Restart");
  
  display.display();
  
  waitForStart();  // Espera o botão "up" para reiniciar o jogo
  resetGame();
}

void resetBall() {
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballSpeedX = -ballSpeedX;
  delay(500);
}

void resetGame() {
  leftScore = 0;
  rightScore = 0;
  resetBall();
}
void showPauseScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println("Paused");

  display.display();
}
