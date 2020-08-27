/*
  Using the ARGOS ARTIC R2 Breakout
  By: Paul Clark
  SparkFun Electronics
  Date: August 18th 2020

  License: please see the license file at:
  https://github.com/sparkfun/SparkFun_ARGOS_ARTIC_R2_Arduino_Library/LICENSE.md

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/

  The ARTIC firmware takes up 127KB of program memory!

  Hardware Connections:
  This example assumes the ARTIC Breakout has been mounted on a SparkFun Thing Plus - Artemis:
  https://www.sparkfun.com/products/15574
  CS_Pin = A5 (D24)
  GAIN8_Pin = D3
  GAIN16_Pin = D4
  BOOT_Pin = D5
  INT1_Pin = D6
  INT2_Pin = D7
  RESET_Pin = D8
  PWR_EN_Pin = D9
  (SPI COPI = D11)
  (SPI CIPO = D12)
  (SPI SCK = D13)
*/

#include <SPI.h>

#include "SparkFun_ARGOS_ARTIC_R2_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ARGOS_ARTIC_R2
ARTIC_R2 myARTIC;

// Pin assignments for the SparkFun Thing Plus - Artemis
// (Change these if required)
uint8_t CS_Pin = 24;
uint8_t GAIN8_Pin = 3;
uint8_t GAIN16_Pin = 4;
uint8_t BOOT_Pin = 5;
uint8_t INT1_Pin = 6;
uint8_t INT2_Pin = 7;
uint8_t RESET_Pin = 8;
uint8_t PWR_EN_Pin = 9;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("ARGOS ARTIC R2 Example"));

  SPI.begin();

  //myARTIC.enableDebugging(); // Enable debug messages to Serial

  // Begin the ARTIC: enable power and upload firmware or boot from flash
  if (myARTIC.begin(CS_Pin, RESET_Pin, BOOT_Pin, PWR_EN_Pin, INT1_Pin, INT2_Pin, GAIN8_Pin, GAIN16_Pin) == false)
  {
    Serial.println("ARTIC R2 not detected. Freezing...");
    while (1)
      ; // Do nothing more
  }

  // Read and print the ARGOS configuration
  ARGOS_Configuration_Register configuration;
  myARTIC.readARGOSconfiguration(&configuration);
  if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_A2_MODE) Serial.println(F("TX mode is: ARGOS PTT A2"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_A3_MODE) Serial.println(F("TX mode is: ARGOS PTT A3"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_ZE_MODE) Serial.println(F("TX mode is: ARGOS PTT ZE"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_HD_MODE) Serial.println(F("TX mode is: ARGOS PTT HD"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_A4_MD_MODE) Serial.println(F("TX mode is: ARGOS PTT A4 MD"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_A4_HD_MODE) Serial.println(F("TX mode is: ARGOS PTT A4 HD"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.TX_CONFIGURATION == TX_CONFIG_ARGOS_PTT_A4_VLD_MODE) Serial.println(F("TX mode is: ARGOS PTT A4 VLD"));
  else Serial.println(F("TX mode is not defined!"));
  if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_3_RX_MODE) Serial.println(F("RX mode is: ARGOS 3"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_3_RX_BACKUP_MODE) Serial.println(F("RX mode is: ARGOS 3 BACKUP"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_4_RX_MODE) Serial.println(F("RX mode is: ARGOS 4"));
  else Serial.println(F("RX mode is not defined!"));

  ARTIC_R2_Firmware_Status status;
  myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register

  if (!status.STATUS_REGISTER_BITS.IDLE) Serial.println(F("Warning! ARTIC is not idle! This is likely to fail..."));

  // Set the satellite detection timeout to 60 seconds
  myARTIC.setSatelliteDetectionTimeout(60);

  // Delay for 10ms
  delay(10);

  // Check that the MCU command was accepted
  myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
  if (status.STATUS_REGISTER_BITS.MCU_COMMAND_ACCEPTED) Serial.println(F("MCU command was accepted"));
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_REJECTED)
  {
    Serial.println(F("MCU command was rejected! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_OVERFLOW)
  {
    Serial.println(F("MCU command overflow! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Set the RX mode to ARGOS 4
  myARTIC.setSatelliteDetectionTimeout(CONFIG_CMD_SET_ARGOS_4_RX_MODE);

  // Delay for 10ms
  delay(10);

  // Check that the MCU command was accepted
  myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
  if (status.STATUS_REGISTER_BITS.MCU_COMMAND_ACCEPTED) Serial.println(F("MCU command was accepted"));
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_REJECTED)
  {
    Serial.println(F("MCU command was rejected! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_OVERFLOW)
  {
    Serial.println(F("MCU command overflow! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Read and print the ARGOS RX configuration
  myARTIC.readARGOSconfiguration(&configuration);
  if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_3_RX_MODE) Serial.println(F("RX mode is: ARGOS 3"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_3_RX_BACKUP_MODE) Serial.println(F("RX mode is: ARGOS 3 BACKUP"));
  else if (configuration.CONFIGURATION_REGISTER_BITS.RX_CONFIGURATION == RX_CONFIG_ARGOS_4_RX_MODE) Serial.println(F("RX mode is: ARGOS 4"));
  else Serial.println(F("RX mode is not defined!"));

  // Enable RX transparent mode so we will receive the first valid message even if it is not addressed to us
  if (myARTIC.enableRXTransparentMode() == false)
  {
    Serial.println(F("enableRXTransparentMode failed! Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // Start the ARTIC in receiving mode for an unlimited time until 1 message has been received.
  // If the message is received the Artic will go to IDLE. The user can abort the reception using the ‘Go to idle’ command.
  myARTIC.sendCommandByte(INST_START_RECEIVING_1_MESSAGE);

  // Delay for 10ms
  delay(10);

  // Check that the MCU command was accepted
  myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
  if (status.STATUS_REGISTER_BITS.MCU_COMMAND_ACCEPTED) Serial.println(F("MCU command was accepted"));
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_REJECTED)
  {
    Serial.println(F("MCU command was rejected! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_OVERFLOW)
  {
    Serial.println(F("MCU command overflow! Freezing..."));
    while (1)
      ; // Do nothing more
  }
}

void loop()
{
  delay(1000); // Query the ARTIC firmware status once per second

  Serial.println();

  boolean messageReceived = false; // Flag to indicate if a message is available for reading

  ARTIC_R2_Firmware_Status status;
  myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register

  if (status.STATUS_REGISTER_BITS.IDLE) // Check the IDLE flag
    Serial.println(F("ARTIC is IDLE"));
  else
    Serial.println(F("ARTIC is not IDLE"));

  if (status.STATUS_REGISTER_BITS.BUSY) // Check the BUSY flag
    Serial.println(F("ARTIC is BUSY"));
  else
    Serial.println(F("ARTIC is not BUSY"));

  if (status.STATUS_REGISTER_BITS.IDLE_STATE) // Check the IDLE_STATE flag
    Serial.println(F("ARTIC firmware has returned to the idle state"));
  else
    Serial.println(F("ARTIC firmware has not yet returned to the idle state"));

  if (status.STATUS_REGISTER_BITS.RX_SATELLITE_DETECTED) // Check the RX_SATELLITE_DETECTED flag
  {
    Serial.println(F("A satellite was detected!"));
  }

  if (status.STATUS_REGISTER_BITS.RX_VALID_MESSAGE) // Check the RX_VALID_MESSAGE flag
  {
    Serial.println(F("A valid message was received!"));
    messageReceived = true;
  }

  if (status.STATUS_REGISTER_BITS.DSP2MCU_INT1) // Check the interrupt 1 flag. This will go high when a satellite is detected
  {
    Serial.println(F("INT1 pin is high. Valid message received!"));
    messageReceived = true;
  }
  else
    Serial.println(F("INT1 pin is low."));

  if (messageReceived) // If a message was received, read it.
  {
    // Read a downlink message from the RX payload buffer
    Downlink_Message downlinkMessage;
    if (myARTIC.readDownlinkMessage(&downlinkMessage))
    {
      Serial.println(F("Message received:"));
      Serial.printf("Payload length:  %d\n", downlinkMessage.payloadLength);
      Serial.printf("Addressee ID:    0x%04X\n", downlinkMessage.addresseeIdentification);
      Serial.printf("ADCS:            0x%02X\n", downlinkMessage.ADCS);
      Serial.printf("Service:         0x%02X\n", downlinkMessage.service);
      Serial.printf("FCS:             0x%04X\n", downlinkMessage.FCS);
      Serial.print(F("Payload buffer:  0x"));
      for (int i = 0; i < 17; i++)
      {
        Serial.printf("%02X", downlinkMessage.payload[i]);
      }
      Serial.println();

      Serial.println(F("Clearing interrupt flags"));

      // Clear both interrupt flags
      myARTIC.clearInterrupts(3);

      // Delay for 10ms
      delay(10);

      // Check that the MCU command was accepted
      myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
      if (status.STATUS_REGISTER_BITS.MCU_COMMAND_ACCEPTED) Serial.println(F("MCU command was accepted"));
      else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_REJECTED)
      {
        Serial.println(F("MCU command was rejected! Freezing..."));
        while (1)
          ; // Do nothing more
      }
      else if (status.STATUS_REGISTER_BITS.MCU_COMMAND_OVERFLOW)
      {
        Serial.println(F("MCU command overflow! Freezing..."));
        while (1)
          ; // Do nothing more
      }

      //while (1)
      //  ; // Do nothing more
    }
    else
    {
      Serial.println(F("readDownlinkMessage failed! Freezing..."));
      while (1)
        ; // Do nothing more
    }
  }
}
