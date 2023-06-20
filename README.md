# Comunicacion IoT mediante peticiones HTTP

Comunicacion bidireccional entre un dispositivo IoT (modulo WiFi) y un servidor (Raspberry Pi) mediante peticiones HTTP.

El modulo WiFi es un ESP8266 NodeMcu configurado como WebServer, conectado a un sensor DHT11 y a un LED mediante red local.

El cliente (Raspberry Pi) realiza peticiones HTTP GET al servicio (ESP8266) para obtener la informacion del sensor de temperatura. Y mediante HTTP POST prende y apaga el LED.

El cliente esta hecho en Vue.js, y se trata de una pagina simple que hace peticiones mediante Axios al modulo de red en un intervalo de 1 segundo, y actualiza la informacion de acuerdo a lo que recibe.