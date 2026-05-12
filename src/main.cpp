#include <util/delay.h>
#include <avr/io.h>

#include "uart/UART.h"
#include "timer/timer.h"

#include "SystemTime/SystemTime.h"

#include "Protocol/Protocol.h"
#include "SensorManager/SensorManager.h"
#include "CommandHandler/CommandHandler.h"

#include "MockSensor/MockSensor.h"



int main() {

    UART::init(115200);
    Timer::timer0_init();
    SystemTime sys_time;
    sys_time.init();

    DDRB |= (1 << PB5);

    Protocol protocol;
    protocol.set_write_callback(UART::write);
    
    MockSensor mock1, mock2;
    SensorManager sensor_mng;
    sensor_mng.add_sensor(1, mock1);
    sensor_mng.add_sensor(2, mock2);

    CommandHandler handler(sensor_mng);

    Protocol::Packet pkt;

    while (1) {
        if (UART::is_available()) {
            uint8_t byte = UART::read();
            protocol.process(byte);
        }

        if(protocol.available()){
            pkt = protocol.get_packet();
            handler.handle(pkt, protocol);
            PORTB ^= (1 << PB5);

        }

        sensor_mng.process();
        // protocol.update();
    }
}