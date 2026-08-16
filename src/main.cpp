#include <Arduino.h>
#include "MenuManager.h"

// Objects -> instances
FileManager fileManager;
DisplayManager displayManager;
InputManager inputManager;
YM2608 ym2608;
VgmManager vgmManager;
MenuManager menuManager(fileManager, displayManager, inputManager, ym2608, vgmManager);


void setup() 
{
  Serial.begin(115200); 
 
  // Initialization -> initial objects routine
  inputManager.init(); 
  displayManager.init(); 
  fileManager.init();
  ym2608.init();
  vgmManager.init();
  menuManager.init();
}


void loop() {
  
  menuManager.run();

}