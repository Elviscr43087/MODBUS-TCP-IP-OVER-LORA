#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>
#include <LoRa.h>

// Configuración de Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server(192, 168, 1, 100);  // IP del equipo Modbus
EthernetClient ethClient;
ModbusTCPClient modbus(ethClient);

#define DEVICE_ID 1
#define MAX_VALORES 30

// Función para calcular CRC-16 (Modbus)
uint16_t calculateCRC(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void reconnectModbus() {
    if (!modbus.begin(server, 502)) {
        Serial.println("Error al reconectar Modbus");
    } else {
        Serial.println("Modbus reconectado");
    }
}



void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("No se encontró hardware Ethernet");
  }
  if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Cable Ethernet no conectado");
  } else {
      Serial.println("Conexión Ethernet establecida!");
  }

  reconnectModbus();

  Serial.println("Configurando LoRa...");
  if (!LoRa.begin(868E6)) {
    Serial.println("Error al iniciar LoRa!");
  }
  LoRa.setSpreadingFactor(7);  // Configurar parámetro SF (7-12)
  LoRa.receive();              // Poner en modo recepción
  //LoRa.setTxPower(20);
  Serial.println("LoRa listo!");
}

void loop() {
  if (LoRa.parsePacket()) {
    recepcion_paquetes();
  }
}

void enviarRespuestaLoRa(uint8_t id, uint8_t mode, uint8_t type, uint16_t from, uint16_t quantity, void* data) {
  Serial.println("Datos recibidos por lora");
  uint8_t responseBuffer[64];
  int index = 0;

  responseBuffer[index++] = id;
  responseBuffer[index++] = mode;
  responseBuffer[index++] = type;
  responseBuffer[index++] = lowByte(from);
  responseBuffer[index++] = highByte(from);
  responseBuffer[index++] = lowByte(quantity);
  responseBuffer[index++] = highByte(quantity);

  if (data) {
    if (type == 1) {  // Holding Registers
      uint16_t* registers = (uint16_t*)data;
      for (int i = 0; i < quantity; i++) {
        responseBuffer[index++] = lowByte(registers[i]);
        responseBuffer[index++] = highByte(registers[i]);
      }
    } else if (type == 0) {  // Coils
      bool* coils = (bool*)data;
      for (int i = 0; i < quantity; i++) {
        responseBuffer[index++] = coils[i];
      }
    }
  }

  uint16_t crc = calculateCRC(responseBuffer, index);
  responseBuffer[index++] = (uint8_t)(crc >> 8);
  responseBuffer[index++] = (uint8_t)crc;

  LoRa.beginPacket();
  LoRa.write(responseBuffer, index);
  LoRa.endPacket();
}
void Peticionmodbus(uint8_t id, uint8_t mode, uint8_t type, uint16_t from, uint16_t quantity, uint16_t *values){
    if (mode == 0) {  // Lectura
    Serial.println("Lectura de Modbus...");
    if (type == 1) {  // Holding Registers
      uint16_t registers[MAX_VALORES];
      if (modbus.requestFrom(1, HOLDING_REGISTERS, from, quantity)) {
        Serial.println("REGISTROS DE MODBUS(HOLDING REGISTERS) cantidad: "+ String(quantity));
        for (int i = 0; i < quantity; i++) {
          registers[i] = modbus.read();
          Serial.println("registro" + String(i) + ":  " + String(registers[i]));
        }
        enviarRespuestaLoRa(id, mode, type, from, quantity, registers);
      }
    } else if (type == 0) {  // Coils
      bool coils[MAX_VALORES];
      if (modbus.requestFrom(id, COILS, from, quantity)) {
        Serial.println("REGISTROS DE MODBUS(HOLDING REGISTERS) cantidad: "+ String(quantity));
        for (int i = 0; i < quantity; i++) {
          coils[i] = modbus.read();
          Serial.println("registro" + String(i) + ":  " + String(coils[i]));
        }
        enviarRespuestaLoRa(id, mode, type, from, quantity, coils);
      }
    }
  } else if (mode == 1) {  // Escritura
    Serial.println("Escritura de Modbus...");
    if (type == 1) {  // Holding Registers
      for (int i = 0; i < quantity; i++) {
        if (!modbus.holdingRegisterWrite(from + i, values[i])) {
          Serial.print("Error en escritura Modbus para registro ");
          Serial.println(from + i);
        }
      }
    } else if (type == 0) {  // Coils
      for (int i = 0; i < quantity; i++) {
        if (!modbus.coilWrite(from + i, values[i])) {
          Serial.print("Error en escritura Modbus para coil ");
          Serial.println(from + i);
        }
      }
    }
    enviarRespuestaLoRa(id, mode, type, from, quantity, nullptr);
  }
    LoRa.receive(); 
}

void recepcion_paquetes(){
  Serial.println("Datos recibidos por LoRa");
  // Leer el paquete recibido
  uint8_t buffer[64];
  int packetSize = LoRa.readBytes(buffer, sizeof(buffer));
  if (packetSize < 7) {  // Tamaño mínimo para un paquete válido
    Serial.println("Paquete demasiado pequeño");
    return;
  }

  // Extraer los datos del paquete
  uint8_t id = buffer[0];
  uint8_t mode = buffer[1];
  uint8_t type = buffer[2];
  uint16_t from = (buffer[3]) | (buffer[4] << 8);
  uint16_t quantity = (buffer[5]) | (buffer[6]<< 8);

  Serial.println("Datos recibidos por LoRa");
  Serial.println("ID: "+ String(id)+", Mode: "+String(mode)+", Type: "+String(type));
  Serial.println("From: "+ String(from)+", Quantity: "+String(quantity));

  uint16_t values[MAX_VALORES] = {0};
  int bufferIndex = 7;
  if (mode == 1 && packetSize >= (7 + quantity * 2 + 2)) {  // Escritura y tamaño suficiente
    for (int i = 0; i < quantity; i++) {
      values[i] = (buffer[bufferIndex + 1] << 8) | buffer[bufferIndex];
      bufferIndex += 2;
      Serial.println("valor"+String(i)+": "+String(values[i]));
    }
  }
  // Verificar el CRC recibido
  uint16_t receivedCRC = (buffer[bufferIndex+1] << 8) | buffer[bufferIndex];
  uint16_t calculatedCRC = calculateCRC(buffer, bufferIndex);
  if (calculatedCRC != receivedCRC) {
    Serial.println("CRC inválido, los datos están corruptos");
    return;
  }
  else{
  if(id==DEVICE_ID){
    reconnectModbus();
    Peticionmodbus(id, mode, type, from, quantity, values);
  }
  }
}
