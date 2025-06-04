#ifndef SERVO_H
#define SERVO_H

#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "lib/mqtt_client.h"

// Definições de parâmetros do PWM
#define PWM_FREQ 50
#define TOP 25000
#define SERVO_PIN 28 // Pino do servo motor
#define CLOCK_DIVIDER 100.0f

// Variáveis globais
uint slice_num;
uint channel;

void uart_config() {
  uart_init(uart0, 115200);
  gpio_set_function(false, GPIO_FUNC_UART);
  gpio_set_function(true, GPIO_FUNC_UART);
}

uint16_t calcula_pulso(uint16_t angulo) {
    return 500 + ((angulo * (2400 - 500) + 90) / 180); // +90 para arredondar corretamente
}

void posicao(uint16_t pulse_us) {
    uint16_t level = (pulse_us * TOP) / 20000;
    pwm_set_chan_level(slice_num, channel, level);
}

void servo_config() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    channel = pwm_gpio_to_channel(SERVO_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, CLOCK_DIVIDER);
    pwm_config_set_wrap(&config, TOP);
    pwm_init(slice_num, &config, true);
}

void desliga_pwm_servo() 
{
    pwm_set_enabled(slice_num, false);
    INFO_printf("[SERVO] PWM desligado\n");
}

void liga_pwm_servo() 
{
    pwm_set_enabled(slice_num, true);
    INFO_printf("[SERVO] PWM ligado\n");
}

#endif 