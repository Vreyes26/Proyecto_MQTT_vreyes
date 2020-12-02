/*
    Host: ioticos.org

    Usuario: TR3oNPxXnCEzopw

    Contraseña: mPhIlrLrLkwAVnq

    Topico Raíz: vghyF5od2LPHke6

    TCP: 1883

    PARA CONFIGURAR NUEVA RED WIFI, PRECIONAR BOTON DENTRO DE
    LOS 10 SEGUNDOS INICIADO EL PROYECTO (PARPADEO DE LED)

    SI NO, INGRESA CONFIGURACION GUARDADA EN LA MEMORIA EEPROM

*/

//LIBRERIAS
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h>
#include <Servo.h>

//DEFINIR
#define USUARIO_MQTT "TR3oNPxXnCEzopw"
#define CONTRA_MQTT "mPhIlrLrLkwAVnq"
#define TOPIC_MQTT "vghyF5od2LPHke6"
#define NOMBRE_WIFI "ProyectoVreyes"
#define CLAVE_WIFI "2721"
#define TOPIC_LED "vghyF5od2LPHke6/led"
#define TOPIC_MOTOR "vghyF5od2LPHke6/motor"
#define TOPIC_SENSOR "vghyF5od2LPHke6/sensor"
#define TOPIC_MENSAJE "vghyF5od2LPHke6/mensaje"
#define TOPIC_MOV "vghyF5od2LPHke6/mov"

char ssid[50];
char pass[50];

Servo servoMotor;
const int pinboton = 5;---//D1
const int led = 16;-------//D0
const int servo = 4;------//D2
const int vibrador = 13;--//D7
int estado;
int value = 0;
String mensaje = "";
int n = 0;
char servidor[50] = "ioticos.org";
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);

//CONFIGURACION DE PAGINA EN STRING
String pagina = "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Configuracion WiFi Proyecto Vreyes</title>"
"<meta charset='UTF-8'>"
"</head>"
"<body style='background-color:black; color:white; text-align:center;'>"
"<h1>Configuracion Proyecto vreyes_01</h1>"
"</form>"
"<form action='guardar_config' method='get'>"
"NOMBRE WIFI:<br><br>"
"<input class='input1' name='ssid' type='text'><br>"
"CONTRASEÑA:<br><br>"
"<input class='input1' name='pass' type='password'><br><br>"
"<input class='boton' type='submit' value='GUARDAR'/><br><br>"
"</form>"
"<a href='escanear'><button class='boton'>ESCANEAR</button></a><br><br>";

String paginafin = "</body>"
"</html>";


//SE INICIA CONFIGURACION DE PAGINA
void config_pagina(){
  server.send(200, "text/html", pagina + mensaje + paginafin);
}


//CONFIGURACION WIFI DIRECTO
void config_direct(){
  delay(2000);
  WiFi.softAP(NOMBRE_WIFI);
  Serial.print("IP punto de acceso: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("ServicioWeb Iniciado...");

  server.on("/", config_pagina); //INICIO CONFIGURACION PAGINA
  server.on("/guardar_config", guardar_config); //GUARDAR EN EEPROM
  server.on("/escanear", escanear); //ESCANEAR REDES WIFI
  server.begin();

  while (true){
    server.handleClient();
  }
}


//GUARDAR DATOS WIFI
void guardar_config() {
  //reciben los datos de la web
  Serial.println(server.arg("ssid"));
  grabar(0,server.arg("ssid"));
  Serial.println(server.arg("pass"));
  grabar(50,server.arg("pass"));

  mensaje = "Configuracion Guardada... Reinicie NODEMCU";
  config_pagina();
}


//GUARDAR WIFI EN EEPROM
void grabar(int addr, String a) {
  int tamano = a.length(); 
  char inchar[50]; 
  a.toCharArray(inchar, tamano+1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr+i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr+i, 255);
  }
  EEPROM.commit();
}


//LEER DATOS EEPROM
String leer(int addr) {
   byte lectura;
   String strlectura;
   for (int i = addr; i < addr+50; i++) {
      lectura = EEPROM.read(i);
      if (lectura != 255) {
        strlectura += (char)lectura;
      }
   }
   return strlectura;
}

//ESCANEAR REDES WIFI
void escanear() {  
  int n = WiFi.scanNetworks(); //devuelve el número de redes encontradas
  Serial.println("escaneo terminado");
  if (n == 0) { //si no encuentra ninguna red
    Serial.println("no se encontraron redes");
    mensaje = "no se han encontraron redes";
  }  
  else
  {
    Serial.print(n);
    Serial.println(" redes encontradas");
    mensaje = "";
    for (int i = 0; i < n; ++i)
    {
      // agrega al STRING "mensaje" la información de las redes encontradas 
      mensaje = (mensaje) + "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") Ch: " + WiFi.channel(i) + " Enc: " + WiFi.encryptionType(i) + " </p>\r\n";
      delay(10);
    }
    Serial.println(mensaje);
    config_pagina();
  }
}


//CONFIGURACION WIFI
void config_wifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(4000);
  if(WiFi.status() == WL_CONNECTED){
    Serial.print("Conectado: ");
    Serial.println(WiFi.localIP());
  }else{
    Serial.println("Error Conexion");
  }
}



void callback(char* topic, byte* payload, unsigned int length) {
  String string;
  Serial.print("Message recibido [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println("");

  //DECLARACION DE VARIABLES PARA EL IF
  String topico = topic;
  String topicoMotor = TOPIC_MOTOR;
  String topicoLed = TOPIC_LED;
  String topicoSensor = TOPIC_SENSOR;
  String topicoMensaje = TOPIC_MENSAJE;
  String topicoMov = TOPIC_MOV;

  //TOPICO MOTOR
  if(topico == topicoMotor){
    //OBTENER MENSAJE COMPLETO
    for (int i = 0; i < length; i++) {
      string+=((char)payload[i]);
    }
    int resultado = string.toInt();
    Serial.println(resultado);
    servoMotor.write(resultado);
    delay(15);
  }

  //TOPICO LED
  if(topico == topicoLed){
    Serial.println("topico led");
    if ((char)payload[0] == '1') {
      digitalWrite(led, HIGH);
    }else if((char)payload[0] == '0'){
      digitalWrite(led, LOW);
    }
  }

  //TOPICO SENSOR
  if(topico == topicoSensor){
    Serial.println("topico sensor");
    if((char)payload[0] == '1'){
      int sensorValor = analogRead(A0);
      char buf[16]; // need a buffer for that
      sprintf(buf,"%d",sensorValor);
      const char* p = buf;
      client.publish(TOPIC_MENSAJE, p);
    }
  }

  //TOPICO MOVER SERVO
  if(topico == topicoMov){
    if((char)payload[0] == '5'){
      for(int i = 0; i<5; i++){
        servoMotor.write(0);
        delay(500);
        servoMotor.write(180);
        delay(500);
      }
    }else if((char)payload[0] == '2'){
      servoMotor.write(0);
    }else if((char)payload[0] == '3'){
      servoMotor.write(180);
    }
  }

  //MUESTRA MENSAJE GENERAL RECIBIDO
  for (int i = 0; i < length; i++) {
    string+=((char)payload[i]);
  }
  Serial.print("MENSAJE: ");
  Serial.println(string);
}

//RECONECTAR SERVIDOR MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");
    String clientId = "ESP8266Cliente-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(),"TR3oNPxXnCEzopw", "mPhIlrLrLkwAVnq")) {
      Serial.println("conectado");
      client.publish(TOPIC_MENSAJE, "reconectando");
      client.subscribe("vghyF5od2LPHke6/#");
    } else {
      Serial.print("fallido, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(pinboton, INPUT);
  pinMode(led, OUTPUT);
  servoMotor.attach(servo);
  client.setServer(servidor, 1883);
  client.setCallback(callback);
  
  //ENTRAR EN MODO CONFIGURACION
  for(int i = 1; i < 10; i++){
    estado = digitalRead(pinboton);
    Serial.print("esperando configuracion ");
    Serial.print(i);
    Serial.println("s");
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    //INGRESAR NUEVA RED WIFI
    if(estado == LOW){
      Serial.println("configuracion de red");
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      WiFi.disconnect();
      
      config_direct();
    }
    delay(500);
  }
  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);
  Serial.println("Configurando wifi");
  WiFi.softAPdisconnect(true);
  
  config_wifi();
}

void loop(){
   delay(1000);
   if(!WiFi.status() == WL_CONNECTED){
    Serial.println("error de conexion");
    config_wifi();
    delay(5000);
   }

   if(!client.connected()){
    Serial.println("reconectando");
    reconnect();
    delay(1000);
   }
   client.loop();
}
