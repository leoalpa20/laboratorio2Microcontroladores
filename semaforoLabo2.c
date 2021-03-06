#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// States to be used later in the code, this correspond to the 
// state machine states, see flow chart 

#define BUTTON_IDLE 0 //
#define GREEN_LIGHT_PEDESTRIANS 1 //
#define BLINK_CARS 2 // 
#define BLINK_PEDESTRIANS 3 //
#define RED_LIGHT_CARS 4 //
#define RED_LIGHT_PEDESTRIANS 5 //

// Variables used to control the code

int state = BUTTON_IDLE;
int counter = 0;
int half_second_counter = 0;
int button_active = 0;


// Interrupt service routine on INT1 (External Interrupt Request 1)
ISR (INT1_vect) 
{
    button_active = 1; 
}

// Interrupt service routine on TIMER COUNTER OVERFLOW (1)

ISR (TIMER0_OVF_vect)
{
    if (counter == 30)
    {
        counter = 0; // Restart the counter
        ++half_second_counter; // Lets increase the half a second counter

        if (state == BLINK_CARS)
        {
            PORTB ^= (1<<PB1);
        }
        if (state == BLINK_PEDESTRIANS)
        {
            PORTB ^= (1<<PB3)|(1<<PB5);
        }
    }
    else
    {
        counter++; // Increase the counter to get to 30
    }
}

// Functions

// This function configures basic values for the outputs for the LEDS, also enables
// Interrupts
void setButtonsForLeds()
{
    // Set PB( 1, 2, 3, 4, 5, 6) as outputs for the LEDS
    DDRB |= (1<<DDB0) | (1<<DDB1) | (1<<DDB2) | (1<<DDB3) | (1<<DDB4) | (1<<DDB5);
    // Enable external interrupts
    GIMSK |= (1<<INT1); 
    // Configure to be active with the raising edge of the signal
    MCUCR |= (1<<ISC10) | (1<<ISC11);
    // Start the LEDS in the LOW state
    PORTB &= (0<<PB0) | (0<<PB1) | (0<<PB2) | (0<<PB3) | (0<<PB4) | (0<<PB5) | (0<<PB6);

}

// This function

void temporizer()
{
    TCCR0A = 0x00;
    TCCR0B = 0x00;
    TCCR0B |= (1<<CS00) | (1<<CS02);
    sei();
    TCNT0 = 0;
    TIMSK |= (1<<TOIE0);
}

int main(void)
{
    setButtonsForLeds(); // Call the I/O function
    temporizer(); // Call the temporizer function
    while(1)
    {   // Begin the state machine 
        switch(state)
        {
            case(BUTTON_IDLE):
            // Activate the LEDS for the CARS (GREEN) and RED for the pedestrians
            PORTB = (1<<PB1) | (1<< PB4) | (1<<PB6); 
            // WAIT 10 seconds and wait for the button
            if(button_active & (half_second_counter >= 20))
            {   
                // Go to the next state
                state = BLINK_CARS;
                // Restart the counter
                half_second_counter = 0;
                counter = 0;
            }
            else
            {
                state = BUTTON_IDLE;
            }
            break;

            case(BLINK_CARS):
            // Blink the Green LED 3 seconds
            if(half_second_counter == 6)
            {
                // Same as before, continue to the next state and restart the counter
                state = RED_LIGHT_CARS;
                half_second_counter = 0;
                counter = 0;
            }
            else
            {
                state = BLINK_CARS;
            }
            break;

            case(RED_LIGHT_CARS):
            // Turno off the green LED for the cars and turn the RED LED for the cars
            PORTB = (0<<PB1) | (1<< PB2) | (1<<PB4) | (1<<PB6);
            // Wait 1 second
            if((half_second_counter  = 1))
            {
                state = GREEN_LIGHT_PEDESTRIANS;
                half_second_counter = 0;
                counter = 0;
            }
            else
            {
                state = RED_LIGHT_CARS;
            }
            break;

            case(GREEN_LIGHT_PEDESTRIANS):
            // Turn On the LED for the pedestrians
            PORTB =  (1<<PB2) | (0<<PB4) | (0<<PB6) | (1<<PB3) | (1<<PB5); 
            // Wait 10 seconds
            if(half_second_counter == 20)
            {
                state = BLINK_PEDESTRIANS;
                half_second_counter = 0;
                counter = 0;

            }
            else
            {
                state = GREEN_LIGHT_PEDESTRIANS;
            }
            break;

            case(BLINK_PEDESTRIANS):
            // Make the LED for the pedestrians BLINK 3 seconds
            if(half_second_counter == 6)
            {
                state = RED_LIGHT_PEDESTRIANS;
                half_second_counter = 0;
                counter = 0;

            } 
            else 
            {
                state = BLINK_PEDESTRIANS;
            }
            break;
            
            case(RED_LIGHT_PEDESTRIANS):
            // Turn back ON the RED LEDS for the pedestrians and turn OFF the green LED
            PORTB = (0<<PB1) | (1<<PB2) | (0<<PB3) | (0<<PB5) | (1<<PB4) | (1<<PB6);
            // Wait 1 second and continue the machine
            if(half_second_counter == 1)
            {
                state = BUTTON_IDLE;
                half_second_counter = 0;
                counter = 0;
                button_active = 0;
            }
            else
            {
                state = RED_LIGHT_PEDESTRIANS;
            }
            break;

            default:
            break;


        }
    }
}

