#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PADDLE_WIDTH  20
#define PADDLE_HEIGHT 3
#define BALL_SIZE 3
#define BRICKS_COLS 8
#define BRICKS_ROWS 4
#define BRICK_WIDTH (SCREEN_WIDTH / BRICKS_COLS)
#define BRICK_HEIGHT 8

// Definindo os botões
#define BUTTON_LEFT 2
#define BUTTON_RIGHT 5
#define BUTTON_UP 3
#define buttonPause 6

// Definindo o buzzer
#define BUZZER_PIN 11

// Posições iniciais
int paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
int paddleY = SCREEN_HEIGHT - 10;
int ballX = SCREEN_WIDTH / 2;
int ballY = paddleY - BALL_SIZE;
int ballSpeedX = 1;
int ballSpeedY = -1;

bool bricks[BRICKS_ROWS][BRICKS_COLS];  // Matriz para representar os blocos

bool gameStarted = false;
bool gameOver = false;
int score = 0;  // Variável para armazenar a pontuação
int level = 1;  // Variável para armazenar o nível atual

// Variáveis para debounce
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // Delay de debounce de 200ms

bool paused = false;

void setup() {
  // Iniciar o display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();

  // Configurar os botões
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(buttonPause, INPUT_PULLUP);

  // Configurar o buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Inicializar os blocos
  initializeBricks();

  // Reiniciar a posição da bola e da barra
  resetBallAndPaddle();
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 10);
  display.print("Breakout");
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.print("Press UP to start");
  display.display();

  // Esperar o botão UP ser pressionado para iniciar o jogo
  while(digitalRead(BUTTON_UP) != LOW) { delay(10); }

  gameStarted = true;
  gameOver = false;

  // Aguardar até que o botão seja solto
  while(digitalRead(BUTTON_UP) == LOW) { delay(10); }

  display.clearDisplay();
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
  if (!gameStarted) return;

  // Verificar os botões para mover a barra
  if (digitalRead(BUTTON_LEFT) == LOW && !gameOver) {
    paddleX -= 4;
    if (paddleX < 0) {
      paddleX = 0;
    }
  }
  
  if (digitalRead(BUTTON_RIGHT) == LOW && !gameOver) {
    paddleX += 4;
    if (paddleX > SCREEN_WIDTH - PADDLE_WIDTH) {
      paddleX = SCREEN_WIDTH - PADDLE_WIDTH;
    }
  }

  if (!gameOver) {
    // Atualizar a posição da bola
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    // Colisão com as bordas da tela
    if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE) {
      ballSpeedX = -ballSpeedX;
    }
    if (ballY <= 0) {
      ballSpeedY = -ballSpeedY;
    }

    // Colisão com a barra
    if (ballY >= paddleY - BALL_SIZE && ballX + BALL_SIZE >= paddleX && ballX <= paddleX + PADDLE_WIDTH) {
      ballSpeedY = -ballSpeedY;
      ballY = paddleY - BALL_SIZE;

      // Som de colisão com a barra
      tone(BUZZER_PIN, 1000, 100);  // 1000 Hz por 100 ms
    }

    // Colisão com os blocos
    int blocksRemaining = 0;
    for(int i = 0; i < BRICKS_ROWS; i++) {
      for(int j = 0; j < BRICKS_COLS; j++) {
        if (bricks[i][j]) {
          blocksRemaining++;
          int brickX = j * BRICK_WIDTH;
          int brickY = i * BRICK_HEIGHT;

          if (ballX + BALL_SIZE >= brickX && ballX <= brickX + BRICK_WIDTH &&
              ballY + BALL_SIZE >= brickY && ballY <= brickY + BRICK_HEIGHT) {
            ballSpeedY = -ballSpeedY;
            bricks[i][j] = false;
            score += 10;  // Incrementar a pontuação ao destruir um bloco

            // Som de colisão com os blocos
            tone(BUZZER_PIN, 1200, 100);  // 1200 Hz por 100 ms
          }
        }
      }
    }

    // Verificar condição de vitória
    if (blocksRemaining == 0) {
      // Passar para o próximo nível
      level++;
      nextLevel();
    }

    // Verificar se a bola passou da barra (fim de jogo)
    if (ballY > SCREEN_HEIGHT) {
      gameOver = true;

      // Som de Game Over
      tone(BUZZER_PIN, 500, 500);  // 500 Hz por 500 ms
    }
  }

  // Se o jogo acabou, mostrar a tela de "Game Over"
  if (gameOver) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(13, 10);
    display.print("Game Over");
    display.setTextSize(1);
    display.setCursor(10, 40);
    display.print("Score: ");
    display.print(score);  // Exibir a pontuação
    display.setCursor(10, 50);
    display.print("Press UP to restart");
    display.display();

    // Debounce para o botão UP
    if (digitalRead(BUTTON_UP) == LOW) {
      unsigned long currentTime = millis();
      if ((currentTime - lastDebounceTime) > debounceDelay) {
        lastDebounceTime = currentTime;
        
        // Aguardar até que o botão seja solto
        while(digitalRead(BUTTON_UP) == LOW) { delay(10); }

        // Reiniciar o jogo
        resetGame();
      }
    }
  } else {
    // Limpar a tela
    display.clearDisplay();

    // Desenhar a barra
    display.fillRect(paddleX, paddleY, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);

    // Desenhar a bola
    display.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, SSD1306_WHITE);

    // Desenhar os blocos
    for(int i = 0; i < BRICKS_ROWS; i++) {
      for(int j = 0; j < BRICKS_COLS; j++) {
        if(bricks[i][j]) {
          display.fillRect(j * BRICK_WIDTH, i * BRICK_HEIGHT, BRICK_WIDTH - 1, BRICK_HEIGHT - 1, SSD1306_WHITE);
        }
      }
    }

    // Atualizar a tela
    display.display();

    // Pequeno delay para controlar a velocidade do jogo
    delay(10);
  }
}

void resetGame() {
  score = 0;  // Reiniciar a pontuação
  level = 1;  // Reiniciar o nível
  setup();    // Reiniciar o setup
}

void nextLevel() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.print("Level ");
  display.print(level);
  display.display();
  
  delay(1000);  // Pequena pausa para transição de nível

  // Aumentar a dificuldade
  ballSpeedX += (ballSpeedX > 0) ? 1 : -1;
  ballSpeedY += (ballSpeedY > 0) ? 1 : -1;

  initializeBricks();
  resetBallAndPaddle();
}

void initializeBricks() {
  for(int i = 0; i < BRICKS_ROWS; i++) {
    for(int j = 0; j < BRICKS_COLS; j++) {
      bricks[i][j] = true;  // Todos os blocos estão presentes no início
    }
  }
}

void resetBallAndPaddle() {
  ballX = SCREEN_WIDTH / 2;
  ballY = paddleY - BALL_SIZE;
  paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
}

void showPauseScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println("Paused");

  display.display();
}