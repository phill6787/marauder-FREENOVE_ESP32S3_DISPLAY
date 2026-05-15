#include "SDInterface.h"
#include "lang_var.h"

#ifdef HAS_C5_SD
  SDInterface::SDInterface(SPIClass* spi, int cs)
    : _spi(spi), _cs(cs) {}
#endif

bool SDInterface::initSD() {
  #ifdef USE_SD_MMC
    // ========== SD_MMC (4-bit mode) ==========
    // Set custom pins for the Freenove ESP32-S3 Display
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0, SDMMC_D1, SDMMC_D2, SDMMC_D3);
    if (!SD_MMC.begin("/sdcard", false, false)) {
        Serial.println("SD_MMC mount failed");
        this->supported = false;
        return false;
    }
    this->supported = true;
    this->cardType = SD_MMC.cardType();
    this->cardSizeMB = SD_MMC.cardSize() / (1024 * 1024);
    char sz[10];
    sprintf(sz, "%d", (int)this->cardSizeMB);
    this->card_sz = String(sz);
    if (!SD_MMC.exists("/SCRIPTS")) SD_MMC.mkdir("/SCRIPTS");
    this->sd_files = new LinkedList<String>();
    Serial.printf("SD_MMC mounted, size: %d MB\n", (int)this->cardSizeMB);
    return true;
  #endif

  #ifdef HAS_SD
    String display_string = "";
    #ifdef KIT
      pinMode(SD_DET, INPUT);
      if (digitalRead(SD_DET) != LOW) {
        this->supported = false;
        return false;
      }
    #endif
    #if SD_CS != -1
        pinMode(SD_CS, OUTPUT);
    #endif

    delay(10);
    #if (defined(MARAUDER_M5STICKC)) || (defined(HAS_CYD_TOUCH)) || (defined(MARAUDER_CARDPUTER)) || (defined(MARAUDER_CARDPUTER_ADV))
      /* Set up SPI SD Card using external pin header
      StickCPlus Header - SPI SD Card Reader
                  3v3   -   3v3
                  GND   -   GND
                   G0   -   CLK
              G36/G25   -   MISO
                  G26   -   MOSI
                        -   CS (jumper to SD Card GND Pin)
      */
      #if defined(MARAUDER_M5STICKC)
        enum { SPI_SCK = 0, SPI_MISO = 36, SPI_MOSI = 26 };
      #elif defined(HAS_CYD_TOUCH) || defined(MARAUDER_CARDPUTER) || defined(MARAUDER_CARDPUTER_ADV) || defined(HAS_SEPARATE_SD)
        enum { SPI_SCK = SD_SCK, SPI_MISO = SD_MISO, SPI_MOSI = SD_MOSI };
      #else
        enum { SPI_SCK = 0, SPI_MISO = 36, SPI_MOSI = 26 };
      #endif
      #if !defined(MARAUDER_CARDPUTER) && !defined(MARAUDER_CARDPUTER_ADV)
        this->spiExt = new SPIClass();
      #else
        this->spiExt = new SPIClass(FSPI);
      #endif
      Serial.println(F("Using external SPI configuration..."));
      this->spiExt->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
      if (!SD.begin(SD_CS, *(this->spiExt))) {
    #elif defined(HAS_C5_SD)
      if (!SD.begin(SD_CS, *_spi)) {
    #else
      if (!SD.begin(SD_CS)) {
    #endif
      Serial.println(F("Failed to mount SD Card"));
      this->supported = false;
      return false;
    }
    else {
      this->supported = true;
      this->cardType = SD.cardType();

      this->cardSizeMB = SD.cardSize() / (1024 * 1024);
    
      if (this->supported) {
        const int NUM_DIGITS = log10(this->cardSizeMB) + 1;

        char sz[NUM_DIGITS + 1];

        sz[NUM_DIGITS] =  0;
        for ( size_t i = NUM_DIGITS; i--; this->cardSizeMB /= 10)
        {
            sz[i] = '0' + (this->cardSizeMB % 10);
            display_string.concat((String)sz[i]);
        }
  
        this->card_sz = sz;
      }

      if (!SD.exists("/SCRIPTS")) {

        SD.mkdir("/SCRIPTS");
      }

      this->sd_files = new LinkedList<String>();
    
      return true;
  }

  #else
    return false;
  #endif
}

File SDInterface::getFile(String path) {
  if (this->supported) {
    #ifdef USE_SD_MMC
      File file = SD_MMC.open(path, FILE_READ);
    #else
      File file = SD.open(path, FILE_READ);
    #endif
    return file;
  }
  return File();
}

bool SDInterface::removeFile(String file_path) {
  #ifdef USE_SD_MMC
    if (SD_MMC.remove(file_path))
  #else
    if (SD.remove(file_path))
  #endif
    return true;
  else
    return false;
}

void SDInterface::listDirToLinkedList(LinkedList<String>* file_names, String str_dir, String ext) {
  if (this->supported) {
    #ifdef USE_SD_MMC
      File dir = SD_MMC.open(str_dir);
    #else
      File dir = SD.open(str_dir);
    #endif
    while (true)
    {
      File entry = dir.openNextFile();
      if (!entry)
      {
        break;
      }

      if (entry.isDirectory())
        continue;

      String file_name = entry.name();
      if (ext != "") {
        if (file_name.endsWith(ext)) {
          file_names->add(file_name);
        }
      }
      else
        file_names->add(file_name);
    }
  }
}

void SDInterface::listDir(String str_dir){
  if (this->supported) {
    #ifdef USE_SD_MMC
      File dir = SD_MMC.open(str_dir);
    #else
      File dir = SD.open(str_dir);
    #endif
    while (true)
    {
      File entry = dir.openNextFile();
      if (! entry)
      {
        break;
      }
      Serial.print(entry.name());
      Serial.print("\t");
      Serial.println(entry.size());
      entry.close();
    }
  }
}

void SDInterface::runUpdate(String file_name) {
  if (file_name == "")
    file_name = "/update.bin";

  #ifdef HAS_SCREEN
    display_obj.tft.setTextWrap(false);
    display_obj.tft.setFreeFont(NULL);
    display_obj.tft.setCursor(0, TFT_HEIGHT / 3);
    display_obj.tft.setTextSize(1);
    display_obj.tft.setTextColor(TFT_WHITE);
  
    display_obj.tft.println("Opening " + file_name + "...");
  #endif

  #ifdef USE_SD_MMC
    File updateBin = SD_MMC.open(file_name);
  #else
    File updateBin = SD.open(file_name);
  #endif

  if (updateBin) {
    if(updateBin.isDirectory()){
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_RED);
        display_obj.tft.println(F(text_table2[0]));
      #endif
      Serial.print(F("Error, could not find \""));
      Serial.print(file_name);
      Serial.println(F("\""));
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_WHITE);
      #endif
      updateBin.close();
      return;
    }

    size_t updateSize = updateBin.size();

    if (updateSize > 0) {
      #ifdef HAS_SCREEN
        display_obj.tft.println(F(text_table2[1]));
      #endif
      Serial.println(F("Starting update over SD. Please wait..."));
      this->performUpdate(updateBin, updateSize);
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_RED);
        display_obj.tft.println(F(text_table2[2]));
      #endif
      Serial.println(F("Error, file is empty"));
      #ifdef HAS_SCREEN
        display_obj.tft.setTextColor(TFT_WHITE);
      #endif
      return;
    }

    updateBin.close();
    
      // whe finished remove the binary from sd card to indicate end of the process
    #ifdef HAS_SCREEN
      display_obj.tft.println(F(text_table2[3]));
    #endif
    const esp_partition_t *running = esp_ota_get_running_partition();

    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

    esp_err_t result = esp_ota_set_boot_partition(next);
     
    ESP.restart();
  }
  else {
    #ifdef HAS_SCREEN
      display_obj.tft.setTextColor(TFT_RED);
      display_obj.tft.println(F(text_table2[4]));
    #endif
    Serial.println(F("Could not load update.bin from sd root"));
    #ifdef HAS_SCREEN
      display_obj.tft.setTextColor(TFT_WHITE);
    #endif
  }
}

void SDInterface::performUpdate(Stream &updateSource, size_t updateSize) {
  if (Update.begin(updateSize)) {   
    #ifdef HAS_SCREEN
      display_obj.tft.println(text_table2[5] + String(updateSize));
      display_obj.tft.println(F(text_table2[6]));
    #endif
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize) {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[7] + String(written) + text_table2[10]);
      #endif
      Serial.print(F("Written : "));
      Serial.print(written);
      Serial.println(F(" successfully"));
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[8] + String(written) + "/" + String(updateSize) + text_table2[9]);
      #endif
      Serial.print(F("Written only : "));
      Serial.print(written);
      Serial.print(F("/"));
      Serial.print(updateSize);
      Serial.println(F(". Retry?"));
    }
    if (Update.end()) {
      if (Update.isFinished()) {

      }
      else {
        #ifdef HAS_SCREEN
          display_obj.tft.setTextColor(TFT_RED);
          display_obj.tft.println(text_table2[12]);
        #endif
        Serial.println(F("Update not finished? Something went wrong!"));
        #ifdef HAS_SCREEN
          display_obj.tft.setTextColor(TFT_WHITE);
        #endif
      }
    }
    else {
      #ifdef HAS_SCREEN
        display_obj.tft.println(text_table2[13] + String(Update.getError()));
      #endif
      Serial.print(F("Error Occurred. Error #: "));
      Serial.println(Update.getError());
    }

  }
  else
  {
    #ifdef HAS_SCREEN
      display_obj.tft.println(text_table2[14]);
    #endif
    Serial.println(F("Not enough space to begin OTA"));
  }
}
