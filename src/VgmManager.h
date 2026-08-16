//VgmManager.h
#ifndef VGMMANAGER_H
#define VGMMANAGER_H

#include "FileManager.h"
#include <memory>
#include "YM2608.h"

class VgmManager : public FileManager {

protected:

    //Variable that determines the number of times the music will loop
    size_t loopTimes;

    //Bool to determine when a file has finished
    bool isVgmEnded;

    //Variable to determine whether a file is vgm or not
    bool isVgmFile;

    //Variable to determine whether the .vgm file has a Data Block (uses samples or not!)
    bool haveDataBlock;

    //Points to the current index read
    size_t bufferVgmIndex = 0;

    //Points to the current byte read
    uint32_t CurrentByte = 0;

    //Points to the initial byte of the read
    uint32_t InitialByte = 0;

    uint32_t NextByte = 0;

    //Indicates whether a file has a gd3 tag or not
    bool haveGd3;
public:

    //File instance to be opened when playing a .vgm file -> close instance externally
    File file_vgm_playing;

    //Struct to store important information contained in the vgm file header
    struct VgmInfo {
        uint32_t EofOffset = 0;
        uint32_t GD3Offset = 0;
        uint32_t GD3Size = 0;
        uint32_t LoopOffset = 0;
        uint32_t VgmDataOffset = 0;
        uint32_t Ym2608Clock = 0;
        uint32_t ExtraHeaderOffset = 0;
        uint32_t DataBlockSize = 0;
        uint32_t DataBlockStartAdressofData = 0;
        uint32_t VgmPlayDataOffset = 0; //Exact byte where musical .vgm data begins!
        uint32_t VgmPlayDataEnd = 0; //Exact byte indicating the 0x66 command -> End of Data
    };


    //Struct to store information regarding the GD3 Tag (when present)
    struct Gd3Info{
        String trackNameEnglish;
        String trackNamenonEnglish;
        String gameNameEnglish;
        String gameNameNonEnglish;
        String systemNameEnglish;
        String systemNameNonEnglish;
        String trackAuthorEnglish;
        String trackAutorNonEnglish;
        String releaseDate;
        String dumper;

        static const size_t incrementVGMLUT[256];
    };

    //Objects for the structs
    Gd3Info gd3Info;
    VgmInfo vgmInfo;

    //Class default constructor
    VgmManager();

    //Class default destructor
    ~VgmManager() = default;

    void init(); //Initial configurations -> clears the structs

    //Function to read music bytes from the .vgm file in a circular manner (goes to the end and returns to the loop point)
    void playVgmRoutine(const String &Path);

    bool isVgmFileOpen() const;

    //Closes the .vgm file that is currently playing 
    void closeVgmFile();

    //Clears the buffer array containing the music data
    void clearBufferVgmData();

    void printVgmStruct();

    //Fills the buffer with header information regarding the selected file
    void fillHeaderBuffer(const String &Path);

    //Fills the buffer with information regarding the gd3 tag, if present
    void fillGd3Buffer(const String &Path);

    // Function to read and decode GD3 information from the array
    void readGd3Info();

    //Method that returns the size in bytes of the Gd3 Tag (reads from the previously populated gd3 buffer)
    uint32_t getGd3Size();

    //Method that returns the exact byte where the music data ends -> end of loop!
    uint32_t getVgmPlayDataEnd();

    //Function to clear the information present in the GD3 struct
    void resetGd3Info();

    //Function to clear the information present in the VgmInfo struct
    void resetVgmInfo();
    
    /*Verifies if a file is of type .vgm by reading and checking the first 3 bytes of the bufferVgmData array. 
    Returns true if the first 3 bytes are "Vgm "(0x56 0x67 0x6d 0x20)*/
    bool checkVgmFile();

    //Fills the buffer with information related to the DataBlock
    void fillDataBlockBuffer(const String &Path);

    //Verifies if the file has a Datablock and populates the struct with the correct information for the start of music data
    bool checkDataBlock();

    //Fills the vgmInfo struct with information regarding the .vgm file's data block
    void getDataBlock();

    //Enum -> initial file verification: is it VGM? If so, which version? -> redirect logic for each version
    enum vgmVersion {Vgm_150, Vgm_151 , Vgm_160, Vgm_161, Vgm_170 , Vgm_171, Non_Vgm};

    /*Reads a fixed amount of bytes from an array starting from an initial byte and a fixed number of bytes, and returns the little endian reading of these bytes.
    TODO: for now, the selected structure works well with 4 bytes (32 bit). Check the need to expand as the project grows
    */
    template <size_t N>
    unsigned int readBytesLittleEndian(const unsigned char (&data)[N], size_t init_byte, size_t n_bytes) {
    // Calculates array size (number of elements)
    size_t arraySize = N;

        // Verifies if parameters are valid
        if (init_byte < 0 || init_byte + n_bytes > arraySize) {
            Serial.println("Error: Invalid indices or byte count exceeds array size.");
            Serial.print("Array size: ");
            Serial.println(arraySize);
            return 0;
        }

        // Initializes the resulting value
        unsigned int resultLittleEndian = 0;

        // Performs little endian reading
        for (size_t i = 0; i < n_bytes; ++i) {
            resultLittleEndian |= (static_cast<unsigned int>(data[init_byte + i]) << (i * 8));
        }

        // Prints values in hexadecimal and decimal
        // Serial.print("Value in Hex: 0x");
        // Serial.println(resultLittleEndian, HEX);
        // Serial.print(", Value in Decimal: ");
        // Serial.println(resultLittleEndian);

        return resultLittleEndian;
    }

    //Verifies the .vgm file version and returns the corresponding version -> enum variable!
    vgmVersion checkVgmVersion();

    //Setters & Getters----------------------------------

    // Getter for isVgmEnded
    bool getIsVgmEnded() const;

    // Setter for isVgmEnded
    void setIsVgmEnded(bool value);

    // Getter that returns a reference to the VgmInfo object
    VgmInfo& GetVgmInfo();
    const VgmInfo& GetVgmInfo() const;

    // Function to verify, process, and instantiate the correct version for an object corresponding to the detected .vgm version
    void processVgmVersion();

    //----------- Vgm Header --------------//
    //Methods for reading bufferVgmData and storing information inside the struct

    //Obtains the End of File offset from the header -> calculates absolute value
    uint32_t getEofOffset(VgmInfo &info);

    //Obtains the offset referring to the start of GD3 information from the header -> calculates absolute value
    bool getGD3Offset();

    //Obtains absolute byte for Loop Offset from the vgm header
    uint32_t getLoopOffset(VgmInfo &info);

    //Obtains absolute byte for Vgm data Offset
    uint32_t getVgmDataOffset(VgmInfo &info);

    //Obtains information about the YM2608 chip clock from the vgm header
    uint32_t getYM2608Clock(VgmInfo &info);

    //Absolute value for offset to the extra header or 0 if no extra header is present.
    uint32_t getExtraHeaderOffset(VgmInfo &info);

    // //Setter for isVgmPlaying
    // void setVgmPlaying(volatile bool value);

    // //Getter for isVgmPlaying
    // bool getVgmPlaying() const;

private:

};

// -------------------------------- Factory Design -------------------------
#pragma region
//Factory -> object instantiator on demand
class VgmVersionFactory : public VgmManager {
public:
    VgmVersionFactory(VgmInfo &info);
    virtual ~VgmVersionFactory() = default;
    virtual void processVgm(VgmInfo &info) = 0;
    static std::unique_ptr<VgmVersionFactory> createVgmVersion(VgmManager::vgmVersion version, VgmInfo &info);

protected:
    VgmInfo &vgmInfo;
};

class Vgm150 : public VgmVersionFactory{

public:
    Vgm150(VgmInfo &info);  // Default constructor
    ~Vgm150(); //Default destructor

    void processVgm(VgmInfo &info);
};

//Daughter class -> implements valid methods for the vgm 1.51 header
class Vgm151 : public VgmVersionFactory{
public:

    Vgm151(VgmInfo &info); // Default constructor
    ~Vgm151(); //Default destructor

    void processVgm(VgmInfo &info);
};

//Daughter class -> implements valid methods for the vgm 1.60 header
class Vgm160 : public VgmVersionFactory{
public:
    Vgm160(VgmInfo &info); // Default constructor
    ~Vgm160(); //Default destructor

    void processVgm(VgmInfo &info);
};

//Daughter class -> implements valid methods for the vgm 1.61 header
class Vgm161 : public VgmVersionFactory{
public:
    Vgm161(VgmInfo &info);              // Default constructor
    ~Vgm161(); //Default destructor

    void processVgm(VgmInfo &info);
};

//Daughter class -> implements valid methods for the vgm 1.70 header
class Vgm170 : public VgmVersionFactory{
public:
    Vgm170(VgmInfo &info);              // Default constructor
    ~Vgm170(); //Default destructor

    void processVgm(VgmInfo &info);
};

//Daughter class -> implements valid methods for the vgm 1.71 header
class Vgm171 : public VgmVersionFactory{
public:
    Vgm171(VgmInfo &info); // Default constructor
    ~Vgm171(); //Default destructor

    void processVgm(VgmInfo &info);
};
#pragma endregion
// ----------------------------------------------------------------------------

#endif