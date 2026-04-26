# 📡 Habitacion Inteligente con ESP32 + RFID

##  Descripcion del proyecto

Este proyecto es una habitacion inteligente desarrollada con ESP32 y lector RFID RC522.  
Permite activar diferentes ambientes (modos) mediante tarjetas RFID, controlando luces LED, buzzer y una interfaz web en tiempo real.

Ademas, el sistema registra cada evento en Google Sheets, incluyendo el UID de la tarjeta, el modo activado, el estado (Encendido/Apagado) y la fecha.

---

##  Tecnologias utilizadas

- ESP32 (Arduino Framework)
- RFID RC522
- WiFi
- WebServer ESP32
- HTTPClient
- Google Apps Script (Google Sheets)
- PWM (LEDC ESP32 3.x)

---

##  Modos de funcionamiento

### Modo Sueno
- Luces suaves verde/amarillo
- Sonido relajante tipo ambiente
- Enfoque en descanso

###  Modo Fiesta
- Luces RGB aleatorias
- Sonido tipo techno
- Ambiente dinamico

###  Modo Naturaleza
- LED verde constante
- Sonidos de bosque y lluvia
- Ambiente relajante

###  Modo Alarma
- LED rojo intermitente
- Sonido de alerta fuerte
- Activacion de emergencia

---

## Funcionamiento RFID (Toggle)

- Primera lectura de tarjeta → Activa el modo
- Segunda lectura de la misma tarjeta → Desactiva el modo

Esto permite usar cada tarjeta como un interruptor inteligente.

---

##  Interfaz web

El ESP32 genera una pagina web accesible desde su IP local.

### Funciones:
- Activar modos manualmente
- Control de volumen
- Control de brillo
- Visualizacion del modo activo
- Indicador de estado (Encendido/Apagado)

---

## Base de datos (Google Sheets)

Cada evento se guarda automaticamente:

| UID | MODO | FECHA | ESTADO |
|-----|------|-------|--------|
| 49aa90 | Fiesta | 23/04/2026 22:23 | Encendido |

---

##  API Google Apps Script

El ESP32 envia datos mediante peticiones HTTP GET:

---

##  Hardware utilizado

- ESP32
- Lector RFID RC522
- LEDs (rojo, verde, amarillo)
- Buzzer
- Resistencias
- Fuente de alimentacion

---

## Caracteristicas principales

- Sistema de ambientes inteligentes
- Control por RFID tipo toggle
- Interfaz web estilo app moderna
- Registro en la nube (Google Sheets)
- Control de brillo y volumen en tiempo real
- Modos interactivos de ambiente

---

##  Autores

Miriam Becerril y Saúl Mejía