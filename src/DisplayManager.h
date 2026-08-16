// DisplayManager.h
#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>
#include <string>


class DisplayManager {
public:
    DisplayManager();

    void init();
    void updateVisibleItems();
    void showMenu(const char *title, const std::vector<String> &items, int selectedIndex, int itemCountLimit, int startIndex, int endIndex); // Main logic to print the menu on the OLED display
    void clearDisplay();
    void debugMessage(const char *message);

    String getCurrentPath();   // Gets the current path
    void notVgmFile(); // Displays the message on the display for files not supported by the software

    void showVgmPlayer(const std::vector<String> &items, String &track, String &author, String &game, String &system, int currentVgmIndex, int totalVgmItems); // Submenu logic regarding the .vgm player

    // Getters and Setters ---------------------------

    // Getter for the bool getClearJustOnetime
    bool getClearJustOnetime() const;

    // Setter for the bool getClearJustOnetime
    void setClearJustOnetime(bool value);

    void setCurrentTitle(const String &title);  // Update the folder names displayed in the top left corner of the OLED display

    void setCurrentPath(const String &currentPath); // Sets the current path (title) for display

    // Getters - Strings regarding the GD3 struct 
    String getTrackNameEnglish() const;
    String getGameNameEnglish() const;
    String getAuthorEnglish() const;
    String getSystemNameEnglish() const;

    // Setters - Strings regarding the GD3 struct
    void setTrackNameEnglish(const String &trackName);
    void setGameNameEnglish(const String &gameName);
    void setAuthorEnglish(const String &author);
    void setSystemNameEnglish(const String &systemName);

private:
    Adafruit_SSD1306 display;

    // Add other state variables as needed
    int selectedIndex;
    int itemCountLimit;
    int verticalOffset;
    String currentPath;  // Current path within the SD card structure, defaults to "/" = root/root folder
    String currentTitle = "/";  // Current title to be displayed in the top left corner of the OLED display. Displays the current name of the selected directory
    std::vector<String> listaDeArquivos;  // Added
    // Internal function to display the title on the display
    void displayTitle();

    bool clearJustOnetime = false; // control variable for when it's needed to clear the display only once -> looped functions

    // GD3Info Struct strings from the VgmManager class
    String Track;
    String Game;
    String Author;
    String System;
};

#endif