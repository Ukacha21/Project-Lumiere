
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <NewPing.h>

String resultString = "--";


// bool buzzer_Loud = false;

bool light = false;



#define ultrasionic_trig 2 //1  // 
#define ultrasionic_echo 15 //2  // 
#define ultrasionic_max_distance 200  // 

#define buzzer 23//4
#define ldr 34 //analog input. Use GPIO34, 35, 32, or 33 — they are input-only ADC pins on many ESP32 boards.
#define interrupt_Button 21

// GPS donanımsal UART1 (RX=13, TX=12)


float currentReading = 0;
//access point stuff----------

const char *ssid = "Lumiere";
const char *password = "12345678";  // Optional, minimum 8 chars

WiFiServer server(80);
WiFiClient client;  // listen for incoming clients
bool connected = false;

String varyok = "var";
bool full = false;
String coord;
float latitude;
float longitude;

// GPS ayarları
// i0
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);// gpsSerial(2); // GPS donanımsal UART1 (RX=16, TX=17)

// Ultrasonic sensor setup
NewPing sonar(ultrasionic_trig, ultrasionic_echo, ultrasionic_max_distance);

void get_distance(){
  // Read ultrasonic sensor
  unsigned int distance = sonar.ping_cm();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  String coordinates = coord;

  if (distance > 0 && distance < 5) {
    // Object detected, trigger buzzer
    varyok = "var";
    // String coordinates = get_coordinates(varyok);
    
    
    if (full == false){
      full = true;
      resultString = "var | " + coordinates;
    }

    if (full == true){
      digitalWrite(buzzer, HIGH);
      delay(300);
      digitalWrite(buzzer, LOW);
      delay(300);
    }
    else{
      digitalWrite(buzzer, LOW);
    }


  } else{
    full = false;
    resultString = "yok";
  }
}

void get_coordinates(){
  // GPS verilerini oku
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }


  if (gps.location.isValid()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();

      String mapLink = "https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6);
      coord = "[coordinates](" + mapLink + ")";
      // String message = "🚨 *var!*\n[coordinates](" + mapLink + ")";

      // return coord;
    } else {
      // latitude = gps.location.lat();
      // longitude = gps.location.lng();


      // String mapLink = "https://maps.google.com/?q=" + String(latitude, 6) + "," + String(longitude, 6);
      // coord = "[coordinates](" + mapLink + ")";
      // coord = mapLink;
      Serial.println("Invalid GPS Info");
      coord = "[No GPS]";
      // coord = String(latitude, 6) + " | " + String(longitude, 6);
      }
 
}


void setup() {
    Serial.begin(9600);//115200
  
    pinMode(ldr, INPUT);
    pinMode(interrupt_Button, INPUT_PULLUP);
    pinMode(buzzer, OUTPUT);
    digitalWrite(buzzer, LOW);
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // GPS RX=16, TX=17


    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point IP address: ");
    Serial.println(IP);  // Should be 192.168.4.1 for local

    server.begin();
    
}

void post(String message){
  
  if (client && connected == true) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println(message);
  }
}




String get_rawDemo(float value) {
  // delay(400);//remove delay if necessary
  // clearDisplay();
  int tempint = (int)value;
  char buffer[10];
  dtostrf(tempint, 4, 2, buffer);
  String newtext = "" + String(buffer);
  
  // displayText(newtext, 2, 0,30); //post to http server
  // displayText(String(millis()), 2, 0,30);
  return newtext;
}

float get_Photoresitance() {
  float res = analogRead(ldr);
  // return get_rawDemo(res);
  return res;
}


void loop() {
    
    get_coordinates();
    // get_distance();
    
    // float temp = get_Photoresitance();
    int ldrValue = get_Photoresitance();
    resultString = String(ldrValue) + coord;
    Serial.println(resultString);
    Serial.println("\n");

    //---------------------

    if (ldrValue<3000 && light == false){
      digitalWrite(buzzer, HIGH);
      delay(300);
      digitalWrite(buzzer, LOW);
      delay(300);

      //send coordinates here, instead of analog value
    }
    else if (light== true || ldrValue >= 3000){ //else if (light == true || ldrValue >= 3000)
      digitalWrite(buzzer, LOW);
    }

    if (digitalRead(interrupt_Button) == LOW){
      // if (light==false){
      //   light=true;
      // }
      // else{
      //   light=false;
      // }

      light= !light;

      // digitalWrite(buzzer, LOW);
      
      Serial.print("light: ");
      Serial.print(light);
      Serial.print("\n");
      Serial.println("Button Pressed");
      while (digitalRead(interrupt_Button)==LOW){}
    }

    //-------------------------------------

  //--------
  // // Auto turn off beeping if light is enough
  // if (ldrValue >= 3000) {
  //   light = true;
  // }

  // // Beep condition
  // if (ldrValue < 3000 && light == false) {
  //   digitalWrite(buzzer, HIGH);
  //   delay(300);
  //   digitalWrite(buzzer, LOW);
  //   delay(300);
  //   // send coordinates here instead of analog value
  // } else {
  //   digitalWrite(buzzer, LOW);
  // }

  // // Button press logic (toggle `light`)
  // if (digitalRead(interrupt_Button) == LOW) {
  //   delay(50); // debounce delay
  //   if (digitalRead(interrupt_Button) == LOW) {
  //     light = !light;
  //     Serial.println("Button Pressed");
  //     Serial.print("light: ");
  //     Serial.println(light);
      
  //     // Wait for button to be released
  //     while (digitalRead(interrupt_Button) == LOW) {
  //       delay(10); // avoid busy wait
  //     }
  //   }
  // }
  // //----------------------


  
  client = server.available(); 
  //-------------------------------------------
  if (client) {

    connected = true;
    Serial.println("New Client Connected");

    String request = client.readStringUntil('\r');
    Serial.println("Request: " + request);
    client.flush();

    // post(get_rawDemo(currentReading));//for Demo, comment this out
    

    // int brightness = map(ldrValue, 0, 4095, 0, 100); // As percentage
    // Serial.println(resultString);
    resultString = resultString + coord;
    post(resultString);
    //send coordinates heres
    Serial.println(resultString);
    // Serial.println("\n");


    client.stop();  // disconnect after sending response
    Serial.println("Client Disconnected");
          
    
  }
  else{
    
    connected = false;
  }

  delay(30);
    

}
