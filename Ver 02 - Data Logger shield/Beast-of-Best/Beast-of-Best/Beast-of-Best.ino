#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

SoftwareSerial co2Serial(A0, A1); // define MH-Z19 RX TX
unsigned long startTime = millis();

int ledPinBlue = 3; //Blue led
int ledPinWhite = 4; //Blue led
int ledPinGreen = 5; //Green led
int ledPinYellow = 6; //Yellow led
int ledPinRed = 7; //Red led
int lastppmvalue;
int count;
int flushcount = 10;
int flushblinkcount;
long measure_every_ms = 10000; // 10000 = 10 seconds
long log_every_ms = 300000; // 300000 = 5 minutes
int write_to_log = 0;
unsigned long write_to_log_counter = 0;
RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  // red LED indicates error
  //digitalWrite(redLEDpin, HIGH);

  while (1);
}

void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  pinMode(9, INPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinWhite, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinRed, OUTPUT);

#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START



  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    lcd.init();                      // initialize the lcd
    lcd.init();
    // Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("ERROR!");
    lcd.setCursor(0, 1);
    lcd.print("INSERT SD CARD");
    //    error("Card failed, or not present");
    digitalWrite(ledPinBlue, HIGH);
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
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


  logfile.println("msrunning,unixtime,datetime,co2uart,temp,co2pwm");
#if ECHO_TO_SERIAL
  Serial.println("msrunning,unixtime,datetime,co2uart,temp,co2pwm");
#endif //ECHO_TO_SERIAL

  // If you want to set the aref to something other than 5v
  //analogReference(EXTERNAL);
  //}

  digitalWrite(ledPinBlue, HIGH);
  digitalWrite(ledPinWhite, HIGH);
  digitalWrite(ledPinGreen, HIGH);
  digitalWrite(ledPinYellow, HIGH);
  digitalWrite(ledPinRed, HIGH);
  delay(1000);
  digitalWrite(ledPinBlue, LOW);
  digitalWrite(ledPinWhite, LOW);
  digitalWrite(ledPinGreen, LOW);
  digitalWrite(ledPinYellow, LOW);
  digitalWrite(ledPinRed, LOW);
  delay(1000);

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
    digitalWrite(ledPinBlue, LOW);
    digitalWrite(ledPinWhite, HIGH);
    delay(500);
    digitalWrite(ledPinBlue, HIGH);
    digitalWrite(ledPinWhite, LOW);
    delay(500);
    count = count - 1;                                 // decrement count
  }
  digitalWrite(ledPinBlue, LOW);
  digitalWrite(ledPinWhite, LOW);


}

void loop(void) {
  DateTime now;
  if (write_to_log_counter == (log_every_ms - measure_every_ms)){
    Serial.println("WRITING TO LOG");
    write_to_log = 1;
    write_to_log_counter = 0;
  } else {
    write_to_log = 0;
    write_to_log_counter = write_to_log_counter + (measure_every_ms);
  }
  // delay for the amount of time we want between readings
  // delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  //  digitalWrite(greenLEDpin, HIGH);

  // log milliseconds since starting
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

  digitalWrite(ledPinWhite, HIGH);
  //  Serial.println("------------ START ------------");
  //  Serial.print("Time from start: ");
  //  Serial.print((millis() - startTime) / 1000);
  //  Serial.println(" s");
  int ppm_uart = readCO2UART();
  int ppm_pwm = readCO2PWM();

  if (lastppmvalue >= 0 && lastppmvalue < 1000) {
    //    Serial.println("LED: GREEN SOLID");
    digitalWrite(ledPinGreen, HIGH);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinWhite, LOW);
    delay(measure_every_ms);
  } else if (lastppmvalue >= 1000 && lastppmvalue < 2000) {
    //    Serial.println("LED: YELLOW SOLID");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, HIGH);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinWhite, LOW);
    delay(measure_every_ms);
  } else if (lastppmvalue >= 2000 && lastppmvalue < 3000) {
    //    Serial.println("LED: YELLOW BLINKING");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinWhite, LOW);
    count = measure_every_ms;
    while (count > 0 )                                  // repeat until count is no longer greater than zero
    {
      digitalWrite(ledPinYellow, HIGH);
      delay(500);
      digitalWrite(ledPinYellow, LOW);
      delay(500);
      count = count - 1000;                                 // decrement count
    }
  } else if (lastppmvalue >= 3000 && lastppmvalue < 5000) {
    //   Serial.println("LED: RED");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinWhite, LOW);
    count = measure_every_ms;
    while (count > 0 )                                  // repeat until count is no longer greater than zero
    {
      digitalWrite(ledPinRed, HIGH);
      delay(500);
      digitalWrite(ledPinRed, LOW);
      delay(500);
      count = count - 1000;                                  // decrement count
    }
  } else {
    //   Serial.println("LED: Ke?");
    digitalWrite(ledPinGreen, HIGH);
    digitalWrite(ledPinYellow, HIGH);
    digitalWrite(ledPinRed, HIGH);
    digitalWrite(ledPinWhite, LOW);
    delay(measure_every_ms);
  }
if (write_to_log == 1) {
  logfile.println();
}
#if ECHO_TO_SERIAL
  Serial.println();
#endif // ECHO_TO_SERIAL

  digitalWrite(ledPinBlue, LOW);

  if (write_to_log == 1) {
    logfile.flush();
    flushblinkcount=5;
    while (flushblinkcount > 0 )                                  // repeat until count is no longer greater than zero
    {
      digitalWrite(ledPinBlue, HIGH);
      delay(250);
      digitalWrite(ledPinBlue, LOW);
      delay(250);
      flushblinkcount = flushblinkcount - 1;                                  // decrement count
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
      //    Serial.print(String(response[i], HEX));
      //    Serial.print("   ");
    }
    //  Serial.println("");

    // checksum
    byte check = getCheckSum(response);
    if (response[8] != check) {
      //    Serial.println("Checksum not OK!");
      //    Serial.print("Received: ");
      //    Serial.println(response[8]);
      //    Serial.print("Should be: ");
      //    Serial.println(check);
    }

    // ppm
    int ppm_uart = 256 * (int)response[2] + response[3];
    lastppmvalue = ppm_uart;
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

  int readCO2PWM() {
    unsigned long th, tl, ppm_pwm = 0;
    do {
      th = pulseIn(9, HIGH, 1004000) / 1000;
      tl = 1004 - th;
      ppm_pwm = 5000 * (th - 2) / (th + tl - 4);
    } while (th == 0);
if (write_to_log == 1) {
    logfile.print(",");
    logfile.print(ppm_pwm);
}
#if ECHO_TO_SERIAL
    Serial.print(",");
    Serial.print(ppm_pwm);
#endif //ECHO_TO_SERIAL
    return ppm_pwm;
  }
