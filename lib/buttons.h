#ifndef BUTTONS_H
#define BUTTONS_H

#include "pico/stdlib.h"
#include "lib/conect_topicos.h"

// Modo bootsel
#define botaoB 6
void iniciar_botoes()
{
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB); // Configura o pino do botão B com pull-up interno
}

#include "pico/bootrom.h" 
void botaoB_callback(uint gpio, uint32_t events) {
    if (gpio == botaoB) {
        INFO_printf("Botão B pressionado, entrando no modo BOOTSEL...\n");
        reset_usb_boot(0, 0);
    }
}

#endif