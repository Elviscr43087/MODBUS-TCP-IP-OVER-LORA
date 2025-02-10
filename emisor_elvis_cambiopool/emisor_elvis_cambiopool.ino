#include <SPI.h>
#include <LoRa.h>
#include <ArduinoModbus.h>

// Configuración LoRa
#define LORA_SENDER_ID 1
#define LORA_RECEIVER_ID 2
#define LORA_FREQ 868E6
const int ledPinLoRa = 0;
const int ledPinLoRaRCV = 1;
#define DEVICE_ID 1  // ID del dispositivo Modbus

void setup() {
  Serial.begin(9600);
  pinMode(ledPinLoRa, OUTPUT);
  pinMode(ledPinLoRaRCV, OUTPUT);

  // Inicialización LoRa
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Error al iniciar LoRa!");
    while (1);
  }
  LoRa.setTxPower(20);  // Configurar potencia de transmisión
  Serial.println("LoRa iniciado.");

}

void loop() {
  // Enviar solicitud Modbus por LoRa
  uint8_t id = 1;           // ID del dispositivo Modbus
  uint8_t mode = 0;         // 0 = Lectura, 1 = Escritura
  uint8_t type = 1;         // 0 = Coils, 1 = Holding Registers
  uint16_t from = 5;        // Dirección inicial
  uint16_t quantity = 10;    // Cantidad de registros a leer
  uint16_t valores[30]={};//insertar valores a enviar , 
  //establecer los valores ue se van a escribir
  valores[0]=0;
  valores[1]=0;
  valores[2]=0;
  valores[3]=0;
  valores[4]=0;
  valores[5]=0;
  valores[6]=0;
  valores[7]=0;
  valores[8]=0;
  valores[9]=0;
  valores[10]=0;


  uint8_t requestBuffer[64];
  int index = 0;
  
  requestBuffer[index++] = id;
  requestBuffer[index++] = mode;
  requestBuffer[index++] = type;
  requestBuffer[index++] = lowByte(from);
  requestBuffer[index++] = highByte(from);
  requestBuffer[index++] = lowByte(quantity); 
  requestBuffer[index++] = highByte(quantity);
  if(mode==1){
  for (int i = 0; i < quantity; i++){
  requestBuffer[index++] = lowByte(valores[i]); 
  requestBuffer[index++] = highByte(valores[i]);
  }
  }

  uint16_t crc = calculateCRC(requestBuffer, index);
  requestBuffer[index++] = lowByte(crc);
  requestBuffer[index++] = highByte(crc);
  //se esta apunto de enviar la solicitud
  digitalWrite(ledPinLoRa, HIGH);
  delay(1000);
  digitalWrite(ledPinLoRa, LOW);

  // Enviar la petición por LoRa
  LoRa.beginPacket();
  LoRa.write(requestBuffer, index);
  LoRa.endPacket();
  Serial.println("Petición enviada por LoRa");
  //pasa a modo lectura
  
  LoRa.receive(); 
  // Esperar respuesta hasta 10 segundos sino vuelve a enviar la peticion
  unsigned long startTime = millis();
  bool receivedResponse = false;
  while (millis() - startTime < 2000) {  // Esperar hasta 2 segundo
  // Esperar y recibir respuesta por LoRa
  int packetSize = LoRa.parsePacket();
    if (LoRa.parsePacket()) {
      digitalWrite(ledPinLoRaRCV, HIGH);
      delay(100);
      digitalWrite(ledPinLoRaRCV, LOW);
    uint8_t responseBuffer[64];
    int packetSize = LoRa.readBytes(responseBuffer, sizeof(responseBuffer));
    
    if (packetSize < 9) {
      Serial.println("Respuesta demasiado pequeña");
      return;
    }
    
    uint8_t respId = responseBuffer[0];
    uint8_t respMode = responseBuffer[1];
    uint8_t respType = responseBuffer[2];
    uint16_t respFrom = (responseBuffer[3] << 8) | responseBuffer[4];
    uint16_t respQuantity = (responseBuffer[5] << 8) | responseBuffer[6];
    
    uint16_t receivedCRC = (responseBuffer[packetSize - 2] << 8) | responseBuffer[packetSize - 1];
    uint16_t calculatedCRC = calculateCRC(responseBuffer, packetSize - 2);
    
    if (receivedCRC != calculatedCRC) {
      Serial.println("CRC inválido, datos corruptos");
      return;
    }
    
    Serial.println("Respuesta recibida:");
    Serial.print("ID: "); Serial.println(respId);
    Serial.print("Modo: "); Serial.println(respMode);
    Serial.print("Tipo: "); Serial.println(respType);
    Serial.print("Desde: "); Serial.println(respFrom);
    Serial.print("Cantidad: "); Serial.println(respQuantity);

    int dataIndex = 7;
    if (respType == 1) {  // Holding Registers
      Serial.println("Valores recibidos:");
      for (int i = 0; i < respQuantity; i++) {
        uint16_t value = (responseBuffer[dataIndex] << 8) | responseBuffer[dataIndex + 1];
        dataIndex += 2;
        Serial.print("Registro "); Serial.print(respFrom + i);
        Serial.print(": "); Serial.println(value);
      }
    } else if (respType == 0) {  // Coils
      Serial.println("Estados de Coils:");
      for (int i = 0; i < respQuantity; i++) {
        bool coilState = responseBuffer[dataIndex++];
        Serial.print("Coil "); Serial.print(respFrom + i);
        Serial.print(": "); Serial.println(coilState);
      }
    }
  }
  }
  //*/
  delay(100);  // Esperar un poco antes de hacer otra solicitud
}


uint16_t calculateCRC(uint8_t *data, int length) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < length; pos++) {
    crc ^= (uint16_t)data[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

