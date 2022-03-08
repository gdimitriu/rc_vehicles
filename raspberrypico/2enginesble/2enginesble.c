/*
 * 2enginesble.c
 *
 *  Created on: Mars 08, 2022
 *      Author: Gabriel Dimitriu 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <string.h>

#define SERIAL_DEBUG_MODE true
#define BLE_BUFFER 256
#define BLE_BUFFER_SEND 512
#define ABSOLUTE_MAX_POWER 255
int bufferIndex = 0;
int maxPower = ABSOLUTE_MAX_POWER;
int currentPower = ABSOLUTE_MAX_POWER;
int minPower = 100;
char inChar = '\0';
char bufferReceive[BLE_BUFFER];
char bufferSend[BLE_BUFFER_SEND];
bool isValidInput = false;

bool isValidNumber(char *data, int size)
{
	if (size == 0 )
		return false;
	return true;
}

void makeCleanup() {
  for (bufferIndex = 0; bufferIndex < BLE_BUFFER; bufferIndex++)
  {
    bufferReceive[bufferIndex] = '\0';
  }
  bufferIndex = 0;
  inChar ='\0';
}

void breakEngines() {
	
}

/*
* Move the platform in predefined directions.
*/
void go(int speedLeft, int speedRight) {
	
}

bool makeMove() {
  if (bufferIndex > 0) {
     bufferReceive[bufferIndex] = '\0';
  }
  if (strlen(bufferReceive) == 1) {
    if (bufferReceive[0] == 'V') {      
#ifdef SERIAL_DEBUG_MODE
		printf("%d\n\r",maxPower);
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",maxPower);
		uart_puts(uart1,bufferSend);
    } else if (bufferReceive[0] =='v') {
#ifdef SERIAL_DEBUG_MODE
      printf("%d\n",minPower);
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",minPower);
		uart_puts(uart1,bufferSend);
    } else if (bufferReceive[0] =='c') {
#ifdef SERIAL_DEBUG_MODE
      printf("%d\n",currentPower);
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",currentPower);
		uart_puts(uart1,bufferSend);
    }  else if (bufferReceive[0] == 'd') {
#ifdef SERIAL_DEBUG_MODE
		printf("unsupported\n");
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",0);
		uart_puts(uart1,bufferSend);
    } else if (bufferReceive[0] == 's') {
#ifdef SERIAL_DEBUG_MODE
		printf("unsupported\n");
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",0);
		uart_puts(uart1,bufferSend);
    } else if (bufferReceive[0] == 'b') {
		breakEngines();
		isValidInput = false;
		makeCleanup();
		return false;
    } else {
#ifdef SERIAL_DEBUG_MODE
		printf("unsupported\n");
#endif
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"%d\r\n",0);
		uart_puts(uart1,bufferSend);
		isValidInput = false;
    }
  } else {
	if (bufferReceive[0] == 'V') {
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"OK\r\n");
		uart_puts(uart1,bufferSend);
		//remove V from command
		for (uint8_t i = 0 ; i < strlen(bufferReceive); i++) {
			bufferReceive[i]=bufferReceive[i+1];
        }
        if (!isValidNumber(bufferReceive, bufferIndex - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(bufferReceive) > ABSOLUTE_MAX_POWER || atol(bufferReceive) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        printf("MaxPower=%s\n",bufferReceive);
#endif        
        maxPower = atol(bufferReceive);
        makeCleanup();
        isValidInput = true;
        return true;
	} else if (bufferReceive[0] == 'v') {
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
        sprintf(bufferSend,"OK\r\n");
		uart_puts(uart1,bufferSend);
        //remove v from command
        for (uint8_t i = 0 ; i < strlen(bufferReceive); i++) {
          bufferReceive[i]=bufferReceive[i+1];
        }
        if (!isValidNumber(bufferReceive, bufferIndex - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(bufferReceive) > maxPower || atol(bufferReceive) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        printf("MinPower=%s\n",bufferReceive);
#endif                
        minPower = atol(bufferReceive);
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (bufferReceive[0] == 'c') {
        sprintf(bufferSend,"OK\r\n");
        uart_puts(uart1,bufferSend);
        //remove c from command
        for (uint8_t i = 0 ; i < strlen(bufferReceive); i++) {
          bufferReceive[i]=bufferReceive[i+1];
        }
        if (!isValidNumber(bufferReceive, bufferIndex - 2)) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
        if (atol(bufferReceive) > maxPower || atol(bufferReceive) < 0) {
          isValidInput = false;
          makeCleanup();
          return false;
        }
#ifdef SERIAL_DEBUG_MODE
        printf("CurrentPower=%s\n",bufferReceive);
#endif                
        currentPower = atol(bufferReceive);
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (bufferReceive[0] == 'd') {
        sprintf(bufferSend,"OK\r\n");
        uart_puts(uart1,bufferSend);
        //this does not support low power distance
#ifdef SERIAL_DEBUG_MODE
		printf("unsupported\n");
#endif        
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (bufferReceive[0] == 's') {
        sprintf(bufferSend,"OK\r\n");
		uart_puts(uart1,bufferSend);
        //this does not support stop distance
#ifdef SERIAL_DEBUG_MODE
		printf("unsupported\n");
#endif        
        makeCleanup();
        isValidInput = true;
        return true;
      } else if (bufferReceive[0] == 'M') {
        //remove M from command
        for (uint8_t i = 0 ; i < strlen(bufferReceive); i++) {
          bufferReceive[i]=bufferReceive[i+1];
        }
        bufferReceive[strlen(bufferReceive)] = '\0';
        int position;
        for (uint8_t i = 0; i < strlen(bufferReceive); i++) {
          if (bufferReceive[i] == ',') {
            position = i;
            break;
          }
        }
        char buf[10];
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        for (int i = 0 ; i < position; i++) {
          buf[i] = bufferReceive[i];
        }
        int moveData = atoi(buf);
        for (int i = 0; i < 10; i++) {
          buf[i] = '\0';
        }
        int idx = 0;
        for (int i = position + 1; i < strlen(bufferReceive); i++) {
          buf[idx] = bufferReceive[i];
          idx++;
        }
        int rotateData = atoi(buf);
#ifdef SERIAL_DEBUG_MODE
		printf("%s\n%d:%d\n",bufferReceive,moveData,rotateData);
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
        sprintf(bufferSend,"%d\r\n",0);
		uart_puts(uart1,bufferSend);
        makeCleanup();
        isValidInput = false;
        return false;
      }
    }
    makeCleanup();
    return true;
}

int main() {
	stdio_usb_init();
	//initialize UART 1
	uart_init(uart1, 38400);

	// Set the GPIO pin mux to the UART - 4 is TX, 5 is RX
	gpio_set_function(4, GPIO_FUNC_UART);
	gpio_set_function(5, GPIO_FUNC_UART);
	uart_set_translate_crlf(uart1, 1);
	makeCleanup();
	while (1) {
		if (uart_is_readable(uart1)) {
			inChar = uart_getc(uart1);
			if(inChar == '\n' || inChar == '\r' || inChar == ' ' || inChar == '\t' || inChar == '\0') {
				continue;
			}
			bufferReceive[bufferIndex] = inChar;
			if (inChar == '#') {
				bufferReceive[bufferIndex + 1] = '\0';
				if (strcmp(bufferReceive, "exit#") == 0) {
					return 1 ;
				}
#ifdef SERIAL_DEBUG_MODE				
				printf("%s\n", bufferReceive);
#endif			
				makeMove();
				makeCleanup();
			} else {
				bufferIndex++;
			}
		}
	}
}
