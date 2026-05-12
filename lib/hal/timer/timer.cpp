#include "timer.h"
#ifdef __AVR__
void Timer::timer0_init(){
    // CTC mode
    TCCR0A = (1 << WGM01);

    // Set prescaler to 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // Calculate and set OCR0A for 1ms tick
    OCR0A = F_CPU/(Timer::F_TIMER0*2*Timer::SCALER) - 1;

    // Enable Timer0 interrupt on compare match
    TIMSK0 = (1 << OCIE0A);

    // Enable global interrupts
    sei();
}

/**
 * @brief Timer0 Compare Match A Interrupt Service Routine
 * 
 * When triggered, this ISR updates the system time by calling 
 * SystemTime::tick(). Used for timekeeping based on system time.
 */
ISR(TIMER0_COMPA_vect) {
    SystemTime::tick();
}

#else

void Timer::timer0_init(){
    //NOP
}

#endif