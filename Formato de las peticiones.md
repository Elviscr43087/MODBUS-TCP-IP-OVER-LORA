# Formato de las peticiones

## El envio del mensaje se realiza con la siguiente estructura:

### ID (1 BYTE): Dirección Modbus del dispositivo.

### MODO (1 BYTE): Tipo de operación.

- 0 = Lectura

- 1 = Escritura

### TIPO (1 BYTE): Tipo de datos Modbus.

- 0 = Coils

- 1 = Holding Registers

### DESDE (2 BYTES): Dirección inicial de la petición Modbus.

### CANTIDAD (2 BYTES): Número de registros a leer o escribir.

### VALOR (Opcional, solo para escritura): Datos a enviar.

- Hasta 30 registros.

- 2 BYTES por cada registro.

### CRC (2 BYTES): Código de verificación para la integridad del mensaje.

### (69 BYTES MAXIMO)
