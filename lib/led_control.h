#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdbool.h>
#include "mqtt_client.h"
#include "hardware/gpio.h"
#include "pico/cyw43_arch.h"
#include <string.h>

void control_led(MQTT_CLIENT_DATA_T *state, bool on) {
    // Publish state on /state topic and on/off led board
    const char* message = on ? "On" : "Off";
    if (on)
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    else
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    mqtt_publish(state->mqtt_client_inst, full_topic(state, "/led/state"), message, strlen(message), MQTT_PUBLISH_QOS, MQTT_PUBLISH_RETAIN, pub_request_cb, state);
}

#define red 13
#define blue 12
#define green 11

void iniciar_leds() 
{
    gpio_init(red);
    gpio_set_dir(red, GPIO_OUT);

    gpio_init(blue);
    gpio_set_dir(blue, GPIO_OUT);

    gpio_init(green);
    gpio_set_dir(green, GPIO_OUT);
}

void control_red_led(bool on) {
    if (on) {
        gpio_put(red, true);
        INFO_printf("[LED] vermelho LIGADO\n");
    } else {
        gpio_put(red, false);
        INFO_printf("[LED] vermelho DESLIGADO\n");
    }
}

void control_blue_led(bool on) {
    if (on) {
        gpio_put(blue, true);
        INFO_printf("[LED] Azul LIGADO\n");
    } else {
        gpio_put(blue, false);
        INFO_printf("[LED] Azul DESLIGADO\n");
    }
}

void control_green_led(bool on) {
    if (on) {
        gpio_put(green, true);
        INFO_printf("[LED] Verde LIGADO\n");
    } else {
        gpio_put(green, false);
        INFO_printf("[LED] Verde DESLIGADO\n");
    }
}

#endif 