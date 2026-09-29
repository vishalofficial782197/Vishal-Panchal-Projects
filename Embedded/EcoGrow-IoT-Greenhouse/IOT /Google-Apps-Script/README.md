# Google Apps Script – IoT Data Logger

## Overview

The Google Apps Script module is used to receive greenhouse sensor data from the IoT system and automatically store the received data in **Google Sheets**.

This module provides a simple cloud-based data-logging system that can be used for **real-time monitoring, historical analysis, and machine-learning dataset generation**.

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
 Google Apps Script
        ↓
   Google Sheets
        ↓
 Machine Learning Dataset
```

## Technologies Used

* Google Apps Script
* Google Sheets
* JavaScript
* NodeMCU ESP8266
* Wi-Fi
* HTTP Request

## Data Parameters

The system can store greenhouse parameters such as:

| Parameter       | Description                      |
| --------------- | -------------------------------- |
| Temperature     | Greenhouse temperature           |
| Humidity        | Relative humidity                |
| Air Quality     | Air-quality sensor reading       |
| Soil Moisture   | Soil moisture level              |
| Water Level     | Water availability               |
| Light Intensity | Light condition                  |
| pH              | Soil/water pH, if implemented    |
| Timestamp       | Date and time of data collection |

## Working

1. The sensors collect environmental data from the greenhouse.
2. Arduino UNO processes the sensor readings.
3. NodeMCU ESP8266 receives the required data.
4. NodeMCU connects to the Internet using Wi-Fi.
5. Sensor data is sent to the Google Apps Script Web App using an HTTP request.
6. Google Apps Script processes the received parameters.
7. The data is automatically added to Google Sheets.
8. The stored data can be used for analysis and machine-learning model development.

## Google Sheets Data Logging

The Google Sheet acts as a cloud-based database for the collected sensor data.

Example:

| Timestamp | Temperature | Humidity | Air Quality | Soil Moisture | Water Level |
| --------- | ----------: | -------: | ----------: | ------------: | ----------: |
| Date/Time |     28.5 °C |     65 % |         120 |           540 |        80 % |

## Google Apps Script Functions

The Apps Script can be used to:

* Receive HTTP requests
* Process sensor parameters
* Add timestamps
* Store sensor readings
* Organize data in Google Sheets
* Generate datasets for ML analysis

## Machine Learning Integration

The data stored in Google Sheets can later be exported or accessed for machine-learning applications.

```text
Google Sheets
      ↓
Historical Dataset
      ↓
Data Preprocessing
      ↓
ML Model Training
      ↓
Prediction
```

## Security Note

API keys, authentication tokens, passwords, Wi-Fi credentials, and other private information should **not be uploaded to a public GitHub repository**.

Use placeholders in publicly shared code, for example:

```javascript
const SHEET_ID = "YOUR_SHEET_ID";
```

## Author

**Vishal Panchal**
