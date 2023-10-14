#include "Particle.h"
  
// v1.0.0
const char* firmware_version = "1.0.0";

// Pins
int pin_twenty_five = D0;
int pin_fifty = D2;
int pin_seventy_five = D4;
int pin_hundered = D6;
int relay_motor = D7;
int relay_additional = D10;
 
// Stores Blynk State
int button_state = TRUE;

// Stores Current Level of Water Tank
int level = 0;

// Store if particle function is called
bool particle_fn_called = TRUE;

// Delay for publishing data
const uint32_t TIMER_INTERVAL_MS = 10000L;
uint32_t timer_last = 0;

// Blynk Auth
#define BLYNK_TEMPLATE_ID "YOUR-BLYNK-TEMPLATE-ID"
#define BLYNK_TEMPLATE_NAME "YOUR-BLYNK-PROJECT-NAME"
#define BLYNK_AUTH_TOKEN "YOUR-BLYNK-PRIVATE-TOKEN"

// Public Function 
int automateMotor(String value);

// Private Functions related to publishing data to blynk
void pubToParticleBlynk(); 
void pubTimer();

void setup(){
  Serial.begin(9600);
  waitFor(Serial.isConnected, 30000);
  delay(1000);

  // Level
  pinMode(pin_twenty_five, INPUT);
  pinMode(pin_fifty, INPUT);
  pinMode(pin_seventy_five, INPUT);
  pinMode(pin_hundered, INPUT);
  // Relay
  pinMode(relay_motor, OUTPUT);
  pinMode(relay_additional, OUTPUT);

  Serial.printlnf("Device OS v%s", System.version().c_str());
  Serial.printlnf("Free RAM %lu bytes", System.freeMemory());
  Serial.printlnf("Firmware version v%s", firmware_version);
 
  // register the Particle Cloud Function
  Particle.function("automation", automateMotor);

  Serial.println("Setup Complete");
} // setup()

void loop(){
  pubTimer();
  if(particle_fn_called == TRUE){
    particle_fn_called = FALSE;
    pubToParticleBlynk();
  }

  // change water level to 25 or reduce from 50
  if(digitalRead(pin_twenty_five)){
    if(level > 0) level -= 25;
    if(level == 0) level += 25;
  } else{
    level = 0;
  }

  // change water level to 50 or reduce from 75
  if(digitalRead(pin_fifty) && digitalRead(pin_twenty_five)){
    if(level > 25) level -= 25;
    if(level == 25) level += 25;
  }

  // change water level to 75 or reduce from 100
  if(digitalRead(pin_twenty_five) && digitalRead(pin_fifty) && digitalRead(pin_seventy_five)){
    if(level > 75) level -= 25;
    if(level == 50) level += 25;
  }

  // change water level to 100
  if(digitalRead(pin_fifty) && digitalRead(pin_twenty_five) && digitalRead(pin_seventy_five) && digitalRead(pin_hundered)){
    if(level == 75) level += 25;
  }

  // start relay if level is 0 or 25
  if(level <= 25) digitalWrite(relay_motor, HIGH);
  // stop if reached 100
  if(level == 50) digitalWrite(relay_motor, LOW);
 
  // start relay when button state changes
  if(button_state == TRUE || button_state == 1) digitalWrite(relay_additional, HIGH);
  else digitalWrite(relay_additional, LOW);
}

void pubToParticleBlynk() {
  if (Particle.connected()) {
    char data[90];
    snprintf(data, sizeof(data), "{\"t\":\"%s\",\"v0\":%d}", BLYNK_AUTH_TOKEN, level);
    Serial.printlnf("Sending to Blynk: '%s' with size of %u bytes", data, strlen(data));
    bool pub_result = Particle.publish("blynk_https_get", data, PRIVATE);
    if (pub_result) {
      timer_last = millis();
    } else {
      Serial.println("ERROR: Particle.publish()");
    }
  }
} 

void pubTimer() {
  if (timer_last > millis())  timer_last = millis();
  if ((millis() - timer_last) > TIMER_INTERVAL_MS && Particle.connected()) {
    pubToParticleBlynk();
    timer_last = millis();
  }
}

int automateMotor(String value){
  if(value == "on" || value == "1"){
    particle_fn_called = TRUE;
    button_state = TRUE;
    return 1;
  } else if(value == "off" || value == "0"){
    particle_fn_called = TRUE;
    button_state = FALSE;
    return 0;
  }else{
    Serial.print("Unexpected on_off value of "); Serial.println(value);
  }
  return -1;
}