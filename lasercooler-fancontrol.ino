#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_FAN_PWM 3
#define PIN_TEMPERATURE_SENSOR 2

#define TEMP_ABSOLUTE_MAX 36
#define TEMP_DIFFERENCE_MAX 10

#define RPM_FAN_FULL 79
#define RPM_FAN_MAX 50
#define RPM_FAN_MIN 23

OneWire oneWire(PIN_TEMPERATURE_SENSOR);
DallasTemperature sensors(&oneWire);

#define SENSOR_WATER 0
#define SENSOR_AIR 1

uint8_t oneWireSensors[][8] = {
  {0x28, 0xFF, 0x89, 0xC1, 0x43, 0x16, 0x04, 0x34}, // Water
  {0x28, 0xFF, 0x97, 0xD9, 0x43, 0x16, 0x03, 0x4D}, // Air (inside)
};

uint8_t rpm = 0;

void setup() {
  pinMode(PIN_FAN_PWM, OUTPUT);

  // 25 kHz
  setupPWM(79);
  setPWM(rpm);
  
  Serial.begin(115200);
  sensors.begin();
}

void loop() {

  sensors.requestTemperatures();
  
  float temperatureAir = sensors.getTempC(oneWireSensors[SENSOR_AIR]);
  float temperatureWater = sensors.getTempC(oneWireSensors[SENSOR_WATER]);
  float temperatureDifference = (temperatureWater - temperatureAir);

  temperatureDifference = constrain(temperatureDifference, 0, TEMP_DIFFERENCE_MAX);
  rpm = map(temperatureDifference, 0, TEMP_DIFFERENCE_MAX, RPM_FAN_MIN, RPM_FAN_MAX);

  // Emergency cooling!
  if (temperatureWater > TEMP_ABSOLUTE_MAX) {
    rpm = RPM_FAN_FULL;
  }

  Serial.println("-----------------");

  Serial.print("AIR: ");
  Serial.print(temperatureAir);
  Serial.println(" °C");

  Serial.print("WATER: ");
  Serial.print(temperatureWater);
  Serial.println(" °C");

  Serial.print("Diff: ");
  Serial.print(temperatureDifference);
  Serial.println(" °C");

  Serial.print("RPM: ");
  Serial.println(rpm);

  setPWM(rpm);
  delay(500);
}

void setupPWM(uint8_t overflow) {
  TCCR2A = 0;                               // TC2 Control Register A
  TCCR2B = 0;                               // TC2 Control Register B
  TIMSK2 = 0;                               // TC2 Interrupt Mask Register
  TIFR2 = 0;                                // TC2 Interrupt Flag Register
  TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);  // OC2B cleared/set on match when up/down counting, fast PWM
  TCCR2B |= (1 << WGM22) | (1 << CS21);     // prescaler 8
  OCR2A = overflow;                               // TOP overflow value (Hz)
  OCR2B = 0;
}

void setPWM(uint8_t dutyCycle) {
  OCR2B = dutyCycle; // PWM Width (duty)
}
