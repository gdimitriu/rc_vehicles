/*
 * Bluetooth controlled vehicle.
 * Copyright 2022 Gabriel Dimitriu
 *
 * This is part of rc_vehicles project.
 * Bluetooth controlled vechicle with 2 engines.

 * rc_vehicles is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * rc_vehicles is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with rc_vehicles; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 */
#include <NeoSWSerial.h>
#include <PinChangeInt.h>

#define RxD 2
#define TxD 3

#define LEFT_MOTOR_PIN1 5
#define LEFT_MOTOR_PIN2 6
#define RIGHT_MOTOR_PIN1 10
#define RIGHT_MOTOR_PIN2 11

//#define SERIAL_DEBUG_MODE true
#define BLE_BUFFER 40
#define ABSOLUTE_MAX_POWER 255
int maxPower = ABSOLUTE_MAX_POWER;
int currentPower = maxPower;
int minPower = 100;
//for communication
bool isValidInput;
char inData[20]; // Allocate some space for the string
char inChar; // Where to store the character read
byte index = 0; // Index into array; where to store the character
boolean cleanupBT;

NeoSWSerial BTSerial(RxD, TxD);

void neoSSerial1ISR() {
    NeoSWSerial::rxISR(*portInputRegister(digitalPinToPort(RxD)));
}

void setup() {
#ifdef SERIAL_DEBUG_MODE  
  Serial.begin(38400);
#endif  
  BTSerial.begin(38400);  
  pinMode(LEFT_MOTOR_PIN1, OUTPUT);
  pinMode(LEFT_MOTOR_PIN2, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
  pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
  attachPinChangeInterrupt(RxD, neoSSerial1ISR, CHANGE);
  cleanupBT = false;
#ifdef SERIAL_DEBUG_MODE
  Serial.println("Starting...");
#endif  
}

/*
* Move the platform in predefined directions.
*/
void go(int speedLeft, int speedRight) {
  if (speedLeft == 0 && speedRight == 0 ) {
    digitalWrite(LEFT_MOTOR_PIN1,LOW);
    digitalWrite(LEFT_MOTOR_PIN2,LOW);
    digitalWrite(RIGHT_MOTOR_PIN1,LOW);
    digitalWrite(RIGHT_MOTOR_PIN2,LOW);
#ifdef SERIAL_DEBUG_MODE    
    Serial.println("All on zero");
#endif
    return;
  }
  if (speedLeft > 0) {
    analogWrite(LEFT_MOTOR_PIN1, speedLeft);
    digitalWrite(LEFT_MOTOR_PIN2,LOW);
#ifdef SERIAL_DEBUG_MODE
    Serial.print("Left "); Serial.print(speedLeft); Serial.print(" , "); Serial.println(0);
#endif
  } 
  else {
    digitalWrite(LEFT_MOTOR_PIN1,LOW);
    analogWrite(LEFT_MOTOR_PIN2, -speedLeft);
#ifdef SERIAL_DEBUG_MODE
    Serial.print("Left "); Serial.print(0); Serial.print(" , "); Serial.println(-speedLeft);
#endif
  }
 
  if (speedRight > 0) {
    analogWrite(RIGHT_MOTOR_PIN1, speedRight);
    digitalWrite(RIGHT_MOTOR_PIN2,LOW);
#ifdef SERIAL_DEBUG_MODE
    Serial.print("Right "); Serial.print(speedRight); Serial.print(" , "); Serial.println(0);
#endif
  }else {
    digitalWrite(RIGHT_MOTOR_PIN1,LOW);
    analogWrite(RIGHT_MOTOR_PIN2, -speedRight);
#ifdef SERIAL_DEBUG_MODE
    Serial.print("Right "); Serial.print(0); Serial.print(" , "); Serial.println(-speedRight);
#endif
  }
}
void breakEngines() {
    digitalWrite(LEFT_MOTOR_PIN1,HIGH);
    digitalWrite(LEFT_MOTOR_PIN2,HIGH);
    digitalWrite(RIGHT_MOTOR_PIN1,HIGH);
    digitalWrite(RIGHT_MOTOR_PIN2,HIGH);
}
boolean isValidNumber(char *data, int size)
{
  if (size == 0 ) return false;
   if(!(data[0] == '+' || data[0] == '-' || isDigit(data[0]))) return false;

   for(byte i=1;i<size;i++)
   {
       if(!(isDigit(data[i]) || data[i] == '.')) return false;
   }
   return true;
}

void makeCleanup() {
  for (index = 0; index < 20; index++)
  {
    inData[index] = '\0';
  }
  index = 0;
  inChar ='0';
}
void makeMove() {
  char buffer[BLE_BUFFER];
  for (int i = 0; i < BLE_BUFFER; i++) {
    buffer[i] = '\0';
  }
  if (index > 0) {
     inData[index-1] = '\0';
  }
  if (strlen(inData) == 1) {
    if (inData[0] == 'I') {
      sprintf(buffer,"unsupported\r\n");
      BTSerial.print(buffer);
      BTSerial.flush();
    } else if (inData[0] == 'V') {      
#ifdef SERIAL_DEBUG_MODE
      Serial.println(maxPower);
#endif
      sprintf(buffer,"%d\r\n",maxPower);
      BTSerial.print(buffer);
      BTSerial.flush();
    } else if (inData[0] =='v') {
#ifdef SERIAL_DEBUG_MODE
      Serial.println(minPower);
#endif      
      sprintf(buffer,"%d\r\n",minPower);
      BTSerial.print(buffer);
      BTSerial.flush();
    } else if (inData[0] =='c') {
#ifdef SERIAL_DEBUG_MODE
      Serial.println(currentPower);
#endif      
      sprintf(buffer,"%d\r\n",currentPower);
      BTSerial.print(buffer);
      BTSerial.flush();
    }  else if (inData[0] == 'd') {
#ifdef SERIAL_DEBUG_MODE
      Serial.println("unsupported");
#endif
      sprintf(buffer,"%d\r\n",0);
      BTSerial.print(buffer);
      BTSerial.flush();
    } else if (inData[0] == 's') {
#ifdef SERIAL_DEBUG_MODE
      Serial.println("unsupported");
#endif            
      sprintf(buffer,"%d\r\n",0);
      BTSerial.print(buffer);
      BTSerial.flush();
    } else if (inData[0] == 'b') {
      breakEngines();
      isValidInput = false;
      makeCleanup();
      return false;
    } else {
#ifdef SERIAL_DEBUG_MODE
      Serial.println("unsupported");
#endif
      sprintf(buffer,"%d\r\n",0);
      BTSerial.print(buffer);
      BTSerial.flush();
      isValidInput = false;
    }
  } else {
      if (inData[0] == 'V') {
        sprintf(buffer,"OK\r\n");
        BTSerial.print(buffer);
        BTSerial.flush();
        //remove V from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        if (!isValidNumber(inData, index - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(inData) > ABSOLUTE_MAX_POWER || atol(inData) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        Serial.print("MaxPower=");Serial.println(inData);
#endif        
        maxPower = atol(inData);
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (inData[0] == 'v') {
        sprintf(buffer,"OK\r\n");
        BTSerial.print(buffer);
        BTSerial.flush();
        //remove v from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        if (!isValidNumber(inData, index - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(inData) > maxPower || atol(inData) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        Serial.print("MinPower=");Serial.println(inData);
#endif                
        minPower = atol(inData);
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (inData[0] == 'c') {
        sprintf(buffer,"OK\r\n");
        BTSerial.print(buffer);
        BTSerial.flush();
        //remove c from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        if (!isValidNumber(inData, index - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(inData) > maxPower || atol(inData) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        Serial.print("CurrentPower=");Serial.println(inData);
#endif                
        currentPower = atol(inData);
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (inData[0] == 'd') {
        sprintf(buffer,"OK\r\n");
        BTSerial.print(buffer);
        BTSerial.flush();
        //this does not support low power distance
#ifdef SERIAL_DEBUG_MODE
      Serial.println("unsupported");
#endif        
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (inData[0] == 's') {
        sprintf(buffer,"OK\r\n");
        BTSerial.print(buffer);
        BTSerial.flush();
        //this does not support stop distance
#ifdef SERIAL_DEBUG_MODE
      Serial.println("unsupported");
#endif        
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (inData[0] == 'M') {
        //remove M from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        inData[strlen(inData)] = '\0';
        int position;
        for (uint8_t i = 0; i < strlen(inData); i++) {
          if (inData[i] == ',') {
            position = i;
            break;
          }
        }
        char buf[10];
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        for (int i = 0 ; i < position; i++) {
          buf[i] = inData[i];
        }
        int moveData = atoi(buf);
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        int idx = 0;
        for (int i = position + 1; i < strlen(inData); i++) {
          buf[idx] = inData[i];
          idx++;
        }
        int rotateData = atoi(buf);
#ifdef SERIAL_DEBUG_MODE   
        Serial.println(inData);
        Serial.print(moveData);Serial.print(":");Serial.println(rotateData);
#endif        
        if (moveData == 0 && rotateData == 0) {
          go(0,0);
        } else if (rotateData == 0) {
          if (moveData < 0) {
            go(-currentPower,-currentPower);
          } else {
            go(currentPower, currentPower);
          }
        } else {
          if (rotateData < 0) {
            go(-currentPower,currentPower);
          } else {
            go(currentPower, -currentPower);
          }
        }
        makeCleanup();
        isValidInput = true;
        return true;
      } else {
        sprintf(buffer,"%d\r\n",0);
        BTSerial.print(buffer);
        BTSerial.flush();
        makeCleanup();
        isValidInput = false;
        return false;
      }
    }
    makeCleanup();
    return true;
}
void loop()
{
  while(BTSerial.available() > 0) // Don't read unless there you know there is data
  {
     if(index < 19) // One less than the size of the array
     {
        inChar = BTSerial.read(); // Read a character
        if (inChar=='\r' || inChar=='\n') {
          continue;
        }        
        inData[index++] = inChar; // Store it
        inData[index] = '\0'; // Null terminate the string
        if (inChar == '#') {
          break;
        }
     } else {
        makeCleanup();
        cleanupBT = true;
     }
 }
 if (index >= 1) {
  if (inData[index - 1] == '#') {
#ifdef SERIAL_DEBUG_MODE
    Serial.print(inData);
#endif
    makeMove();
  } else if (cleanupBT) {
    makeCleanup();
    cleanupBT = false;
  } else {
    delay(10);
  }
 }
}
