// InputManager.h
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <Bounce2.h>

class InputManager {
public:
    InputManager(); // Standard constructor

    void init(); // Initializes the InputManager and adds the initial configuration necessary for the buttons to work
    void checkInput(); // Constantly checks the state of the buttons
    void debugButtons(); // Function to debug the buttons -> serial print to see if they work

    // Public methods to check button status
    bool isButtonDown();
    bool isButtonUp();
    bool isButtonSelect();
    bool isButtonReturn();

private:
    // Bounce2 -> object declarations
    // Declare your buttons here -> mainly 4: up, down, select, return
    Bounce2::Button btnDown;
    Bounce2::Button btnUp;
    Bounce2::Button btnSelect;
    Bounce2::Button btnReturn;
};

#endif