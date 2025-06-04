#ifndef MATRIZ_CONTROL_H
#define MATRIZ_CONTROL_H

#include <stdbool.h>
#include "mqtt_client.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include <string.h>

#include "lib/matrixws.h"
/*
void control_red_led(bool on) {
    if (on) {
        gpio_put(red, true);
        INFO_printf("[LED] vermelho LIGADO\n");
    } else {
        gpio_put(red, false);
        INFO_printf("[LED] vermelho DESLIGADO\n");
        extern async_at_time_worker_t temperature_worker;
        async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &temperature_worker, 0);
    }
}
*/
#endif