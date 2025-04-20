#include <Stepper.h>
#include <LiquidCrystal_I2C.h>

#define POT_PIN A0

int stepsPerRevolution = 2048;
int minRPM = 1;
int maxRPM = 30;
int rpm = 10;
int numberOfRotations = 2;

float angularPosition = 0;
float angularVelocity = 0;
float angularAcceleration = 0; 
float previousAngularVelocity = 0;
float alphaFilter = 0.2;

unsigned long lastStepTime = 0;
unsigned long currentTime = 0;
unsigned long lastReadTime = 0;
const int readInterval = 500; 

LiquidCrystal_I2C lcd(0x27, 16, 2);
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    myStepper.setSpeed(rpm);
    executeRotations();
}

void loop() {
}

void executeRotations() {
    for (int i = 0; i < numberOfRotations; i++) {
        executeOneRotation(1);
    }
    delay(1000);
    executeOneRotation(-1);
    Serial.println("Start position reached");
}

void executeOneRotation(int direction) {
    for (int stepCount = 0; stepCount < stepsPerRevolution; stepCount++) {
        myStepper.step(direction);
        updatePositionAndSpeed(direction);
        if (millis() - lastReadTime >= readInterval) {
            readPotentiometer();
            lastReadTime = millis();
        }
    }
    delay(500);
}

void readPotentiometer() {
    int potValue = analogRead(POT_PIN);  
    rpm = map(potValue, 0, 1023, minRPM, maxRPM);  
    myStepper.setSpeed(rpm);  

    displayData();
}

void updatePositionAndSpeed(int direction) {
    currentTime = micros();
    unsigned long deltaTime = currentTime - lastStepTime;

    if (deltaTime > 0) {
        previousAngularVelocity = angularVelocity;
        angularVelocity = (360.0 / stepsPerRevolution) / (deltaTime / 1000000.0);
        
        float rawAcceleration = (angularVelocity - previousAngularVelocity) / (deltaTime / 1000000.0);
        angularAcceleration = angularAcceleration * (1 - alphaFilter) + rawAcceleration * alphaFilter;
    }

    angularPosition += direction * (360.0 / stepsPerRevolution);
    
    if (angularPosition >= 360.0) {
        angularPosition -= 360.0;
    } else if (angularPosition < 0.0) {
        angularPosition += 360.0;
    }

    lastStepTime = currentTime;
}

void displayData() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pos:");
    lcd.print(angularPosition, 1);
    lcd.print(" deg");

    lcd.setCursor(0, 1);
    lcd.print("Vel:");
    lcd.print(angularVelocity, 2);
    lcd.print(" d/s");
    
    lcd.setCursor(8, 1);
    lcd.print("Acc:");
    lcd.print(angularAcceleration, 2);

    Serial.print("Position: ");
    Serial.print(angularPosition, 1);
    Serial.print("° | Velocity: ");
    Serial.print(angularVelocity, 2);
    Serial.print("°/s | Acceleration: ");
    Serial.print(angularAcceleration, 2);
    Serial.print("°/s² | RPM: ");
    Serial.println(rpm);
}
