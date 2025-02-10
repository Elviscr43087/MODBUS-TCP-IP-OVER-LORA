#include <AlPlc_Opta.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

// Server will listen on Modbus TCP standard port 502
EthernetServer ethServer(502); 

// Create a Modbus TCP server instance
ModbusTCPServer modbusTCPServer;

// Define the IP Address of the Modbus TCP server (Opta device)
IPAddress ip(192, 168, 1, 100);

// Define the pin for the LED
const int ledPin = LED_D0; // Use LED_D0 as the LED pin



// Tiempos base
#define PERIODIC_UPDATE_TIME  1000
#define DELAY_AFTER_SETUP     1000



// -----------------  Modbus TCP ----------------- //
void lecturamodbus() {
  if (modbusTCPServer.poll()) {  // Procesar solicitudes Modbus TCP
    // leer, holding registers
        // Escribir en 40 Coils
    for (uint16_t i = 0; i < 40; i++) {
        modbusTCPServer.holdingRegisterRead(i); //lee las 40  dir
    }
    for (uint16_t i = 0; i < 40; i++) {
        modbusTCPServer.coilRead(i); //lee las 40  dir
    }



  }
}
void escritura_primera_vez(){
    modbusTCPServer.holdingRegisterWrite(0x00,0);
    modbusTCPServer.holdingRegisterWrite(0x01,1);
    modbusTCPServer.holdingRegisterWrite(0x02,2);
    modbusTCPServer.holdingRegisterWrite(0x03,3);
    modbusTCPServer.holdingRegisterWrite(0x04,4);
    modbusTCPServer.holdingRegisterWrite(0x05,0);
    modbusTCPServer.holdingRegisterWrite(0x06,1);
    modbusTCPServer.holdingRegisterWrite(0x07,2);
    modbusTCPServer.holdingRegisterWrite(0x08,3);
    modbusTCPServer.holdingRegisterWrite(0x09,4);
    modbusTCPServer.holdingRegisterWrite(0x0A,0);
    modbusTCPServer.holdingRegisterWrite(0x0B,1);
    modbusTCPServer.holdingRegisterWrite(0x0C,2);
    modbusTCPServer.holdingRegisterWrite(0x0D,3);
    modbusTCPServer.holdingRegisterWrite(0x0E,4);
    modbusTCPServer.holdingRegisterWrite(0x0F,0);
    modbusTCPServer.holdingRegisterWrite(0x10,1);
    modbusTCPServer.holdingRegisterWrite(0x11,2);
    modbusTCPServer.holdingRegisterWrite(0x12,3);
    modbusTCPServer.holdingRegisterWrite(0x13,4);
    modbusTCPServer.holdingRegisterWrite(0x14,0);
    modbusTCPServer.holdingRegisterWrite(0x15,1);
    modbusTCPServer.holdingRegisterWrite(0x16,2);
    modbusTCPServer.holdingRegisterWrite(0x17,3);
    modbusTCPServer.holdingRegisterWrite(0x18,4);
    modbusTCPServer.holdingRegisterWrite(0x19,0);
    modbusTCPServer.holdingRegisterWrite(0x1A,1);
    modbusTCPServer.holdingRegisterWrite(0x1B,2);
    modbusTCPServer.holdingRegisterWrite(0x1C,3);
    modbusTCPServer.holdingRegisterWrite(0x1D,4);
    modbusTCPServer.holdingRegisterWrite(0x1E,0);
    modbusTCPServer.holdingRegisterWrite(0x1F,1);
    modbusTCPServer.holdingRegisterWrite(0x20,2);
    modbusTCPServer.holdingRegisterWrite(0x21,3);
    modbusTCPServer.holdingRegisterWrite(0x22,4);
    modbusTCPServer.holdingRegisterWrite(0x23,0);
    modbusTCPServer.holdingRegisterWrite(0x24,1);
    modbusTCPServer.holdingRegisterWrite(0x25,2);
    modbusTCPServer.holdingRegisterWrite(0x26,3);
    modbusTCPServer.holdingRegisterWrite(0x27,4);
    
    // Escribir en 40 Coils
    for (uint16_t i = 0; i < 40; i++) {
        modbusTCPServer.coilWrite(i, (i % 2 == 0)); // Alternar entre true y false
    }
}

// ----------------- INICIO ----------------- //

void setup() {
  Serial.begin(115200);
  delay(DELAY_AFTER_SETUP);

  // Inicializar Opta
  OptaController.begin();

  // Inicializar Ethernet
  Ethernet.begin(NULL, ip); 
  
  // Inicializar servidor Modbus TCP
  // Start the Modbus TCP server
  ethServer.begin();
  if (!modbusTCPServer.begin()) {
    Serial.println("- Failed to start Modbus TCP Server!");
    while (1);
  }

  // Configure the LED pin as an output
  pinMode(ledPin, OUTPUT);

  // Configurar registros y coils
  modbusTCPServer.configureHoldingRegisters(0x00, 200);
  modbusTCPServer.configureCoils(0x00, 50);
  escritura_primera_vez();
}

void loop() {
  delay(10);
  EthernetClient client = ethServer.available();
  if (client) {
    Serial.println("- Client connected!");
    // Accept and handle the client connection for Modbus communication
    modbusTCPServer.accept(client);
    // Update the LED state based on Modbus coil value
    while (client.connected()) {
      // Process Modbus requests
      modbusTCPServer.poll(); 
      lecturamodbus();
    }
    Serial.println("Client disconnected.");
  }

}