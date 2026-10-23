/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;

const float conversion_factor = 3.3f / (1 << 12);

volatile int flag_leitura = 0;

volatile int flag_pisca = 0;

bool timer_led_callback(repeating_timer_t *rt) {
    flag_pisca = 1;
    return true;
}

bool timer_leitura_callback(repeating_timer_t *rt) {
    flag_leitura = 1;
    return true;
}

int zona_de(float tensao) {
    if (tensao < 1.0f) return 0;
    else if (tensao < 2.0f) return 1;
    else return 2;
}

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 */
int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(28);
    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);

    repeating_timer_t timer_adc;
    add_repeating_timer_ms(100, timer_leitura_callback, NULL, &timer_adc);

    repeating_timer_t timer_led = {0};
    int zona_atual = -1;    
    int estado_led = 0;

    while (1) {
        if (flag_leitura){
            flag_leitura = 0;

            adc_select_input(2);
            uint16_t result = adc_read();
            float tensao = result * conversion_factor;
            printf("voltage 2: %f V\n", tensao);

            int nova_zona = zona_de(tensao);

            if(nova_zona != zona_atual){
                cancel_repeating_timer(&timer_led);
                flag_pisca = 0;
                zona_atual = nova_zona;

                if (zona_atual == 1){
                    add_repeating_timer_ms(300, timer_led_callback, NULL, &timer_led);
                } else if(zona_atual == 2) {
                    add_repeating_timer_ms(500, timer_led_callback, NULL, &timer_led);
                } else if(zona_atual == 0){
                    gpio_put(PIN_LED_B, 0);
                    estado_led = 0;
                }
            }
        }

        if (flag_pisca) {
            flag_pisca = 0;
            estado_led = !estado_led;
            gpio_put(PIN_LED_B, estado_led);
        }
        
    }
}
