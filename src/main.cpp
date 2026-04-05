#include "uart/UART.h"
#include <util/delay.h>

int main() {

    UART::init(115200);

    const char* msg = "microDAQ UART OK\r\n";
	

    while (1) {

        UART::write((const uint8_t*)msg, 18);
		_delay_ms(1000);

        if (UART::is_available()) {
            uint8_t c = UART::read();
            UART::write(c); // echo
        }
    }
}