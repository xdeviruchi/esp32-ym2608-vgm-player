#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

const unsigned int sampleRate = 44100;      //Standard sample rate -> .vgm format
const unsigned int esp32_freq = 240000000;  // Esp32 frequency in Hz
const unsigned int opna_freq = 8000000;     // OPNA frequency in Hz
double esp32_cycle_time = 4.17; //Each cycle in ESP32 takes 4.17ns at 240MHz

#endif