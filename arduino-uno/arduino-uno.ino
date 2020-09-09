#include <PololuLedStrip.h>

int getHueFromSensor();
void hsv2rgb(unsigned char*, unsigned char*, unsigned char*, float, float, float);
int stableAnalogRead(int);

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<12> ledStrip;
int potPin = A0;    // select the input pin for the potentiometer

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 60
rgb_color colors[LED_COUNT];

void setup() {}

unsigned int lastHue = -1;
void loop() {
    rgb_color color;
    unsigned int h = getHueFromSensor();
    if (h == lastHue) {
        goto end_loop;
    }
    lastHue = h;

    // convert hue to rgb
    hsv2rgb(&color.red, &color.green, &color.blue, h, 1, 1);

    // update the colors buffer
    for(uint16_t i = 0; i < LED_COUNT; i++) {
        colors[i] = color;
    }

    // write to the led strip
    ledStrip.write(colors, LED_COUNT);

end_loop:
    delay(100);
}

int getHueFromSensor() {
    // read the value from the sensor:
    long sensorValue = stableAnalogRead(potPin);
    long h = sensorValue * 6 / 25 + 120;
    if (h >= 360) {
        h = 0;
    }
    return (int)h;
}

void hsv2rgb(unsigned char *r, unsigned char *g, unsigned char *b, float h, float s, float v) {
    int i;
    float f, p, q, t;

    if (s == 0) {
        // achromatic (grey)
        *r = *g = *b = 255 * v;
        return;
    }

    h /= 60;
    i = floor(h);
    f = h - i;
    p = v * ( 1 - s );
    q = v * ( 1 - s * f );
    t = v * ( 1 - s * ( 1 - f ) );

    switch (i) {
        case 0:
            *r = 255 * v;
            *g = 255 * t;
            *b = 255 * p;
            break;
        case 1:
            *r = 255 * q;
            *g = 255 * v;
            *b = 255 * p;
            break;
        case 2:
            *r = 255 * p;
            *g = 255 * v;
            *b = 255 * t;
            break;
        case 3:
            *r = 255 * p;
            *g = 255 * q;
            *b = 255 * v;
            break;
        case 4:
            *r = 255 * t;
            *g = 255 * p;
            *b = 255 * v;
            break;
        default:
            *r = 255 * v;
            *g = 255 * p;
            *b = 255 * q;
            break;
    }
}

// max 64 without overflow
#define READ_COUNT (10)
int stableAnalogRead(int pin) {
    unsigned int sum = 0;
    for (int i = 0; i < READ_COUNT; i++) {
        sum += analogRead(pin);
    }
    return sum / READ_COUNT;
}
