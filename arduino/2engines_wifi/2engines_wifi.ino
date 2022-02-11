#include <SoftwareSerial.h>

#define RxD 2
#define TxD 3

#define DEBUG_SERIAL 1
int wifiCommand;

int maxPower = 255;
int minPower = 100;
// for input data from wifi
#define BUFFER_WIFI 255
#define BUFFER_ESP_COMM 100
boolean cleanupWiFi;
bool isValidInput;
int connectionId = 0;
char inData[BUFFER_WIFI]; // Allocate some space for the string
char inChar; // Where to store the character read
byte index = 0; // Index into array; where to store the character

SoftwareSerial WiFiSerial(RxD, TxD);

boolean isValidNumber(char *data, int size)
{
  if (size == 0 ) return false;
   boolean isNum=false;
   if(!(data[0] == '+' || data[0] == '-' || isDigit(data[0]))) return false;

   for(byte i=1;i<size;i++)
   {
       if(!(isDigit(data[i]) || data[i] == '.')) return false;
   }
   return true;
}

boolean makeCleanup() {
  for (index = 0; index < BUFFER_WIFI; index++)
  {
    inData[index] = '\0';
  }
  index = 0;
  inChar ='0';
  WiFiSerial.flush();
}

void justConsumeData() {
  char buffer[BUFFER_ESP_COMM];
  int localIndex = 0;
  while(WiFiSerial.available() > 0) // Don't read unless there you know there is data
  {
    if(localIndex < BUFFER_ESP_COMM) // One less than the size of the array
     {
        inChar = WiFiSerial.read(); // Read a character
        buffer[localIndex] = inChar; // Store it
        localIndex++; // Increment where to write next
        buffer[localIndex] = '\0'; // Null terminate the string
     } else {
#ifdef DEBUG_SERIAL
        Serial.println(buffer);
        for(int i = 0; i < BUFFER_ESP_COMM; i++)
          buffer[i] = '\0';
        localIndex  = 0;
#endif
     }
  }
#ifdef DEBUG_SERIAL  
  Serial.println(buffer);
  Serial.flush();
#endif
}

void initEsp01() {
  WiFiSerial.print("ATE0\r\n");
  WiFiSerial.flush();
  delay(200);
  justConsumeData();
  WiFiSerial.print("AT+CWMODE=3\r\n");
  WiFiSerial.flush();
  delay(200);
  justConsumeData();
  WiFiSerial.print("AT+CIPMUX=1\r\n");
  WiFiSerial.flush();
  delay(200);
  justConsumeData();
  WiFiSerial.print("AT+CIPSERVER=1,8080\r\n");
  WiFiSerial.flush();
  delay(200);
  justConsumeData();
}

void setup() {
    WiFiSerial.begin(115200);
#ifdef DEBUG_SERIAL
    Serial.begin(9600);
    Serial.println("Starting...");
#endif
    isValidInput = false;
    cleanupWiFi = false;
    maxPower=4000;
    initEsp01();
}
/*
 * parse the receiving data from esp01
 */
void parseEsp01() {
  int secondIndex = 0;
  int firstIndex = 0;
  char temp[100];
  int i,j;
  for(i = 0;i < index; i++) {
    if (inData[i] == ',') {
       if (firstIndex == 0)
          firstIndex = i;
       else {
          secondIndex = i;
          break;
       }
    }
  }
  j = 0;
  for (i = firstIndex + 1; i < secondIndex; i++, j++) {
    temp[j] = inData[i];
  }
  temp[j+1] = '\0';
  connectionId  = atoi(temp);
#ifdef DEBUG_SERIAL
  Serial.print("ConnectionId=");Serial.println(connectionId);
  Serial.println(inData);
#endif
  firstIndex = 0;
  for (i = 0; i < index; i++) {
    if (inData[i] == ':' || inData[i] == ';') {
      firstIndex = i;
    }
    if (inData[i] == '#') {
      secondIndex = i;
      break;
    }
  }  
  for (i = firstIndex+1, index = 0; i <= secondIndex; i++, index++) {
    inData[index] = inData[i];
  }
  inData[index] = '\0';
#ifdef DEBUG_SERIAL
  Serial.print("Command=");Serial.println(inData);
#endif
}
boolean makeMove() {
    parseEsp01();
    if (index > 0) {
      inData[index-1] = '\0';
    }
    if (strlen(inData) == 1) {
      
    } else {
      if (inData[0] == 'v') {
        //remove v from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        if (!isValidNumber(inData, index - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(inData) > 255 || atol(inData) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        maxPower = atol(inData);
        makeCleanup();
        isValidInput = true;
      } else if (inData[0] == 'd') {
        //remove d from command
        for (uint8_t i = 0 ; i < strlen(inData); i++) {
          inData[i]=inData[i+1];
        }
        if (!isValidNumber(inData, index - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
      } else {
        makeCleanup();
        isValidInput = false;
      }
    }
    makeCleanup();
    return true;
}
void loop() {
  index = 0;
  while(WiFiSerial.available() > 0) // Don't read unless there you know there is data
  {
     if(index < BUFFER_WIFI) // One less than the size of the array
     {
        inChar = WiFiSerial.read(); // Read a character
        inData[index] = inChar; // Store it
        index++; // Increment where to write next
        inData[index] = '\0'; // Null terminate the string
     } else {
        makeCleanup();
        cleanupWiFi = true;
    }
  }
  if (index > 0)
    makeMove();
  delay(5);
}
