// DisplayManager.cpp
#include "DisplayManager.h"



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define MAX_CHARACTERS_PER_LINE 12 //Maximum number of characters allowed to allow displaying the name of a file on one line of the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
    // Constructor
}

void DisplayManager::init() {
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    clearJustOnetime = false;
}

void DisplayManager::setCurrentTitle(const String &title) {
    currentTitle = title;
}

void DisplayManager::displayTitle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Displays the current title
    display.setCursor(0, 0);
    display.println(currentTitle);
}

void DisplayManager::showMenu(const char *title, const std::vector<String> &items, int selectedIndex, int itemCountLimit, int startIndex, int endIndex) {
    // Displays and updates the title to reflect the current directory
    displayTitle();

    // Adds the selected items counter
    display.println("(" + String(selectedIndex + 1) + "/" + String(items.size()) + ")");
    display.println();  // Skips a line before starting to display the items

    // Displays the items in the visible block
    for (int i = startIndex; i < endIndex && i < items.size(); ++i) {
        // Adds ">" before the selected item's name
        if (i == selectedIndex) {
            display.print(">");
        } else {
            // Adds whitespace before the other items
            display.print(" ");
        }

        display.println(items[i]);
    }

    display.display();
}

void DisplayManager::showVgmPlayer(const std::vector<String> &items, String &track, String &author, String &game, String &system, int currentVgmIndex, int totalVgmItems){

    // Clears the display only once
    if (!clearJustOnetime){
        display.clearDisplay();
        clearJustOnetime = true;
    }

    display.setCursor(0, 0);

    // Logic for the .vgm counter -> indicates the current index for the .vgm file against the whole list
    display.print("(");
    display.print(currentVgmIndex + 1); // Current index (starting from 1)
    display.print("/");
    display.print(totalVgmItems);
    display.println(")");
    display.println(); // Leaves a blank line

    // Logic for displaying the information contained in GD3 info
    //display.print("T:");
    display.println(track); //-> get info from VgmManager::gd3Info.trackNameEnglish to print on the screen
    //display.print("A:");
    display.println(author); //-> get info from VgmManager::gd3Info.trackAuthorEnglish to print on the screen
    //display.print("G:");
    display.println(game); //-> get info from VgmManager::gd3Info.gameNameEnglish to print on the screen
    //display.print("S:");
    display.println(system); //-> get info from VgmManager::gd3Info.systemNameEnglish to print on the screen
    display.display();
}

void DisplayManager::notVgmFile(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WARNING!");
    display.println();
    display.println("File not supported.");
    display.println("This device is meant to play .vgm files");
    display.println("only!");
    display.display();
}

void DisplayManager::updateVisibleItems() {
   
    clearDisplay();

    // Displays and updates the title to reflect the current directory
    displayTitle();

    // Displays the visible items in the block
    for (int i = 0; i < itemCountLimit && i + verticalOffset < this->listaDeArquivos.size(); ++i) {
        // Adds ">" before the selected item's name
        if (i + verticalOffset == selectedIndex) {
            display.print(">");
        } else {
            // Adds whitespace before the other items
            display.print(" ");
        }

        display.println(this->listaDeArquivos[i + verticalOffset]);
    }

    display.display();
}

void DisplayManager::setCurrentPath(const String &currentPath) {
    this->currentPath = currentPath;
}

// Gets the current path
String DisplayManager::getCurrentPath() {
    return currentPath;
}

void DisplayManager::clearDisplay() {
    display.clearDisplay();
}

// Getter for the bool getClearJustOnetime
bool DisplayManager::getClearJustOnetime() const {
    return clearJustOnetime;
}

// Setter for the bool getClearJustOnetime
void DisplayManager::setClearJustOnetime(bool value) {
    clearJustOnetime = value;
}

// Debug -> Test OLED display
void DisplayManager::debugMessage(const char *message) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(message);
    display.display();
}


// Getters - Strings regarding the GD3 struct
String DisplayManager::getTrackNameEnglish() const {
    return Track;
}

String DisplayManager::getGameNameEnglish() const {
    return Game;
}

String DisplayManager::getAuthorEnglish() const {
    return Author;
}

String DisplayManager::getSystemNameEnglish() const {
    return System;
}

// Setters - Strings regarding the GD3 struct
void DisplayManager::setTrackNameEnglish(const String &trackName) {
    Track = trackName;
}

void DisplayManager::setGameNameEnglish(const String &gameName) {
    Game = gameName;
}

void DisplayManager::setAuthorEnglish(const String &author) {
    Author = author;
}

void DisplayManager::setSystemNameEnglish(const String &systemName) {
    System = systemName;
}