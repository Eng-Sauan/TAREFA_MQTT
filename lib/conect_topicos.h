#ifndef CONECT_TOPICOS_H
#define CONECT_TOPICOS_H

#include "lib/mqtt_client.h"
#include "lib/matrixws.h"
#include "lib/servo.h"
#include "inc/display.h"

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
#if MQTT_UNIQUE_TOPIC
    const char *basic_topic = state->topic + strlen(state->mqtt_client_info.client_id) + 1;
#else
    const char *basic_topic = state->topic;
#endif
    strncpy(state->data, (const char *)data, len);
    state->len = len;
    state->data[len] = '\0';

    INFO_printf("[DEBUG] Recebido comando no tópico %s: %s\n", basic_topic, state->data);
    ssd1306_fill(&ssd, false);
    escrever(&ssd, basic_topic, 5, 10, cor);
    DEBUG_printf("Topic: %s, Message: %s\n", state->topic, state->data);
    // Organize topic handling using a switch for better structure
    enum topic_cmd {
        TOPIC_LED,
        TOPIC_PRINT,
        TOPIC_MATRIZ,
        TOPIC_PING,
        TOPIC_EXIT,
        TOPIC_SENSORES,
        TOPIC_SERVO,
        TOPIC_UNKNOWN
    } topic = TOPIC_UNKNOWN;

    // Corrige: aceita comandos em todos os tópicos válidos
    if (strcmp(basic_topic, "/led") == 0) topic = TOPIC_LED;
    else if (strcmp(basic_topic, "/print") == 0) topic = TOPIC_PRINT;
    else if (strcmp(basic_topic, "/matriz") == 0) topic = TOPIC_MATRIZ;
    else if (strcmp(basic_topic, "/ping") == 0) topic = TOPIC_PING;
    else if (strcmp(basic_topic, "/exit") == 0) topic = TOPIC_EXIT;
    else if (strcmp(basic_topic, "/sensores") == 0) topic = TOPIC_SENSORES;
    else if (strcmp(basic_topic, "/servo") == 0) topic = TOPIC_SERVO;
    else {
        INFO_printf("[DEBUG] Tópico desconhecido: %s\n", basic_topic);
        return; // Não processa tópicos desconhecidos
    }

    switch (topic) {
        case TOPIC_LED: {
            INFO_printf("[DEBUG] Comando recebido: %s\n", state->data);
            // Subcomandos de LED
            enum led_cmd {
                LED_CMD_NONE,
                LED_CMD_RED_ON,
                LED_CMD_RED_OFF,
                LED_CMD_BLUE_ON,
                LED_CMD_BLUE_OFF,
                LED_CMD_GREEN_ON,
                LED_CMD_GREEN_OFF,
                LED_CMD_SIMPLE_ON,
                LED_CMD_SIMPLE_OFF,
                LED_CMD_OFF
            } cmd = LED_CMD_NONE;

                if (strcmp((const char *)state->data, "led red on") == 0) cmd = LED_CMD_RED_ON;
                else if (strcmp((const char *)state->data, "led red off") == 0) cmd = LED_CMD_RED_OFF;
                else if (strcmp((const char *)state->data, "led blue on") == 0) cmd = LED_CMD_BLUE_ON;
                else if (strcmp((const char *)state->data, "led blue off") == 0) cmd = LED_CMD_BLUE_OFF;
                else if (strcmp((const char *)state->data, "led green on") == 0) cmd = LED_CMD_GREEN_ON;
                else if (strcmp((const char *)state->data, "led green off") == 0) cmd = LED_CMD_GREEN_OFF;
                else if (lwip_stricmp((const char *)state->data, "On") == 0 || strcmp((const char *)state->data, "1") == 0) cmd = LED_CMD_SIMPLE_ON;
                else if (lwip_stricmp((const char *)state->data, "Off") == 0 || strcmp((const char *)state->data, "0") == 0) cmd = LED_CMD_SIMPLE_OFF;
                else if (strcmp((const char *)state->data, "leds off") == 0) cmd = LED_CMD_OFF;

            switch (cmd) {
                case LED_CMD_RED_ON:
                    control_red_led(true);
                    //ssd1306_fill(&ssd, false);
                    escrever(&ssd, "led red on", 5, 20, cor);
                    break;
                case LED_CMD_RED_OFF:
                    control_red_led(false);
                    escrever(&ssd, "led red off", 5, 20, cor);
                    break;
                case LED_CMD_BLUE_ON:
                    control_blue_led(true);
                    escrever(&ssd, "led blue on", 5, 20, cor);
                    break;
                case LED_CMD_BLUE_OFF:
                    control_blue_led(false);
                    escrever(&ssd, "led blue off", 5, 20, cor);
                    break;
                case LED_CMD_GREEN_ON:
                    control_green_led(true);
                    escrever(&ssd, "led green on", 5, 20, cor);
                    break;
                case LED_CMD_GREEN_OFF:
                    control_green_led(false);
                    escrever(&ssd, "led green off", 5, 20, cor);
                    break;
                case LED_CMD_SIMPLE_ON:
                    control_led(state, true);
                    escrever(&ssd, "led on", 5, 20, cor);
                    break;
                case LED_CMD_SIMPLE_OFF:
                    control_led(state, false);
                    escrever(&ssd, "led off", 5, 20, cor);
                    break;
                case LED_CMD_OFF:
                    control_red_led(false);
                    control_blue_led(false);
                    control_green_led(false);
                    escrever(&ssd, "leds off", 5, 20, cor);
                    break;
                default:
                    INFO_printf("[DEBUG] Comando de LED desconhecido: %s\n", state->data);
                    escrever(&ssd, "desconhecido", 5, 20, cor);
                    break;
            }
            break;
        }
        case TOPIC_PRINT: // Escreve uma mensagem no monitor serial
            INFO_printf("%.*s\n", len, data);
            escrever(&ssd, (const char *)state->data, 5, 20, cor);
            break;
        case TOPIC_MATRIZ: {
            // Remove espaços e quebras de linha do comando recebido
            char matriz_cmd[64];
            strncpy(matriz_cmd, (const char *)state->data, sizeof(matriz_cmd) - 1);
            matriz_cmd[sizeof(matriz_cmd) - 1] = '\0';
            for (int i = strlen(matriz_cmd) - 1; i >= 0 && (matriz_cmd[i] == '\n' || matriz_cmd[i] == '\r' || matriz_cmd[i] == ' '); i--) {
            matriz_cmd[i] = '\0';
            }
            if (strcmp(matriz_cmd, "desliga") == 0) {
            INFO_printf("[MATRIZ] Desligando todos os LEDs da matriz\n");
            desliga(); // Desliga todos os LEDs da matriz
            escrever(&ssd, "Matriz desligada", 2, 20, cor);
            } else if (strncmp(matriz_cmd, "ledr", 4) == 0) {
            int led_num = 0;
            if (sscanf(matriz_cmd + 4, "%d", &led_num) == 1 && led_num >= 1 && led_num <= 25) {
                INFO_printf("[MATRIZ] Ligando LED %d da matriz na cor vermelha\n", led_num);
                cores_matriz((uint)(led_num-1), BRILHO, 0, 0); // vermelho
                bf();
                char msg[32];
                snprintf(msg, sizeof(msg), "led %d vermelho", led_num);
                escrever(&ssd, msg, 5, 20, cor);
            } 
            } else if (strncmp(matriz_cmd, "ledg", 4) == 0) {
            int led_num = 0;
            if (sscanf(matriz_cmd + 4, "%d", &led_num) == 1 && led_num >= 1 && led_num <= 25) {
                INFO_printf("[MATRIZ] Ligando LED %d da matriz na cor verde\n", led_num);
                cores_matriz((uint)(led_num-1), 0, BRILHO, 0); // verde
                bf();
                char msg[32];
                snprintf(msg, sizeof(msg), "led %d verde", led_num);
                escrever(&ssd, msg, 5, 20, cor);
            } 
            } else if (strncmp(matriz_cmd, "ledb", 4) == 0) {
            int led_num = 0;
            if (sscanf(matriz_cmd + 4, "%d", &led_num) == 1 && led_num >= 1 && led_num <= 25) {
                INFO_printf("[MATRIZ] Ligando LED %d da matriz na cor azul\n", led_num);
                cores_matriz((uint)(led_num-1), 0, 0, BRILHO); // azul
                bf();
                char msg[32];
                snprintf(msg, sizeof(msg), "led %d azul", led_num);
                escrever(&ssd, msg, 5, 20, cor);
            } 
            } else {
                INFO_printf("[MATRIZ] Comando inválido para matriz: %s\n", matriz_cmd);
                escrever(&ssd, "Comando", 2, 20, cor);
                escrever(&ssd, "invalido", 2, 30, cor);
            }
            break;
        }
        case TOPIC_PING: {
            char buf[11];
            snprintf(buf, sizeof(buf), "%u", to_ms_since_boot(get_absolute_time()) / 1000);
            mqtt_publish(state->mqtt_client_inst, full_topic(state, "/uptime"), buf, strlen(buf), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);
            break;
        }
        case TOPIC_EXIT:
            state->stop_client = true; // stop the client when ALL subscriptions are stopped
            sub_unsub_topics(state, false); // unsubscribe
            break;
        case TOPIC_SENSORES: {
            // Remove possíveis quebras de linha e espaços extras do comando recebido
            char cmd[64];
            strncpy(cmd, (const char *)state->data, sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
            // Remove espaços à direita e '\n'
            for (int i = strlen(cmd) - 1; i >= 0 && (cmd[i] == '\n' || cmd[i] == '\r' || cmd[i] == ' '); i--) {
            cmd[i] = '\0';
            }

            if (lwip_stricmp(cmd, "temperatura") == 0) {
            INFO_printf("[SENSORES] Ativando leitura de temperatura\n");
            temperature_worker.user_data = state;
            async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &temperature_worker, 0);
            
            } else if (lwip_stricmp(cmd, "temperatura") == 0) {
            } else if (lwip_stricmp(cmd, "temperatura off") == 0) {
            INFO_printf("[SENSORES] Desativando leitura de temperatura\n");
            remover_worker_temperatura();
            escrever(&ssd, "Temperatura off", 5, 20, cor);
            } else if (lwip_stricmp(cmd, "adc") == 0) {
            } else {
            INFO_printf("[SENSORES] Comando desconhecido: %s\n", cmd);
            }
            break;
        }
        case TOPIC_SERVO: {
            // Se receber "desligar servo motor", volta para 0 e desliga PWM
            if (lwip_stricmp((const char *)state->data, "desligar servo motor") == 0) {
            INFO_printf("[SERVO] Desligando servo motor (posição 0 e PWM off)\n");
            uint16_t pulso = calcula_pulso(0);
            posicao(pulso);
            desliga_pwm_servo(); // Função para desligar o PWM do servo
            escrever(&ssd, "Servo desligado", 5, 20, cor);
            } else {
            // Espera receber o ângulo desejado como string (ex: "90")
            int angulo = 0;
            if (sscanf((const char *)state->data, "%d", &angulo) == 1 && angulo >= 0 && angulo <= 180) {
                liga_pwm_servo(); // Garante que o PWM está ligado antes de mover o servo
                INFO_printf("[SERVO] Posicionando servo em %d° graus\n", angulo);
                uint16_t pulso = calcula_pulso((uint16_t)angulo);
                posicao(pulso);
                char msg[32];
                snprintf(msg, sizeof(msg), "Servo em: %d", angulo);
                escrever(&ssd, msg, 5, 20, cor);
            } else {
                INFO_printf("[SERVO] Comando inválido para servo: %s\n", state->data);
                escrever(&ssd, "Comando invalido", 2, 20, cor);
            }
            }
            break;
        }
        default:
            // Unknown topic, do nothing or log
            break;
    }
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)arg;
    if (status == MQTT_CONNECT_ACCEPTED) {
        state->connect_done = true;
        sub_unsub_topics(state, true); // subscribe;

        // indicate online
        if (state->mqtt_client_info.will_topic) {
            mqtt_publish(state->mqtt_client_inst, state->mqtt_client_info.will_topic, "1", 1, MQTT_WILL_QOS, true, pub_request_cb, state);
        }
        // Removido: ativação automática da leitura de temperatura
    } else if (status == MQTT_CONNECT_DISCONNECTED) {
        if (!state->connect_done) {
            panic("Failed to connect to mqtt server");
        }
    }
    else {
        panic("Unexpected status");
    }
}

void start_client(MQTT_CLIENT_DATA_T *state) {
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    const int port = MQTT_TLS_PORT;
    INFO_printf("Using TLS\n");
#else
    const int port = MQTT_PORT;
    INFO_printf("Warning: Not using TLS\n");
#endif

    state->mqtt_client_inst = mqtt_client_new();
    if (!state->mqtt_client_inst) {
        panic("MQTT client instance creation error");
    }
    INFO_printf("IP address of this device %s\n", ipaddr_ntoa(&(netif_list->ip_addr)));
    INFO_printf("Connecting to mqtt server at %s\n", ipaddr_ntoa(&state->mqtt_server_address));

    cyw43_arch_lwip_begin();
    if (mqtt_client_connect(state->mqtt_client_inst, &state->mqtt_server_address, port, mqtt_connection_cb, state, &state->mqtt_client_info) != ERR_OK) {
        panic("MQTT broker connection error");
    }
#if LWIP_ALTCP && LWIP_ALTCP_TLS
    // This is important for MBEDTLS_SSL_SERVER_NAME_INDICATION
    mbedtls_ssl_set_hostname(altcp_tls_context(state->mqtt_client_inst->conn), MQTT_SERVER);
#endif
    mqtt_set_inpub_callback(state->mqtt_client_inst, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, state);
    cyw43_arch_lwip_end();
}

void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    MQTT_CLIENT_DATA_T *state = (MQTT_CLIENT_DATA_T*)arg;
    if (ipaddr) {
        state->mqtt_server_address = *ipaddr;
        start_client(state);
    } else {
        panic("dns request failed");
    }
}

#endif