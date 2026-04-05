#include "uart/UART.h"
#include <util/delay.h>

int main() {

    UART::init(9600);

    const char* msg = "microDAQ UART OK\n";
	

    while (1) {

        UART::write((const uint8_t*)msg, 17);

        if (UART::is_available()) {
            // uint8_t c = UART::read();
            // UART::write(c); // echo
			const char* msg2 = "Is available\n";
			UART::write((const uint8_t*)msg2, 13);
			_delay_ms(1000);
        }
    }
}