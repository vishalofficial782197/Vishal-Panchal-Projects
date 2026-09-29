# ThingSpeak – IoT Cloud Monitoring

## Overview

**ThingSpeak** is used in the EcoGrow project for collecting, storing, visualizing, and monitoring greenhouse sensor data through the Internet.

The NodeMCU ESP8266 provides Wi-Fi connectivity and sends sensor readings to the ThingSpeak cloud platform for remote monitoring and data analysis.

## System Architecture

```text
Greenhouse Sensors
        ↓
    Arduino UNO
        ↓
   NodeMCU ESP8266
        ↓
       Wi-Fi
        ↓
    ThingSpeak
        ↓
Cloud Data Visualization
```

## Technologies Used

* NodeMCU ESP8266
* Wi-Fi
* ThingSpeak
* Arduino IDE
* Embedded C / Arduino Programming
* HTTP Communication

## Sensor Data

The system can transmit parameters such as:

| Field   | Parameter          |
| ------- | ------------------ |
| Field 1 | Temperature        |
| Field 2 | Humidity           |
| Field 3 | Air Quality        |
| Field 4 | Soil Moisture      |
| Field 5 | Water Level        |
| Field 6 | Light Intensity    |
| Field 7 | pH, if implemented |

The exact field mapping can be changed according to the final system configuration.

## Working

1. Sensors collect environmental information from the greenhouse.
2. Arduino UNO processes the sensor readings.
3. The required sensor data is transferred to NodeMCU ESP8266.
4. NodeMCU connects to the Internet using Wi-Fi.
5. NodeMCU sends the sensor values to the configured ThingSpeak channel.
6. ThingSpeak stores the incoming data in cloud-based channels.
7. The collected data can be visualized using graphs and used for further analysis.
8. Historical data can also be used to create datasets for machine-learning applications.

## Data Visualization

ThingSpeak provides graphical visualization of the collected sensor parameters.

Example monitoring parameters:

* Temperature trends
* Humidity trends
* Air-quality variations
* Soil-moisture levels
* Water-level variations
* Light-intensity changes

## IoT Data Flow

```text
Sensor Data
     ↓
Arduino UNO
     ↓
NodeMCU ESP8266
     ↓
Wi-Fi
     ↓
ThingSpeak Channel
     ↓
Data Storage
     ↓
Graphs / Analysis
     ↓
ML Dataset
```

## Machine Learning Integration

The historical data collected through ThingSpeak can be used as a dataset for the Machine Learning module.

```text
ThingSpeak
     ↓
Historical Sensor Data
     ↓
Dataset
     ↓
Data Preprocessing
     ↓
ML Model
     ↓
Prediction
```

## Applications

* Remote greenhouse monitoring
* Environmental data logging
* Sensor trend analysis
* Smart agriculture
* Precision farming
* Machine-learning dataset generation

## Security Note

Do not publish your actual **ThingSpeak Write API Key** or other private credentials in a public GitHub repository.

Use a placeholder such as:

```cpp
const char* API_KEY = "YOUR_THINGSPEAK_API_KEY";
```

## Author

**Vishal Panchal**

