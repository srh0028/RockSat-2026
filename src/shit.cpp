#include "main.h"

int loopCount = 0;
float lastTemperature = 0.0;

// Initialize MAX31865 on SPI1 for 3-wire RTD
Adafruit_MAX31865 max31865 = Adafruit_MAX31865(MAX_CS, &SPI1);

void blinkLED(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(delayMs);
        digitalWrite(LED_PIN, LOW);
        delay(delayMs);
    }
}

void setupSD(void) {
    SPI.setRX(SD_MISO);
    SPI.setTX(SD_MOSI);
    SPI.setSCK(SD_SCK);
    SPI.begin();
    
    delay(100);
    
    if (!SD.begin(SD_CS)) {
        // SD card failed - blink continuously
        while (true) {
            blinkLED(1, 500);
            delay(500);
        }
    }
    
    blinkLED(2, 100);
}

void setupMAX31865(void) {
    SPI1.setRX(MAX_MISO);
    SPI1.setTX(MAX_MOSI);
    SPI1.setSCK(MAX_SCK);
    SPI1.begin();
    
    // Initialize and configure MAX31865 for 3-wire
    max31865.begin(MAX31865_3WIRE);
    max31865.setWires(MAX31865_3WIRE);
    max31865.enable50Hz(false);
    
    blinkLED(1, 100);
}

float readTemperature(void) {
    uint16_t rtd = max31865.readRTD();
    
    if (rtd == 0xFFFF) {
        uint8_t fault = max31865.readFault();
        String faultMsg = "FAULT: ";
        
        if (fault & MAX31865_FAULT_HIGHTHRESH) faultMsg += "HighThreshold ";
        if (fault & MAX31865_FAULT_LOWTHRESH) faultMsg += "LowThreshold ";
        if (fault & MAX31865_FAULT_REFINLOW) faultMsg += "RefinLow ";
        if (fault & MAX31865_FAULT_REFINHIGH) faultMsg += "RefinHigh ";
        if (fault & MAX31865_FAULT_RTDINLOW) faultMsg += "RTDinLow ";
        if (fault & MAX31865_FAULT_OVUV) faultMsg += "OverUnderVoltage ";
        
        writeToSD(faultMsg);
        max31865.clearFault();
        return -999.0;
    }
    
    // Calculate resistance
    float ratio = rtd / 32768.0;
    float resistance = RREF * ratio;
    
    // Callendar-Van Dusen equation for PT100 (0°C to 850°C)
    // R(t) = R0 * (1 + A*t + B*t^2)
    // where A = 3.9083e-3, B = -5.775e-7
    float R0 = RNOMINAL;  // 100.0
    float A = 0.0039083;
    float B = -0.0000005775;
    
    // Solve quadratic: B*t^2 + A*t + (1 - R/R0) = 0
    float c = 1.0 - (resistance / R0);
    float discriminant = (A * A) - (4 * B * c);
    
    if (discriminant < 0) return -999.0;
    
    float temperature = (-A + sqrt(discriminant)) / (2 * B);
    
    return temperature;
}

void writeToSD(String message) {
    File dataFile = SD.open("/temperature_log.csv", FILE_WRITE);
    if (dataFile) {
        dataFile.println(message);
        dataFile.close();
        blinkLED(1, 50);
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    
    // Startup pattern: 3 slow blinks
    blinkLED(3, 200);
    
    // Initialize components
    setupSD();        // 2 fast blinks on success
    setupMAX31865();  // 1 fast blink on success
    
    // Write CSV header with diagnostic columns
    writeToSD("Loop,Time_ms,RawRTD,Resistance_Ohms,Temp_Manual_C,Temp_Library_C");
    
    // Ready pattern: 3 fast blinks
    blinkLED(3, 100);
}

void loop() {

    loopCount++;
    
    uint16_t rawRTD = max31865.readRTD();
    float ratio = rawRTD / 32768.0;
    float resistance = RREF * ratio;
    
    // Manual calculation
    float temperatureManual = (resistance - 100.0) / 0.385;
    
    // Library calculation (for comparison)
    float temperatureLibrary = max31865.temperature(RNOMINAL, resistance);
    
    String dataLine = String(loopCount) + "," + 
                      String(millis()) + "," + 
                      String(rawRTD) + "," + 
                      String(resistance, 3) + "," + 
                      String(temperatureManual, 2) + "," +
                      String(temperatureLibrary, 2);
    writeToSD(dataLine);
    
    // Blink based on manual temperature
    int blinks = abs((int)temperatureManual % 10);
    if (blinks == 0) blinks = 1;
    blinkLED(blinks, 100);
    
    delay(1000);
}