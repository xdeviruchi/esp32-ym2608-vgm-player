// MenuManager.cpp
#include "MenuManager.h"
#include <functional>
#include <algorithm>



// Constructor for the class -> gets an instance of the other classes to be able to access public methods
MenuManager::MenuManager(FileManager &fileManager, DisplayManager &displayManager, InputManager &inputManager, YM2608 &ym2608, VgmManager &vgmManager)
    : fileManager(fileManager), displayManager(displayManager), inputManager(inputManager), ym2608(ym2608), vgmManager(vgmManager) {}

void MenuManager::init() {

    // Initializes the state machine for the main menu
    currentState = MENU_MAIN_STATE;
    // Initializes the file list in the main directory ("/")

    isOnAutoPlay = true;  // Autoplay mode starts as true

    listaDeArquivos = fileManager.listFiles("/", false);
    totalItems = static_cast<int>(listaDeArquivos.size());
    selectedIndex = 0;  // Sets the initial value of selectedIndex
    currentPath = "/";   // Initializes the current directory as the root directory
    verticalOffset = 0;  // Initializes the vertical offset

    // Logic for .vgm files
    selectedVgmIndex = 0;  // Current index for the .vgm file in the list -> variable initialized to 0
    totalVgmItems = static_cast<int>(vgmFilesList.size()); // Total quantity of .vgm items

    Serial.println("Total items after init(): " + String(totalItems));
}

void MenuManager::updateVisibleItems() {
    displayManager.updateVisibleItems();
}

void MenuManager::getGd3Info(){
    getTrack = vgmManager.gd3Info.trackNameEnglish;
    getAuthor = vgmManager.gd3Info.trackAuthorEnglish;
    getGame = vgmManager.gd3Info.gameNameEnglish;
    getSystem = vgmManager.gd3Info.systemNameEnglish;
}

// Routine executed when opening .vgm files
void MenuManager::openVgmFileRoutine(const String &selectedvgmfile){

    vgmManager.setIsVgmEnded(false);

    // Initial routine - Clearing structs and buffers
    vgmManager.resetGd3Info();
    vgmManager.resetVgmInfo();
    vgmManager.clearBufferVgmData();

    // Fills a buffer array with bytes from the header of the selected .vgm file
    vgmManager.fillHeaderBuffer(selectedvgmfile.c_str());

    if (vgmManager.getGD3Offset() == true) // Bool -> Checks if it has a GD3 tag
    {
        vgmManager.fillGd3Buffer(selectedvgmfile.c_str()); // If it has a GD3 tag, fills the buffer related to it
        vgmManager.vgmInfo.GD3Size = vgmManager.getGd3Size();
        vgmManager.readGd3Info();  // Reads, decodes, and stores the GD3 tag information in a public struct present in VgmManager
    }else{
        vgmManager.vgmInfo.GD3Size = 0;
    }

    // Gets info from vgmManager's struct to print information on the OLED display
    getGd3Info();

    vgmManager.processVgmVersion(); /* Factory! -> creates an object on demand according to the .vgm file version and executes the 
    corresponding methods and fills the struct related to .vgm data in the VgmManager class */

    // Gets the musical end byte -> Loop return point / end of the song
    vgmManager.vgmInfo.VgmPlayDataEnd = vgmManager.getVgmPlayDataEnd();

    // Fills the buffer with data related to the Data Block
    vgmManager.fillDataBlockBuffer(selectedvgmfile.c_str());

    // Checks if the file has data block / samples
    if(vgmManager.checkDataBlock() == true){
        // Updates information in the struct!
        vgmManager.getDataBlock();
    }else{
        // If the .vgm file has no data block, the musical data starts exactly at VgmDataOffset!
        vgmManager.vgmInfo.VgmDataOffset = vgmManager.vgmInfo.VgmPlayDataOffset;
    }

    ym2608.resetChip();
    delay(200);
    vgmManager.printVgmStruct();
} 


// To do: move this method to the FileManager class
// Finds the index item of the selected .vgm file within the .vgm files list
int MenuManager::findVgmIndexInList(const String& selectedItem, const std::vector<String>& vgmFilesList) {
    auto it = std::find(vgmFilesList.begin(), vgmFilesList.end(), selectedItem);
    if (it != vgmFilesList.end()) {
        return std::distance(vgmFilesList.begin(), it);
    } else {
        // If not found, returns -1
        return -1;
    }
}

// Navigation methods within the main menu
void MenuManager::navigateUp() {
    if (selectedIndex > 0) {
        selectedIndex--;
    } else {
        // If at the beginning of the list, wrap selection to the end of the list.
        selectedIndex = totalItems - 1;

        // Adjusts to the beginning of the list in cases of circularity
        if (totalItems > itemCountLimit) {
            verticalOffset = totalItems - itemCountLimit;
            updateVisibleItems();
            return;
        }
    }

    // Checks if selection exceeded the visible beginning of the list
    if (totalItems > itemCountLimit && selectedIndex < verticalOffset) {
        // Moves the item block up
        verticalOffset--;

        // Checks if the block exceeded the beginning of the list
        if (verticalOffset < 0) {
            // Adjusts to the beginning of the list
            verticalOffset = totalItems - itemCountLimit;
        }
    }
}

void MenuManager::navigateDown() {
    if (selectedIndex < totalItems - 1) {
        selectedIndex++;
    } else {
        // If at the end of the list, wrap selection to the beginning of the list.
        selectedIndex = 0;

        // Adjusts to the end of the list in cases of circularity
        if (totalItems > itemCountLimit) {
            verticalOffset = 0;
            updateVisibleItems();
            return;
        }
    }

    // Checks if selection exceeded the visible end of the list
    if (totalItems > itemCountLimit && selectedIndex >= verticalOffset + itemCountLimit) {
        // Moves the item block down
        verticalOffset++;

        // Checks if the block exceeded the end of the list
        if (verticalOffset > totalItems - itemCountLimit) {
            // Adjusts to the end of the list
            verticalOffset = 0;
        }
    }
}

void MenuManager::selectItem() {
    Serial.println("Entering selectItem()");
    selectedItem = listaDeArquivos[selectedIndex];
    displayManager.setCurrentPath(selectedItem);
    displayManager.setCurrentTitle(selectedItem);

    // Create separate list for .vgm files initially
    vgmFilesList = fileManager.listVgmFilesInCurrentDirectory();
    totalVgmItems = static_cast<int>(vgmFilesList.size()); // Updates total .vgm items
    
    // Find the current index of the selected .vgm file 
    selectedVgmIndex = findVgmIndexInList(selectedItem, vgmFilesList);

    // Adds "/" to the beginning of the new directory, if necessary
    if (!selectedItem.startsWith("/")) {
            selectedItem = "/" + selectedItem;
    }

    // Builds the full path considering the current directory
    fullPath = fileManager.getCurrentDirectory() + selectedItem;
    selectedVgmFilePath = fullPath;

    // Prints debug message
    Serial.println("Opening: " + fullPath);

    // Here you can check if it is a directory using the SD library function
    if (fileManager.isDirectory(fullPath)) {
        // It is a directory, navigate inside
        Serial.println("Navigating inside directory: " + fullPath);

        // Changes the current directory in FileManager
        fileManager.changeDirectory(fullPath, false);

        // Updates the file list with files from the new directory
        listaDeArquivos = fileManager.listCurrentDirectory(false);

        // Moves the item block to the initial position
        verticalOffset = 0;

        // Sets selectedIndex to 0 when entering a folder
        selectedIndex = 0;

        // Updates visible items
        updateVisibleItems();
    } else if (selectedItem.endsWith(".vgm")) {
        // Control variable to execute displayManager.clearDisplay() only once!
        displayManager.setClearJustOnetime(false);

        // It is a .vgm file -> redirects to runVgmPlayer
        inVgmMode = true;

        openVgmFileRoutine(selectedVgmFilePath);

        currentState = VGM_PLAYER_STATE;
        
        // Serial.print("selectItem -> trying to open path: ");
        // Serial.println(selectedItem);

        // vgmFilesList contains only the .vgm files of this directory
        // Serial.println(".vgm files in directory:");
        // for (const auto &vgmFile : vgmFilesList) {
        //     Serial.println(vgmFile);
        // }
    } else {
        // Unsupported file -> redirects to runException()
        inWrongFileMode = true;
        currentState = WRONG_FILE_STATE;
    }

    // Updates total items
    totalItems = static_cast<int>(listaDeArquivos.size());
    Serial.println("Total items after selectItem(): " + String(totalItems));

    Serial.println("Exiting selectItem()");
}

void MenuManager::returnToPrevious() {
    Serial.println("Entering returnToPrevious()");
    // Gets the previous directory from FileManager
    String previousDirectory = fileManager.returnToPreviousDirectory();

    currentState = MENU_MAIN_STATE;
    inVgmMode = false;
    inWrongFileMode = false;

    // Adds "/" to the beginning of the previous directory, if necessary
    if (!previousDirectory.startsWith("/")) {
        previousDirectory = "/" + previousDirectory;
    }

    // Prints debug message
    Serial.println("Returning to previous directory: " + previousDirectory);

    // Here you can check if it is a directory using the SD library function
    if (fileManager.isDirectory(previousDirectory) == true) {
        // It is a directory, navigate inside
        Serial.println("Navigating inside directory: " + previousDirectory);

        // Changes the current directory in FileManager
        fileManager.changeDirectory(previousDirectory, false);

        // Updates the file list with files from the previous directory
        listaDeArquivos = fileManager.listCurrentDirectory(false);

        // Extracts only the previous directory name to update the title on the display
        String previousDirectoryName = fileManager.getDirectoryName(previousDirectory);
        displayManager.setCurrentTitle(previousDirectoryName);

        // Moves the item block to the initial position
        verticalOffset = 0;

        // Sets selectedIndex to 0 when entering a folder
        selectedIndex = 0;
    } 

    // Updates total items
    totalItems = static_cast<int>(listaDeArquivos.size());
    Serial.println("Total items after returnToPrevious(): " + String(totalItems));

    Serial.println("Exiting returnToPrevious()");
}

void MenuManager::navigateVgmUpRoutine(){
    // Decrements current index, checking if it does not exceed the lower limit
    selectedVgmIndex = (selectedVgmIndex - 1 + totalVgmItems) % totalVgmItems;

    // Gets the path for the current file -> Also considers current directory path if inside a folder
    selectedVgmFilePath = fileManager.getCurrentDirectory() + "/" + vgmFilesList[selectedVgmIndex];

    // Checks if the first character is a slash -> if not, adds it
    if (!selectedVgmFilePath.startsWith("/"))
    {
        // Adds a slash at the beginning of the path
        selectedVgmFilePath = "/" + selectedVgmFilePath;
        }

        // Control variable to execute displayManager.clearDisplay() only once!
        displayManager.setClearJustOnetime(false);

        // Calls the function to open the current .vgm file
        openVgmFileRoutine(selectedVgmFilePath);

        // Serial.print("navigateVgmUp -> trying to open path: ");
        // Serial.println(selectedVgmFilePath);
}

// Navigation methods within VgmPlayer
void MenuManager::navigateVgmUp() {
    // Standard routine
    stopMusicRoutine();  // Stops the music
    currentState = VGM_PLAYER_STATE;  // Changes to VgmPlayer state
    navigateVgmUpRoutine();   // Accesses the new .vgm file, updates OLED display info, etc

    // If in autoplay mode, plays the music automatically
    if(isOnAutoPlay){
        updateDisplayVgmMode();
        vgm_is_playing = true;
        currentState = VGM_PLAYING_STATE;
    }
}

void MenuManager::navigateVgmDownRoutine(){
    // Increments current index, checking if it does not exceed the upper limit
    selectedVgmIndex = (selectedVgmIndex + 1) % totalVgmItems;

        // Gets the path for the current file -> Also considers current directory path if inside a folder
        selectedVgmFilePath = fileManager.getCurrentDirectory() + "/" + vgmFilesList[selectedVgmIndex];

        // Checks if the first character is a slash -> if not, adds it
        if (!selectedVgmFilePath.startsWith("/")) {
            // Adds a slash at the beginning of the path
            selectedVgmFilePath = "/" + selectedVgmFilePath;
        }

        // Control variable to execute displayManager.clearDisplay() only once!
        displayManager.setClearJustOnetime(false);

        // Calls the function to open the current .vgm file
        openVgmFileRoutine(selectedVgmFilePath);

        // Serial.print("navigateVgmDown -> trying to open path: ");
        // Serial.println(selectedVgmFilePath);
}

void MenuManager::navigateVgmDown() {

    // Standard routine
    stopMusicRoutine();  // Stops the music
    currentState = VGM_PLAYER_STATE;  // Changes to VgmPlayer state
    navigateVgmDownRoutine();   // Accesses the new .vgm file, updates OLED display info, etc

    // If in autoplay mode, plays the music automatically
    if(isOnAutoPlay){
        updateDisplayVgmMode();
        vgm_is_playing = true;
        currentState = VGM_PLAYING_STATE;
    }
}

void MenuManager::selectItemVgmPlayer(){
    
    if(currentState == VGM_PLAYER_STATE && !vgm_is_playing){
    vgm_is_playing = true;
    currentState = VGM_PLAYING_STATE;
    }
}

void MenuManager::returnVgmPlayer(){
    stopMusicRoutine();  // Stops the music
    currentState = VGM_PLAYER_STATE;
}

void MenuManager::stopMusicRoutine(){
    vgm_is_playing = false;
    vgmManager.closeVgmFile();  // Closes the .vgm file currently playing
    suspendAndDestroyTask();  // Suspends and destroys the task
    ym2608.resetChip();
}

// State machine to manage all menus
void MenuManager::run(){

    switch (currentState) {
        case MENU_MAIN_STATE: //
            runMenu();
            break;
        case WRONG_FILE_STATE://
            runException();
            break;
        case VGM_PLAYING_STATE: 
            if(vgm_is_playing){  // Checks if the flag indicating whether we are playing music or not is valid
                playVgmState();
            } else{  // Otherwise, return to vgm player menu
                currentState = VGM_PLAYER_STATE;
            }
            break;
        case VGM_PLAYER_STATE: //
            runVgmPlayer();
            break;
    }
}

void MenuManager::runMenu() {
    // Constantly checks button states
    inputManager.checkInput();

    // Updates selectedIndex and handles special cases if necessary
    if (inputManager.isButtonDown()) {
        navigateDown();
        Serial.println("Down Button Pressed!");
    } else if (inputManager.isButtonUp()) {
        navigateUp();
        Serial.println("Up Button Pressed!");
    } else if (inputManager.isButtonSelect()) {
        selectItem();
        Serial.println("Select Button Pressed!");
    } else if (inputManager.isButtonReturn()) {
        returnToPrevious();
        Serial.println("Return Button Pressed!");
    }

    // Displays only the visible item block
    int startIndex = verticalOffset;
    int endIndex = startIndex + itemCountLimit;
    displayManager.showMenu(currentPath.c_str(), listaDeArquivos, selectedIndex, itemCountLimit, startIndex, endIndex);
}

void MenuManager::checkButtonsVgmMode(){

    // Constantly checks button states
    inputManager.checkInput();

    // Checks buttons -> navigation logic within VgmPlayer
    if (inputManager.isButtonDown()) {
        navigateVgmDown();
        Serial.println("Down Button Pressed! (inVgmMode)");
    } else if (inputManager.isButtonUp()) {
        navigateVgmUp();
        Serial.println("Up Button Pressed! (inVgmMode)");
    } else if (inputManager.isButtonSelect()) {
        selectItemVgmPlayer();
        Serial.println("Select Button Pressed! (inVgmMode)");
    } else if (inputManager.isButtonReturn()) {
        returnVgmPlayer();
        Serial.println("Return Button Pressed! (inVgmMode)");
    }
}

void MenuManager::updateDisplayVgmMode(){
    // Displays the menu related to VgmPlayer
    displayManager.showVgmPlayer(vgmFilesList, getTrack, getAuthor, getGame, getSystem, selectedVgmIndex, totalVgmItems);
}

void MenuManager::playVgm(){
    // Plays the selected .vgm file
    vgmManager.playVgmRoutine(selectedVgmFilePath);
}

void MenuManager::playVgmState() {
    vgm_has_ended = vgmManager.getIsVgmEnded();

    // Serial.print("(I) vgm_has_ended = ");
    // Serial.println(vgm_has_ended);
    if(!vgm_has_ended){
    const uint32_t stackSize = 110000;

    xTaskCreatePinnedToCore(
        TaskHighPriorityWrapper,
        "Task1",
        stackSize, // Stack size
        this,
        configMAX_PRIORITIES - 1,    // Task priority  
        &Task1,
        1  // Core where the task will run
    );

    checkButtonsVgmMode();
    }else{
        // Serial.print("(F) vgm_has_ended = ");
        // Serial.println(vgm_has_ended);
        navigateVgmDown();
    }
}

// Function to suspend and delete the task
void MenuManager::suspendAndDestroyTask() {
    if (Task1 != NULL) {
        vTaskSuspend(Task1);  // Suspends the task

        // Now the task is suspended, we can delete it
        vTaskDelete(Task1);
        Task1 = NULL;  // Resets identifier after deletion
    }
}

void MenuManager::runVgmPlayer(){
    // // Checks button input and executes corresponding methods
    checkButtonsVgmMode();

    // Updates the OLED display with information related to the selected .vgm file
    updateDisplayVgmMode();
}

void MenuManager::runException(){
    // Constantly checks button states
    inputManager.checkInput();

    // Updates selectedIndex and handles special cases if necessary
    if (inputManager.isButtonReturn()) {
        returnToPrevious();
        Serial.println("Return Button Pressed! Returning from exception!");
    }

    // Displays the menu related to VgmPlayer
    displayManager.notVgmFile();

}

// Play music -> maximum priority core 1
void MenuManager::TaskHighPriority(void *pvParameters) {
    Serial.println("TaskHighPriority running on core: " + String(xPortGetCoreID()));
    MenuManager* menuManagerInstance = reinterpret_cast<MenuManager*>(pvParameters);
    menuManagerInstance->playVgm();
}

// Check buttons 
void MenuManager::TaskLowPriority(void *pvParameters) {
    Serial.println("TaskLowPriority running on core: " + String(xPortGetCoreID()));
    MenuManager* menuManagerInstance = reinterpret_cast<MenuManager*>(pvParameters);
    menuManagerInstance->checkButtonsVgmMode();
}

void MenuManager::TaskHighPriorityWrapper(void *pvParameters) {
    MenuManager* menuManagerInstance = reinterpret_cast<MenuManager*>(pvParameters);
    menuManagerInstance->TaskHighPriority(pvParameters);
}

void MenuManager::TaskLowPriorityWrapper(void *pvParameters) {
    MenuManager* menuManagerInstance = reinterpret_cast<MenuManager*>(pvParameters);
    menuManagerInstance->TaskLowPriority(pvParameters);
}