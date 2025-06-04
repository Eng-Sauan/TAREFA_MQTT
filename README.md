# 💡 Sistema de Automação IoT com Raspberry Pi Pico W e MQTT

Este projeto implementa um sistema embarcado usando o **Raspberry Pi Pico W** com conectividade **Wi-Fi** para controle e monitoramento remoto via **protocolo MQTT**. O sistema utiliza periféricos da **placa BitDogLab**, incluindo botões físicos, display OLED, matriz de LEDs, LED RGB e sensor de temperatura interno.

---

## 📋 Funcionalidades

- Conexão automática à rede Wi-Fi configurada
- Identificação única do dispositivo via ID da placa
- Comunicação segura com broker MQTT (com suporte a TLS)
- Publicação periódica da temperatura interna do microcontrolador
- Resposta a comandos MQTT como:
  - `/led`: ligar/desligar LEDs
  - `/print`: exibir mensagens no terminal
  - `/ping`: resposta imediata para testes de conectividade
  - `/exit`: encerra a conexão MQTT
- Controle local por botões físicos (com interrupção)
- Indicação de status via display OLED e LED RGB

---

## 🔧 Periféricos Utilizados (BitDogLab)

| Periférico        | Uso no Projeto                                     |
|-------------------|----------------------------------------------------|
| Wi-Fi             | Comunicação com broker MQTT                        |
| Botões            | Interrupção para acionar comandos físicos          |
| Display OLED      | Exibição de mensagens de status                    |
| Matriz de LEDs    | Inicializada para efeitos visuais                  |
| LED RGB           | Indicação de estados do sistema                    |
| Buzzer            | Emitir sons via MQTT                               |

