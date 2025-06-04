#include "pico/stdlib.h"             // Funções padrão do Raspberry Pi Pico (GPIO, delay, etc)
#include "pico/cyw43_arch.h"         // Suporte Wi-Fi CYW43 para Pico W
#include "pico/unique_id.h"          // Função para obter ID único da placa

#include "hardware/gpio.h"           // Acesso direto ao hardware GPIO
#include "hardware/irq.h"            // Manipulação de interrupções
#include "hardware/adc.h"            // Conversor ADC do Pico

#include "lwip/apps/mqtt.h"          // Cliente MQTT LWIP
#include "lwip/apps/mqtt_priv.h"     // Funções internas do MQTT
#include "lwip/dns.h"                // Resolução DNS
#include "lwip/altcp_tls.h"          // TLS para conexões seguras

#include "lib/mqtt_client.h"
#include "lib/temperature.h"
#include "lib/led_control.h"
#include "inc/display.h"

#define WIFI_SSID "Sem senha"
#define WIFI_PASS "canariohouse"
#define MQTT_BROKER "192.168.0.103"
#define MQTT_USER "frank_rabbit"
#define MQTT_PASSWD "inside10"

#include "lib/conect_topicos.h"  // Funções para conexão dos tópicos MQTT
#include "lib/buttons.h"         // Funções para controle dos botões físicos

int main(void) {
    sleep_ms(1000); // Espera estabilizar alimentação
    
    // Inicializa a interface padrão de I/O
    stdio_init_all();
    printf("Inicializando sistema...\n");
    INFO_printf("Iniciando cliente MQTT\n");

    iniciar_botoes(); // Configura botões
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &botaoB_callback);
    iniciar_leds();   // Inicializa LEDs
    servo_config();   // Configura PWM do servo motor
    controle(PINO_MATRIZ); // Configura matriz de LEDs
    init_display();   // Configura display OLED
    
    // Configura ADC para sensor de temperatura interna
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    static MQTT_CLIENT_DATA_T mqtt_state; // Estrutura do cliente MQTT

    // Inicializa o módulo Wi-Fi CYW43
    if (cyw43_arch_init() != 0) {
        panic("Erro na inicialização do CYW43");
    }

    // Obtém ID único da placa para gerar Client ID
    char id_unico[5];
    pico_get_unique_board_id_string(id_unico, sizeof(id_unico));
    for (int i = 0; i < sizeof(id_unico) - 1; i++) {
        id_unico[i] = tolower(id_unico[i]);
    }

    char nome_cliente[sizeof(MQTT_DEVICE_NAME) + sizeof(id_unico) - 1];
    memcpy(nome_cliente, MQTT_DEVICE_NAME, sizeof(MQTT_DEVICE_NAME) - 1);
    memcpy(&nome_cliente[sizeof(MQTT_DEVICE_NAME) - 1], id_unico, sizeof(id_unico) - 1);
    nome_cliente[sizeof(nome_cliente) - 1] = '\0';

    INFO_printf("Nome do dispositivo: %s\n", nome_cliente);

    mqtt_state.mqtt_client_info.client_id = nome_cliente;
    mqtt_state.mqtt_client_info.keep_alive = MQTT_KEEP_ALIVE_S;

#if defined(MQTT_USERNAME) && defined(MQTT_PASSWORD)
    mqtt_state.mqtt_client_info.client_user = MQTT_USER;
    mqtt_state.mqtt_client_info.client_pass = MQTT_PASSWD;
#else
    mqtt_state.mqtt_client_info.client_user = NULL;
    mqtt_state.mqtt_client_info.client_pass = NULL;
#endif

    static char topico_will[MQTT_TOPIC_LEN];
    strncpy(topico_will, full_topic(&mqtt_state, MQTT_WILL_TOPIC), sizeof(topico_will));
    mqtt_state.mqtt_client_info.will_topic = topico_will;
    mqtt_state.mqtt_client_info.will_msg = MQTT_WILL_MSG;
    mqtt_state.mqtt_client_info.will_qos = MQTT_WILL_QOS;
    mqtt_state.mqtt_client_info.will_retain = true;

#if LWIP_ALTCP && LWIP_ALTCP_TLS
#ifdef MQTT_CERT_INC
    static const uint8_t cert_ca[] = TLS_ROOT_CERT;
    static const uint8_t key_cli[] = TLS_CLIENT_KEY;
    static const uint8_t cert_cli[] = TLS_CLIENT_CERT;
    mqtt_state.mqtt_client_info.tls_config = altcp_tls_create_config_client_2wayauth(cert_ca, sizeof(cert_ca),
            key_cli, sizeof(key_cli), NULL, 0, cert_cli, sizeof(cert_cli));
#if ALTCP_MBEDTLS_AUTHMODE != MBEDTLS_SSL_VERIFY_REQUIRED
    WARN_printf("Atenção: TLS sem verificação é inseguro\n");
#endif
#else
    mqtt_state.mqtt_client_info.tls_config = altcp_tls_create_config_client(NULL, 0);
    WARN_printf("Atenção: TLS sem certificado é inseguro\n");
#endif
#endif

    INFO_printf("Tentando conectar ao Wi-Fi SSID: %s\n", WIFI_SSID);
    cyw43_arch_enable_sta_mode();

    int resultado_wifi = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (resultado_wifi != 0) {
        ERROR_printf("Erro ao conectar Wi-Fi! Código: %d\n", resultado_wifi);
        ssd1306_fill(&ssd, true);
        escrever(&ssd, "Erro", 2, 10, true);
        escrever(&ssd, "conectar WiFi", 2, 20, true);
        panic("Não conectou ao Wi-Fi");
    } else {
        INFO_printf("Wi-Fi conectado com sucesso!\n");
        ssd1306_fill(&ssd, false);
        escrever(&ssd, "Wi-Fi", 2, 10, false);
        escrever(&ssd, "conectado", 2, 20, false);
    }
    INFO_printf("Wi-Fi conectado!\n");

    cyw43_arch_lwip_begin();
    int dns_err = dns_gethostbyname(MQTT_BROKER, &mqtt_state.mqtt_server_address, dns_found, &mqtt_state);
    cyw43_arch_lwip_end();

    if (dns_err == ERR_OK) {
        start_client(&mqtt_state);
    } else if (dns_err != ERR_INPROGRESS) {
        panic("Falha na requisição DNS");
    }

    // Aguarda conexão MQTT ativa
    while (!mqtt_state.connect_done || mqtt_client_is_connected(mqtt_state.mqtt_client_inst)) {
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
    }

    INFO_printf("Cliente MQTT finalizado\n");
    return 0;
}
