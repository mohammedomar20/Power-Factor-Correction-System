# Power-Factor-Correction-System
This repository contains the code and schematics for an Arduino-based power factor correction system. The system monitors and compensates for the power factor in real-time, displaying voltage, current, and power factor values on an LCD screen and controlling capacitor banks with LED indicators.

Features:
Real-time monitoring of voltage, current, and power factor
Calculation and display of average power and reactive power requirements
Automatic activation of capacitor banks to achieve desired power factor
User-friendly interface with LCD display and button controls
Indicator LEDs for capacitor bank status

Hardware Components:
Arduino Uno
LCD display
Analog sensors for voltage and current measurement
LEDs for capacitor bank indication
Push buttons for user interaction

Software Techniques:
C++ programming for signal processing and real-time data acquisition
Control algorithms for power factor correction
Implementation of best combination selection for reactive power compensation

Usage:
Connect the hardware components as per the schematic.
Power on the system.
Use the buttons to navigate the LCD menu and monitor real-time power factor data.
The system will automatically activate the required capacitor banks based on the calculated reactive power.
