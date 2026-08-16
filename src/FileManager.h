// FileManager.h
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <FS.h>
#include <SD.h>
#include <vector>


class FileManager {
public:
    FileManager();

    // Initial class settings -> instantiate the object and then call this function in void setup
    void init(); 
    
    // List the names of all files on the SD card
    std::vector<String> listFiles(const char *path, bool recursive = false, bool isForMenu = false);

    void listFilesRecursive(File &dir, const String &basePath, std::vector<String> &fileList, bool recursive, bool isForMenu);

    // Gets only the item names from the directories to be displayed on the OLED display
    String getCleanItemName(const String &itemPath);

    // Debug function to list .vgm files and existing directories in the root directory of the SD card
    void debugSDContent(); 

    // Function to enter the "System Volume Information" folder -> check syntax for opening folders using SD.open
    void debugFolderOpen();

    // Method to go back a directory
    void rewindDirectory();

    // Method to get the current directory
    String getCurrentDirectory() const;

    // Gets the current directory name, ignoring the full path (nested folders logic)
    String getDirectoryName(const String& path);

    // Changes the current directory to the specified one
    void changeDirectory(const String &newDirectory, bool recursive);

    // Lists the files in the current directory
    std::vector<String> listCurrentDirectory(bool recursive);

    // Lists the .vgm files in the current directory
    std::vector<String> listVgmFilesInCurrentDirectory();

    // Lists the subdirectories contained in a directory/folder
    void listSubdirectories(const String &path, std::vector<String> &subdirectories);

    // Make public the method to check if something inside the sd is a directory or not, returns isDir
    bool isDirectory(const String &path);

    // Return to the previous directory
    String returnToPreviousDirectory();

    // Adds the current directory to the list of visited directories
    void addToVisitedDirectories(const String &directory);

private:
    const char *sdCardPath = "/sdcard"; // Define the SD card path as needed
    bool debugPrinted = false; // Flag to prevent multiple prints of the SD content
    String currentDirectory; // Variable to allocate the current directory accessed by the SD card
    
    // List of visited directories
    std::vector<String> visitedDirectories;

    // Adds a list of subdirectories as a class member
    std::vector<String> subdirectories;

    // Private method to update the current directory
    void updateCurrentDirectory(const String &newDirectory);
};

#endif