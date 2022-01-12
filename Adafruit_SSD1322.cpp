/*********************************************************************
This is a library for the SSD1322, based directly on the Adafruit SSD1327
library. The original comment for that library is left below for reference.
*********************************************************************/

/*********************************************************************
This is a library for our grayscale OLEDs based on SSD1327 drivers

  Pick one up today in the adafruit shop!
  ------> https://www.adafruit.com/products/4741

These displays use I2C or SPI to communicate

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any
redistribution
*********************************************************************/

#include "Adafruit_SSD1322.h"
#include "splash.h"

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for SPI SSD1322 displays, using software (bitbang)
            SPI.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  mosi_pin
            MOSI (master out, slave in) pin (using Arduino pin numbering).
            This transfers serial data from microcontroller to display.
    @param  sclk_pin
            SCLK (serial clock) pin (using Arduino pin numbering).
            This clocks each bit from MOSI.
    @param  dc_pin
            Data/command pin (using Arduino pin numbering), selects whether
            display is receiving commands (low) or data (high).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  cs_pin
            Chip-select pin (using Arduino pin numbering) for sharing the
            bus with other devices. Active low.
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1322::Adafruit_SSD1322(int8_t mosi_pin,
                                   int8_t sclk_pin, int8_t dc_pin,
                                   int8_t rst_pin, int8_t cs_pin)
    : Adafruit_GrayOLED(4, 256, 64, mosi_pin, sclk_pin, dc_pin, rst_pin, cs_pin),
      spi(NULL) {}

/*!
    @brief  Constructor for SPI SSD1322 displays, using native hardware SPI.
    @param  w
            Display width in pixels
    @param  h
            Display height in pixels
    @param  spi
            Pointer to an existing SPIClass instance (e.g. &SPI, the
            microcontroller's primary SPI bus).
    @param  dc_pin
            Data/command pin (using Arduino pin numbering), selects whether
            display is receiving commands (low) or data (high).
    @param  rst_pin
            Reset pin (using Arduino pin numbering), or -1 if not used
            (some displays might be wired to share the microcontroller's
            reset pin).
    @param  cs_pin
            Chip-select pin (using Arduino pin numbering) for sharing the
            bus with other devices. Active low.
    @param  bitrate
            SPI clock rate for transfers to this display. Default if
            unspecified is 8000000UL (8 MHz).
    @note   Call the object's begin() function before use -- buffer
            allocation is performed there!
*/
Adafruit_SSD1322::Adafruit_SSD1322(SPIClass *spi,
                                   int8_t dc_pin, int8_t rst_pin, int8_t cs_pin,
                                   uint32_t bitrate)
    : Adafruit_GrayOLED(4, 256, 64, spi, dc_pin, rst_pin, cs_pin, bitrate),
      spi(spi) {}

/*!
    @brief  Destructor for Adafruit_SSD1322 object.
*/
Adafruit_SSD1322::~Adafruit_SSD1322(void) {}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  reset
            If true, and if the reset pin passed to the constructor is
            valid, a hard reset will be performed before initializing the
            display. If using multiple SSD1322 displays on the same bus, and
            if they all share the same reset pin, you should only pass true
            on the first display being initialized, false on all others,
            else the already-initialized displays would be reset. Default if
            unspecified is true.
    @return true on successful allocation/init, false otherwise.
            Well-behaved code should check the return value before
            proceeding.
    @note   MUST call this function before any drawing or updates!
*/
bool Adafruit_SSD1322::begin(bool reset) {

  // set pin directions (superclass takes care of dcPin and rstPin)
  pinMode(csPin, OUTPUT);

  // The SSD1322 doesn't support I2C, so the address will never be used.
  if (!Adafruit_GrayOLED::_init(0, reset)) {
    return false;
  }

  // Init sequence. This is copied from the initialization in:
  // https://github.com/winneymj/ESP8266_SSD1322
  // with a couple of modifications gleaned from the Arduino tutorial here:
  // https://www.buydisplay.com/white-3-2-inch-arduino-raspberry-pi-oled-display-module-256x64-spi

	spi_command(SSD1322_CMDLOCK, // 0xFD
	0x12);// Unlock OLED driver IC

	spi_command(SSD1322_DISPLAYOFF);// 0xAE

	spi_command(SSD1322_DCLK,// 0xB3
	0x91);

	spi_command(SSD1322_SETMUXRATIO, // 0xCA
	0x3F);// duty = 1/64

	spi_command(SSD1322_SETDISPLAYOFFSET, // 0xA2
	0x00);

	spi_command(SSD1322_SETSTARTLINE, // 0xA1
	0x00);

	spi_command(SSD1322_SEGREMAP, // 0xA0
	0x14, //Horizontal address increment,Disable Column Address Re-map,Enable Nibble Re-map,Scan from COM[N-1] to COM0,Disable COM Split Odd Even
	0x11);//Enable Dual COM mode

	spi_command(SSD1322_SETGPIO, // 0xB5
	0x00);// Disable GPIO Pins Input

	spi_command(SSD1322_REGULATOR, // 0xAB
	0x01);// selection external vdd

	spi_command(SSD1322_DISPLAYENHANCE, // 0xB4
	0xA0,// enables the external VSL
	0xFD);// 0xfFD,Enhanced low GS display quality;default is 0xb5(normal),

	spi_command(SSD1322_SETCONTRASTCURRENT, // 0xC1
	// 0xFF);// 0xFF - default is 0x7f
  0x80);

	spi_command(SSD1322_MASTERCURRENTCONTROL, // 0xC7
	0x0F);// default is 0x0F

	// Set grayscale
	spi_command(SSD1322_SELECTDEFAULTGRAYSCALE); // 0xB9

 	spi_command(SSD1322_PHASELEN, // 0xB1
	0xE2);// default is 0x74

	spi_command(SSD1322_DISPLAYENHANCEB, // 0xD1
	0x82, // Reserved;default is 0xa2(normal)
	0x20);//

	spi_command(SSD1322_SETPRECHARGEVOLTAGE, // 0xBB
	0x1F);// 0.6xVcc

	spi_command(SSD1322_PRECHARGE2, // 0xB6
	0x08);// default

	spi_command(SSD1322_SETVCOM, // 0xBE
	0x07);// 0.86xVcc;default is 0x04

	spi_command(SSD1322_NORMALDISPLAY);// 0xA6

	spi_command(SSD1322_EXITPARTIALDISPLAY);// 0xA9

  delay(100);                      // 100ms delay recommended

  spi_command(SSD1322_DISPLAYON); // 0xaf

  // The default "set contrast" command (0x81) doesn't appear in the SSD1322 datasheet.
  // Calling this might be bad?
//   setContrast(0x2F);

  return true; // Success
}

/*!
    @brief  Do the actual writing of the internal frame buffer to display RAM
*/
void Adafruit_SSD1322::display(void) {
  // ESP8266 needs a periodic yield() call to avoid watchdog reset.
  // With the limited size of SSD1322 displays, and the fast bitrate
  // being used (1 MHz or more), I think one yield() immediately before
  // a screen write and one immediately after should cover it.  But if
  // not, if this becomes a problem, yields() might be added in the
  // 32-byte transfer condition below.
  yield();

  if ((window_x1 >= window_x2) || (window_y1 >= window_y2))
  {
    // Serial.printf("Dirty window is empty, writing nothing.\n");
    window_x1 = 1024;
    window_y1 = 1024;
    window_x2 = -1;
    window_y2 = -1;
    return;
  }

  uint8_t *ptr = buffer;
  uint8_t rows = HEIGHT;

  uint8_t bytes_per_row = WIDTH / 2;

  // Column addresses seem to be in 2-byte (4-pixel) units.
  int16_t row_start_column = min(int16_t((WIDTH/4)), int16_t(window_x1 / 4));
  int16_t row_end_column = max(int16_t(0), int16_t(window_x2 / 4));

  int16_t first_row = min(int16_t(rows - 1), int16_t(window_y1));
  // The dirty window seems to need to be expanded by 1 in y.
  int16_t last_row = max(int16_t(0), int16_t(window_y2 + 1));

  spi_command(SSD1322_SETCOLUMN, uint8_t(row_start_column + 0x1c), uint8_t(row_end_column + 0x1c));
  spi_command(SSD1322_SETROW, uint8_t(first_row), uint8_t(last_row));
  spi_command(SSD1322_WRITERAM);

  digitalWrite(dcPin, HIGH);

  // Need to write two bytes for every column.
  size_t bytes = (row_end_column - row_start_column + 1) * 2;

  if (bytes == bytes_per_row)
  {
    // Contiguous write case -- just write the entire buffer
    bytes *= last_row - first_row;
    ptr = buffer + (uint16_t)first_row * (uint16_t)bytes_per_row;
    ptr += (row_start_column * 2);
    // Write the entire buffer in one go.
    spi_data(ptr, bytes);
  }
  else
  {
    // Serial.printf("writing %d rows at %d bytes each\n", int(last_row - first_row), int(bytes));

    for (uint8_t row = first_row; row <= last_row; row++) {
      ptr = buffer + (uint16_t)row * (uint16_t)bytes_per_row;

      // fast forward to dirty rectangle beginning
      ptr += (row_start_column * 2);

      // Write the entire contents of this row in one go.
      spi_data(ptr, bytes);
      yield();

    }
  }

  // reset dirty window
  window_x1 = 1024;
  window_y1 = 1024;
  window_x2 = -1;
  window_y2 = -1;
}

void Adafruit_SSD1322::spi_command(uint8_t c)
{
  // Serial.printf("command: %02x\n", c);
  spi_command_data(c, (uint8_t*)NULL, 0);
}

void Adafruit_SSD1322::spi_command(uint8_t c, uint8_t d1)
{
  // Serial.printf("command: %02x %02x\n", c, d1);
  spi_command_data(c, &d1, 1);
}

void Adafruit_SSD1322::spi_command(uint8_t c, uint8_t d1, uint8_t d2)
{
  // Serial.printf("command: %02x %02x %02x\n", c, d1, d2);
  uint8_t buf[] = {d1, d2};
  spi_command_data(c, buf, 2);
}

void Adafruit_SSD1322::spi_command_data(uint8_t c, uint8_t *data, size_t count)
{
  digitalWrite(dcPin, LOW);
  spi_dev->write(&c, 1);
  if (count > 0) {
    spi_data(data, count);
  }
}

void Adafruit_SSD1322::spi_data(uint8_t *data, size_t count)
{
#if defined(ARDUINO_ARCH_ESP32)
  if (spi != NULL) 
  {
    // We're using hardware SPI. 
    // On ESP32, this is a much more optimal way to do a bulk transfer than what's in Adafruit_SPIDevice::write().
    digitalWrite(dcPin, HIGH);
    spi_dev->beginTransaction();
    digitalWrite(csPin, LOW );
    spi->writeBytes(data, count);
    digitalWrite(csPin, HIGH);
    spi_dev->endTransaction();
  } 
  else 
#endif
  {
    digitalWrite(dcPin, HIGH);
    spi_dev->write(data, count);    
  }
}

/*!
    @brief  Enable or disable display invert mode (white-on-black vs
            black-on-white). Handy for testing!
    @param  i
            If true, switch to invert mode (black-on-white), else normal
            mode (white-on-black).
*/
void Adafruit_SSD1322::invertDisplay(bool i) {
  spi_command(i ? SSD1322_INVERTDISPLAY : SSD1322_NORMALDISPLAY);
}
