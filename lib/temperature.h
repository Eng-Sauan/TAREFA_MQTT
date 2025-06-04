#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "lib/mqtt_client.h"
#include "hardware/gpio.h"          // Biblioteca de hardware de GPIO
#include "hardware/irq.h"           // Biblioteca de hardware de interrupções
#include "hardware/adc.h"           // Biblioteca de hardware para conversão ADC
#include "inc/display.h"
#include "pico/async_context.h"

// Temporização da coleta de temperatura 
#define TEMP_WORKER_TIME_S 2 // 2 segundos

void temperature_worker_fn(async_context_t *context, async_at_time_worker_t *worker) {
    MQTT_CLIENT_DATA_T* state = (MQTT_CLIENT_DATA_T*)worker->user_data;
    INFO_printf("[DEBUG] temperature_worker_fn executado\n");
    publish_temperature(state);
    async_context_add_at_time_worker_in_ms(context, worker, TEMP_WORKER_TIME_S * 1000);
}

static async_at_time_worker_t temperature_worker = { .do_work = temperature_worker_fn };

// Implementações das funções (sem static)
float read_onboard_temperature(const char unit) {
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);
    uint16_t adc_raw = adc_read();
    float adc = (float)adc_raw * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;
    INFO_printf("[DEBUG] ADC raw: %u, ADC voltage: %.4f V, TempC: %.2f\n", adc_raw, adc, tempC);
    if (unit == 'C' || unit != 'F') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }
    return -1.0f;
}

void publish_temperature(MQTT_CLIENT_DATA_T *state) {
    static float old_temperature;
    const char *temperature_key = full_topic(state, "/temperatura");
    float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
    INFO_printf("[DEBUG] publish_temperature chamada. Temperatura: %.2f\n", temperature);
    if (temperature != old_temperature) {
        old_temperature = temperature;
        // Publish temperature on /temperature topic
        char temp_str[16];
        snprintf(temp_str, sizeof(temp_str), "%.2f", temperature);
        INFO_printf("Publishing %s to %s\n", temp_str, temperature_key);
        float temp = read_onboard_temperature(TEMPERATURE_UNITS);
        snprintf(temp_str, sizeof(temp_str), "Temp: %.2f C", temp);
        ssd1306_fill(&ssd, !cor); // Limpa o display
        escrever(&ssd, "/sensores", 5, 10, cor);
        escrever(&ssd, temp_str, 5, 20, cor);
        mqtt_publish(state->mqtt_client_inst, temperature_key, temp_str, strlen(temp_str), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);
    }
}

// Remove função inválida e substitua por função correta para remover o worker de temperatura
static inline void remover_worker_temperatura() {
    INFO_printf("[DEBUG] Removendo worker de temperatura\n");
    async_context_remove_at_time_worker(cyw43_arch_async_context(), &temperature_worker);
}

#endif // TEMPERATURE_H
