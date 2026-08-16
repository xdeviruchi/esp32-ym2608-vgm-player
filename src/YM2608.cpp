//YM2608.cpp
#include "YM2608.h"
#include "YM2608Config.h" //Header file containing all macros related to GPIO usage
#include "Constants.h" //File containing the declaration of all constant variables (Sample rate, etc.)


//Class default constructor
YM2608::YM2608(){}

// Initializes the class with default configurations -> Instantiates and configures the clock to 8MHz. Starts GPIO and bit mask configurations.
void YM2608::init(){

  //Configuration -> GPIO
  gpio_config_t io_conf; //GPIO configuration
  io_conf.mode = GPIO_MODE_OUTPUT; //Sets config to OUTPUT

  //Defining RXD and TXD pins to be GPIO -> loose the capability of debugging code through serial monitor
  pinMode(RX_PIN, OUTPUT);    
    //pinMode(TX_PIN, OUTPUT);   

  //Definition of GPIOs part of each configuration
  io_conf.pin_bit_mask = GPIO_BIT_MASK; //Bitmask for data/byte sending GPIOs
  gpio_config(&io_conf);

  // //Object -> Si5351 class -> programmable clock generator
  // Si5351 si5351;
  // bool i2c_found;

  //   i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  //       if(!i2c_found)
  //       {
  //       Serial.println("Device not found on I2C bus!");
  //       }

  //   // Set CLK0 to output 8 MHz
  //   si5351.set_freq(800000000ULL, SI5351_CLK0); 

  //Resets the chip (class function) -> leaves everything ready to play music
  resetChip();
}


//Main function -> verifies buffer bytes and sends them to the chip according to the detected .vgm command
void YM2608::playVGM(size_t index_, unsigned char *arrayPointer){
//VGM Commands
  command_ = arrayPointer[index_];
  reg_ = arrayPointer[index_ + 1];
  data_ = arrayPointer[index_ + 2];

  switch (command_)
  {

  case 0x55:
  case 0x56:  //YM2608 - A1 = 0
    // Addressing mode
    {
    sendData(reg_);
    A0_LOW;
    A1_LOW;
    CS_LOW;
    WR_LOW;
    delay_nop(67); //2,010 us
    // delay_nop(45); //1,350us
    // delay_nop(30); //900ns
    // delay_nop(17); //510ns
    // delay_nop(15); //450ns
    WR_HIGH;

    if(reg_ >= 0x10){ //Not SSG nor ADPCM -> Rhythm, common FM and FM CH1 - CH3
      //delay_nop(400); //~12us
      delay_nop(334); // ~10us
      //delay_nop(267); // ~8us
       //delay_nop(200); // ~6us
      // delay_nop(134); // ~4us
      // delay_nop(67); // ~2us
    }

    
    //End of addressing mode for 0x56 (A1 = 0 at the beginning)
  
    // Write data mode
    CS_HIGH;
    sendData(data_);
    A0_HIGH;
    CS_LOW;
    WR_LOW;
    delay_nop(67);
    WR_HIGH;

    // Insert the Delays based on each component of the Sound Source
    // A0 = 1; A1 = 0;

    if(reg_ == 0x10){ //Rhythm -> A1 = 0   (576 cycles at 8MHz) - 72 µs

      //delay_nop(2531); // 76 µs
      delay_nop(2467); //74 µs
      //delay_nop(2398); //72 µs
      //delay_nop(2331); //70us
    }

    else if(reg_ >= 0x11 && reg_ <= 0x1D){ //Rhythm -> A1 = 0   (83 cycles at 8MHz)  10,375µs
      //delay_nop(599);  //18 us
      //delay_nop(534);  //16 us
      delay_nop(467);  //14 us
      //delay_nop(400);  //12 us
      //delay_nop(333);  //10 us
    }

    else if(reg_ >= 0x21 && reg_ <= 0x9E){ //Commonness part of FM + FM CH1 - CH3 -> A1 = 0 (83 cycles at 8MHz) - 10,375µs
      // delay_nop(534);  //16 us
      delay_nop(467);  //14 us
    }

    else if(reg_ >= 0xA0 && reg_ <= 0xB6){ //Commonness part of FM + FM CH1 - CH3 -> A1 = 0 (47 cycles at 8MHz) - 5,875µs
      //delay_nop(334);  //10us
      delay_nop(267);  //8us
    }

    CS_HIGH;
    A0_LOW;
    A1_LOW;
    //index_ += 3;
    //return 3;
    break;
    }


  ///////////////////////////////

  case 0x57: //Same as 0x56 (port 0) but now A1 is HIGH at the beginning
  
    // Addressing mode
    sendData(reg_);
    A0_LOW;
    A1_HIGH;
    CS_LOW;
    WR_LOW;
    delay_nop(67); //2,010 us
    // delay_nop(45); //1,350us
    // delay_nop(30); //900ns
    // delay_nop(17); //510ns
    // delay_nop(15); //450ns
    WR_HIGH;

    // Insert the Delays based on each component of the Sound Source
    // A0 = 1; A1 = 0;

    if(reg_ >= 0x30 && reg_ <= 0xB6){ //FM CH4 - CH6 -> A1 = 1 (17 cycles at 8MHz) - 2,125µS
      delay_nop(134); // ~4us
    }


    //End of Addressing mode
    // Write data mode
    CS_HIGH;
    sendData(data_);
    A0_HIGH;
    CS_LOW;
    WR_LOW;
    delay_nop(67);
    WR_HIGH;

    // Insert the Delays based on each component of the Sound Source
    // A0 = 1; A0 = 1;

    if(reg_ >= 0x21 && reg_ <= 0x9E){ //FM CH4 - CH6 -> A1 = 1 (83 cycles at 8MHz) - 10,375µs
      // delay_nop(534);  //16 us
      delay_nop(467);  //14 us
    }

    else if(reg_ >= 0xA0 && reg_ <= 0xB6){ //Commonness part of FM + FM CH1 - CH3 -> A1 = 0 (47 cycles at 8MHz) - 5,875µs
      //delay_nop(334);  //10us
      delay_nop(267); //8us
    }
    
    CS_HIGH;
    A0_LOW;
    A1_LOW;
    //index_ += 3;
    //return 3;
  break;
 
case 0x61: // Delay of n samples -> little endian
{
    unsigned int delaySamples = (data_ << 8) | reg_;
    
    if (delaySamples < 45) { //Delay on the order of microseconds
        unsigned long startMicros = micros();

        while ((micros() - startMicros) < (delaySamples * 1000000 / sampleRate)) {
            // Wait for the necessary time in microseconds
        }
    } else {  //Delay on the order of milliseconds
        delay(delaySamples * 1000 / sampleRate);
    }

    //return 3;
    break;
}


case 0x62:  //Delay of 735 samples
{
    delay(delay735Samples);
    //return 1;
    break;
}

case 0x63: //Delay of 882 samples
{
    delay(delay882Samples);
    //return 1;
    break;
}

//Delays of n+1 samples with n starting at 0 and going up to 15 
case 0x70:
case 0x71:
case 0x72:
case 0x73:
case 0x74:
case 0x75:
case 0x76:
case 0x77:
case 0x78:
case 0x79:
case 0x7A:
case 0x7B:
case 0x7C:
case 0x7D:
case 0x7E:
case 0x7F:
{
    unsigned long startMicros = micros();
    unsigned int delayMicroseconds = delays0x70[command_ - 0x70]; //Pre-calculated delays stored in an array -> avoid unnecessary multiplications

    while ((micros() - startMicros) < delayMicroseconds) {
        // Wait for the necessary time in microseconds
    }

    //return 1;
    break;
}


  default:
  //index_ ++;
  //return 1;
  break;
 }

  #pragma endregion
  //------------------------------------------------------------
}

//Resets the YM2608 chip and leaves it ready to receive musical data from the .vgm file
void YM2608::resetChip(){
  resetDataPins();
  CS_LOW;
  WR_HIGH;
  RD_HIGH;
  A0_LOW;
  A1_LOW;
  IC_LOW;
  delay_using_micros(300); // at least 12 cycles master clock -> at 8Mhz = 1,5 us
  IC_HIGH;
  CS_HIGH;
  delay_using_micros(300);
}


void YM2608::resetDataPins(){
  D0_LOW;
  D1_LOW;
  D2_LOW;
  D3_LOW;
  D4_LOW;
  D5_LOW;
  D6_LOW;
  D7_LOW;
}

void YM2608::resetControlPins(){
  A0_LOW;
  A1_LOW;
  RD_LOW;
  WR_LOW;
  CS_LOW;
  IC_LOW;
}

  // //Resets control pins -> LOW
  // void resetControlPins();

//Takes the 8 individual bits of a byte and passes the values as HIGH or LOW to data pins D0~D7
void YM2608::writeBitsFromByte(unsigned char data) {
  for (int bit = 0; bit < 8; bit++) {
    if (data & (1 << bit)) {
      // Set bit to HIGH
      switch (bit) {
        case 0: D0_HIGH; break;
        case 1: D1_HIGH; break;
        case 2: D2_HIGH; break;
        case 3: D3_HIGH; break;
        case 4: D4_HIGH; break;
        case 5: D5_HIGH; break;
        case 6: D6_HIGH; break;
        case 7: D7_HIGH; break;
      }
    } else {
      // Set bit to LOW
      switch (bit) {
        case 0: D0_LOW; break;
        case 1: D1_LOW; break;
        case 2: D2_LOW; break;
        case 3: D3_LOW; break;
        case 4: D4_LOW; break;
        case 5: D5_LOW; break;
        case 6: D6_LOW; break;
        case 7: D7_LOW; break;
      }
    }
  }
}

//Sends a read byte to the chip through pins D0~D7
void YM2608::sendData(unsigned char reg){
  writeBitsFromByte(reg);
}

//Function that TRIES to create a delay on the order of microseconds using NOP
 void YM2608::delay_ns(int time_ns){
  double _time_ns = round(time_ns / esp32_cycle_time);
  for (size_t i = 0; i < _time_ns; i++)
  {
    asm("NOP");
  }
}

 IRAM_ATTR void YM2608::delay_nop(size_t nop_times){
  for (size_t i = 0; i < nop_times; i++)
  {
    asm volatile("NOP");
  }
  
}

IRAM_ATTR void YM2608::delay_using_micros(unsigned long microseconds) {
    unsigned long start_time = micros(); // Gets initial time

    while (micros() - start_time < microseconds) {
        // Wait until desired time has passed
        // Do nothing here, just wait
        // Make sure the compiler optimizer does not eliminate the empty loop
        asm("nop");
    }
}

//Private function to initialize the ESP32 timer with 240 MHz resolution
void YM2608::init_timer() {
  timer_config.divider = 240;   // Clock divider (240 for 240MHz)
  timer_config.counter_dir = TIMER_COUNT_UP;
  timer_config.counter_en = TIMER_PAUSE;  // Modified to TIMER_PAUSE
  timer_config.alarm_en = TIMER_ALARM_DIS;
  timer_config.auto_reload = TIMER_AUTORELOAD_EN;
  timer_config.intr_type = TIMER_INTR_LEVEL;

  timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
  timer_start(TIMER_GROUP_0, TIMER_0);
  timer_pause(TIMER_GROUP_0, TIMER_0);  // Modified to timer_pause
}


// Function that creates a nanosecond delay using an ESP32 timer
void YM2608::delay_ns_esp32(uint32_t ns) {
  if (ns == 0) {
    return;  // No delay needed
  }

  // Initialize timer if not already done
  if (timer_config.counter_en == TIMER_PAUSE) {
    init_timer();
  }

  // Set timer period to create the desired delay
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, ns);

  // Restart timer
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Wait until timer reaches the alarm value
  while (timer_group_get_counter_value_in_isr(TIMER_GROUP_0, TIMER_0) < ns) {
    continue;
  }

  // Stop timer
  timer_pause(TIMER_GROUP_0, TIMER_0);
}


void YM2608::GPIOLedsTest(){
  //Control pins
  A1_HIGH; delay(200); A1_LOW; delay(200);
  A0_HIGH; delay(200); A0_LOW; delay(200);
  RD_HIGH; delay(200); RD_LOW; delay(200);
  WR_HIGH; delay(200); WR_LOW; delay(200);
  CS_HIGH; delay(200); CS_LOW; delay(200);

  //Data pins ~ D0 - D7
  D0_HIGH; delay(200); D0_LOW; delay(200);
  D1_HIGH; delay(200); D1_LOW; delay(200);
  D2_HIGH; delay(200); D2_LOW; delay(200);
  D3_HIGH; delay(200); D3_LOW; delay(200);
  D4_HIGH; delay(200); D4_LOW; delay(200);
  D5_HIGH; delay(200); D5_LOW; delay(200);
  D6_HIGH; delay(200); D6_LOW; delay(200);
  D7_HIGH; delay(200); D7_LOW; delay(200);
}


void YM2608::delay_nop_empiric_test(size_t number_of_nops){
  D7_HIGH;
  delay_nop(number_of_nops);
  D7_LOW;
  delay_nop(number_of_nops);
}

void YM2608::high_low_emphiric_calc(){
  D7_HIGH;
  D7_LOW;
}

//Getter for command_
unsigned char YM2608::getCommand_() const{
  return command_;
}

unsigned char YM2608::getReg_() const{
  return reg_;
}
unsigned char YM2608::getData_() const{
  return data_;
}