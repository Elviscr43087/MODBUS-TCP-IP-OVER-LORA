#include <SPI.h>
#include <LoRa.h>
#include <ArduinoModbus.h>

// Configuración LoRa
#define LORA_SENDER_ID 1
#define LORA_RECEIVER_ID 2
#define LORA_FREQ 868E6
const int ledPinLoRa = 0;
const int ledPinLoRaRCV = 1;
const int ledPinLoRaRCV2 = 2;

#define DEVICE_ID 999  // ID del dispositivo Modbus

void setup() {
  Serial.begin(9600);
  pinMode(ledPinLoRa, OUTPUT);
  pinMode(ledPinLoRaRCV, OUTPUT);
  pinMode(ledPinLoRaRCV2, OUTPUT);

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
  valores[0]=1666;
  valores[1]=1555;
  valores[2]=1333;
  valores[3]=1222;
  valores[4]=1111;
  valores[5]=1111;
  valores[6]=1345;
  valores[7]=1254;
  valores[8]=1255;
  valores[9]=2121;
  valores[10]=1006;


  enviarPeticionLoRa(id, mode, type, from, quantity, valores,ledPinLoRa);
  recibirRespuestaLoRa();
    // Enviar solicitud Modbus por LoRa
  uint8_t id2 = 2;           // ID del dispositivo Modbus
  uint8_t mode2 = 1;         // 0 = Lectura, 1 = Escritura
  uint8_t type2 = 1;         // 0 = Coils, 1 = Holding Registers
  uint16_t from2 = 5;        // Dirección inicial
  uint16_t quantity2 = 10;    // Cantidad de registros a leer
  uint16_t valores2[30]={};//insertar valores a enviar , 
  //establecer los valores ue se van a escribir
  valores2[0]=666;
  valores2[1]=655;
  valores2[2]=333;
  valores2[3]=222;
  valores2[4]=111;
  valores2[5]=111;
  valores2[6]=345;
  valores2[7]=254;
  valores2[8]=255;
  valores2[9]=221;
  valores2[10]=106;
  enviarPeticionLoRa(id2, mode2, type2, from2, quantity2, valores2,ledPinLoRa);
  recibirRespuestaLoRa();

  //LECTURA

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



// Función para construir y enviar la petición por LoRa
void enviarPeticionLoRa(uint8_t id, uint8_t mode, uint8_t type, uint16_t from, uint16_t quantity, uint16_t *valores,int LED) {
  uint8_t requestBuffer[64];
  int index = 0;

  requestBuffer[index++] = id;
  requestBuffer[index++] = mode;
  requestBuffer[index++] = type;
  requestBuffer[index++] = lowByte(from);
  requestBuffer[index++] = highByte(from);
  requestBuffer[index++] = lowByte(quantity);
  requestBuffer[index++] = highByte(quantity);

  // Si es escritura, incluir los valores en el paquete
  if (mode == 1) {
    for (int i = 0; i < quantity; i++) {
      requestBuffer[index++] = lowByte(valores[i]);
      requestBuffer[index++] = highByte(valores[i]);
    }
  }

  // Calcular CRC
  uint16_t crc = calculateCRC(requestBuffer, index);
  requestBuffer[index++] = lowByte(crc);
  requestBuffer[index++] = highByte(crc);

  // Indicar que se está enviando la solicitud
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);

  // Enviar la petición por LoRa
  LoRa.beginPacket();
  LoRa.write(requestBuffer, index);
  LoRa.endPacket();
  Serial.println("Petición enviada por LoRa");
}

// Función para recibir y procesar la respuesta de LoRa
void recibirRespuestaLoRa() {
  LoRa.receive();
  unsigned long startTime = millis();
  
  while (millis() - startTime < 2000) {  // Esperar hasta 2 segundos
    int packetSize = LoRa.parsePacket();
    if (packetSize) {      
      uint8_t responseBuffer[64];
      int bytesRead = LoRa.readBytes(responseBuffer, sizeof(responseBuffer));
      
      if (bytesRead < 9) {
        Serial.println("Respuesta demasiado pequeña");
        return;
      }

      uint8_t respId = responseBuffer[0];
      uint8_t respMode = responseBuffer[1];
      uint8_t respType = responseBuffer[2];
      uint16_t respFrom = (responseBuffer[3]) | (responseBuffer[4]<< 8);
      uint16_t respQuantity = (responseBuffer[5]) | (responseBuffer[6]<< 8);

      uint16_t receivedCRC = (responseBuffer[bytesRead - 2] << 8) | responseBuffer[bytesRead - 1];
      uint16_t calculatedCRC = calculateCRC(responseBuffer, bytesRead - 2);

      if (receivedCRC != calculatedCRC) {
        Serial.println("CRC inválido, datos corruptos");
        return;
      }
      if (respId==1){
      digitalWrite(ledPinLoRaRCV, HIGH);
      delay(100);
      digitalWrite(ledPinLoRaRCV, LOW);
      }
      if (respId==2){
      digitalWrite(ledPinLoRaRCV2, HIGH);
      delay(100);
      digitalWrite(ledPinLoRaRCV2, LOW);
      }

      Serial.println("Respuesta recibida:");
      Serial.print("ID: "); Serial.println(respId);
      Serial.print("Modo: "); Serial.println(respMode);
      Serial.print("Tipo: "); Serial.println(respType);
      Serial.print("Desde: "); Serial.println(respFrom);
      Serial.print("Cantidad: "); Serial.println(respQuantity);
      if(respMode==0){
      int dataIndex=7;
      if (respType == 1) {  // Holding Registers
        Serial.println("Valores recibidos:");
        for (int i = 0; i < respQuantity; i++) {
          uint16_t value = (responseBuffer[dataIndex] ) | (responseBuffer[dataIndex+1]<< 8) ;
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
      }else if(respMode==1){
        Serial.println("reespuesta de escritura correccta x modbus");
      }
      
      return; // Salir tras recibir un paquete válido
    }
  }
  Serial.println("Tiempo de espera agotado, no se recibió respuesta.");
}
