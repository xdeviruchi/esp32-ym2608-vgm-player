// MenuManager.h
#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "FileManager.h"
#include "DisplayManager.h"
#include "InputManager.h"
#include "VgmManager.h"
#include "YM2608.h"


class MenuManager {
public:
    // Initial constructor -> instantiates references of objects from other classes -> access to public methods
    MenuManager(FileManager &fileManager, DisplayManager &displayManager, InputManager &inputManager, YM2608 &ym2608, VgmManager &vgmManager);

    void init();
    void run(); // Main logic in loop -> menu display
    void runMenu(); // Display the main menu
    void runVgmPlayer(); // Main logic for .vgm files -> Vgm Player!
    void runException(); // Display method for files not supported by the player
    void getGd3Info(); // Gets info from each element of the GD3 struct from the VgmManager class

    // Vgm Player Methods
    void checkButtonsVgmMode();  // Check buttons and execute corresponding methods
    void updateDisplayVgmMode(); // Menu updates
    void openVgmFileRoutine(const String &selectedvgmfile); // Routine to execute when opening a vgm file
    void playVgm(); // Main function to play the selected .vgm file -> main method belongs to VgmManager
    void playVgmState();
    void stopMusicRoutine();
    void suspendAndDestroyTask(); // Method to suspend and delete the task

    // Starting 2 RTOS tasks -> Parallel execution on both cores
    TaskHandle_t Task1;  // Core 1
    TaskHandle_t Task2; // Core 0

    void TaskHighPriority(void * pvParameters);
    void TaskLowPriority(void * pvParameters);

    static void TaskHighPriorityWrapper(void *pvParameters);
    static void TaskLowPriorityWrapper(void *pvParameters);

private:
    FileManager &fileManager;
    DisplayManager &displayManager;
    InputManager &inputManager;
    YM2608 &ym2608;
    VgmManager &vgmManager;

    // Enum -> State machine for navigation menus
    enum State {
    MENU_MAIN_STATE,
    VGM_PLAYER_STATE,
    WRONG_FILE_STATE,
    VGM_PLAYING_STATE,
    // Add more states as needed
    };
    

    // Flags - VgmPlayer
    bool vgm_is_playing = false;

    // Flag -> indicates if we are in shuffle/randomizer mode for music 
    bool isOnShuffleMode;

    // Flag -> indicates if we are in music autoplay mode
    bool isOnAutoPlay;

    // Enum variable declaration
    State currentState;

    // Add other state variables as needed
    String selectedVgmFilePath;
    String selectedItem;
    String fullPath;
    int selectedIndex = 0;
    int selectedVgmIndex;
    int totalVgmItems;
    int itemCountLimit = 4;  // Define the limit as needed
    String currentPath = "/";  // Added to track the current directory
    std::vector<String> listaDeArquivos;  // Dynamic vector to store the list of file and folder names in a directory
    std::vector<String> vgmFilesList; // Dynamic vector to store the list of .vgm files inside a directory

    // Vertical scrolling navigation
    int verticalOffset;  // Vertical offset in the file list
    int totalItems;      // Total number of items in the list

    // Control variable to check if we are in VgmPlayer mode -> initialized as false!
    bool inVgmMode = false;
    bool inWrongFileMode = false;

    // Private methods to handle main menu logic
    void navigateUp();
    void navigateDown();
    void selectItem();
    void returnToPrevious();
    void updateVisibleItems();

    // Private methods to handle vgmPlayer logic
    void navigateVgmUp();
    void navigateVgmDown();
    void selectItemVgmPlayer();
    void returnVgmPlayer();
    void navigateVgmDownRoutine();
    void navigateVgmUpRoutine();

    // Finds the index item of the selected .vgm file within the .vgm files list
    int findVgmIndexInList(const String &selectedItem, const std::vector<String> &vgmFilesList);

    bool vgmIsPlaying = false;
    bool vgm_has_ended = false;

    // Strings -> Gd3 tag struct
    String getTrack;
    String getAuthor;
    String getGame;
    String getSystem;
};

#endif