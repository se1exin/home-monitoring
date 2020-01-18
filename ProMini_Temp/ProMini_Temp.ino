/**
 * Useful Links
 * 
 * DS18B20 (XC3700) Temp Sensor: https://www.jaycar.com.au/medias/sys_master/images/9292327944222/XC3700-manualMain.pdf
 * LM335Z (ZL3336) Temp Sensor: https://www.jaycar.com.au/lm335z-temperature-sensor-linear-ic/p/ZL3336
 * 
 * LCD Screen Datasheet: https://www.jaycar.com.au/medias/sys_master/images/9278669783070/QP5512-dataSheetMain.pdf
 * LCD Wiring Guide (pin are slightly different: https://www.makerguides.com/character-lcd-arduino-tutorial/
 * Alternate LCD Wiring Guide (used as reference, including resistors): https://www.arduino.cc/en/Tutorial/LiquidCrystalDisplay
 * 
 * 
 * Required Libraries:
 *  - DS18B20
 *  - LiquidCrystal 
 *  
 * Pins:
 * D4: RS (LCD)
 * D5: E (LCD)
 * D6: DATA4 (LCD)
 * D7: DATA5 (LCD)
 * D8: DATA6 (LCD)
 * D9: DATA7 (LCD)
 * D10: Temp Sensor (DS18B20)
 * A0: Temp Sensor (LM355Z)
 */


#include <DS18B20.h>
#include <LiquidCrystal.h>


/**
 * Pin Definitions
 */
int pin_LCD_RS = 4;
int pin_LCD_E = 5;
int pin_LCD_DATA4 = 6;
int pin_LCD_DATA5 = 7;
int pin_LCD_DATA6 = 8;
int pin_LCD_DATA7 = 9;
int pin_DS18B20_TEMP = 10;
int pin_LM335Z_TEMP = A0;

int timeBetweenReadings = 1000 * 15;

/**
 * Init Libraries
 */
DS18B20 ds(pin_DS18B20_TEMP);
LiquidCrystal lcd = LiquidCrystal(
  pin_LCD_RS,
  pin_LCD_E,
  pin_LCD_DATA4,
  pin_LCD_DATA5,
  pin_LCD_DATA6,
  pin_LCD_DATA7
);

/**
 * Globals used in loop()
 */
char LCD_textTop[16];
char LCD_textBottom[16];

// Offset in celsius to account for LM355Z inaccuracy
// This value was determined manually by comparing with an accurate source
// over many samples in different environments (in a fridge, in the sun, etc)
float LM355Z_offset = -8.2;

void serialPrintLn(char* text) {
  Serial.println(text);
  // Serial1.println(text);
}


void setup(void) {
  lcd.begin(16, 2);
  lcd.clear();
  Serial.begin(9600);
  // Serial1.begin(9600);
}

void loop(void) {
  char strOneWireTemp[6];
  char strLm355Temp[6];

  // Strings for sending over serial
  char payloadOneWire[44];
  char payloadLM355[16];

  // Flash the 'updating' status while we get the latest reading
  sprintf(LCD_textBottom, "%-16s", "Updating...");
  updateLCD(LCD_textTop, LCD_textBottom);
  
  // Read temp from sensor
  float oneWireTemp = NULL;

  while (ds.selectNext()) {
    oneWireTemp = ds.getTempC();
  }
  dtostrf(oneWireTemp, 5, 2, strOneWireTemp);
  sprintf(LCD_textTop, "Temp: %s", strOneWireTemp);
  sprintf(payloadOneWire, "home/promini_ds18b20_01/temperature,%s", strOneWireTemp);

  /*
  float lm355Temp = readLM335ZCelsius();
  dtostrf(lm355Temp, 5, 2, strLm355Temp);
  sprintf(LCD_textBottom, "LM355Z:  %s", strLm355Temp);
  sprintf(payloadLM355, "LM355Z,%s", strLm355Temp);
  */

  // Clear the 'updating' status and update the display with the new reading
  sprintf(LCD_textBottom, "%-16s", " ");
  updateLCD(LCD_textTop, LCD_textBottom);

  // Send final reading over serial
  serialPrintLn(payloadOneWire);
  // delay(1000);
  // serialPrintLn(payloadLM355);

  // Wait for our delay period until another reading
  delay(timeBetweenReadings);
}



/**
 * Print to both lines of the LCD
 */
void updateLCD(char* textTop, char* textBottom) {
  printToLCD(0, textTop);
  printToLCD(1, textBottom);
}

/**
 * Print to a single line on the LCD
 * 
 * @param line - 0 for top line, 1 for bottom line
 * @param text - the text to be printed. Keep to <= 16 chars.
 */
void printToLCD(int line, char* text) {
  // Display on screen
  lcd.setCursor(0, line);
  lcd.print(text);
}

float readLM335ZCelsius() {
  int sensorValue = analogRead(pin_LM335Z_TEMP);

  float millivolts = sensorValue * (4.96 / 1023.0) * 1000;

  float kelvin = (millivolts/10);

  float celsius = kelvin - 273.15;

  celsius += LM355Z_offset;

  return celsius;
}
