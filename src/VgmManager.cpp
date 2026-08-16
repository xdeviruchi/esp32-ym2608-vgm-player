//VgmManager.cpp
#include "VgmManager.h"
#include <algorithm>


#define HEADER_SIZE_BYTES  256
#define MUSIC_BUFFER_SIZE 65536
#define GD3_ARRAY_SIZE 1024
#define DATA_BLOCK_ARRAY_SIZE 11

//Buffer to store VGM file header data
unsigned char bufferVgmData[HEADER_SIZE_BYTES] = {};

//Buffer for sending music data to the chip -> circular logic
unsigned char bufferMusicData[MUSIC_BUFFER_SIZE] = {};

//Buffer to store information regarding the Tag
unsigned char bufferGd3Info[GD3_ARRAY_SIZE] = {};

//Buffer to store data block information present in the .vgm file
unsigned char bufferDataBlock[DATA_BLOCK_ARRAY_SIZE] = {};

const size_t incrementVGMLUT[256] = {
    //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x00
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x10
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x20
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x30
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x40
        1, 1, 1, 1, 1, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x50
        1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x60
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x70
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x80
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x90
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xa0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xb0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xc0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xd0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xe0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0xf0
    };


//Default Constructor
VgmManager::VgmManager() {
}

void VgmManager::init(){
    //Initializes the class ensuring that the GD3 and Vgm Info structs are empty
    resetVgmInfo();
    resetGd3Info();
}

// Main function: reads music bytes from the .vgm file in a circular manner (goes to the end and returns to the loop point)
// Reads the bytes stored internally in the buffer
void VgmManager::playVgmRoutine(const String &Path) {
    YM2608 ym2608_;
    file_vgm_playing = SD.open(Path.c_str());
    clearBufferVgmData();

    if (!file_vgm_playing) {
        Serial.println("Error opening file!");
        return;
    }
    isVgmEnded = false;
    InitialByte = vgmInfo.VgmPlayDataOffset;
    file_vgm_playing.seek(InitialByte);
    CurrentByte = InitialByte;
    bufferVgmIndex = 0;
    loopTimes = 1; // Number of times the music will loop -> TODO: GET THIS FROM THE FILE!
    size_t actualLoop = 0; // Current loop -> always starts at zero
    unsigned char indexIncrement = 0;
    size_t increment = 0;

    // Flag to control whether the loop should be repeated
    bool repeatLoop = false;

    do {
        //Serial.println("Entering main loop");

        while (CurrentByte < vgmInfo.VgmPlayDataEnd) {
            file_vgm_playing.seek(CurrentByte);

            //Circular buffer filling logic
            uint32_t remainingBytes = std::abs(static_cast<int32_t>(vgmInfo.VgmPlayDataEnd - CurrentByte)); //Calculates the amount of remaining bytes until the end of the music
            uint32_t blockSize = (remainingBytes < MUSIC_BUFFER_SIZE) ? remainingBytes : MUSIC_BUFFER_SIZE;  //Determines block size
            uint32_t bytesRead = file_vgm_playing.read(bufferMusicData, blockSize);  //Reads block size and populates the buffer
            bufferVgmIndex = 0; //Initial index = 0

            //Calls playVGM to read the buffer and play the music
            while (bufferVgmIndex < bytesRead) {
                //Obtains the byte currently read in the buffer 
                indexIncrement = bufferMusicData[bufferVgmIndex];

                //Obtains the increment directly from LUT -> byte value = element read from LUT
                increment = incrementVGMLUT[indexIncrement];

                //Calls playVGM to process the current byte
                ym2608_.playVGM(bufferVgmIndex, bufferMusicData);

                //Adds the increment to bufferVgmIndex
                bufferVgmIndex += increment;
            }

            //Positions the read cursor for the next block of bytes
            CurrentByte += bytesRead;
        }


        // Reached the end of the music -> If it's not 'oneShot', return to the loop point
        if (actualLoop < loopTimes && vgmInfo.LoopOffset != 0x0) {

            InitialByte = vgmInfo.LoopOffset;
            CurrentByte = InitialByte;

            file_vgm_playing.seek(CurrentByte);
            actualLoop++; // Increments the value every time a loop is executed
            repeatLoop = true;
        } else {
            repeatLoop = false;
        }

    } while (repeatLoop);

    //Serial.println("Exiting music play condition! Destroying file_vgm_playing instance");
    isVgmEnded = true;

    //Small delay to allow time for the file to close
    delay(100);

    //File closure done manually -> MenuManager.cpp
    //file_vgm_playing.close();
}

// Returns true if the file is still open, false otherwise
bool VgmManager::isVgmFileOpen() const {
    return file_vgm_playing;
}

void VgmManager::closeVgmFile(){
    if (file_vgm_playing){
        file_vgm_playing.close();
    }
}

void VgmManager::clearBufferVgmData() {
    memset(bufferMusicData, 0, MUSIC_BUFFER_SIZE);
}

void VgmManager::printVgmStruct(){

    Serial.print("EofOffset: 0x");
    Serial.println(vgmInfo.EofOffset, HEX);

    Serial.print("GD3 Offset: 0x");
    Serial.println(vgmInfo.GD3Offset, HEX);

    Serial.print("GD3 Size Absolute: 0x");
    Serial.println(vgmInfo.GD3Size, HEX);

    Serial.print("Loop Offset: 0x");
    Serial.println(vgmInfo.LoopOffset, HEX);

    Serial.print("Vgm Data Offset: 0x");
    Serial.println(vgmInfo.VgmDataOffset, HEX);

    Serial.print("YM2608 Clock: ");
    Serial.println(vgmInfo.Ym2608Clock);

    Serial.print("Extra HeaderOffset: 0x");
    Serial.println(vgmInfo.ExtraHeaderOffset, HEX);

    Serial.print("Data Block Size: 0x");
    Serial.println(vgmInfo.DataBlockSize, HEX);

    Serial.print("Data Block Start Adress of Data: 0x");
    Serial.println(vgmInfo.DataBlockStartAdressofData, HEX);

    Serial.print("Vgm Play Data Offset: 0x");
    Serial.println(vgmInfo.VgmPlayDataOffset, HEX);

    Serial.print("Vgm End Data Offset: 0x");
    Serial.println(vgmInfo.VgmPlayDataEnd, HEX);
}

//Fills the buffer with header information regarding the selected file
void VgmManager::fillHeaderBuffer(const String &Path) {
    File file = SD.open(Path.c_str());

    if (file) {
        // Positions the pointer at the beginning of the file
        file.seek(0);

        // Performs buffer reading
        file.read(bufferVgmData, HEADER_SIZE_BYTES);

        // Closes the file
        file.close();
    } else {
        // Handles the case where the file cannot be opened
        Serial.println("Error opening file!");
    }
}

//Fills the buffer with information regarding the Data Block in the .vgm file
void VgmManager::fillDataBlockBuffer(const String &Path){
    File file = SD.open(Path.c_str());

    //While the file instance is open
    if (file) {
    // Positions the pointer at the beginning of Vgm Data
    //file.seek(vgmInfo.VgmDataOffset);
    file.seek(vgmInfo.VgmDataOffset);

    // Performs buffer reading
    file.read(bufferDataBlock, DATA_BLOCK_ARRAY_SIZE);

    // Closes the file
    file.close();
    } else {
    // Handles the case where the file cannot be opened
        Serial.println("Error opening file!");
    }
}

//Fills the buffer with information regarding the gd3 tag, if present
void VgmManager::fillGd3Buffer(const String &Path) {
    File file = SD.open(Path.c_str());

    if (file) {
        // Positions the pointer at the beginning of the file
        file.seek(vgmInfo.GD3Offset);

        // Performs buffer reading
        file.read(bufferGd3Info, GD3_ARRAY_SIZE);

        // Closes the file
        file.close();
    } else {
        // Handles the case where the file cannot be opened
        Serial.println("Error opening file!");
    }
}

void VgmManager::readGd3Info() {
    // Array of pointers to strings in the struct
    String* gd3Strings[] = {
        &gd3Info.trackNameEnglish,
        &gd3Info.trackNamenonEnglish,
        &gd3Info.gameNameEnglish,
        &gd3Info.gameNameNonEnglish,
        &gd3Info.systemNameEnglish,
        &gd3Info.systemNameNonEnglish,
        &gd3Info.trackAuthorEnglish,
        &gd3Info.trackAutorNonEnglish,
        &gd3Info.releaseDate,
        &gd3Info.dumper
    };

    int bufferIndex = 11;

    for (String* currentString : gd3Strings) {
        // Adds the current string to the struct
        while (bufferGd3Info[bufferIndex] != 0x00 || bufferGd3Info[bufferIndex + 1] != 0x00) {
            // Ignores 0x00 bytes between characters
            if (bufferGd3Info[bufferIndex] != 0x00) {
                *currentString += char(bufferGd3Info[bufferIndex]);
            }
            bufferIndex++;
        }

        bufferIndex += 2; // Skips the 0x00, 0x00 byte sequence at the end of the string
    }

    // // Prints each element of the struct to the Serial Monitor at the end of the function
    // Serial.println("GD3 Info:");
    // Serial.print("Track Name (English): ");
    // Serial.println(gd3Info.trackNameEnglish);
    // // Serial.print("Track Name (Non-English): ");
    // // Serial.println(gd3Info.trackNamenonEnglish);
    // Serial.print("Game Name (English): ");
    // Serial.println(gd3Info.gameNameEnglish);
    // // Serial.print("Game Name (Non-English): ");
    // // Serial.println(gd3Info.gameNameNonEnglish);
    // Serial.print("System Name (English): ");
    // Serial.println(gd3Info.systemNameEnglish);
    // // Serial.print("System Name (Non-English): ");
    // // Serial.println(gd3Info.systemNameNonEnglish);
    // Serial.print("Track Author (English): ");
    // Serial.println(gd3Info.trackAuthorEnglish);
    // // Serial.print("Track Author (Non-English): ");
    // // Serial.println(gd3Info.trackAutorNonEnglish);
    // // Serial.print("Release Date: ");
    // // Serial.println(gd3Info.releaseDate);
    // // Serial.print("Dumper: ");
    // // Serial.println(gd3Info.dumper);
}

//Function to clear the information present in the GD3 struct
void VgmManager::resetGd3Info() {
    gd3Info.trackNameEnglish.clear();
    gd3Info.trackNamenonEnglish.clear();
    gd3Info.gameNameEnglish.clear();
    gd3Info.gameNameNonEnglish.clear();
    gd3Info.systemNameEnglish.clear();
    gd3Info.systemNameNonEnglish.clear();
    gd3Info.trackAuthorEnglish.clear();
    gd3Info.trackAutorNonEnglish.clear();
    gd3Info.releaseDate.clear();
    gd3Info.dumper.clear();
}

//Function to clear the information present in the VgmInfo struct
void VgmManager::resetVgmInfo() {
    vgmInfo.EofOffset = 0;
    vgmInfo.GD3Offset = 0;
    vgmInfo.GD3Size = 0;
    vgmInfo.LoopOffset = 0;
    vgmInfo.VgmDataOffset = 0;
    vgmInfo.Ym2608Clock = 0;
    vgmInfo.ExtraHeaderOffset = 0;
    vgmInfo.DataBlockSize = 0;
    vgmInfo.DataBlockStartAdressofData = 0;
    vgmInfo.VgmPlayDataOffset = 0;
    vgmInfo.VgmPlayDataEnd = 0;
}

//Verifies if a file is of type .vgm -> execute after buffer population
bool VgmManager::checkVgmFile() {

    //Checks the first 4 bytes and verifies if they match the characters "Vgm "
    if(bufferVgmData[0] == 0x56 && bufferVgmData[1] == 0x67 && bufferVgmData[2] == 0x6D && bufferVgmData[3] == 0x20){
        Serial.println("File is of type .vgm");
        return isVgmFile = true;

        // Not a .vgm file
    } else{
        Serial.println("Selected file is not in .vgm format");
        return isVgmFile = false;
    }
}

//Checks the vgm file version and updates the enum variable with the corresponding version
VgmManager::vgmVersion VgmManager::checkVgmVersion(){
    //Reads the bytes pertaining to the VGM file version and stores them in a uint32_t
    // Bytes read -> 0x08 to 0x0B (4 bytes) - Little Endian
    uint32_t readedBytesVersion = readBytesLittleEndian(bufferVgmData, 8, 4);

    //Enum type member variable -> store the VGM version value
    VgmManager::vgmVersion version;

    switch (readedBytesVersion)
    {
    case 0x150:
        version = Vgm_150;
        return version;
        Serial.println("Vgm 1.50");
        break;

    case 0x151:
        version = Vgm_151;
        return version;
        Serial.println("Vgm 1.51");
        break;

    case 0x160:
        version = Vgm_160;
        return version;
        Serial.println("Vgm 1.60");
        break;

    case 0x161:
        version = Vgm_161;
        return version;
        Serial.println("Vgm 1.61");
        break;

    case 0x170:
        version = Vgm_170;
        return version;
        Serial.println("Vgm 1.70");
        break;

    case 0x171:
        version = Vgm_171;
        return version;
        //Serial.println("Vgm 1.71");
        break;

    default:
        //Serial.println("unrecognized VGM version");
        version = Non_Vgm;
        break;
    }

    return version;
}



// Main function to verify, process, and instantiate the correct version for an object corresponding to the detected .vgm version
void VgmManager::processVgmVersion() {
    // Verifies the VGM file version
    VgmManager::vgmVersion currentVersion = checkVgmVersion();

    // Call the factory to create the correct instance based on the enum value
    std::unique_ptr<VgmVersionFactory> vgmVersionInstance = VgmVersionFactory::createVgmVersion(currentVersion, vgmInfo);

    if (vgmVersionInstance) {
        // Gets the raw pointer to the underlying object and prints the address
        // Serial.print("Object address: ");
        // Serial.println((uintptr_t)vgmVersionInstance.get(), HEX);

        // Call the specific method for this version
        vgmVersionInstance->processVgm(GetVgmInfo());
    } else {
        // If the version is not recognized, print an error message
        Serial.println("Unrecognized VGM version");
    }
}

//Getter for isVgmEnded
bool VgmManager::getIsVgmEnded() const {
    return isVgmEnded;
}

//Setter for isVgmEnded
void VgmManager::setIsVgmEnded(bool value) {
    isVgmEnded = value;
}


// Getter that returns a reference to the VgmInfo object
VgmManager::VgmInfo& VgmManager::GetVgmInfo() {
    return vgmInfo;
}

// Getter that returns a constant reference to the VgmInfo object
const VgmManager::VgmInfo& VgmManager::GetVgmInfo() const {
    return vgmInfo;
}

// ------------ Vgm Bytes/Manipulation ----------------

//Obtains End of file Offset from the header and updates the struct with the obtained information
uint32_t VgmManager::getEofOffset(VgmInfo &info){
    uint32_t readedBytesEof = readBytesLittleEndian(bufferVgmData, 4, 4);

    //Adds 0x04 to already consider the absolute byte -> not an offset
    info.EofOffset = readedBytesEof + 0x04;

    // //Debug -> checks if the struct was updated correctly with the information
    // Serial.print("- EOF Offset (absolute byte) = 0x");
    // Serial.println(info.EofOffset, HEX);
    return info.EofOffset;
}

//Obtains the offset referring to the start of GD3 information from the header -> calculates absolute value
bool VgmManager::getGD3Offset(){
    
    //Gets the absolute position of the Gd3Offset byte from the header -> bufferBgmData
    uint32_t readedBytesGd3 = readBytesLittleEndian(bufferVgmData, 20, 4);

    //Adds 0x14 to already consider the absolute byte -> not an offset
    if(vgmInfo.GD3Offset = readedBytesGd3 != 0x00){

        vgmInfo.GD3Offset = readedBytesGd3 + 0x14; //+ 0x08; //Adding original offset + header start to begin directly at strings
        //Debug -> checks if the struct was updated correctly with the information
        // Serial.print("- GD3 (absolute byte) = 0x");
        // Serial.println(vgmInfo.GD3Offset, HEX);
        haveGd3 = true;
        return haveGd3;
    }else{
        vgmInfo.GD3Offset = 0;
        // Serial.println("File does not contain GD3 tag");
        haveGd3 = false;
        return haveGd3;
    }
}

uint32_t VgmManager::getGd3Size(){
    uint32_t readedBytesGd3Size = readBytesLittleEndian(bufferGd3Info, 8, 4);
    return readedBytesGd3Size;
}

uint32_t VgmManager::getVgmPlayDataEnd(){
    uint32_t readedBytesVgmPlayDataEnd;

    //If it has Gd3 tag!
    if(haveGd3 == true){
        //Subtracts 1 byte -> subtracts Gd3 size -> subtracts Gd3 remainder = 12 bytes 
        readedBytesVgmPlayDataEnd = vgmInfo.EofOffset - vgmInfo.GD3Size - 13; 

        //Updates the struct with the corresponding value
        vgmInfo.VgmPlayDataEnd = readedBytesVgmPlayDataEnd;

        return readedBytesVgmPlayDataEnd;
    }else{ 
        //Does not have Gd3 tag! EofOffset represents the final music byte + 1
        readedBytesVgmPlayDataEnd = vgmInfo.EofOffset - 1;

        vgmInfo.VgmPlayDataEnd = readedBytesVgmPlayDataEnd;
        return readedBytesVgmPlayDataEnd;
    }
}

//Obtains Loop Offset from the header and updates the struct with the obtained information
uint32_t VgmManager::getLoopOffset(VgmInfo &info){
    uint32_t readedBytesLoop = readBytesLittleEndian(bufferVgmData, 28, 4);

    //Adds 0x01C to already consider the absolute byte -> not an offset
    info.LoopOffset = readedBytesLoop;

    if(readedBytesLoop == 0x00){ //If value = 0x1C -> There is no loop offset, therefore, the music does not loop!
        return info.LoopOffset;
    } else{ // If the music loops, returns the absolute byte
        info.LoopOffset = readedBytesLoop + 0x1C;
        return info.LoopOffset;
    }

    // //Debug -> checks if the struct was updated correctly with the information
    // Serial.print("- Loop offset (absolute byte) = 0x");
    // Serial.println(info.LoopOffset, HEX);
    
}

//Obtains Loop Offset from the header and updates the struct with the obtained information (Polymorphism)!
uint32_t VgmManager::getVgmDataOffset(VgmInfo &info) {

    uint32_t readedBytesVgmOffset = readBytesLittleEndian(bufferVgmData, 52, 4);

    // Adds 0x04 to already consider the absolute byte and updates the struct -> not an offset
    info.VgmDataOffset = readedBytesVgmOffset + 0x34;

    // // Debug -> checks if the struct was updated correctly with the information
    // Serial.print("- Vgm Data Offset (absolute byte) = 0x");
    // Serial.println(info.VgmDataOffset, HEX);

    return info.VgmDataOffset;
}

//Verifies if the file has a Datablock and populates the struct with the correct information for the start of music data
bool VgmManager::checkDataBlock(){

    // Verifies if the first 3 bytes are 0x67 0x66 0x81 -> Refers to sample usage in the music!
    if (bufferDataBlock[0] == 0x67 && bufferDataBlock[1] == 0x66 && bufferDataBlock[2] == 0x81)
    {
        haveDataBlock = true; //File has Data Block - Type 81: YM2608 DELTA-T ROM
        Serial.println("File has Data Block - Type 81: YM2608 DELTA-T ROM!");
        return haveDataBlock;
    }else{
        haveDataBlock = false; //File does not have Data Block - Type 81: YM2608 DELTA-T ROM
        Serial.println("File does not have Data Block - Type 81: YM2608 DELTA-T ROM!");
        return haveDataBlock;
    }
}

void VgmManager::getDataBlock(){
    //Reads 4 bytes from the third byte to obtain -> Size of the entire ROM
    uint32_t readedBytesDataBlockSize = readBytesLittleEndian(bufferDataBlock, 3, 4);

    //Updates the struct with the read bytes
    vgmInfo.DataBlockSize = readedBytesDataBlockSize;

    //Reads the next 4 bytes and updates the information in the struct -> Start adress of data
    uint32_t readedBytesDataBlockStartAdressofData = readBytesLittleEndian(bufferDataBlock, 7, 4);

    //Updates the struct with the read bytes
    vgmInfo.DataBlockStartAdressofData = readedBytesDataBlockStartAdressofData;

    //Corrects the offset to point to the specific byte where the data block ends and .vgm data begins
    vgmInfo.VgmPlayDataOffset = readedBytesDataBlockSize + 0x107;

    //Serial print for debugging
    Serial.print("- Data Block Size: 0x");
    Serial.println(vgmInfo.DataBlockSize, HEX);
    Serial.print("- Data Block Start Adress of Data: 0x");
    Serial.println(vgmInfo.DataBlockStartAdressofData, HEX);
    Serial.print("- VGM Play Data Offset (absolute byte): 0x");
    Serial.println(vgmInfo.VgmPlayDataOffset, HEX);
}

//Obtains YM2608 clock from the header and updates the struct with the obtained information
uint32_t VgmManager::getYM2608Clock(VgmInfo &info){
    //Reads the 4 bytes regarding the clock inside the header (little endian)
    uint32_t readedBytesYM2608Clock = readBytesLittleEndian(bufferVgmData, 72, 4);

    //Updates the struct with the byte read
    info.Ym2608Clock = readedBytesYM2608Clock;

    if(info.Ym2608Clock != 0){
        // //Debug -> checks if the struct was updated correctly
        // Serial.print("- YM2608 Clock = ");
        // Serial.print(info.Ym2608Clock);
        // Serial.println(" Hz");
    } else{
        Serial.println("Not a .vgm for the OPNA / YM2608 chip");
    }

    return info.Ym2608Clock;
}

//Absolute value for offset to the extra header or 0 if no extra header is present.
uint32_t VgmManager::getExtraHeaderOffset(VgmInfo &info){
    uint32_t readedBytesExtraHeaderOffset = readBytesLittleEndian(bufferVgmData, 188, 4);

    //Adds 0x04 to already consider the absolute byte -> not an offset
    info.ExtraHeaderOffset = readedBytesExtraHeaderOffset + 0xBC;

    // //Debug -> checks if the struct was updated correctly with the information
    // Serial.print("- Vgm Extra Header Offset (absolute byte) = 0x");
    // Serial.println(info.ExtraHeaderOffset, HEX);
    return info.ExtraHeaderOffset;
}

// -------------------------------- Factory Design -------------------------
#pragma region
//Factory -> object instantiator on demand
std::unique_ptr<VgmVersionFactory> VgmVersionFactory::createVgmVersion(VgmManager::vgmVersion version, VgmInfo &info) {
    switch (version) {
    case VgmManager::Vgm_150:
        return std::unique_ptr<Vgm150>(new Vgm150(info));

    case VgmManager::Vgm_151:
        return std::unique_ptr<Vgm151>(new Vgm151(info));

    case VgmManager::Vgm_160:
        return std::unique_ptr<Vgm160>(new Vgm160(info));

    case VgmManager::Vgm_161:
        return std::unique_ptr<Vgm161>(new Vgm161(info));

    case VgmManager::Vgm_170:
        return std::unique_ptr<Vgm170>(new Vgm170(info));

    case VgmManager::Vgm_171:
        return std::unique_ptr<Vgm171>(new Vgm171(info));

    // Add cases for other versions as needed
    default:
        // Handle unrecognized cases or return nullptr if preferred
        return nullptr;
    }
}
#pragma endregion

//Constructors -> Confirm that the object was actually created

VgmVersionFactory::VgmVersionFactory(VgmInfo &info) : vgmInfo(info) {
    // Initialize here if necessary
}

Vgm150::Vgm150(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm150 object successfully created!");
}

Vgm151::Vgm151(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm151 object successfully created!");
}

Vgm160::Vgm160(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm160 object successfully created!");
}

Vgm161::Vgm161(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm61 object successfully created!");
}

Vgm170::Vgm170(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm170 object successfully created!");
}

Vgm171::Vgm171(VgmInfo &info) : VgmVersionFactory(info){
    Serial.println("Vgm171 object successfully created!");
}


//Destructors -> Confirm that the object was actually destroyed
Vgm150::~Vgm150(){
    Serial.println("Vgm150 object successfully destroyed!");
}

Vgm151::~Vgm151(){
    Serial.println("Vgm151 object successfully destroyed!");
}

Vgm160::~Vgm160(){
    Serial.println("Vgm160 object successfully destroyed!");
}

Vgm161::~Vgm161(){
    Serial.println("Vgm61 object successfully destroyed!");
}

Vgm170::~Vgm170(){
    Serial.println("Vgm170 object successfully destroyed!");
}

Vgm171::~Vgm171(){
    Serial.println("Vgm171 object successfully destroyed!");
}

//Individual logic for the virtual method -> each version has implementations corresponding to the .vgm version
void Vgm150::processVgm(VgmInfo &info) {
    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
}

void Vgm151::processVgm(VgmInfo &info) {
    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
    getYM2608Clock(info); //Updates relative to the previous version
}

void Vgm160::processVgm(VgmInfo &info) {
    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
    getYM2608Clock(info);
}

void Vgm161::processVgm(VgmInfo &info) {
    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
    getYM2608Clock(info);
}

void Vgm170::processVgm(VgmInfo &info) {
    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
    getYM2608Clock(info);
    getExtraHeaderOffset(info); //Updates relative to the previous version
}

void Vgm171::processVgm(VgmInfo &info) {

    getEofOffset(info);
    getLoopOffset(info);
    getVgmDataOffset(info);
    getYM2608Clock(info);
    getExtraHeaderOffset(info);
}
// ----------------------------------------------------------------------------