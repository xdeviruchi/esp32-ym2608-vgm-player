//YM2608.h
#ifndef YM2608_H
#define YM2608_H

#include "si5351.h" //Programmable clock generator
#include <driver/timer.h>

//
class YM2608 {
    
public:

    //Default constructor for the class
    YM2608();
    //Default destructor for the class
    ~YM2608() = default;

    //Function to initialize the initial values defined for the class
    void init();

    //Main function: reads bytes from the vgm music data buffer and processes each vgm command correctly for the chip
    void playVGM(size_t index, unsigned char *arrayPointer);

    //Resets the YM2608 chip and leaves it ready to receive music data from the .vgm file
    void resetChip();

    //Resets data pins -> LOW
    void resetDataPins();

    //Resets control pins -> LOW
    void resetControlPins();

    //Takes the 8 individual bits of a byte and converts it to a HIGH/LOW logic
    static void writeBitsFromByte(unsigned char data);



    //"NOP" -> Function that TRIES to create a delay on the order of microseconds using NOP
    void delay_ns(int time_ns);

    //Delay -> executes the "NOP" command nop_times times
    IRAM_ATTR void delay_nop(size_t nop_times);

    IRAM_ATTR void delay_using_micros(unsigned long microseconds);

    // Function that creates a nanosecond delay using an ESP32 timer
    void delay_ns_esp32(uint32_t ns);

    //Sends a read byte to the chip through pins D0~D7
    static void sendData(unsigned char reg);

    //Test -> Blinks all LEDs to verify if all connections are correct
    void GPIOLedsTest();

    void delay_nop_empiric_test(size_t number_of_nops);

    void high_low_emphiric_calc();

    //Getters -> bytes array
    unsigned char getCommand_() const;
    unsigned char getReg_() const;
    unsigned char getData_() const;

private:

    // Timer configuration
    timer_config_t timer_config;

  // Private function to initialize the timer
  void init_timer();

    //Storage of buffer bytes containing music data from the .vgm file
    unsigned char command_;
    unsigned char reg_;
    unsigned char data_;

    //Pre-calculate delays to avoid unnecessary multiplications in VGM commands -> considering sample rate = 44100
    const unsigned int delay1Samples = 1 * 1000000 / 44100;   //0x70    ->    1 sample       (microseconds)
    const unsigned int delay2Samples = 2 * 1000000 / 44100;   //0x71    ->    2 samples      (microseconds)
    const unsigned int delay3Samples = 3 * 1000000 / 44100;   //0x72    ->    3 samples      (microseconds)
    const unsigned int delay4Samples = 4 * 1000000 / 44100;   //0x73    ->    4 samples      (microseconds)
    const unsigned int delay5Samples = 5 * 1000000 / 44100;   //0x74    ->    5 samples      (microseconds)
    const unsigned int delay6Samples = 6 * 1000000 / 44100;   //0x75    ->    6 samples      (microseconds)
    const unsigned int delay7Samples = 7 * 1000000 / 44100;   //0x76    ->    7 samples      (microseconds)
    const unsigned int delay8Samples = 8 * 1000000 / 44100;   //0x77    ->    8 samples      (microseconds)
    const unsigned int delay9Samples = 9 * 1000000 / 44100;   //0x78    ->    9 samples      (microseconds)
    const unsigned int delay10Samples = 10 * 1000000 / 44100; //0x79    ->    10 samples     (microseconds)
    const unsigned int delay11Samples = 11 * 1000000 / 44100; //0x7A    ->    11 samples     (microseconds)
    const unsigned int delay12Samples = 12 * 1000000 / 44100; //0x7B    ->    12 samples     (microseconds)
    const unsigned int delay13Samples = 13 * 1000000 / 44100; //0x7C    ->    13 samples     (microseconds)
    const unsigned int delay14Samples = 14 * 1000000 / 44100; //0x7D    ->    14 samples     (microseconds)
    const unsigned int delay15Samples = 15 * 1000000 / 44100; //0x7E    ->    15 samples     (microseconds)
    const unsigned int delay16Samples = 16 * 1000000 / 44100; //0x7F    ->    16 samples     (microseconds)


    //Array of delays
    const unsigned int delays0x70[16] = {
        delay1Samples,        //0x70
        delay2Samples,        //0x71
        delay3Samples,        //0x72
        delay4Samples,        //0x73
        delay5Samples,        //0x74
        delay6Samples,        //0x75
        delay7Samples,        //0x76
        delay8Samples,        //0x77
        delay9Samples,        //0x78
        delay10Samples,       //0x79
        delay11Samples,       //0x7A
        delay12Samples,       //0x7B
        delay13Samples,       //0x7C
        delay14Samples,       //0x7D
        delay15Samples,       //0x7E
        delay16Samples,       //0x7F
        };

    const unsigned int delay735Samples = 735 * 1000 / 44100;  //0x62 -> 735 samples (milliseconds)
    const unsigned int delay882Samples = 882 * 1000 / 44100;  //0x63 -> 882 samples (milliseconds)

    bool i2c_found; //Returns true if an Si5351 is found
};

#endif