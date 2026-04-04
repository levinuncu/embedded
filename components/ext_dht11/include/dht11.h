#pragma once

#include <stdbool.h>

typedef struct {
    int humidity_int;
    int humidity_dec;
    int temperature_int;
    int temperature_dec;
} dht11_reading_t;

void dht11_init(int pin);
bool dht11_read(dht11_reading_t *reading);