#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include "nRF24L01.h"

#define SCK_PIN 5
#define MISO_PIN 19
#define MOSI_PIN 27
#define CS_PIN 23

#define CE_PIN 2

SPIClass spi(VSPI);

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CS_PIN);

// Let these addresses be used for the pair
uint8_t address[][6] = {"1Node", "2Node"};
// It is very helpful to think of an address as a path instead of as
// an identifying device destination

// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
// 0 uses address[0] to transmit, 1 uses address[1] to transmit
bool radioNumber = 1;

// Used to control whether this node is sending or receiving
// true = TX role, false = RX role
bool role = true;

// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float payload = 0.0;

/* CONFIGURAÇÕES PARA O USO DO LORA E DO DISPLAY */
#include "heltec.h"
#include "images.h"

#define BAND 433E6
#define CS_LORAPIN 18

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet;

void logo() {
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

void setupDisplay() {
  // WIFI Kit series V1 not support Vext control

  /* DisplayEnable Enable */
  /* Heltec.LoRa Disable */
  /* Serial Enable */
  /* PABOOST Enable */
  /* long BAND */
  Heltec.begin(true, true, true, true, BAND);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  
  logo();
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->display();
  delay(1000);
}

void updateDisplay() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Heltec.display->drawString(0, 0, "Sending packet: ");
  Heltec.display->drawString(90, 0, String(counter));
  Heltec.display->display();
}

void sendPacket() {
  Serial.println("Enviando pacotes...");

  // Send packet
  LoRa.beginPacket();

  /*
   * LoRa.setTxPower(txPower, RFOUT_pin);
   * txPower -- 0 ~ 20
   * RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
   *   - RF_PACONFIG_PASELECT_PABOOST
   *        LoRa single output via PABOOST, maximum output 20dBm
   *   - RF_PACONFIG_PASELECT_RFO
   *        LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
  */
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();
}

void setup() {
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  delay(100);
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    // hold in infinite loop
    while (1) {}
  }

  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));

  // To set the radioNumber via the Serial monitor on startup
  Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  while (!Serial.available()) {
    // wait for user input
  }
  char input = Serial.parseInt();
  radioNumber = input == 1;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  // role variable is hardcoded to RX behavior, inform the user of this
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  delay(100);
  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  // RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  // float datatype occupies 4 bytes
  radio.setPayloadSize(sizeof(payload));

  // set the TX address of the RX node into the TX pipe
  // always uses pipe 0
  radio.openWritingPipe(address[radioNumber]);     

  // set the RX address of the TX node into a RX pipe
  // using pipe 1
  radio.openReadingPipe(1, address[!radioNumber]); 

  // additional setup specific to the node's role
  if (role) {
    // put radio in TX mode
    radio.stopListening();  
  } else {
    // put radio in RX mode
    radio.startListening(); 
  }

  setupDisplay();
}

void loop() {

  digitalWrite(CS_PIN, LOW);
  if (role) {
    // This device is a TX node

    delay(100);
    unsigned long start_timer = micros();                    // start the timer
    bool report = radio.write(&payload, sizeof(float));      // transmit & save the report
    unsigned long end_timer = micros();                      // end the timer

    if (report) {
      Serial.print(F("Transmission successful! "));          // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);                 // print the timer result
      Serial.print(F(" us. Sent: "));
      Serial.println(payload);                               // print payload sent
      payload += 0.01;                                       // increment float payload
    } else {
      Serial.println(F("Transmission failed or timed out")); // payload was not delivered
    }

    // to make this example readable in the serial monitor
    // slow transmissions down by 1 second
    delay(1000);

  } else {
    // This device is a RX node

    delay(100);
    uint8_t pipe;
    if (radio.available(&pipe)) {             // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
      radio.read(&payload, bytes);            // fetch payload from FIFO
      Serial.print(F("Received "));
      Serial.print(bytes);                    // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      Serial.print(pipe);                     // print the pipe number
      Serial.print(F(": "));
      Serial.println(payload);                // print the payload's value
    }
  }

  if (Serial.available()) {
    // change the role via the serial monitor

    char c = toupper(Serial.read());
    if (c == 'T' && !role) {
      // Become the TX node

      role = true;
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      radio.stopListening();

    } else if (c == 'R' && role) {
      // Become the RX node

      role = false;
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      radio.startListening();
    }
  }
  digitalWrite(CS_PIN, HIGH);
  
  
  digitalWrite(CS_LORAPIN, LOW);
  updateDisplay();
  sendPacket();
  digitalWrite(CS_LORAPIN, HIGH);
}
