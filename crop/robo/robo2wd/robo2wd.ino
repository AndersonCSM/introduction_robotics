#include <Arduino.h>
#include <DabbleESP32.h>

/* ============================================================================
 * PROJETO: Robô 2WD com ESP32 controlado via Bluetooth (Dabble)
 * OBJETIVO: Código didático, otimizado e comentado para iniciantes em robótica.
 * ============================================================================
 * ESQUEMA DE LIGAÇÃO (Ponte H L298N ao ESP32):
 * - Motor Esquerdo: ENA -> GPIO 25 | IN1 -> GPIO 26 | IN2 -> GPIO 27
 * - Motor Direito:  ENB -> GPIO 14 | IN3 -> GPIO 33 | IN4 -> GPIO 32
 * - Alimentação: GND do ESP32 DEVE estar conectado ao GND da Ponte H e da Bateria.
 * ============================================================================
 */

// ----------------------------------------------------------------------------
// PINOS DOS MOTORES
// ----------------------------------------------------------------------------
#define MOTOR_ESQ_ENA 25  // Controle de velocidade do motor esquerdo (PWM)
#define MOTOR_ESQ_IN1 26  // Sentido do motor esquerdo
#define MOTOR_ESQ_IN2 27  // Sentido do motor esquerdo

#define MOTOR_DIR_ENB 14  // Controle de velocidade do motor direito (PWM)
#define MOTOR_DIR_IN3 33  // Sentido do motor direito
#define MOTOR_DIR_IN4 32  // Sentido do motor direito

// ----------------------------------------------------------------------------
// CONFIGURAÇÕES DO PWM (Modulação por Largura de Pulso)
// O ESP32 usa o periférico LEDC para gerar sinais PWM com precisão.
// ----------------------------------------------------------------------------
#define CANAL_PWM_ESQ   0     // Canal 0 do ESP32 para o motor esquerdo
#define CANAL_PWM_DIR   1     // Canal 1 do ESP32 para o motor direito
#define FREQUENCIA_PWM  1000  // 1 kHz (ótimo para motores DC comuns)
#define RESOLUCAO_PWM   8     // 8 bits: valores de velocidade vão de 0 a 255

// ----------------------------------------------------------------------------
// PARÂMETROS DE VELOCIDADE
// ----------------------------------------------------------------------------
int pwmValor = 180;          // Velocidade inicial (0 = parado, 255 = máximo)
const int PWM_MIN = 80;      // Mínimo para vencer o atrito dos motores
const int PWM_MAX = 255;     // Velocidade máxima permitida
const int PWM_PASSO = 20;    // Quanto a velocidade varia a cada toque

// ----------------------------------------------------------------------------
// CONTROLE DE ESTADOS (Para evitar travar o Bluetooth com prints repetidos)
// ----------------------------------------------------------------------------
enum EstadoMovimento {
  PARADO,
  FRENTE,
  TRAS,
  ESQUERDA,
  DIREITA
};

EstadoMovimento estadoAtual = PARADO;
EstadoMovimento ultimoEstado = PARADO;
int ultimoPwmImpresso = -1;

// Controle de tempo para botões de velocidade sem usar delay()
unsigned long tempoUltimoAjusteVel = 0;
const unsigned long INTERVALO_AJUSTE = 250; // ms

// ============================================================================
// FUNÇÕES DE ACIONAMENTO DIRETO DOS MOTORES
// ============================================================================

/**
 * Controla o sentido e velocidade do motor esquerdo.
 * @param frente: true para avançar, false para recuar.
 * @param pwm: intensidade da velocidade (0 a 255).
 */
void acionarMotorEsquerdo(bool frente, int pwm) {
  if (frente) {
    digitalWrite(MOTOR_ESQ_IN1, HIGH);
    digitalWrite(MOTOR_ESQ_IN2, LOW);
  } else {
    digitalWrite(MOTOR_ESQ_IN1, LOW);
    digitalWrite(MOTOR_ESQ_IN2, HIGH);
  }
  ledcWrite(CANAL_PWM_ESQ, pwm);
}

/**
 * Controla o sentido e velocidade do motor direito.
 * @param frente: true para avançar, false para recuar.
 * @param pwm: intensidade da velocidade (0 a 255).
 */
void acionarMotorDireito(bool frente, int pwm) {
  if (frente) {
    digitalWrite(MOTOR_DIR_IN3, LOW);
    digitalWrite(MOTOR_DIR_IN4, HIGH);
  } else {
    digitalWrite(MOTOR_DIR_IN3, HIGH);
    digitalWrite(MOTOR_DIR_IN4, LOW);
  }
  ledcWrite(CANAL_PWM_DIR, pwm);
}

// ============================================================================
// FUNÇÕES DE MOVIMENTAÇÃO DE ALTO NÍVEL
// ============================================================================

void moverFrente() {
  acionarMotorEsquerdo(true, pwmValor);
  acionarMotorDireito(true, pwmValor);
}

void moverTras() {
  acionarMotorEsquerdo(false, pwmValor);
  acionarMotorDireito(false, pwmValor);
}

void girarDireita() {
  acionarMotorEsquerdo(true, pwmValor);
  acionarMotorDireito(false, pwmValor);
}

void girarEsquerda() {
  acionarMotorEsquerdo(false, pwmValor);
  acionarMotorDireito(true, pwmValor);
}

void pararRobo() {
  ledcWrite(CANAL_PWM_ESQ, 0);
  ledcWrite(CANAL_PWM_DIR, 0);
  digitalWrite(MOTOR_ESQ_IN1, LOW);
  digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW);
  digitalWrite(MOTOR_DIR_IN4, LOW);
}

/**
 * Imprime na Serial APENAS quando o estado do robô ou velocidade mudam.
 * IMPORTANTE: Nunca envie Serial continuamente no loop(), pois isso enche o buffer
 * e faz a conexão Bluetooth cair por falta de tempo de processamento.
 */
void atualizarMonitorSerial() {
  if (estadoAtual != ultimoEstado || pwmValor != ultimoPwmImpresso) {
    switch (estadoAtual) {
      case FRENTE:
        Serial.print("Movimento: Frente | PWM: ");
        Serial.println(pwmValor);
        break;
      case TRAS:
        Serial.print("Movimento: Ré | PWM: ");
        Serial.println(pwmValor);
        break;
      case DIREITA:
        Serial.print("Movimento: Direita | PWM: ");
        Serial.println(pwmValor);
        break;
      case ESQUERDA:
        Serial.print("Movimento: Esquerda | PWM: ");
        Serial.println(pwmValor);
        break;
      case PARADO:
        Serial.println("Movimento: Parado");
        break;
    }
    ultimoEstado = estadoAtual;
    ultimoPwmImpresso = pwmValor;
  }
}

// ============================================================================
// CONFIGURAÇÃO INICIAL (SETUP)
// ============================================================================
void setup() {
  Serial.begin(115200);

  // Inicializa o módulo Bluetooth do Dabble
  Dabble.begin("Robo_2WD_ESP32");

  // Configura os pinos de controle de direção como saídas
  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);

  // Configuração do PWM para o ESP32 (Core 2.x)
  // Caso use a versão 3.0+ da placa ESP32, utilize ledcAttach(pino, freq, res)
  ledcSetup(CANAL_PWM_ESQ, FREQUENCIA_PWM, RESOLUCAO_PWM);
  ledcSetup(CANAL_PWM_DIR, FREQUENCIA_PWM, RESOLUCAO_PWM);
  ledcAttachPin(MOTOR_ESQ_ENA, CANAL_PWM_ESQ);
  ledcAttachPin(MOTOR_DIR_ENB, CANAL_PWM_DIR);

  pararRobo();

  Serial.println("----------------------------------------------");
  Serial.println("Robô 2WD pronto e aguardando conexão Bluetooth!");
  Serial.println("Abra o app Dabble no celular e conecte no 'Robo_2WD_ESP32'.");
  Serial.println("Controles: Direcionais para mover | Quadrado/Círculo para velocidade.");
  Serial.println("----------------------------------------------");
}

// ============================================================================
// CICLO PRINCIPAL (LOOP)
// ============================================================================
void loop() {
  // Atualiza as mensagens recebidas via Bluetooth (obrigatório a cada ciclo)
  Dabble.processInput();

  unsigned long agora = millis();

  // 1. AJUSTE DE VELOCIDADE (Sem travar o processamento com delay)
  if (agora - tempoUltimoAjusteVel >= INTERVALO_AJUSTE) {
    if (GamePad.isSquarePressed()) {
      pwmValor += PWM_PASSO;
      if (pwmValor > PWM_MAX) pwmValor = PWM_MAX;
      tempoUltimoAjusteVel = agora;
    } 
    else if (GamePad.isCirclePressed()) {
      pwmValor -= PWM_PASSO;
      if (pwmValor < PWM_MIN) pwmValor = PWM_MIN;
      tempoUltimoAjusteVel = agora;
    }
  }

  // 2. LEITURA DOS DIRECIONAIS DO GAMEPAD
  if (GamePad.isUpPressed()) {
    moverFrente();
    estadoAtual = FRENTE;
  } 
  else if (GamePad.isDownPressed()) {
    moverTras();
    estadoAtual = TRAS;
  } 
  else if (GamePad.isRightPressed()) {
    girarDireita();
    estadoAtual = DIREITA;
  } 
  else if (GamePad.isLeftPressed()) {
    girarEsquerda();
    estadoAtual = ESQUERDA;
  } 
  else {
    pararRobo();
    estadoAtual = PARADO;
  }

  // 3. ATUALIZA A SERIAL DE FORMA OTIMIZADA (somente quando há mudança)
  atualizarMonitorSerial();
}

