# IoT-Agricultural-Sensing-and-Control-with-ESP-NOW
Sensing agricultural data and transmitting it to node for monitoring and controlling the actuators with Web-Dashboard

# Smart Agriculture IoT Dashboard
![image](https://github.com/Vigneshsundar128/IoT-Agricultural-Sensing-and-Control-with-ESP-NOW/assets/65441369/7209c18c-3d2e-4c2d-a285-c678b0dcf338)

# Schematic for connection
![image](https://github.com/Vigneshsundar128/IoT-Agricultural-Sensing-and-Control-with-ESP-NOW/assets/65441369/0c0714f0-8c72-4785-88cb-5421a84754db)


## Overview

The Smart Agriculture IoT Dashboard is a project that combines the power of ESP-NOW and Wi-Fi to create a robust agricultural monitoring and control system. This system enables real-time data collection from multiple sensor nodes and provides a user-friendly web-based dashboard for remote monitoring and control of agricultural processes. 

The system is designed to:

- Gather data on temperature, humidity, and soil moisture from sensor nodes deployed in the field (in the future, we can add other relevant sensors to monitor the field in a more efficient way).
- Transmit the collected data to a central node via ESP-NOW mesh networking.
- Display real-time sensor data on a web dashboard accessible from any device with an internet connection.
- Allow users to remotely control actuators (e.g., motor pumps) through the web dashboard.

## Features

- **Multi-Node Sensor Network:** The system can handle data from multiple sensor nodes, making it suitable for large agricultural areas.

- **Real-Time Monitoring:** The web dashboard provides real-time data updates, enabling farmers to make informed decisions.

- **Actuator Control:** Users can remotely control actuators (e.g., motor pumps) through the web dashboard.

## Getting Started

### Hardware Requirements

- ESP32 boards (for sensor nodes and the central node)
- Sensors (e.g., DHT11 for temperature and humidity, soil moisture sensors)
- Actuators (e.g., motor pumps)
- Power supplies (e.g., lithium batteries)
- Wi-Fi network for internet connectivity

### Software Requirements

- Arduino IDE
- Required Arduino libraries: WiFi, AsyncTCP, ESPAsyncWebServer, Arduino_JSON
- Web browser for dashboard access

### Installation

1. Clone this repository to your local machine.
2. Open the Arduino IDE and load the code for the central node (`esp_receiver_ap+wifi_mode.ino`) and the sensor nodes (`esp_sender_apmode.ino`).
3. Customize the Wi-Fi credentials and other configuration settings in the code to match your network and requirements.
4. Upload the code to the ESP32 boards.
5. Connect the sensors and actuators to the appropriate pins on the ESP32 boards.
6. Power up the devices.

### Usage

1. Open the Serial Monitor in the Arduino IDE for each ESP32 board to monitor their status.
2. Connect to the Wi-Fi network created by the central node and note its IP address.
3. Access the web dashboard by entering the central node's IP address in your web browser.

### Web Dashboard

The web dashboard provides real-time data for each sensor node and allows you to control the actuators. Here's a quick overview of the dashboard features:

- **Temperature, Humidity, and Soil Moisture Readings:** View real-time sensor data for each sensor node.

- **Actuator Control:** Toggle the motor pumps on or off remotely.

### Contributing

We welcome contributions from the community. If you have ideas for improvements or would like to report issues, please open an issue or create a pull request.

### License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- This project was inspired by the need to modernize agriculture and make it more data-driven and efficient.
- We would like to thank the open-source community for their contributions to the libraries and tools used in this project.
   - [randomnerdtutorials](randomnerdtutorials.com)
   - [ESP-NOW-WiFi-Gateway](https://github.com/lukalafaye/ESP-NOW-WiFi-Gateway) 
   

