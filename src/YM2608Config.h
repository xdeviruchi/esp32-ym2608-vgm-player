//Ym2608Config.h

#ifndef YM2608CONFIG_H
#define YM2608CONFIG_H


//Caution! By defining the RX pin as a GPIO, you can no longer read from the serial monitor.
//Disconnect if you need to debug via the serial monitor!
#define RX_PIN 3   // GPIO3 
//#define TX_PIN 1   // GPIO1

//YM2608 -> configurations and macros for GPIOs
//Definition to allow changing state from high to low by writing directly to the register! Faster than the standard Arduino method

//YM2608 - Control pins - ESP32
#define YM_IC (16) 
#define YM_CS (3)
#define YM_WR (17)
#define YM_RD (4) 
#define YM_A0 (2) 
#define YM_A1 (15) 

//YM2608 - Data pins - 8 bit) - ESP32
#define YM_D0 (13)
#define YM_D1 (12)
#define YM_D2 (14)
#define YM_D3 (27)
#define YM_D4 (26)
#define YM_D5 (25)
#define YM_D6 (33)
#define YM_D7 (32)

#define GPIO_BIT_MASK  ((1ULL<<YM_D0) | (1ULL<<YM_D1) | (1ULL<<YM_D2) | (1ULL<<YM_D3) | (1ULL<<YM_D4) | (1ULL<<YM_D5) | (1ULL<<YM_D6) | (1ULL<<YM_D7) | (1ULL<<YM_A1) | _BV(YM_A0) | (1ULL<<YM_RD) | (1ULL<<YM_WR) | (1ULL<<YM_CS) | (1ULL<<YM_IC) )

//A0
#define A0_HIGH GPIO.out_w1ts = _BV(YM_A0)
#define A0_LOW GPIO.out_w1tc = _BV(YM_A0)

//A1
#define A1_HIGH GPIO.out_w1ts = _BV(YM_A1)
#define A1_LOW GPIO.out_w1tc = _BV(YM_A1)

//WR
#define WR_HIGH GPIO.out_w1ts = _BV(YM_WR)
#define WR_LOW GPIO.out_w1tc = _BV(YM_WR)

//RD
#define RD_HIGH GPIO.out_w1ts = _BV(YM_RD)
#define RD_LOW GPIO.out_w1tc = _BV(YM_RD)

//IC
#define IC_HIGH GPIO.out_w1ts = _BV(YM_IC)
#define IC_LOW GPIO.out_w1tc = _BV(YM_IC)

//CS
#define CS_HIGH GPIO.out_w1ts = _BV(YM_CS)
#define CS_LOW GPIO.out_w1tc = _BV(YM_CS)

//D0
#define D0_HIGH GPIO.out_w1ts = _BV(YM_D0)
#define D0_LOW GPIO.out_w1tc = _BV(YM_D0)

//D1
#define D1_HIGH GPIO.out_w1ts = _BV(YM_D1)
#define D1_LOW GPIO.out_w1tc = _BV(YM_D1)

//D2
#define D2_HIGH GPIO.out_w1ts = _BV(YM_D2)
#define D2_LOW GPIO.out_w1tc = _BV(YM_D2)

//D3
#define D3_HIGH GPIO.out_w1ts = _BV(YM_D3)
#define D3_LOW GPIO.out_w1tc = _BV(YM_D3)

//D4
#define D4_HIGH GPIO.out_w1ts = _BV(YM_D4)
#define D4_LOW GPIO.out_w1tc = _BV(YM_D4)

//D5
#define D5_HIGH GPIO.out_w1ts = _BV(YM_D5)
#define D5_LOW GPIO.out_w1tc = _BV(YM_D5)

//D6
#define D6_HIGH GPIO.out1_w1ts.val = ((uint32_t)1 << (YM_D6 - 32));
#define D6_LOW GPIO.out1_w1tc.val = ((uint32_t)1 << (YM_D6 - 32));

//D7
#define D7_HIGH GPIO.out1_w1ts.val = ((uint32_t)1 << (YM_D7 - 32));
#define D7_LOW GPIO.out1_w1tc.val = ((uint32_t)1 << (YM_D7 - 32));

#endif