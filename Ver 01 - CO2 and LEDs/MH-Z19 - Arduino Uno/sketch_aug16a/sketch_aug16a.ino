
#include <SoftwareSerial.h>
SoftwareSerial co2Serial(A0, A1); // define MH-Z19 RX TX
unsigned long startTime = millis();

int ledPinBlue = 4; //Blue led
int ledPinGreen = 5; //Green led
int ledPinYellow = 6; //Yellow led
int ledPinRed = 7; //Red led
int lastppmvalue;
int count;
 
void setup() {
  Serial.begin(9600);
  co2Serial.begin(9600);
  pinMode(9, INPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  pinMode(ledPinRed, OUTPUT);

  digitalWrite(ledPinGreen, HIGH);
  digitalWrite(ledPinYellow, HIGH);
  digitalWrite(ledPinRed, HIGH);
  digitalWrite(ledPinBlue, HIGH);
  delay(1000);
  digitalWrite(ledPinGreen, LOW);
  digitalWrite(ledPinYellow, LOW);
  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinBlue, LOW);
  delay(1000);

  count = 30;
    while(count > 0 )                                   // repeat until count is no longer greater than zero
      {
        digitalWrite(ledPinBlue, HIGH);
        delay(500);
        digitalWrite(ledPinBlue, LOW);  
        delay(500);  
        count = count -1;                                  // decrement count
      } 
}
 
void loop() {
  digitalWrite(ledPinBlue, HIGH);
  Serial.println("------------ START ------------");
  Serial.print("Time from start: ");
  Serial.print((millis() - startTime) / 1000);
  Serial.println(" s");
  int ppm_uart = readCO2UART();
  int ppm_pwm = readCO2PWM();

  if (lastppmvalue < 1000) {
    Serial.println("LED: GREEN SOLID");
    digitalWrite(ledPinGreen, HIGH);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinBlue, LOW);
    delay(10000);
  } else if (lastppmvalue >= 1000 && lastppmvalue < 2000) {
    Serial.println("LED: YELLOW SOLID");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, HIGH);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinBlue, LOW);
    delay(10000);
  } else if (lastppmvalue >= 2000 && lastppmvalue < 3000) {
    Serial.println("LED: YELLOW BLINKING");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinRed, LOW);
    digitalWrite(ledPinBlue, LOW);
    count = 10;
    while(count > 0 )                                   // repeat until count is no longer greater than zero
      {
        digitalWrite(ledPinYellow, HIGH);
        delay(500);
        digitalWrite(ledPinYellow, LOW);  
        delay(500);  
        count = count -1;                                  // decrement count
      } 
  } else if (lastppmvalue >= 3000) {
    Serial.println("LED: RED");
    digitalWrite(ledPinGreen, LOW);
    digitalWrite(ledPinYellow, LOW);
    digitalWrite(ledPinBlue, LOW);
    count = 10;
    while(count > 0 )                                   // repeat until count is no longer greater than zero
      {
        digitalWrite(ledPinRed, HIGH);
        delay(500);
        digitalWrite(ledPinRed, LOW);  
        delay(500);  
        count = count -1;                                  // decrement count
      } 
  } else {
    Serial.println("LED: Ke?");
    digitalWrite(ledPinGreen, HIGH);
    digitalWrite(ledPinYellow, HIGH);
    digitalWrite(ledPinRed, HIGH);
    digitalWrite(ledPinBlue, LOW);
    delay(10000);    
  }
}

int readCO2UART(){
  byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  byte response[9]; // for answer

  Serial.println("Sending CO2 request...");
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
    Serial.print(String(response[i], HEX));
    Serial.print("   ");
  }
  Serial.println("");

  // checksum
  byte check = getCheckSum(response);
  if (response[8] != check) {
    Serial.println("Checksum not OK!");
    Serial.print("Received: ");
    Serial.println(response[8]);
    Serial.print("Should be: ");
    Serial.println(check);
  }
 
  // ppm
  int ppm_uart = 256 * (int)response[2] + response[3];
  Serial.print("PPM UART: ");
  Serial.println(ppm_uart);
  lastppmvalue = ppm_uart;
  // temp
  byte temp = response[4] - 40;
  Serial.print("Temperature? ");
  Serial.println(temp);

  // status
  byte status = response[5];
  Serial.print("Status? ");
  Serial.println(status);
  if (status == 0x40) {
    Serial.println("Status OK");
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
    ppm_pwm = 5000 * (th-2)/(th+tl-4);
  } while (th == 0);
  Serial.print("PPM PWM: ");
  Serial.println(ppm_pwm);
  Serial.println("------------ END ------------");
  return ppm_pwm; 
}
