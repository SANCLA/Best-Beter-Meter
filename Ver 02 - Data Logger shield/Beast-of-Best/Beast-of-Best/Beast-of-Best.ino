#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define ECHO_TO_SERIAL   1 // echo data to serial port

SoftwareSerial co2Serial(A0, A1); // define MH-Z19 RX TX
unsigned long startTime = millis();

const byte PINtoRESET = 8; // reset pin
const byte ledPinStatus = 4; //Blue led
const byte ledPinGreen = 5; //Green led
const byte ledPinYellow = 6; //Yellow led
const byte ledPinRed = 7; //Red led
const byte chipSelect = 10;
int watchdog_uart_ppm;
byte watchdog_temp;
byte count;
int measure_every_ms = 10000; // 10000 = 10 seconds
unsigned long log_every_ms = 300000; // 300000 = 5 minutes
byte write_to_log = 0;
unsigned long write_to_log_counter = 0;
RTC_DS1307 RTC;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  while (1);
}

void setup() {
  digitalWrite(PINtoRESET, LOW);
  Serial.begin(9600);
  co2Serial.begin(9600);
 // pinMode(9, INPUT);
  pinMode(ledPinStatus, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinRed, OUTPUT);

  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);
  if (!SD.begin(chipSelect)) {
    lcd.init();
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("ERROR!");
    lcd.setCursor(0, 1);
    lcd.print("INSERT SD CARD");
    //    error("Card failed, or not present");
    digitalWrite(ledPinStatus, HIGH);
  }
  Serial.println("card initialized.");

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  logfile.println("msrunning,unixtime,datetime,co2uart,temp");
#if ECHO_TO_SERIAL
  Serial.println("msrunning,unixtime,datetime,co2uart,temp");
#endif //ECHO_TO_SERIAL

  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Best Beter Meter");
  lcd.setCursor(0, 1);
  lcd.print("Initialiseren...");

  Serial.println("Heating up the sensor for 30 seconds...");
  count = 30;
  while (count > 0 )                                  // repeat until count is no longer greater than zero
  {
    digitalWrite(ledPinStatus, HIGH);
    delay(500);
    digitalWrite(ledPinStatus, LOW);
    delay(500);
    count = count - 1;                                 // decrement count
  }
  digitalWrite(ledPinStatus, LOW);
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop(void) {
  DateTime now;
  if (write_to_log_counter == (log_every_ms - measure_every_ms)) {
    Serial.println("WRITING TO LOG");
    write_to_log = 1;
    write_to_log_counter = 0;
  } else {
    write_to_log = 0;
    write_to_log_counter = write_to_log_counter + (measure_every_ms);
  }
  uint32_t m = millis();
  if (write_to_log == 1) {
    logfile.print(m);           // milliseconds since start
    logfile.print(",");
  }
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(",");
#endif


  // fetch the time
  now = RTC.now();

  if (write_to_log == 1) {
    // log time
    logfile.print(now.unixtime()); // seconds since 1/1/1970
    logfile.print(",");
    //logfile.print('"');
    logfile.print(now.year(), DEC);
    logfile.print("/");
    logfile.print(now.month(), DEC);
    logfile.print("/");
    logfile.print(now.day(), DEC);
    logfile.print(" ");
    logfile.print(now.hour(), DEC);
    logfile.print(":");
    logfile.print(now.minute(), DEC);
    logfile.print(":");
    logfile.print(now.second(), DEC);
    //logfile.print('"');
  }
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(",");
  //Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  //Serial.print('"');
#endif //ECHO_TO_SERIAL

  digitalWrite(ledPinStatus, HIGH);
  int ppm_uart = readCO2UART();

  if (watchdog_uart_ppm >= 0 && watchdog_uart_ppm < 1000) {
    //    Serial.println("LED: GREEN SOLID");
    digitalWrite(ledPinGreen, HIGH);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinStatus, LOW);
    delay(measure_every_ms);
  } else if (watchdog_uart_ppm >= 1000 && watchdog_uart_ppm < 4000) {
    //    Serial.println("LED: YELLOW SOLID");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, HIGH);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinStatus, LOW);
    delay(measure_every_ms);
  } else if (watchdog_uart_ppm >= 4000) {
    //   Serial.println("LED: RED");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinStatus, LOW);
  }
  if (write_to_log == 1) {
    logfile.println();
  }
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL

  if ((watchdog_uart_ppm < 0) || (watchdog_uart_ppm > 9999) || (watchdog_temp > 70)) {
    delay(3000);
    lcd.setCursor(0, 0);
    lcd.print("Sensor error !  ");
    delay(1000);
    count = 30;
    while (count > 0) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Reset in ");
      lcd.setCursor(10, 1);
      lcd.print(count);
      lcd.setCursor(12, 1);
      lcd.print("s...");
      delay(1000);
      count = count - 1;
    }
    //pinMode(PINtoRESET, OUTPUT);
      resetFunc();  //call reset
  }

  if (write_to_log == 1) {
    logfile.flush();
    count = 5;
    while (count > 0 )                                  // repeat until count is no longer greater than zero
    {
      digitalWrite(ledPinStatus, HIGH);
      delay(100);
      digitalWrite(ledPinStatus, LOW);
      delay(100);
      count = count - 1;                                  // decrement count
    }
  }
}
int readCO2UART() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte response[9]; // for answer

  //  Serial.println("Sending CO2 request...");
  co2Serial.write(cmd, 9); //request PPM CO2

  // clear the buffer
  memset(response, 0, 9);
  int i = 0;
  while (co2Serial.available() == 0) {
    //    Serial.print("Waiting for response ");
    //    Serial.print(i);
    //    Serial.println(" s");
    delay(1000);
    i++;
  }
  if (co2Serial.available() > 0) {
    co2Serial.readBytes(response, 9);
  }
  // print out the response in hexa
  for (int i = 0; i < 9; i++) {
  }
  byte check = getCheckSum(response);
  if (response[8] != check) {
  }

  // ppm
  int ppm_uart = 256 * (int)response[2] + response[3];
  // temp
  byte temp = response[4] - 40;
  if (write_to_log == 1) {
    logfile.print(",");
    logfile.print(ppm_uart);
    logfile.print(",");
    logfile.print(temp);
  }
#if ECHO_TO_SERIAL
  Serial.print(",");
  Serial.print(ppm_uart);
  Serial.print(",");
  Serial.print(temp);
#endif //ECHO_TO_SERIAL

  watchdog_uart_ppm = ppm_uart;
  watchdog_temp = temp;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO2 : ");
  lcd.print(ppm_uart);
  lcd.print(" PPM");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print("\337C");

  // status
  byte status = response[5];
  //  Serial.print("Status? ");
  //  Serial.println(status);
  if (status == 0x40) {
    //    Serial.println("Status OK");
  }
  return ppm_uart;


}

byte getCheckSum(char *packet) {
  byte i;
  unsigned char checksum = 0;
  for (i = 1; i < 8; i++) {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}
