#include <uart_serial.h>

extern uint32_t last_auto_tick;

uint8_t drive_msg[26];  // 25 data bytes + 1 null terminator
uint8_t current_pos = 0;

void uart_serial_start(UART_HandleTypeDef *huart) {
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
}

void uart_serial_stop(UART_HandleTypeDef *huart) {
	__HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
	__HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
}


// make sure you go to stm32f4xx_it.c and replace the default usart6 handler with this function
/**
 * @brief UART interface with computer via USB-TTL serial ports
 * 
 * @param huart 
 */
void uart_serial_irq_handler(UART_HandleTypeDef *huart) {
	// If data overrun, clear the overrun flag and reset
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
		current_pos = 0;
		memset(drive_msg, '\0', sizeof(drive_msg));

		__HAL_UART_CLEAR_OREFLAG(huart);
		return;
	}

	// receive the data and reset to start upon completion
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) == SET) {
		uint8_t data = (uint8_t) (huart->Instance->DR & (uint8_t) 0x00FF);
		drive_msg[current_pos] = data;
		current_pos ++;

		if (current_pos == 25){
			drive_msg[25] = '\0';
			current_pos = 0;
			last_auto_tick = HAL_GetTick();
		}

		return;
	}

	// If dataline idle, clear the idle flag
	// If we are somehow in the middle of receiving an array
	// clear the data and reset to start so we don't get bit shift in the next cycle
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == SET) {
		__HAL_UART_CLEAR_IDLEFLAG(huart);

		if(current_pos != 0){
			current_pos = 0;
			memset(drive_msg, '\0', sizeof(drive_msg));
		}
		return;
	}
};
