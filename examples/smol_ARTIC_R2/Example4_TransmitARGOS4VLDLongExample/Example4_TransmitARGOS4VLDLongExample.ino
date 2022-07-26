/*
  Using the SparkFun smôl ARGOS ARTIC R2 Board
  By: Paul Clark
  SparkFun Electronics
  Date: August 30th 2021

  This example shows how to use the smôl ARTIC R2 for ARGOS satellite communication.
  The smôl ESP32 is the board which runs this example.

  Feel like supporting our work? Buy a board from SparkFun!

  The smôl stack-up for this example is:
  smôl ZOE-M8Q:          https://www.sparkfun.com/products/18358
  smôl ARTIC R2:         https://www.sparkfun.com/products/18363
  
  The way the boards are stacked is important:

  OUT ---smôl ARTIC R2--- IN
                           |
   ________________________/
  /
  |
  OUT ---  smôl ESP32 --- IN

  Arranged like this:
  The ESP32 GPIO0 (Digital Pin 27) controls the power for the ARTIC R2
  ARTIC R2 uses SPI Chip Select 0 (ESP32 Digital Pin 5)
  
  This example:
    begins (initializes) the ARTIC;
    reads and prints the ARTIC TX and RX configuration;
    reads and prints the firmware status;
    sets the TCXO voltage;
    sets the TCXO warmup time;
    sets the satellite detection timeout to 60 seconds;
    sets the TX mode to ARGOS A4 VLD;
    sets the TX frequency;
    instructs the ARTIC to Transmit One Package And Go Idle;
    keeps checking the MCU status until transmit is complete;
    repeats.

  The transmit power can be reduced by 8dB by uncommenting the line: myARTIC.attenuateTXgain(true);

  The messages are ARGOS 4 VLD (Long) and contain the example used in A4-SS-TER-SP-0079-CNES.

*/

// From v1.1.0 of the library, the platform ID is stored in PMEM and, for this example, should be 0x01234567

const unsigned long repetitionPeriod = 50000; // Define the repetition period in milliseconds

const uint32_t tcxoWarmupTime = 10; // Define the TCXO warmup time (in seconds)

#include <SPI.h>

#include "SparkFun_ARGOS_ARTIC_R2_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ARGOS_ARTIC_R2
ARTIC_R2 myARTIC;

#include <Wire.h> //Needed for I2C to ARTIC R2 GPIO and GNSS

#include "SparkFun_u-blox_GNSS_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

// Pin assignments for the smôl stack-up described above
int CS_Pin = 5;            // smôl CS0 = ESP32 Pin 5
int ARTIC_PWR_EN_Pin = 27; // smôl GPIO0 = ESP32 Pin 27

// The ARTIC RESETB, INT1, BOOT and G8 signals are accessed through a PCA9536 I2C-GPIO chip on the smôl ARTIC R2

// Loop Steps - these are used by the switch/case in the main loop
// This structure makes it easy to jump between any of the steps
typedef enum {
  configure_ARTIC,     // Configure the ARTIC (set the satellite detection timeout and TX mode)
  ARTIC_TX,            // Start the ARTIC TX
  wait_for_ARTIC_TX    // Wait for the ARTIC to transmit
} loop_steps;
loop_steps loop_step = configure_ARTIC; // Make sure loop_step is set to configure_ARTIC

// AS3-SP-516-2098-CNES specifies a ±10% 'jitter' on the repetition period to reduce the risk of transmission collisions
uint32_t nextTransmitTime; // Time of the next satellite transmission (before jitter is added)
uint32_t nextTransmitTimeActual; // Actual time of the next satellite transmission (including jitter)

void setup()
{
  delay(1000);
  
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("SparkFun smôl ARTIC R2 Example"));
  Serial.println();

  Wire.begin();

  SPI.begin();

  // Uncomment the next line to enable the helpful debug messages
  //myARTIC.enableDebugging(); // Enable debug messages to Serial

  Serial.println(F("Starting the ARTIC R2..."));
  Serial.println();

  // Begin the ARTIC: enable power and upload firmware or boot from flash
  if (myARTIC.beginSmol(CS_Pin, ARTIC_PWR_EN_Pin) == false) // Default to using Wire to communicate with the PCA9536 I2C-GPIO chip on the smôl ARTIC R2
  {
    Serial.println("ARTIC R2 not detected. Please check the smôl stack-up and flexible circuits. Freezing...");
    while (1)
      ; // Do nothing more
  }

  // From v1.1.0 of the ARTIC R2 library: we were instructed by Kineis to ensure the Platform ID was written into each module
  // and not stored in a configuration file accessible to standard users. To comply with this, SparkFun
  // ARTIC R2 boards are now shipped with the Platform ID programmed into PMEM. Customers who have
  // earlier versions of the board will need to use version 1.0.9 of the library.
  uint32_t platformID = myARTIC.readPlatformID();
  if (platformID == 0)
  {
    Serial.println(F("You appear to have an early version of the SparkFun board."));
    Serial.println(F("Please use the Library Manager to select version 1.0.9 of this library."));
    Serial.println(F("Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.print(F("Your Platform ID is: 0x"));
    Serial.println(platformID, HEX);
  }

  // Define the time of the first transmit
  nextTransmitTime = repetitionPeriod;
  nextTransmitTimeActual = nextTransmitTime - tcxoWarmupTime; // Start the transmit early to compensate for the TCXO warmup time
}

void loop()
{
  // loop is one large switch/case that controls the sequencing of the code
  switch (loop_step) {

    // ************************************************************************************************
    // Configure the ARTIC
    case configure_ARTIC:
    {
      // Set the TCXO voltage to 1.8V and autoDisable to 1
      if (myARTIC.setTCXOControl(1.8, true) == false)
      {
        Serial.println("setTCXOControl failed. Freezing...");
        while (1)
          ; // Do nothing more
      }

      // Set the TCXO warm-up time
      if (myARTIC.setTCXOWarmupTime(tcxoWarmupTime) == false)
      {
        Serial.println("setTCXOWarmupTime failed. Freezing...");
        while (1)
          ; // Do nothing more
      }

      // Set the satellite detection timeout to 60 seconds
      if (myARTIC.setSatelliteDetectionTimeout(60) == false)
      {
        Serial.println("setSatelliteDetectionTimeout failed. Freezing...");
        while (1)
          ; // Do nothing more
      }

      // Set the TX mode to ARGOS 4 VLD
      ARTIC_R2_MCU_Command_Result result = myARTIC.sendConfigurationCommand(CONFIG_CMD_SET_ARGOS_4_PTT_VLD_TX_MODE);
      myARTIC.printCommandResult(result); // Pretty-print the command result to Serial
      if (result != ARTIC_R2_MCU_COMMAND_ACCEPTED)
      {
        Serial.println("sendConfigurationCommand failed. Freezing...");
        while (1)
          ; // Do nothing more
      }

      // Read and print the ARGOS configuration
      ARGOS_Configuration_Register configuration;
      myARTIC.readARGOSconfiguration(&configuration);
      myARTIC.printARGOSconfiguration(configuration);

      // Set the ARGOS 4 TX frequency to 401.630 MHz
      // From A4-SS-TER-SP-0079-CNES:
      // The transmission frequency for PTT-VLD-A4 platforms shall be set between 399.91 MHz to 401.68 MHz.
      // Due to frequency regulations, the frequency ranges [400.05 MHz to 401.0 MHz] and [401.2 MHz to 401.3 MHz] are forbidden for VLD-A4 transmissions.
      if (myARTIC.setARGOS4TxFrequency(401.630) == false)
      {
        Serial.println("setARGOS4TxFrequency failed. Freezing...");
        while (1)
          ; // Do nothing more
      }

      // Print the TX frequency
      float tx4freq = myARTIC.getARGOS4TxFrequency();
      Serial.print(F("The ARGOS 4 TX Frequency is "));
      Serial.print(tx4freq, 3);
      Serial.println(F(" MHz."));

      // Uncomment the next line if you want to attenuate the transmit power by 8dB
      //myARTIC.attenuateTXgain(true);

      // Configure the Tx payload for ARGOS 4 VLD (A4-SS-TER-SP-0079-CNES)
      // If the Platform ID has also been set to 0x01234567, the complete over-air data stream, including sync pattern and length, should be:
      // 0xAC5353DC651CECA2F6E328E76517B719473B28BD followed by 0b1
      if (myARTIC.setPayloadARGOS4VLDLong(0x01234567, 0x01234567) == false)
      {
        Serial.println(F("setPayloadARGOS4VLDLatLon failed!"));
        Serial.println();
        // Read the payload back again and print it
        myARTIC.readTxPayload();
        myARTIC.printTxPayload();
        Serial.println();
        Serial.println(F("Freezing..."));
        while (1)
          ; // Do nothing more
      }

/*
        // Read the payload back again and print it
        myARTIC.readTxPayload();
        myARTIC.printTxPayload();
        Serial.println();
*/

      loop_step = ARTIC_TX; // Move on
    }
    break;

    // ************************************************************************************************
    // Start the ARTIC in Transmit One Package And Go Idle mode
    case ARTIC_TX:
    {
      // Wait for the next repetition period
      while (nextTransmitTimeActual > millis())
      {
        if ((millis() % 1000) < 50) // Print how long it is until the next transmit
        {
          Serial.print(F("The next transmit will take place in "));
          unsigned long remaining = (nextTransmitTimeActual - millis()) / 1000;
          Serial.print(remaining);
          Serial.println(F(" seconds"));
        }
        delay(50);
      }
      nextTransmitTime += repetitionPeriod;
      nextTransmitTimeActual = nextTransmitTime + random((-0.1 * repetitionPeriod), (0.1 * repetitionPeriod));
      nextTransmitTimeActual -= tcxoWarmupTime; // Start the transmit early to compensate for the TCXO warmup time

      // Tell the ARTIC to do its thing!
      ARTIC_R2_MCU_Command_Result result = myARTIC.sendMCUinstruction(INST_TRANSMIT_ONE_PACKAGE_AND_GO_IDLE);
      if (result != ARTIC_R2_MCU_COMMAND_ACCEPTED)
      {
        Serial.println("sendMCUinstruction(INST_TRANSMIT_ONE_PACKAGE_AND_GO_IDLE) failed!");
        Serial.println();
        ARTIC_R2_Firmware_Status status;
        myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
        Serial.println(F("ARTIC R2 Firmware Status:"));
        myARTIC.printFirmwareStatus(status); // Pretty-print the firmware status to Serial
        Serial.println();
        Serial.println(F("ARTIC_R2_MCU_Command_Result:"));
        myARTIC.printCommandResult(result); // Pretty-print the command result to Serial
        Serial.println();
        Serial.println("Freezing...");
        while (1)
          ; // Do nothing more
      }

      loop_step = wait_for_ARTIC_TX; // Move on
    }
    break;

    // ************************************************************************************************
    // Start the ARTIC in Transmit One Package And Go Idle mode
    case wait_for_ARTIC_TX:
    {
      delay(1000); // Check the status every second

      // Read and print the ARTIC R2 status register
      ARTIC_R2_Firmware_Status status;
      myARTIC.readStatusRegister(&status); // Read the ARTIC R2 status register
      Serial.println(F("ARTIC R2 Firmware Status:"));
      myARTIC.printFirmwareStatus(status); // Pretty-print the firmware status to Serial

      if (status.STATUS_REGISTER_BITS.DSP2MCU_INT1) // Check the interrupt 1 flag. This will go high when TX is finished
      {
        Serial.println(F("INT1 pin is high. TX is finished (or MCU is in IDLE_STATE)!"));
      }

      if (status.STATUS_REGISTER_BITS.DSP2MCU_INT2) // Check the interrupt 2 flag. This will go high when if the message was invalid
      {
        Serial.println(F("INT2 pin is high. TX message was invalid! (Something really bad must have happened...)"));
      }

      Serial.println();

      // Read and print the instruction progress
      ARTIC_R2_MCU_Instruction_Progress progress;
      // checkMCUinstructionProgress will return true if the instruction is complete
      boolean instructionComplete = myARTIC.checkMCUinstructionProgress(&progress); // Check the instruction progress
      Serial.println(F("ARTIC R2 instruction progress:"));
      myARTIC.printInstructionProgress(progress); // Pretty-print the progress to Serial

      Serial.println();

      if (instructionComplete)
      {
        Serial.println(F("Transmission is complete!"));
        Serial.println();

        Serial.println(F("Clearing INT1."));
        Serial.println();

        // Clear INT1
        if (myARTIC.clearInterrupts(1) == false)
        {
          Serial.println("clearInterrupts failed! Freezing...");
          while (1)
            ; // Do nothing more
        }

        loop_step = ARTIC_TX; // Do over...
      }
    }
    break;
  }
}