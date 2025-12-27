#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinos dos botões e do buzzer
const int buttonLeft = 2;
const int buttonRight = 5;
const int buttonShoot = 7;
const int buttonPause = 6;
const int buttonUp = 3;
const int buzzer = 11;

int playerX = 60;  // Posição inicial do jogador
int playerY = 56;  // Posição fixa do jogador

// Tiros
const int maxShots = 3;
bool shotActive[maxShots];
int shotX[maxShots];
int shotY[maxShots];
unsigned long lastShotTime = 0;
unsigned long shotCooldown = 200;  // Cooldown inicial

// Inimigos
const int enemyRows = 3;
const int enemyCols = 6;
bool enemyActive[enemyRows][enemyCols];
int enemyX[enemyRows][enemyCols];
int enemyY[enemyRows][enemyCols];
int enemyDirection = 1;  // 1 = direita, -1 = esquerda

// Fases e Dificuldade
int currentPhase = 1;
int enemySpeed = 1;  // Velocidade inicial dos inimigos

// Vidas e Pontuação
int lives = 3;
int score = 0;
bool gameOver = false;
bool paused = false;

// Power-Ups
bool powerUpActive = false;
int powerUpType = 0;  // 0 = nenhum, 1 = tiro duplo, 2 = tiro rápido, 3 = tiro explosivo
int powerUpX, powerUpY;
bool powerUpVisible = false;
unsigned long powerUpDuration = 10000;  // Duração do power-up em milissegundos
unsigned long powerUpStartTime = 0;

// Desenho da nave do jogador (8x6 pixels)
const uint8_t playerShip[] = {
  0b00111100,
  0b01111110,
  0b11111111,
  0b11111111,
  0b11111111,
  0b01111110
};

// Desenho do inimigo (8x6 pixels)
const uint8_t enemyIcon[] = {
  0b01100110,
  0b11111111,
  0b11011011,
  0b11111111,
  0b01000010,
  0b10000001
};

// Desenho do power-up (um simples quadrado 6x6)
const uint8_t powerUpIcon[] = {
  0b11111111,
  0b10000001,
  0b10000001,
  0b10000001,
  0b10000001,
  0b11111111
};

void setup() {
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonShoot, INPUT_PULLUP);
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
  resetGame();
  showStartScreen();
}

void loop() {
  if (!gameOver) {
    // Controle de movimento do jogador
    if (digitalRead(buttonLeft) == LOW && playerX > 0) {
      playerX -= 2;
    }
    if (digitalRead(buttonRight) == LOW && playerX < SCREEN_WIDTH - 8) {  // Ajuste para largura da nave
      playerX += 2;
    }

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

    // Controle de tiro com cadência ajustada
    if (digitalRead(buttonShoot) == LOW && (millis() - lastShotTime > shotCooldown)) {
      if (powerUpType == 1) {  // Tiro Duplo
        for (int i = 0; i < maxShots; i++) {
          if (!shotActive[i]) {
            shotActive[i] = true;
            shotX[i] = playerX;  // Ajuste para tiro duplo
            shotY[i] = playerY - 4;
            lastShotTime = millis();
            tone(buzzer, 1000, 100);
            break;
          }
        }
        for (int i = 0; i < maxShots; i++) {
          if (!shotActive[i]) {
            shotActive[i] = true;
            shotX[i] = playerX + 6;  // Ajuste para tiro duplo
            shotY[i] = playerY - 4;
            lastShotTime = millis();
            tone(buzzer, 1000, 100);
            break;
          }
        }
      } else {  // Tiro Normal e Tiro Explosivo
        for (int i = 0; i < maxShots; i++) {
          if (!shotActive[i]) {
            shotActive[i] = true;
            shotX[i] = playerX + 3;  // Ajuste para centralizar o tiro
            shotY[i] = playerY - 4;
            lastShotTime = millis();
            tone(buzzer, 1000, 100);
            break;
          }
        }
      }
    }

    // Movimento dos tiros
    for (int i = 0; i < maxShots; i++) {
      if (shotActive[i]) {
        shotY[i] -= 4;
        if (shotY[i] < 0) {
          shotActive[i] = false;
        }
      }
    }

    // Movimento dos inimigos
    bool changeDirection = false;
    for (int row = 0; row < enemyRows; row++) {
      for (int col = 0; col < enemyCols; col++) {
        if (enemyActive[row][col]) {
          enemyX[row][col] += enemyDirection * enemySpeed;
          if (enemyX[row][col] <= 0 || enemyX[row][col] >= SCREEN_WIDTH - 8) {  // Ajuste para largura do inimigo
            changeDirection = true;
          }
        }
      }
    }
    if (changeDirection) {
      enemyDirection = -enemyDirection;
      for (int row = 0; row < enemyRows; row++) {
        for (int col = 0; col < enemyCols; col++) {
          if (enemyActive[row][col]) {
            enemyY[row][col] += 2;
                // Verifica se algum inimigo colidiu com o jogador
            if (enemyY[row][col] >= playerY) {
                    // Agora, ao invés de acabar o jogo, apenas perde uma vida e reinicia a fase
              loseLife();
              if (!gameOver) {
                resetPhase();  // Reinicia a fase ao perder uma vida
              }
              return;  // Sai da função loop para evitar continuar com o ciclo
            }
          }
        }
      }
    }

    // Detecção de colisão com tiros
    for (int i = 0; i < maxShots; i++) {
      if (shotActive[i]) {
        for (int row = 0; row < enemyRows; row++) {
          for (int col = 0; col < enemyCols; col++) {
            if (enemyActive[row][col] && shotX[i] > enemyX[row][col] && shotX[i] < enemyX[row][col] + 8 &&
                shotY[i] > enemyY[row][col] && shotY[i] < enemyY[row][col] + 6) {
              enemyActive[row][col] = false;
              shotActive[i] = false;
              score += 10;
              tone(buzzer, 1500, 100);  // Som ao atingir um inimigo
              
              if (powerUpType == 3) {  // Tiro Explosivo
                // Destrói inimigos próximos
                for (int r = row - 1; r <= row + 1; r++) {
                  for (int c = col - 1; c <= col + 1; c++) {
                    if (r >= 0 && r < enemyRows && c >= 0 && c < enemyCols) {
                      enemyActive[r][c] = false;
                      score += 5;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    // Verifica se todos os inimigos foram destruídos
    bool allEnemiesDestroyed = true;
    for (int row = 0; row < enemyRows; row++) {
      for (int col = 0; col < enemyCols; col++) {
        if (enemyActive[row][col]) {
          allEnemiesDestroyed = false;
          break;
        }
      }
    }

    if (allEnemiesDestroyed) {
      currentPhase++;
      enemySpeed += 1; 
      resetEnemies();
      showPhaseTransitionScreen(currentPhase);
    }

    // Geração de Power-Ups
    if (!powerUpVisible && random(1000) < 5) {  // 0.5% de chance por ciclo
      powerUpX = random(SCREEN_WIDTH - 6);
      powerUpY = 0;
      powerUpType = random(1, 4);  // Gera um tipo de power-up aleatório (1 a 3)
      powerUpVisible = true;
    }

    // Movimento e coleta de Power-Ups
    if (powerUpVisible) {
      powerUpY += 2;
      if (powerUpY > SCREEN_HEIGHT) {
        powerUpVisible = false;  // Desaparece se sair da tela
      }
      if (powerUpX > playerX && powerUpX < playerX + 8 && powerUpY > playerY && powerUpY < playerY + 6) {
        powerUpActive = true;
        powerUpStartTime = millis();
        powerUpVisible = false;
        shotCooldown = 100;  // Reduz o tempo de espera para tiro rápido (ajuste conforme necessário)
      }
    }

    // Verifica se o Power-Up expirou
    if (powerUpActive && millis() - powerUpStartTime > powerUpDuration) {
      powerUpActive = false;
      powerUpType = 0;  // Volta ao tiro normal
      shotCooldown = 200;  // Restaura o cooldown padrão
    }

    // Desenho na tela
    display.clearDisplay();

    // Desenho da nave do jogador
    display.drawBitmap(playerX, playerY, playerShip, 8, 6, WHITE);

    // Desenho dos tiros
    for (int i = 0; i < maxShots; i++) {
      if (shotActive[i]) {
        display.fillRect(shotX[i], shotY[i], 2, 4, WHITE);
      }
    }

    // Desenho dos inimigos
    for (int row = 0; row < enemyRows; row++) {
      for (int col = 0; col < enemyCols; col++) {
        if (enemyActive[row][col]) {
          display.drawBitmap(enemyX[row][col], enemyY[row][col], enemyIcon, 8, 6, WHITE);
        }
      }
    }

    // Desenho dos Power-Ups
    if (powerUpVisible) {
      display.drawBitmap(powerUpX, powerUpY, powerUpIcon, 6, 6, WHITE);
    }

    // Desenho das informações (vidas e pontuação)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Score: ");
    display.print(score);
    display.setCursor(80, 0);
    display.print("Lives: ");
    display.print(lives);

    display.display();
    delay(50);  // Pequeno delay para suavizar o jogo
  } else {
    showGameOverScreen();
  }
}

void resetGame() {
  resetEnemies();
  playerX = 60;  // Reposiciona o jogador
  lives = 3;
  score = 0;
  gameOver = false;
}

void resetEnemies() {
  for (int row = 0; row < enemyRows; row++) {
    for (int col = 0; col < enemyCols; col++) {
      enemyActive[row][col] = true;
      enemyX[row][col] = col * 16;
      enemyY[row][col] = row * 10;
    }
  }
  enemyDirection = 1;
}

void loseLife() {
  lives--;
  tone(buzzer, 400, 500);  // Som ao perder uma vida
  if (lives <= 0) {
    gameOver = true;
  }
}

void showPhaseTransitionScreen(int phase) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(25, 20);
  display.println("Phase " + String(phase));

  display.display();
  delay(2000);  // Pausa para mostrar a tela de transição
  display.clearDisplay();
  display.display();
}

void showGameOverScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Game Over");

  display.setTextSize(1);
  display.setCursor(10, 40);
  display.print("Final Score: ");
  display.print(score);

  display.setCursor(10, 50);
  display.print("Press UP to Restart");

  display.display();

  while (digitalRead(buttonUp) == HIGH) {
    delay(50);
  }

  currentPhase = 1;  // Reinicia para a primeira fase
  lives = 3;  // Reinicia as vidas
  score = 0;  // Reinicia a pontuação
  resetGame();
  display.clearDisplay();
  display.display();
}

void showStartScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.println("SPACE INVADERS");
  display.setCursor(15, 30);
  display.println("Press UP");
  display.setCursor(15, 50);
  display.println("to Start");

  display.display();

  while (digitalRead(buttonUp) == HIGH) {
    delay(50);
  }
  
  display.clearDisplay();
  display.display();
}

void resetPhase() {
    resetEnemies();
    powerUpActive = false;  // Desativa qualquer power-up ativo
    powerUpVisible = false; // Remove qualquer power-up visível
    shotCooldown = 200;     // Reseta o cooldown do tiro
    powerUpType = 0;        // Reseta para tiro normal
}

void showPauseScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 20);
  display.println("Paused");

  display.display();
}
