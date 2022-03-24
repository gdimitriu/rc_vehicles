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
#include <hardware/pwm.h>
#include <string.h>

#define SERIAL_DEBUG_MODE true
#define BLE_BUFFER 256
#define BLE_BUFFER_SEND 512
#define ABSOLUTE_MAX_POWER 65025
#define LEFT_MOTOR_PIN1 15
#define LEFT_MOTOR_PIN2 14
#define RIGHT_MOTOR_PIN1 11
#define RIGHT_MOTOR_PIN2 10
#define HIGH 1
#define LOW 0
int bufferIndex = 0;
int maxPower = ABSOLUTE_MAX_POWER;
int currentPower = ABSOLUTE_MAX_POWER;
int minPower = 30000;
char inChar = '\0';
char bufferReceive[BLE_BUFFER];
char bufferSend[BLE_BUFFER_SEND];
bool isValidInput = false;

bool isValidNumber(char *data, int size)
{
	if (size == 0 )
		return false;
	for (int i =0 ;i < size; i++) {
		if (!(data[i] < 58 && data[i] > 47 ))
			return false;
	}
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
	pwm_set_gpio_level(LEFT_MOTOR_PIN1,ABSOLUTE_MAX_POWER);
    pwm_set_gpio_level(LEFT_MOTOR_PIN2,ABSOLUTE_MAX_POWER);
    pwm_set_gpio_level(RIGHT_MOTOR_PIN1,ABSOLUTE_MAX_POWER);
    pwm_set_gpio_level(RIGHT_MOTOR_PIN2,ABSOLUTE_MAX_POWER);
}

/*
* Move the platform in predefined directions.
*/
void go(int speedLeft, int speedRight) {
	if (speedLeft == 0 && speedRight == 0 ) {
		pwm_set_gpio_level(LEFT_MOTOR_PIN1,LOW);
		pwm_set_gpio_level(LEFT_MOTOR_PIN2,LOW);
		pwm_set_gpio_level(RIGHT_MOTOR_PIN1,LOW);
		pwm_set_gpio_level(RIGHT_MOTOR_PIN2,LOW);
#ifdef SERIAL_DEBUG_MODE    
		printf("All on zero\n");
#endif
		return;
	}
	if (speedLeft > 0) {
		pwm_set_gpio_level(LEFT_MOTOR_PIN1, speedLeft);
		pwm_set_gpio_level(LEFT_MOTOR_PIN2, 0);
#ifdef SERIAL_DEBUG_MODE
		printf("Left %d,0\n",speedLeft);
#endif
	} 
	else {
		pwm_set_gpio_level(LEFT_MOTOR_PIN1, 0);
		pwm_set_gpio_level(LEFT_MOTOR_PIN2, -speedLeft);
#ifdef SERIAL_DEBUG_MODE
		printf("Left 0,%d\n",-speedLeft);
#endif
	}
 
	if (speedRight > 0) {
		pwm_set_gpio_level(RIGHT_MOTOR_PIN1, speedRight);
		pwm_set_gpio_level(RIGHT_MOTOR_PIN2, 0);
#ifdef SERIAL_DEBUG_MODE
		printf("Right %d,0\n",speedRight);
#endif
	}else {
		pwm_set_gpio_level(RIGHT_MOTOR_PIN2, 0);		
		pwm_set_gpio_level(RIGHT_MOTOR_PIN2, -speedRight);
#ifdef SERIAL_DEBUG_MODE
		printf("Right 0,%d\n",-speedRight);
#endif
	}
}

bool makeMove() {
  if (bufferIndex > 0) {
     bufferReceive[bufferIndex] = '\0';
  }
  if (strlen(bufferReceive) == 1) {
	if (bufferReceive[0] == 'I')  {
		memset(bufferSend,'\0',sizeof(char)*BLE_BUFFER_SEND);
		sprintf(bufferSend,"unsupported\r\n");
		uart_puts(uart1,bufferSend);		
	} else if (bufferReceive[0] == 'V') {      
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
#ifdef SERIAL_DEBUG_MODE
	printf("Starting...\n");
	fflush(stdout);
#endif
	gpio_set_function(LEFT_MOTOR_PIN1, GPIO_FUNC_PWM);
	// Figure out which slice we just connected
    uint slice_num = pwm_gpio_to_slice_num(LEFT_MOTOR_PIN1);
    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);
	gpio_set_function(LEFT_MOTOR_PIN2, GPIO_FUNC_PWM);
	slice_num = pwm_gpio_to_slice_num(LEFT_MOTOR_PIN2);
	config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);	
	gpio_set_function(RIGHT_MOTOR_PIN1, GPIO_FUNC_PWM);	
	slice_num = pwm_gpio_to_slice_num(RIGHT_MOTOR_PIN1);
	config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);	
	gpio_set_function(RIGHT_MOTOR_PIN2, GPIO_FUNC_PWM);
	slice_num = pwm_gpio_to_slice_num(RIGHT_MOTOR_PIN2);
	config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    pwm_config_set_clkdiv(&config, 4.f);
    // Load the configuration into our PWM slice, and set it running.
    pwm_init(slice_num, &config, true);	
	makeCleanup();
#ifdef SERIAL_DEBUG_MODE	
	printf("Started\n");
	fflush(stdout);
#endif	
	while (1) {
		if (uart_is_readable(uart1)) {
			inChar = uart_getc(uart1);
			//commands does not have terminators
			if(inChar == '\n' || inChar == '\r' || inChar == ' ' || inChar == '\t' || inChar == '\0' || inChar < 35 || inChar > 122) {
				continue;
			}
			//commands start with a letter capital or small
			if (bufferIndex == 0 && !((inChar >64 && inChar <91) || (inChar > 96 && inChar<123))) {
				continue;
			}
			bufferReceive[bufferIndex] = inChar;
			if (inChar == '#') {
				bufferReceive[bufferIndex + 1] = '\0';
				if (strcmp(bufferReceive, "exit#") == 0) {
					return 1 ;
				}
#ifdef SERIAL_DEBUG_MODE				
				printf("<<%s>>index=%d\n", bufferReceive,bufferIndex);
#endif			
				makeMove();
				makeCleanup();
			} else {
				bufferIndex++;
			}
		}
	}
}
