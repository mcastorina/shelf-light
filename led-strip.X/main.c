/*
 * File:   main.c
 * Author: miccah
 *
 * Created on October 10, 2020, 10:38 AM
 */

/* Disable the watchdog timer */
#pragma config WDTE = OFF

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>

#define _XTAL_FREQ (1000000)
#define OUTPUT     (0)
#define INPUT      (1)
#define NUM_LEDS   (60)
#define LOW        (0)
#define HIGH       (1)

#define VDD_REF     (0b00)
#define FOSC_DIV_2  (0b000)
#define FOSC_DIV_4  (0b100)
#define FOSC_DIV_8  (0b100)
#define FOSC_DIV_16 (0b101)
#define RIGHT_JUST  (1)
#define LEFT_JUST   (0)
#define CH4         (0b000100)

typedef int16_t fixed_point;
#define FP_1_10 (10)
#define FP_2_10 (20)
#define FP_1_5  (20)
#define FP_3_10 (30)
#define FP_4_10 (40)
#define FP_2_5  (40)
#define FP_5_10 (50)
#define FP_1_2  (50)
#define FP_6_10 (60)
#define FP_3_5  (60)
#define FP_7_10 (70)
#define FP_8_10 (80)
#define FP_4_5  (80)
#define FP_9_10 (90)
#define FP_1    (100)

void hsv2rgb(uint8_t *r, uint8_t *g, uint8_t *b, fixed_point h, fixed_point s, fixed_point v) {
    int16_t i;
    fixed_point f, p, q, t;

    if (s == 0) {
        // achromatic (grey)
        *r = *g = *b = 0xff * v / 100;
        return;
    }

    h = h * 10 / 6;
    i = h / 100 * 100;
    f = h - i;
    p = v * ( 100 - s ) / 100;
    q = v * ( 1000 - (s * f) / 10 ) / 1000;
    t = v * ( 1000 - (s * ( 100 - f )) / 10 ) / 1000;

    switch (i/100) {
        case 0:
            *r = 0xff * v / 100;
            *g = 0xff * t / 100;
            *b = 0xff * p / 100;
            break;
        case 1:
            *r = 0xff * q / 100;
            *g = 0xff * v / 100;
            *b = 0xff * p / 100;
            break;
        case 2:
            *r = 0xff * p / 100;
            *g = 0xff * v / 100;
            *b = 0xff * t / 100;
            break;
        case 3:
            *r = 0xff * p / 100;
            *g = 0xff * q / 100;
            *b = 0xff * v / 100;
            break;
        case 4:
            *r = 0xff * t / 100;
            *g = 0xff * p / 100;
            *b = 0xff * v / 100;
            break;
        default:
            *r = 0xff * v / 100;
            *g = 0xff * p / 100;
            *b = 0xff * q / 100;
            break;
    }
}

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
}

uint16_t adc() {
    ADCON0bits.GOnDONE = 1;
    while (ADCON0bits.GOnDONE);
    // right justified read
    return (ADRESH << 8) | ADRESL;
}

#define NUM_ADC_READS   (10)
uint16_t stable_adc() {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < NUM_ADC_READS; i++) {
        sum += adc();
    }
    return sum / NUM_ADC_READS;
}

uint16_t hue_from_sensor() {
    uint16_t h = stable_adc() * 6 / 25 + 120;
    if (h >= 360) {
        h = 0;
    }
    return h;
}

/*
 * Set the LED color once, then sleep.
 */
int main(int argc, char** argv) {
    OSCFRQbits.HFFRQ = 0b110; // 32 MHz internal oscillator

    TRISAbits.TRISA2 = OUTPUT;

    // Setup analog input on A4
    TRISAbits.TRISA4 = INPUT;
    ANSELAbits.ANSA4 = HIGH;
    ADCON0bits.CHS = CH4;
    ADCON1bits.ADPREF = VDD_REF;
    ADCON1bits.ADCS = FOSC_DIV_16;
    ADCON1bits.ADFM = RIGHT_JUST;
    ADCON0bits.ADON = 1;

    // TODO: properly wait for oscillator to stabilize
    __delay_ms(500);

    uint16_t last_value = 0x00;
    uint8_t r, g, b;
    while (1) {
        uint16_t hue = hue_from_sensor();
        if (hue != last_value) {
            hsv2rgb(&r, &g, &b, hue, FP_1, FP_2_10);
            write_leds(g, r, b);
        }
        last_value = hue;
        __delay_ms(100);
    }

    return (EXIT_SUCCESS);
}
