#include "timer.h"

void Timer::timer0_init(){
    TCCR0A = (1 << WGM01);

    TCCR0B = (1 << CS01) | (1 << CS00);

    OCR0A = F_CPU/(Timer::F_TIMER0*2*Timer::SCALER) - 1;

    TIMSK0 = (1 << OCIE0A);

    sei();
}

ISR(TIMER0_COMPA_vect) {
    SystemTime::tick();
}