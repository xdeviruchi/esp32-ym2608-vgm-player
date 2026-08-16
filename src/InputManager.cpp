// InputManager.cpp
#include "InputManager.h"

//Macros to define the GPIO input used for the buttons
//Change the number in () to match your GPIO configuration
#define BUTTON_DOWN (36)
#define BUTTON_UP (39)
#define BUTTON_SELECT (34)
#define BUTTON_RETURN (35)

InputManager::InputManager() {
    //Initialize the variables here, when necessary
}

void InputManager::init() {
    //Initial config for buttons
    btnDown.attach(BUTTON_DOWN, INPUT);
    btnUp.attach(BUTTON_UP, INPUT);
    btnSelect.attach(BUTTON_SELECT, INPUT);
    btnReturn.attach(BUTTON_RETURN, INPUT);

    btnDown.interval(1);
    btnUp.interval(1);
    btnSelect.interval(1);
    btnReturn.interval(1);

    btnDown.setPressedState(LOW);     // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
    btnUp.setPressedState(LOW);       // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
    btnSelect.setPressedState(LOW);   // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
    btnReturn.setPressedState(LOW);   // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
}

void InputManager::checkInput() {
    // Logic to check button input and notify MenuManager
    btnDown.update();
    btnUp.update();
    btnSelect.update();
    btnReturn.update();
}

//Debug function to test button functionality
void InputManager::debugButtons() {
    if (btnDown.pressed() == true) {
        Serial.println("Down button pressed!");
    }

    if (btnUp.pressed() == true) {
        Serial.println("Up button pressed!");
    }

    if (btnSelect.pressed() == true) {
        Serial.println("Select button pressed!");
    }

    if (btnReturn.pressed() == true) {
        Serial.println("Return button pressed!");
    }
}

// Public methods to check button status
bool InputManager::isButtonDown() {
    return btnDown.pressed()==true;
}

bool InputManager::isButtonUp() {
    return btnUp.pressed()==true;
}

bool InputManager::isButtonSelect() {
    return btnSelect.pressed()==true;
}

bool InputManager::isButtonReturn() {
    return btnReturn.pressed()==true;
}