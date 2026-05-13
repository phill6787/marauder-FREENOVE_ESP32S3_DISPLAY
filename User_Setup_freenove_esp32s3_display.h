//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pin connections and SPI control method

#define ST7789_DRIVER         // or ILI9341_DRIVER - test both
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Pin connections (same as configs.h)
#define TFT_CS   10
#define TFT_MOSI 11
#define TFT_MISO 13
#define TFT_SCLK 12
#define TFT_DC   46
#define TFT_RST  -1
#define TFT_BL   45

#define TOUCH_CS -1          // no touch

#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

#define LOAD_GLCD   // Font 1
#define LOAD_FONT2  // Font 2
#define LOAD_FONT4  // Font 4
#define LOAD_FONT6  // Font 6
#define LOAD_FONT7  // Font 7
#define LOAD_FONT8  // Font 8
#define LOAD_GFXFF  // FreeFonts

#define SMOOTH_FONT

