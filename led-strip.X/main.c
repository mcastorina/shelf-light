/* 
 * File:   main.c
 * Author: miccah
 *
 * Created on October 10, 2020, 10:38 AM
 */

#pragma config WDTE = OFF

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>

#define _XTAL_FREQ (1000000)
#define OUTPUT     (0)
#define INPUT      (1)
#define NUM_LEDS   (60)

/*
 * write_bit writes the least significant bit of arg to A2
 * using the timing scheme defined by the WS2812B protocol.
 * See: https://mcastorina.github.io/knowledge/ws2812b.html
 */
void write_bit(uint8_t arg) {
    asm("bsf LATA, 0x2");
    asm("nop");
    asm("btfss WREG, 0");
    asm("bcf LATA, 0x2");
    asm("nop");
    asm("nop");
    asm("bcf LATA, 0x2");
}

/*
 * write_leds writes the GRB color using write_bit for each
 * LED. It temporarily sets the oscillator to 32 MHz so each
 * instruction takes 125 ns (4 cycles per instruction).
 */
void write_leds(uint8_t g, uint8_t r, uint8_t b) {
    int8_t i;
    OSCFRQbits.HFFRQ = 0b110; // 32 MHz internal oscillator

    for (uint8_t n = 0; n < NUM_LEDS; n++) {
        for (i = 7; i >= 0; i--) {
            write_bit(g >> i);
        }
        for (i = 7; i >= 0; i--) {
            write_bit(r >> i);
        }
        for (i = 7; i >= 0; i--) {
            write_bit(b >> i);
        }
    }
    
    OSCFRQbits.HFFRQ = 0b000; // 1 MHz internal oscillator
}

/*
 * Set the LED color once, then sleep.
 */
int main(int argc, char** argv) {
    TRISAbits.TRISA2 = OUTPUT;
    // TODO: properly wait for oscillator to stabilize
    __delay_ms(500);

    write_leds(0x0, 0x4, 0x0);
    while (1);

    return (EXIT_SUCCESS);
}
