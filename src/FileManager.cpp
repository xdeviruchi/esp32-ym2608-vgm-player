// FileManager.cpp
#include "FileManager.h"
#include <SPI.h>

//SD Card Module
#define SD_CS (5)
#define SD_CLK (18)
#define SD_MOSI (23)
#define SD_MISO (19)

FileManager::FileManager() {
    // Initialize variables as needed
}

void FileManager::init() {
    // Initial configuration
    SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_CS);
    SPI.setDataMode(SPI_MODE0);
    SD.begin(SD_CS);

// Initializes the SD card 
  if (!SD.begin(5)) {
      Serial.println("Failed to initialize SD card");
  } else {
      Serial.println("SD card successfully initialized");
  }
}

std::vector<String> FileManager::listFiles(const char *path, bool recursive, bool isForMenu) {
    std::vector<String> fileList;

    File root = SD.open(path);
    if (!root) {
        Serial.println("Failed to open directory");
        return fileList;
    }

    listFilesRecursive(root, String(path), fileList, recursive, isForMenu);

    root.close();
    return fileList;
}

void FileManager::listFilesRecursive(File &dir, const String &basePath, std::vector<String> &fileList, bool recursive, bool isForMenu) {
    dir.rewindDirectory();  // Ensures reading starts from the beginning of the directory

    File file = dir.openNextFile();
    while (file) {
        String itemName = isForMenu ? getCleanItemName(file.name()) : file.name();
        String fullPath = basePath + itemName;  // Keeps the full path for navigation

        fileList.push_back(itemName);

        // If it is a directory and the recursive option is enabled, lists the files in this directory
        if (recursive && file.isDirectory()) {
            listFilesRecursive(file, fullPath + "/", fileList, recursive, isForMenu);
        }

        file = dir.openNextFile();
    }
}

String FileManager::getCleanItemName(const String &itemPath) {
    // Finds the last slash in the string
    size_t lastSlashIndex = itemPath.lastIndexOf('/');

    // Returns the item name (without additional slashes)
    return itemPath.substring(lastSlashIndex + 1);
}

String FileManager::getCurrentDirectory() const {
    return currentDirectory;
}

String FileManager::getDirectoryName(const String& path) {
    int lastSlashIndex = path.lastIndexOf("/");
    if (lastSlashIndex != -1) {
        return path.substring(lastSlashIndex + 1);
    }
    return path;
}

std::vector<String> FileManager::listCurrentDirectory(bool recursive) {
    // Lists the files in the current directory
    std::vector<String> fileList = listFiles(currentDirectory.c_str(), recursive);

    // Removes parent directories ('.') and parent of the current directory ('..')
    fileList.erase(std::remove_if(fileList.begin(), fileList.end(), [](const String &file) {
        return file == "." || file == "..";
    }), fileList.end());

    return fileList;
}

std::vector<String> FileManager::listVgmFilesInCurrentDirectory() {
    // Checks if the current directory is empty and adds a slash if necessary -> issues when opening files in the root directory
    if (currentDirectory == "") {
        currentDirectory = "/";
    }

    // Lists the files in the current directory
    std::vector<String> fileList = listFiles(currentDirectory.c_str(), false);


    // Filters only the files with the .vgm extension in the current directory
    fileList.erase(std::remove_if(fileList.begin(), fileList.end(), [this](const String &file) {
        String filePath = currentDirectory + "/" + file;
        return !SD.exists(filePath.c_str()) || !file.endsWith(".vgm");
    }), fileList.end());

    return fileList;
}

void FileManager::changeDirectory(const String &newDirectory, bool recursive) {

    // Adds the current directory to the visited directories list before changing
    addToVisitedDirectories(currentDirectory);

    // Adds "/" to the beginning of the new directory, if necessary
    String targetDirectory = (!newDirectory.startsWith("/")) ? ("/" + newDirectory) : newDirectory;

    currentDirectory = targetDirectory;

    // Updates the file list with the files from the new directory
    listSubdirectories(currentDirectory, subdirectories);

    // Add this log to check the list size after updating
    Serial.println("List size after listCurrentDirectory(): " + String(subdirectories.size()));
}

void FileManager::listSubdirectories(const String &path, std::vector<String> &subdirectories) {
    File directory = SD.open(path.c_str());
    if (!directory || !directory.isDirectory()) {
        Serial.println("Failed to open or not a directory");
        return;
    }

    Serial.println("Listing subdirectories in: " + path);

    directory.rewindDirectory();  // Ensures reading starts from the beginning of the directory

    File file = directory.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            String directoryName = file.name();
            subdirectories.push_back(directoryName);
            Serial.println("Found subdirectory: " + directoryName);
        }
        file = directory.openNextFile();
    }

    directory.close();
}

bool FileManager::isDirectory(const String &path) {
    File file = SD.open(path.c_str());
    bool isDir = file.isDirectory();
    file.close();
    return isDir;
}

String FileManager::returnToPreviousDirectory() {
    String previousDir;

    // If the visited directories vector is not empty, returns to the last visited directory
    if (!visitedDirectories.empty()) {
        previousDir = visitedDirectories.back();
        visitedDirectories.pop_back();

        // Checks if the previous directory is equal to the current directory
        if (previousDir != currentDirectory) {
            // Directly updates the currentDirectory variable
            currentDirectory = previousDir;

            // Clears the subdirectories vector, since we are returning to a previous directory
            subdirectories.clear();
        } else {
            Serial.println("Previous directory is the same as the current directory. Ignoring.");
            return "";
        }
    } else {
        Serial.println("Visited directories vector is empty. Cannot change.");
    }

    return previousDir;
}

void FileManager::addToVisitedDirectories(const String &directory) {
    visitedDirectories.push_back(directory);
}


void FileManager::updateCurrentDirectory(const String &newDirectory) {
    Serial.println("Updating current directory to: " + newDirectory);
    currentDirectory = newDirectory;
}

void FileManager::rewindDirectory() {
    Serial.println("Calling rewindDirectory");

    // If the visited directories vector is not empty, returns to the last visited directory
    if (!visitedDirectories.empty()) {
        String previousDir = visitedDirectories.back();
        visitedDirectories.pop_back();

        // Updates the current directory
        updateCurrentDirectory(previousDir);

        Serial.println("Current directory after rewindDirectory: " + currentDirectory);
    } else {
        Serial.println("Visited directories vector is empty. Cannot change.");
    }
}

void FileManager::debugSDContent() {
    if (!debugPrinted) {
        Serial.println("SD card root directory content:");
        File root = SD.open("/");
        while (File file = root.openNextFile()) {
            if (file.isDirectory() || strstr(file.name(), ".vgm")) {
                Serial.print("  ");
                Serial.println(file.name());
            }
            file.close();
        }
        root.close();
        debugPrinted = true;
    }
}

void FileManager::debugFolderOpen() {
    Serial.println("'System Volume Information' folder content:");

    File systemVolumeInfo = SD.open("/System Volume Information");
    if (!systemVolumeInfo || !systemVolumeInfo.isDirectory()) {
        Serial.println("Failed to open or not a directory");
        return;
    }

    while (File file = systemVolumeInfo.openNextFile()) {
        Serial.print("  ");
        Serial.println(file.name());
        file.close();
    }

    systemVolumeInfo.close();
}