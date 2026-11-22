# Robô 2WD com ESP32

Este módulo contém a implementação de controle de um robô com tração diferencial de duas rodas (2WD) utilizando o microcontrolador **ESP32** e a ponte H **L298N**.

---

## 📋 Sumário
1. [Pinagem e Ligações](#-pinagem-e-ligações)
2. [Código Principal (`robo2wd.ino`)](#-código-principal-robo2wdino)
3. [Dicas de Hardware e Melhoria com Capacitores](#-dicas-de-hardware-e-melhoria-com-capacitores)
4. [Alternativa A: Bluetooth Serial Clássico (SPP)](#-alternativa-a-bluetooth-serial-clássico-spp)
5. [Alternativa B: Controle via Ponto de Acesso Wi-Fi (Web Server)](#-alternativa-b-controle-via-ponto-de-acesso-wi-fi-web-server)

---

## 🔌 Pinagem e Ligações

Abaixo está o mapeamento padrão dos pinos entre o **ESP32** e a **Ponte H L298N**:

| Função | Pino Ponte H (L298N) | GPIO ESP32 | Descrição |
| :--- | :---: | :---: | :--- |
| **PWM Motor Esquerdo** | `ENA` | `GPIO 25` | Controle de velocidade (LEDC Canal 0) |
| **Sentido Motor Esquerdo** | `IN1` | `GPIO 26` | Direção de giro do motor esquerdo |
| **Sentido Motor Esquerdo** | `IN2` | `GPIO 27` | Direção de giro do motor esquerdo |
| **PWM Motor Direito** | `ENB` | `GPIO 14` | Controle de velocidade (LEDC Canal 1) |
| **Sentido Motor Direito** | `IN3` | `GPIO 33` | Direção de giro do motor direito |
| **Sentido Motor Direito** | `IN4` | `GPIO 32` | Direção de giro do motor direito |
| **Referência Elétrica** | `GND` | `GND` | **Obrigatório:** Interligar com GND da bateria |

> [!IMPORTANT]
> Lembre-se de remover os *jumpers* dos pinos `ENA` e `ENB` da ponte H L298N para poder controlar a velocidade dos motores via PWM.

---

## 🕹️ Código Principal (`robo2wd.ino`)

O código principal [robo2wd.ino](robo2wd.ino) utiliza a biblioteca **DabbleESP32** para controle via smartphone pelo módulo **GamePad** do aplicativo Dabble:

- **Direcionais (Cima, Baixo, Esquerda, Direita)**: Movem o robô.
- **Botão Quadrado**: Aumenta a velocidade do PWM.
- **Botão Círculo**: Diminui a velocidade do PWM.
- **Otimizações inclusas**:
  - Máquina de estados que impede o envio contínuo e repetido na porta serial (*Serial flooding*), evitando quedas de comunicação.
  - Temporização não bloqueante com `millis()` para o ajuste de velocidade, eliminando travamentos de ciclo com `delay()`.

---

## ⚡ Dicas de Hardware e Melhoria com Capacitores

Muitos problemas de desconexão repentina de Bluetooth/Wi-Fi no ESP32 ocorrem por interferências elétricas ou quedas de tensão causadas pelos motores DC.

### 1. Adição de Capacitores de Desacoplamento nos Motores (Recomendado)
Motores DC geram faiscamento interno nas escovas, emitindo ruído eletromagnético (EMI) de alta frequência que se propaga pela fiação e atinge o rádio do ESP32, derrubando a conexão.

- **Solução**: Solde um **capacitor cerâmico de 100 nF (código 104)** diretamente nos dois terminais de cada motor DC.
- Para uma filtragem ainda mais robusta, solde três capacitores de 100 nF por motor:
  1. Um entre os dois terminais do motor.
  2. Um entre o terminal positivo e a carcaça metálica do motor.
  3. Um entre o terminal negativo e a carcaça metálica do motor.

```
      +---[ Capacitor 100nF ]---+
      |                         |
  Terminal (+) ------------ Terminal (-)
      |                         |
  [Cap. 100nF]              [Cap. 100nF]
      |                         |
      +----- Carcaça Metal -----+
```

### 2. Alimentação Adequada
- **Não use bateria retangular de 9V**: Elas possuem alta resistência interna e não fornecem a corrente instantânea exigida pelos motores, causando *Brownout Reset* (o ESP32 reinicia ou perde conexão).
- **Alimentação Ideal**:
  - Duas células de lítio **18650 em série (7.4V - 8.4V)** para a ponte H e motores.
  - Alimentar o ESP32 via cabo micro-USB / *powerbank*, ou através de um conversor Step-Down (como o LM2596) regulado em 5V ligado ao pino `VIN` do ESP32.
- **GND Comum**: Todos os terras (GND da bateria, GND da ponte H e GND do ESP32) **devem estar obrigatoriamente conectados entre si**.

---

## 🔄 Alternativa A: Bluetooth Serial Clássico (SPP)

Se houver incompatibilidade com a biblioteca Dabble ou com BLE, a melhor solução é usar o **Bluetooth Serial Nativo** do ESP32. Não requer nenhuma biblioteca externa e funciona com diversos aplicativos gratuitos da Play Store.

### 📱 Como controlar? Preciso criar um aplicativo?
**Não precisa programar nenhum aplicativo!** Existem diversos apps gratuitos e prontos na Google Play Store.

Os mais recomendados são:

1. **Bluetooth RC Controller** (Recomendado para robôs de carrinho):
   - Já vem de fábrica configurado para enviar os mesmos caracteres que o código espera:
     - `F` (Frente), `B` (Ré), `L` (Esquerda), `R` (Direita), `S` (Parar ao soltar o dedo).
     - Velocidades de `0` a `9`.
   - Basta parear o ESP32 nas configurações de Bluetooth do celular, abrir o app e conectar no `Robo_2WD_BT`.

2. **Arduino Bluetooth Controller** (Giumig Apps):
   - Possui o modo **"Controller Mode"** com layout de joystick/gamepad.
   - Ao abrir pela primeira vez, toque na engrenagem de configurações e associe:
     - Botão Cima: caractere `F`
     - Botão Baixo: caractere `B`
     - Botão Esquerda: caractere `L`
     - Botão Direita: caractere `R`
     - Ao soltar o botão (*On release*): caractere `S`

3. **Serial Bluetooth Terminal** (Kai Morich):
   - Excelente para testes. Permite enviar comandos digitando e salvar botões de atalho rápidos (macros).

> [!NOTE]
> *(Caso queira criar o seu próprio app no futuro)*: você pode usar o **[MIT App Inventor](https://appinventor.mit.edu/)**, que permite montar um aplicativo Android por blocos visuais de arrastar e soltar em poucos minutos, sem precisar digitar código Java/Kotlin.

### Código de Exemplo (`bluetooth_serial_2wd.ino`):
```cpp
#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Definição dos pinos
#define MOTOR_ESQ_ENA 25
#define MOTOR_ESQ_IN1 26
#define MOTOR_ESQ_IN2 27
#define MOTOR_DIR_ENB 14
#define MOTOR_DIR_IN3 33
#define MOTOR_DIR_IN4 32

#define CANAL_PWM_ESQ 0
#define CANAL_PWM_DIR 1
int velocidade = 190;

void parar() {
  ledcWrite(CANAL_PWM_ESQ, 0);
  ledcWrite(CANAL_PWM_DIR, 0);
  digitalWrite(MOTOR_ESQ_IN1, LOW);
  digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW);
  digitalWrite(MOTOR_DIR_IN4, LOW);
}

void moverFrente() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW);  digitalWrite(MOTOR_DIR_IN4, HIGH);
  ledcWrite(CANAL_PWM_ESQ, velocidade);
  ledcWrite(CANAL_PWM_DIR, velocidade);
}

void moverTras() {
  digitalWrite(MOTOR_ESQ_IN1, LOW);  digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
  ledcWrite(CANAL_PWM_ESQ, velocidade);
  ledcWrite(CANAL_PWM_DIR, velocidade);
}

void girarEsquerda() {
  digitalWrite(MOTOR_ESQ_IN1, LOW);  digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, LOW);  digitalWrite(MOTOR_DIR_IN4, HIGH);
  ledcWrite(CANAL_PWM_ESQ, velocidade);
  ledcWrite(CANAL_PWM_DIR, velocidade);
}

void girarDireita() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
  ledcWrite(CANAL_PWM_ESQ, velocidade);
  ledcWrite(CANAL_PWM_DIR, velocidade);
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Robo_2WD_BT"); // Nome Bluetooth visível no celular

  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);

  ledcSetup(CANAL_PWM_ESQ, 1000, 8);
  ledcSetup(CANAL_PWM_DIR, 1000, 8);
  ledcAttachPin(MOTOR_ESQ_ENA, CANAL_PWM_ESQ);
  ledcAttachPin(MOTOR_DIR_ENB, CANAL_PWM_DIR);

  parar();
  Serial.println("Bluetooth pronto. Pareie com 'Robo_2WD_BT'.");
}

void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    switch (cmd) {
      case 'F': moverFrente(); break;
      case 'B': moverTras(); break;
      case 'L': girarEsquerda(); break;
      case 'R': girarDireita(); break;
      case 'S': parar(); break;
      case '1': velocidade = 120; break;
      case '2': velocidade = 180; break;
      case '3': velocidade = 255; break;
    }
  }
}
```

---

## 🌐 Alternativa B: Controle via Ponto de Acesso Wi-Fi (Web Server)

Esta solução transforma o ESP32 em um roteador Wi-Fi próprio (*Access Point*). **Não precisa instalar nenhum aplicativo**:
1. Conecte o celular ou computador à rede Wi-Fi criada pelo robô (`Robo_2WD_WiFi`).
2. Abra qualquer navegador (Chrome, Safari, Firefox) no endereço `http://192.168.4.1`.
3. Controle o robô diretamente através de botões na tela.

### Código de Exemplo (`wifi_ap_web_2wd.ino`):
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Robo_2WD_WiFi";
const char* password = ""; // Rede aberta (ou defina uma senha com min. 8 caracteres)

WebServer server(80);

#define MOTOR_ESQ_ENA 25
#define MOTOR_ESQ_IN1 26
#define MOTOR_ESQ_IN2 27
#define MOTOR_DIR_ENB 14
#define MOTOR_DIR_IN3 33
#define MOTOR_DIR_IN4 32

#define CANAL_PWM_ESQ 0
#define CANAL_PWM_DIR 1
int velocidade = 200;

void parar() {
  ledcWrite(CANAL_PWM_ESQ, 0); ledcWrite(CANAL_PWM_DIR, 0);
  digitalWrite(MOTOR_ESQ_IN1, LOW); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW); digitalWrite(MOTOR_DIR_IN4, LOW);
}

void moverFrente() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, LOW);  digitalWrite(MOTOR_DIR_IN4, HIGH);
  ledcWrite(CANAL_PWM_ESQ, velocidade); ledcWrite(CANAL_PWM_DIR, velocidade);
}

void moverTras() {
  digitalWrite(MOTOR_ESQ_IN1, LOW);  digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
  ledcWrite(CANAL_PWM_ESQ, velocidade); ledcWrite(CANAL_PWM_DIR, velocidade);
}

void girarEsquerda() {
  digitalWrite(MOTOR_ESQ_IN1, LOW);  digitalWrite(MOTOR_ESQ_IN2, HIGH);
  digitalWrite(MOTOR_DIR_IN3, LOW);  digitalWrite(MOTOR_DIR_IN4, HIGH);
  ledcWrite(CANAL_PWM_ESQ, velocidade); ledcWrite(CANAL_PWM_DIR, velocidade);
}

void girarDireita() {
  digitalWrite(MOTOR_ESQ_IN1, HIGH); digitalWrite(MOTOR_ESQ_IN2, LOW);
  digitalWrite(MOTOR_DIR_IN3, HIGH); digitalWrite(MOTOR_DIR_IN4, LOW);
  ledcWrite(CANAL_PWM_ESQ, velocidade); ledcWrite(CANAL_PWM_DIR, velocidade);
}

// Interface Web responsiva enviada ao navegador
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Controle Robo 2WD</title>
  <style>
    body { font-family: Arial; text-align: center; margin: 0; padding-top: 30px; background: #1e1e2f; color: #fff; }
    h1 { margin-bottom: 25px; }
    .btn { display: inline-block; width: 85px; height: 85px; font-size: 26px; margin: 8px; border: none; border-radius: 12px; background: #3b82f6; color: white; cursor: pointer; user-select: none; }
    .btn:active { background: #1d4ed8; transform: scale(0.95); }
    .stop { background: #ef4444; }
    .grid { display: grid; grid-template-columns: repeat(3, 95px); justify-content: center; margin-top: 10px; }
  </style>
</head>
<body>
  <h1>Controle Robô 2WD</h1>
  <div class="grid">
    <div></div><button class="btn" onclick="sendCmd('frente')">▲</button><div></div>
    <button class="btn" onclick="sendCmd('esquerda')">◄</button>
    <button class="btn stop" onclick="sendCmd('parar')">■</button>
    <button class="btn" onclick="sendCmd('direita')">►</button>
    <div></div><button class="btn" onclick="sendCmd('tras')">▼</button><div></div>
  </div>
  <script>
    function sendCmd(cmd) {
      fetch('/' + cmd);
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() { server.send_P(200, "text/html", HTML_PAGE); }
void handleFrente() { moverFrente(); server.send(200, "text/plain", "OK"); }
void handleTras() { moverTras(); server.send(200, "text/plain", "OK"); }
void handleEsquerda() { girarEsquerda(); server.send(200, "text/plain", "OK"); }
void handleDireita() { girarDireita(); server.send(200, "text/plain", "OK"); }
void handleParar() { parar(); server.send(200, "text/plain", "OK"); }

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_ESQ_IN1, OUTPUT);
  pinMode(MOTOR_ESQ_IN2, OUTPUT);
  pinMode(MOTOR_DIR_IN3, OUTPUT);
  pinMode(MOTOR_DIR_IN4, OUTPUT);

  ledcSetup(CANAL_PWM_ESQ, 1000, 8);
  ledcSetup(CANAL_PWM_DIR, 1000, 8);
  ledcAttachPin(MOTOR_ESQ_ENA, CANAL_PWM_ESQ);
  ledcAttachPin(MOTOR_DIR_ENB, CANAL_PWM_DIR);
  parar();

  WiFi.softAP(ssid, password);
  Serial.print("Ponto de acesso criado: "); Serial.println(ssid);
  Serial.print("IP para acesso no navegador: "); Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/frente", handleFrente);
  server.on("/tras", handleTras);
  server.on("/esquerda", handleEsquerda);
  server.on("/direita", handleDireita);
  server.on("/parar", handleParar);

  server.begin();
}

void loop() {
  server.handleClient();
}
```
