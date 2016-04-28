#define SPI_AMONG_WORDS_MICRO_SECONDS_DELAY 10

// Declare input/output
const byte rheostatAnalogInput = A0; // use Analog input 0 for rheostat volgate (range from 0v to 3.3v)
const byte ledDigitalOutput = 6; // use Digital input/output 6 for LED blinking output (frequency from 1Hz to 10Hz according to Rheostat)
const byte motorPWMOutput = 9; // use PWM input/output 9 for motor output (on/off motor)

// Initial value for in/out
bool ledValue = HIGH; // LED is connected to VDD with a resistor. Output LOW to make LED on
int motorValue = 0; // PWM value for Motor control
int rheostatValue; //input volate from Rheostat
byte ledPeriod = 50; // period for led blinking
byte motorOnOffFromMaster = 'L';// default is LOW that indicates the motor is off

// Declare prototypes to avoid the error "was not declared in this scope"
byte SPITransceiver(byte sendingByte);
void SlaveInit(void);

void setup() {
    Serial.begin(9600);
    SlaveInit();
    pinMode(ledDigitalOutput, OUTPUT);
    pinMode(motorPWMOutput, OUTPUT);
}

void loop() {
  //LED blinking
  digitalWrite(ledDigitalOutput, ledValue);
  delay(ledPeriod);
  ledValue = !ledValue;

  //Motor control
  rheostatValue = analogRead(rheostatAnalogInput);
  motorValue = map(rheostatValue, 0, 1023, 0, 255);
  ledPeriod = map(rheostatValue, 0, 1023, 255, 25);
  if (motorOnOffFromMaster == 'H'){
    analogWrite(motorPWMOutput, motorValue);  
  } else if (motorOnOffFromMaster == 'L') {
    analogWrite(motorPWMOutput, 0);  
  } 
   SPDR = ledPeriod;
}


ISR(SPI_STC_vect)
{
//    motorOnOffFromMaster = SPITransceiver(100);
//    Serial.println(motorOnOffFromMaster, DEC);
//  while(!(SPSR & (1<<SPIF)));
//    delayMicroseconds(SPI_AMONG_WORDS_MICRO_SECONDS_DELAY);
//  Serial.println(SPDR, DEC);
  motorOnOffFromMaster = SPDR;
}

// Function definitions
byte SPITransceiver(byte sendingByte){
  SPDR = sendingByte;
  while(!(SPSR & (1<<SPIF)));
    delayMicroseconds(SPI_AMONG_WORDS_MICRO_SECONDS_DELAY);
  return SPDR;
}

//Initialize Arduino as slave.
void SlaveInit(void) {
  // Initialize SPI pins.
  pinMode(SCK, INPUT);
  pinMode(MOSI, INPUT);
  pinMode(MISO, OUTPUT);
  pinMode(SS, INPUT);
  // config working mode for SPI as Slave and using interrupt. Prefer  www.iotbreaks.vn for explanation
  SPCR = (1 << SPIE ) | ( 1 << SPE) | (0 << DORD) | (0 << MSTR) | (0 << CPOL) | (0 << CPHA) | (0 << SPR1 ) | (0 << SPR0); 
}

