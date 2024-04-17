#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BUTTON1_PIN   PC2
#define BUTTON2_PIN   PC3

// Variable global para almacenar el valor del contador
volatile uint8_t counter = 0;
volatile uint8_t cont_disp = 0;
volatile uint8_t disp1 = 0;
volatile uint8_t disp2 = 0;
const tabla[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

// Función para inicializar los pines
void init_pins() {
	// Configurar los pines PC2 y PC3 como entradas y habilitar resistencias de pull-up
	DDRC &= ~(1 << DDC2) & ~(1 << DDC3) & ~(1 << DDC6);
	PORTC |= (1 << PORTC2) | (1 << PORTC3);

	// Pines de salida
	DDRB = 0x3F; // PB0-PB5 como salidas
	DDRC |= (1 << DDC0) | (1 << DDC1)| (1 << DDC4) | (1 << DDC5);
	DDRD = 0xFF; // PD0-PD7 como salidas
	
	// Habilitar interrupciones por cambio de estado en PC2 y PC3
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);
	
	//ADC-------------------------------------------------
	ADMUX=0;
	ADCSRA=0;
	// Configurar el ADC para usar AVCC como referencia de voltaje y habilitar el ADC
	ADMUX |= (1 << REFS0) | (1 << ADLAR); // Configurar referencia de voltaje en AVCC
	
	// Habilitar ADC y configurar preescalador a 128
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	// Configurar pin A6 como entrada analógica
	ADMUX |= (1 << MUX2) | (1 << MUX1); // A6 en ADMUX[5:0]

	// Habilitar interrupción de conversión completa del ADC
	ADCSRA |= (1 << ADIE);

	// Iniciar primera conversión
	//ADCSRA |= (1 << ADSC);

	// Habilitar interrupciones globales
	sei();
}

// Función para aumentar el contador
void increment_counter() {
	counter++;
	if (counter == 256) {
		// Si el contador llega a su máximo, rebobinar a cero
		counter = 0;
	}
}

// Función para decrementar el contador
void decrement_counter() {
	if (counter > 0) {
		counter--;
		} else {
		// Si el contador está en cero, rebobinar al máximo
		counter = 255;
	}
}

// Función para mostrar el valor del contador en los LEDs
void display_counter() {
	// Apagar todos los LEDs
	PORTB &= ~((1 << PINB0) | (1 << PINB1) | (1 << PINB2) | (1 << PINB3) | (1 << PINB4) | (1 << PINB5));
	PORTC &= ~((1 << PINC0) | (1 << PINC1));
	
	// Mostrar el valor del contador en los LEDs
	PORTB |= ((counter & 0x3F)); // LED1 a LED6
	PORTC |= ((counter >> 6) & 0x03); // LED7 a LED8
}

// Rutina de interrupción
ISR(PCINT1_vect) {
	
	// Comprobar si el botón de incremento (PC2) está presionado
	if (!(PINC & (1 << BUTTON1_PIN))) {
		increment_counter();
	}

	// Comprobar si el botón de decremento (PC3) está presionado
	if (!(PINC & (1 << BUTTON2_PIN))) {
		decrement_counter();
	}
}

ISR(ADC_vect){
	
	cont_disp = ADCH;
	disp1 = cont_disp & 0b00001111;
	disp2 = cont_disp >> 4;
	
	ADCSRA |= (1<<ADIF);
	
}

void comparar(void){
	if (cont_disp>counter){
		PORTC |= (1 << PORTC4);
		}else{
		PORTC &= ~(1 << PORTC4);
	}
}

// Función principal
int main() {
	
	// Deshabilitar la funcionalidad de los pines RX y TX (PD0 y PD1)
	UCSR0B =0;
	
	// Inicializar los pines
	init_pins();
	
	while (1) {
		display_counter();
		PORTD |= (1 << PD0);
		PORTC &= ~(1 << PC5);
		PORTD &= 0x01;
		PORTD |= ((tabla[disp1] << 1) & 0xFE);
		//PORTD |= ((0x66 << 1) & 0xFE);
		_delay_ms(10);
		PORTC |= (1<<PC5);
		PORTD &= ~(1 << PD0);
		PORTD &= 0x01;
		PORTD |= ((tabla[disp2] << 1) & 0xFE);
		//PORTD |= ((0x3F << 1) & 0xFE);
		_delay_ms(10);
		ADCSRA |= (1<<ADSC);
		comparar();
	}

	return 0;
}