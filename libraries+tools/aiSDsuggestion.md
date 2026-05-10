🛠️ Step 1: Define SD_MMC pins and enable the feature in configs.h

In your configs.h, inside the FREENOVE_ESP32S3_DISPLAY block, add these lines before the #endif:
```cpp

#ifdef FREENOVE_ESP32S3_DISPLAY
    #define HAS_SCREEN
    #define HAS_FULL_SCREEN
    #define HAS_SD
    #define USE_SD
    #define HAS_BUTTONS
    #define HAS_PSRAM
    #define HAS_BT
    #define HAS_BT_REMOTE

    // --- SD_MMC pins (4‑bit mode) ---
    #define SDMMC_HOST       1
    #define SDMMC_CLK        38
    #define SDMMC_CMD        40
    #define SDMMC_D0         39
    #define SDMMC_D1         41
    #define SDMMC_D2         48
    #define SDMMC_D3         47

    // Tell the code we are using SD_MMC, not SPI
    #define USE_SD_MMC
#endif
```
Important: Remove any existing SD_CS definition for this board – we won't need it.
🛠️ Step 2: Modify SDInterface.cpp to support SD_MMC

Open SDInterface.cpp and find the initSD() function. Add a new branch for USE_SD_MMC before the existing SPI code.

At the top of the function, add:
```cpp

bool SDInterface::initSD() {
#ifdef USE_SD_MMC
    // Use SD_MMC (4‑bit mode)
    if (!SD_MMC.begin()) {
        Serial.println("SD_MMC initialization failed!");
        this->supported = false;
        return false;
    }
    this->supported = true;
    this->cardType = SD_MMC.cardType();
    this->cardSizeMB = SD_MMC.cardSize() / (1024 * 1024);
    // ... rest of common code (list directories etc.)
    // Note: The rest of the function (creating /SCRIPTS, etc.) is the same,
    // but replace SD.xxx with SD_MMC.xxx where needed.
    // For now, we'll just return true and fill the card size.
    char sz[10];
    sprintf(sz, "%d", this->cardSizeMB);
    this->card_sz = String(sz);
    if (!SD_MMC.exists("/SCRIPTS")) SD_MMC.mkdir("/SCRIPTS");
    this->sd_files = new LinkedList<String>();
    return true;
#else
    // Original SPI SD code (unchanged)
    #ifdef HAS_SD
    ...
#endif
}
```
    Note: The original function uses SD (the global SPI object). For SD_MMC, you must change every SD. to SD_MMC. (e.g., SD_MMC.open, SD_MMC.exists). To keep it clean, you can copy the whole existing logic and replace SD with SD_MMC inside the #ifdef USE_SD_MMC block.

Here is a complete replacement for the initSD() function that supports both SPI and SD_MMC:
```cpp

bool SDInterface::initSD() {
#ifdef USE_SD_MMC
    // ========== SD_MMC (4‑bit) ==========
    if (!SD_MMC.begin()) {
        Serial.println("SD_MMC mount failed");
        this->supported = false;
        return false;
    }
    this->supported = true;
    this->cardType = SD_MMC.cardType();
    this->cardSizeMB = SD_MMC.cardSize() / (1024 * 1024);
    char sz[10];
    sprintf(sz, "%d", this->cardSizeMB);
    this->card_sz = String(sz);
    if (!SD_MMC.exists("/SCRIPTS")) SD_MMC.mkdir("/SCRIPTS");
    this->sd_files = new LinkedList<String>();
    Serial.printf("SD_MMC mounted, size: %d MB\n", this->cardSizeMB);
    return true;
#else
    // ========== Original SPI SD code ==========
    #ifdef HAS_SD
    ... (your existing code stays untouched)
    #else
    return false;
    #endif
#endif
}

You also need to update all other methods (getFile, removeFile, listDirToLinkedList, listDir, runUpdate, performUpdate). The easiest way is to add #ifdef USE_SD_MMC branches that use SD_MMC instead of SD. For example:
cpp

File SDInterface::getFile(String path) {
#ifdef USE_SD_MMC
    if (this->supported) return SD_MMC.open(path, FILE_READ);
    return File();
#else
    if (this->supported) return SD.open(path, FILE_READ);
    return File();
#endif
}
```
Do the same for removeFile, listDirToLinkedList, etc. – wherever SD. appears, duplicate the line with SD_MMC. inside a #ifdef USE_SD_MMC.
🧪 Step 3: Recompile and test

    Clean the build (delete the .pio/build folder or use pio run --target clean).

    Recompile with your board target.

    Flash the firmware.

    Check the serial output – you should see SD_MMC mounted, size: X MB instead of SD Card NOT Supported.

Now you can use SD card commands like ls, mkdir, cat, and PCAP logging will work.
