#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PACMAN_SIZE 6  // Tamanho do Pac-Man (6x6 pixels)
#define GRID_SIZE 8    // Tamanho de cada célula do grid (8x8 pixels)

int pacmanX = GRID_SIZE * 1;
int pacmanY = GRID_SIZE * 1;
int directionX = 0;  // Direção em X
int directionY = 0;  // Direção em Y

int score = 0;  // Pontuação

// Labirinto com pontos (0: vazio, 1: parede, 2: ponto)
// Novo labirinto baseado na imagem enviada
int maze[8][16] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 2, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 1},
  {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 0, 1, 2, 0, 0, 2, 0, 2, 0, 2, 0, 2, 1, 2, 1},
  {1, 2, 1, 2, 0, 2, 0, 2, 0, 2, 0, 0, 2, 1, 0, 1},
  {1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
  {1, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 2, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

void setup() {
  pinMode(2, INPUT_PULLUP); // Esquerda
  pinMode(3, INPUT_PULLUP); // Cima
  pinMode(4, INPUT_PULLUP); // Baixo
  pinMode(5, INPUT_PULLUP); // Direita
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  // Desenhar labirinto e pontos
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      if (maze[y][x] == 1) {
        display.fillRect(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE); // Paredes
      } else if (maze[y][x] == 2) {
        display.drawCircle(x * GRID_SIZE + GRID_SIZE / 2, y * GRID_SIZE + GRID_SIZE / 2, 2, WHITE); // Pontos
      }
    }
  }

  // Ler direções e atualizar o movimento
  if (digitalRead(2) == LOW) { directionX = -1; directionY = 0; } // Esquerda
  if (digitalRead(3) == LOW) { directionX = 0; directionY = -1; } // Cima
  if (digitalRead(4) == LOW) { directionX = 0; directionY = 1; }  // Baixo
  if (digitalRead(5) == LOW) { directionX = 1; directionY = 0; }  // Direita

  // Movimentar Pac-Man
  int nextX = pacmanX + directionX * GRID_SIZE;
  int nextY = pacmanY + directionY * GRID_SIZE;

  if (canMove(nextX, nextY)) {
    pacmanX = nextX;
    pacmanY = nextY;

    // Verificar se Pac-Man comeu um ponto
    int gridX = pacmanX / GRID_SIZE;
    int gridY = pacmanY / GRID_SIZE;
    if (maze[gridY][gridX] == 2) {
      maze[gridY][gridX] = 0; // Remover o ponto
      score++; // Incrementar pontuação
    }
  }

  // Desenhar Pac-Man
  display.fillRect(pacmanX, pacmanY, PACMAN_SIZE, PACMAN_SIZE, WHITE);

  display.display();
  delay(200); // Ajuste o delay para controlar a velocidade do Pac-Man
}

bool canMove(int x, int y) {
  int gridX = x / GRID_SIZE;
  int gridY = y / GRID_SIZE;
  return maze[gridY][gridX] != 1;
}
