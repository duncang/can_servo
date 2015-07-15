
/* can_servo.c
 * running on AT90CAN128
 * Duncan Greer 13 Jan 2012
 */



#include "config.h"

#include <avr/interrupt.h>
#include <util/delay.h>


#include "uart_lib.h"
#include "can_lib.h"

#define CAN_SERVO_COMMAND_MESSAGE 0x3E8
#define CAN_SERVO_STATUS_MESSAGE 0x3E9

#define CAN_STROBE_LIGHT_MESSAGE 0x7FE

void delay_ms(uint16_t count)
{
	while(count--)
	{
		_delay_ms(1);
	}
}



// SetupServos
// use Timer 1 (16 bit)
// remember to disable interrupts when 
// accessing 16 bit registers
	
void setupServos(void)
{
	uint16_t val_Top = 39999;

	uart_mini_printf("Initialising servo output...");

	// set Data direction register to output
	// OC1A = PB5
	// OC1B = PB6
	// OC1C = PB7
	DDRB |= (1<<PB5) | (1<<PB6) | (1<<PB7);

	// set fast PWM mode (WGM=14)
	TCCR1A |= (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);

	// set TOP value in ICR1
	ICR1 = val_Top;


	// set compare output mode
	TCCR1A |= (1<<COM1A1);
	TCCR1A |= (1<<COM1B1);
	TCCR1A |= (1<<COM1C1);


	// set clock select - prescaler = 8
	TCCR1B |= (1<<CS11);


	// set clock output initially to 1.5ms
	OCR1A = 2000;
	OCR1B = 3000;
	OCR1C = 4000;

	uart_mini_printf("done\r\n");

}


/* servo time arrives in microseconds between 1000 & 2000 */

void setServoValue(uint8_t servo, uint16_t value)
{
	uint16_t output = 0;

	/* limit values */
	if (value > 2000) value = 2000;
	if (value < 1000) value = 1000;

	// map to output settings
	output = 2 * value;

	switch(servo)
	{
	  case 1:
	  	OCR1A = output;
		break;
	  case 2:
		OCR1B = output;
		break;
	  case 3:
	  	OCR1C = output;
		break;
	  default:
	  	break;

	}


}


int main(void)
{

	uart_init(CONF_8BIT_NOPAR_1STOP,UART_BAUDRATE);
    uart_mini_printf ("CAN Servo Demo - UART initialised\r\n");


	can_init(0);

	uint8_t receive_buffer[8];
	uint8_t transmit_buffer[8];
	uint8_t i;

	uint8_t servo_id = 0;
	uint16_t servo_value = 0;

	st_cmd_t receive_msg;
	st_cmd_t transmit_msg;


	setupServos();

	while(1)
	{

		// setup to receive servo messages - message id 0x3E8
		for(i=0; i<8; i++) receive_buffer[i]=0;
		for(i=0; i<8; i++) transmit_buffer[i]=0;

		transmit_buffer[0] = servo_id;
		transmit_buffer[1] = (uint8_t)(servo_value >> 8);  // uppper byte
		transmit_buffer[2] = (uint8_t)(servo_value);  // lower byte


		transmit_msg.cmd = CMD_TX_DATA;
		transmit_msg.id.std = CAN_SERVO_STATUS_MESSAGE;
		transmit_msg.dlc = 3;
		transmit_msg.pt_data = &transmit_buffer[0];
		transmit_msg.ctrl.ide = 0;
		transmit_msg.ctrl.rtr = 0;


		while(can_cmd(&transmit_msg) != CAN_CMD_ACCEPTED) { }

		// --- Wait for message to be transmitted
        while(can_get_status(&transmit_msg) == CAN_STATUS_NOT_COMPLETED) { }


		uart_mini_printf("Sent message\r\n");


		
		delay_ms(50);


		receive_msg.cmd = CMD_RX_DATA_MASKED;
		receive_msg.id.std = CAN_SERVO_COMMAND_MESSAGE;
		receive_msg.dlc = 3;
		receive_msg.pt_data = &receive_buffer[0];
		receive_msg.ctrl.ide = 0;
		receive_msg.ctrl.rtr = 0;

		while(can_cmd(&receive_msg) != CAN_CMD_ACCEPTED);
		uart_mini_printf("Waiting for message...\r\n");


		while(can_get_status(&receive_msg) != CAN_STATUS_COMPLETED);
		uart_mini_printf ("Received message (0x%03X)!\r\n",receive_msg.id.std);

		servo_id = receive_buffer[0];
		servo_value = (uint16_t)receive_buffer[2] + (uint16_t)(receive_buffer[1] << 8);
		uart_mini_printf("      Servo: %d\r\n",servo_id);
		uart_mini_printf("      Value: %d\r\n",servo_value);


		setServoValue(servo_id,servo_value);



	}

}

