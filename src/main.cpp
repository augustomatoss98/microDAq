#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

int main(void) {
	UART::init(115200);

	while(1){
		char teste = UART::read();
		_delay_ms(1000);
		UART::write(teste + 1);
		_delay_ms(1000);
	}
}
