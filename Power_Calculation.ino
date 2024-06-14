const int sensorPin = A0;  // Analog pin where the current sensor is connected
const int voltagePin = A1; // Analog pin where the voltage divider is connected
const int relayPin = 8;    // Digital pin where the relay is connected
const float Vref = 5.0;    // Reference voltage (5V for most Arduino boards)
const int sensorResolution = 1024; // 10-bit ADC resolution

// Calibration value for ACS712-30A
const float sensitivity = 0.072164958;  // 66mV/A for ACS712-30A

// Voltage divider resistors (adjust these values to match your setup)
const float R1 = 100.0;  // Resistor R1 value in ohms
const float R2 = 4200.0;  // Resistor R2 value in ohms

// Variables
int sensorValue = 0;
float current = 0.0;
float offsetVoltage = 2.52;  // Default value (Vcc/2), should be measured for more accuracy
const int numReadings = 100;  // Number of readings for averaging
const float batteryCapacity = 1000.0; // Battery capacity in mAh
float totalConsumedmAh = 0.0;
float batteryVoltage = 0.0;

unsigned long startTime = 0;
unsigned long lastUpdateTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Assume relay is active LOW; turn it on initially
  
  // Measure the offset voltage (sensor reading at 0A)
  offsetVoltage = measureOffset();
  startTime = millis(); // Start the timer
  lastUpdateTime = startTime; // Initialize last update time
}

void loop() {
  // Calculate time since measurement started
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;

  // Calculate hours, minutes, and seconds
  unsigned long hours = elapsedTime / 3600000;
  unsigned long minutes = (elapsedTime % 3600000) / 60000;
  unsigned long seconds = (elapsedTime % 60000) / 1000;

  // Average multiple readings to reduce noise
  long sensorSum = 0;
  for (int i = 0; i < numReadings; i++) {
    sensorSum += analogRead(sensorPin);
    delay(1);  // Short delay between readings
  }
  float averageSensorValue = sensorSum / (float)numReadings;

  // Convert the sensor value to current
  float voltage = (averageSensorValue / (float)sensorResolution) * Vref;
  current = (voltage - offsetVoltage) / sensitivity;

  // Measure battery voltage
  long voltageSum = 0;
  for (int i = 0; i < numReadings; i++) {
    voltageSum += analogRead(voltagePin);
    delay(1);  // Short delay between readings
  }
  float averageVoltageValue = voltageSum / (float)numReadings;
  float measuredVoltage = (averageVoltageValue / (float)sensorResolution) * Vref;
  batteryVoltage = measuredVoltage * (R1 + R2) / R2;

  // Calculate the time difference in hours since the last update
  float timeDifference = (currentTime - lastUpdateTime) / 3600000.0;

  // Update total mAh consumed
  totalConsumedmAh += current * timeDifference;
  lastUpdateTime = currentTime;

  // Print current, voltage, time, and total mAh consumed to the serial monitor
  Serial.print("Current: ");
  Serial.print(current, 3);
  Serial.print(" A | Voltage: ");
  Serial.print(batteryVoltage /0.35, 2);
  Serial.print(" V | Time: ");
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
  Serial.print(" | Consumed: ");
  Serial.print(totalConsumedmAh, 3);
  Serial.println(" mAh");

  // Plot the data on the Serial Plotter
  unsigned long totalTimeInSeconds = hours * 3600 + minutes * 60 + seconds;
  Serial.print(totalTimeInSeconds); // Convert time to seconds
  Serial.print(" ");
  Serial.println(current);

  // Check if 80% of the battery has been consumed
  if (totalConsumedmAh >= (0.8 * batteryCapacity)) {
    Serial.println("Battery 80% consumed. Battery exhausted or drained.");
  }

  // Check battery voltage and turn off relay if it is less than or equal to 3.95V
  if (batteryVoltage <= 3.50) {
    digitalWrite(relayPin, LOW);  // Turn off the relay
    Serial.println("Battery voltage is low. Relay turned off.");
  } else {
    digitalWrite(relayPin, HIGH);  // Keep the relay on
  }

  delay(1000);  // Delay for a second
}

float measureOffset() {
  long sum = 0;
  int offsetReadings = 100;

  // Take multiple readings and average them to find the offset voltage
  for (int i = 0; i < offsetReadings; i++) {
    sum += analogRead(sensorPin);
    delay(10);
  }

  float average = sum / offsetReadings;
  float offset = (average / (float)sensorResolution) * Vref;

  Serial.print("Offset Voltage: ");
  Serial.println(offset, 3);

  return offset;
}