# IoT-based smart greenhouse system for real-time environmental monitoring, automatic climate control, and plant surveillance.

##Project Overview

EcoGrow is an IoT-based smart greenhouse automation system designed for real-time monitoring and automatic control of important environmental parameters required for healthy plant growth.
The system monitors parameters such as temperature, humidity, air quality, soil moisture, water level, and light intensity. Based on the sensor conditions, the system can automatically control devices such as cooling fans, exhaust fans, water pumps, and lighting.
The project also integrates NodeMCU ESP8266 for IoT-based data transmission and ESP32-CAM for plant surveillance.
## Objectives

* Monitor greenhouse environmental conditions in real time.
* Automatically control important greenhouse devices.
* Monitor soil moisture and control irrigation.
* Monitor temperature and humidity.
* Monitor air quality inside the greenhouse.
* Monitor the water level.
* Provide IoT-based remote data monitoring.
* Provide visual surveillance of plants using ESP32-CAM.

## Key Features

* Real-time temperature and humidity monitoring
* Air-quality monitoring using MQ135
* Soil-moisture monitoring
* Water-level monitoring
* Automatic cooling-fan control
* Automatic exhaust-fan control
* Automatic water-pump control
* Automatic lighting control
* IoT-based data transmission
* Cloud-based data monitoring
* Plant surveillance using ESP32-CAM

## Hardware Components

* Arduino UNO
* NodeMCU ESP8266
* ESP32-CAM
* DHT11 Temperature and Humidity Sensor
* MQ135 Air Quality Sensor
* Soil Moisture Sensor
* Water Level Sensor
* LDR Sensor
* Relay Module
* Cooling Fan
* Exhaust Fan
* Water Pump
* Lighting/Lamp

## Software and Technologies

* Arduino IDE
* Embedded C / Arduino Programming
* ESP8266 Wi-Fi
* ThingSpeak
* Google Sheets
* IoT Data Monitoring
* ESP32-CAM

## System Architecture

The system uses **Arduino UNO** as the primary controller for collecting sensor data and controlling the greenhouse devices.
NodeMCU ESP8266** is used to transmit the collected data through Wi-Fi to IoT platforms for remote monitoring.
ESP32-CAM** provides visual surveillance of the plants and greenhouse environment.

```text
                ┌─────────────────────┐
                │     Greenhouse      │
                │     Environment     │
                └──────────┬──────────┘
                           │
          ┌────────────────┼────────────────┐
          │                │                │
       DHT11             MQ135          Soil Sensor
          │                │                │
          └────────────────┼────────────────┘
                           │
                    ┌──────▼──────┐
                    │  Arduino    │
                    │     UNO     │
                    └──────┬──────┘
                           │
             ┌─────────────┼─────────────┐
             │             │             │
          Fan/Relay      Pump/Relay    Lamp/Relay
                           │
                           ▼
                  ┌────────────────┐
                  │   NodeMCU      │
                  │    ESP8266     │
                  └───────┬────────┘
                          │ Wi-Fi
                          ▼
                 ┌──────────────────┐
                 │ ThingSpeak /     │
                 │ Google Sheets    │
                 └──────────────────┘

                  ESP32-CAM
                       │
                       ▼
                Plant Surveillance
```

## Working Principle

1. The sensors continuously measure the environmental conditions inside the greenhouse.
2. Arduino UNO receives the sensor readings.
3. The controller compares the sensor values with predefined operating conditions.
4. Based on the measured conditions, the corresponding relay-controlled devices are activated or deactivated.
5. The cooling fan is controlled according to temperature conditions.
6. The exhaust fan is controlled according to humidity/environmental conditions.
7. The water pump is controlled according to soil moisture conditions.
8. The lighting system is controlled according to light intensity.
9. NodeMCU ESP8266 receives the required data and transmits it through Wi-Fi.
10. Sensor information can be stored and monitored using IoT platforms.
11. ESP32-CAM provides visual monitoring of the plants and greenhouse.

## IoT Monitoring

The NodeMCU ESP8266 provides Wi-Fi connectivity for sending greenhouse sensor data to cloud-based platforms.

The collected information can be used for:

* Remote monitoring
* Historical data analysis
* Environmental condition tracking
* Plant growth analysis
* Future machine-learning applications

## Applications

* Smart agriculture
* Automated greenhouse systems
* Precision agriculture
* Remote plant monitoring
* IoT-based farming
* Environmental monitoring

## Future Scope

The system can be further enhanced by integrating:
* Machine Learning for plant-health prediction
* AI-based plant disease detection
* Automated pH monitoring
* Advanced irrigation control
* Mobile application monitoring
* Weather-based greenhouse control
* AI-based vision analysis using ESP32-CAM


 Author

Vishal Panchal
B.Tech – VLSI Design and Technology

