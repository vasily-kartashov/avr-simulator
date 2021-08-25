#include "avr/AT90S1200.h"
#include "avr/AT90S2313.h"
#include "avr/AT90S2323.h"
#include "avr/AT90S2343.h"
#include "avr/AT90S2333.h"
#include "avr/AT90S4433.h"
/*  #include "avr/AT90S4414.h" */
#include "avr/AT90S4434.h"
/*  #include "avr/AT90S8515.h" */
#include "avr/AT90S8535.h"
#include "avr/ATmega103.h"
#include "avr/ATmega161.h"
#include "avr/ATmega162.h"
#include "avr/ATmega163.h"
#include "avr/ATmega323.h"
#include "avr/ATmega8.h"
#include "avr/ATmega16.h"
#include "avr/ATmega32.h"
/*  #include "avr/ATmega64.h" */
#include "avr/ATmega128.h"
#include "avr/ATmega8515.h"
/*  #include "avr/ATmega8535.h" */

Microcontroller controllers[]=
{
  {"AT90S1200",32,64,0,512,64,at90s1200_int,at90s1200_io,at90s1200_pins,
   at90s1200_flags},
  {"AT90S2313",32,64,128,1024,128,at90s2313_int,at90s2313_io,at90s2313_pins,
   at90s2313_flags},
  {"AT90S2323",32,64,128,1024,128,at90s2323_int,at90s2323_io,
   at90s2323_pins,at90s2323_flags},
  {"AT90S2343",32,64,128,1024,128,at90s2343_int,at90s2343_io,
   at90s2343_pins,at90s2343_flags},
  {"AT90S2333",32,64,128,1024,128,at90s2333_int,at90s2333_io,
   at90s2333_pins,at90s2333_flags},
  {"AT90S4433",32,64,128,2048,256,at90s4433_int,at90s4433_io,
   at90s4433_pins,at90s4433_flags},
/*    {"AT90S4414",32,64,256,2048,256,at90s4414_int,at90s4414_io, */
/*     at90s4414_pins,at90s4414_flags}, */
/*    {"AT90S8515",32,64,512,4096,512,at90s8515_int,at90s8515_io, */
/*     at90s8515_pins,at90s8515_flags}, */
  {"AT90S4434",32,64,256,2048,256,at90s4434_int,at90s4434_io,
   at90s4434_pins,at90s4434_flags},
  {"AT90S8535",32,64,512,4096,512,at90s8535_int,at90s8535_io,
   at90s8535_pins,at90s8535_flags},

  {"ATmega103",32,64,4000,65536,4096,atmega103_int,atmega103_io,
   atmega103_pins,atmega103_flags},
  {"ATmega161",32,64,1024,8192,512,atmega161_int,atmega161_io,
   atmega161_pins,atmega161_flags},
  {"ATmega162",32,224,1024,8192,512,atmega162_int,atmega162_io,
   atmega162_pins,atmega162_flags},
  {"ATmega163",32,64,1024,8192,512,atmega163_int,atmega163_io,
   atmega163_pins,atmega163_flags},
  {"ATmega323",32,64,2048,16384,1024,atmega323_int,atmega323_io,
   atmega323_pins,atmega323_flags},
  {"ATmega8",32,64,1024,4096,512,atmega8_int,atmega8_io,
   atmega8_pins,atmega8_flags},
  {"ATmega16",32,64,1024,8192,512,atmega16_int,atmega16_io,
   atmega16_pins,atmega16_flags},
  {"ATmega32",32,64,2048,16384,1024,atmega32_int,atmega32_io,
   atmega32_pins,atmega32_flags},
  /*  {"ATmega64",32,224,4096,64*1024,2048,atmega64_128_int,atmega64_io,
      atmega64_pins,atmega64_flags},*/
  {"ATmega128",32,224,4096,65536,4096,atmega128_int,atmega128_io,
   atmega128_pins,atmega128_flags},
  {"ATmega8515",32,64,512,4096,512,atmega8515_int,atmega8515_io,
   atmega8515_pins,atmega8515_flags},
/*    {"ATmega8535",32,64,512,4096,512,atmega8515_int,atmega8515_io, */
/*     atmega8515_pins,atmega8515_flags}, */
  {(char *)NULL,0,0,0,0,0,NULL,NULL,NULL},
};

Descr_io_port descr_io_ports[]=
{{"UBRR1L","UART1 Baud Rate Register Low"},
 {"UBRR1H","UART1 Baud Rate Register High"},
 {"UCSR1C","UART1 Control Register and Status Register"},
 {"UCSR1B","UART1 Control Register"},
 {"UCSR1A","UART1 Status Register"},
 {"UDR1  ","UART1 I/O Data Register"},
 {"UBRR0L","UART0 Baud Rate Register Low"},
 {"UBRR0H","UART0 Baud Register High"},
 {"UCSR0C","UART0 Control Register and Status Register"},
 {"UCSR0B","UART0 Control Register"},
 {"UCSR0A","UART0 Status Register"},
 {"UDR0  ","UART0 I/O Data Register"},
 {"UBRRHI","UART Baud Register High"},


 {"PING  ","Input Pins, Port G"}, 
 {"DDRG  ","Data Direction Register, Port G"},
 {"PORTG ","Data Register, Port G"}, 
 {"PINF  ","Input Pins, Port F"}, 
 {"DDRF  ","Data Direction Register, Port F"},
 {"PORTF ","Data Register, Port F"},
 {"PINE  ","Input Pins, Port E"},
 {"DDRE  ","Data Direction Register, Port E"},
 {"PORTE ","Data Register, Port E"},
 {"PIND  ","Input Pins, Port D"},
 {"DDRD  ","Data Direction Register, Port D"},
 {"PORTD ","Data Register, Port D"},
 {"PINC  ","Input Pins, Port C"},
 {"DDRC  ","Data Direction Register, Port C"},
 {"PORTC ","Data Register, Port C"},
 {"PINB  ","Input Pins, Port B"},
 {"DDRB  ","Data Direction Register, Port B"},
 {"PORTB ","Data Register, Port B"},
 {"PINA  ","Input Pins, Port A"},
 {"DDRA  ","Data Direction Register, Port A"},
 {"PORTA ","Data Register, Port A"},

 {"ACSR  ","Analog Comparator Control and Status Register"},
 
 {"SPCR  ","SPI Control Register"},
 {"SPSR  ","SPI Status Register"},
 {"SPDR  ","SPI I/O Data Register"},

 {"EECR  ","EEPROM Control Register"},
 {"EEDR  ","EEPROM Data Register"},
 {"EEARL ","EEPROM Address Register Low"},
 {"EEARH ","EEPROM Address Register High"},

 {"WDTCR ","Watchdog Timer Control Register"},

 {"ICR3L ","Timer/Counter3 Input Capture Register Low Byte"},
 {"ICR3H ","Timer/Counter3 Input Capture Register High Byte"},
 {"OCR3CL","Timer/Counter3 Output Capture RegisterC Low Byte"},
 {"OCR3CH","Timer/Counter3 Output Capture RegisterC High Byte"},
 {"OCR3BL","Timer/Counter3 Output Capture RegisterB Low Byte"},
 {"OCR3BH","Timer/Counter3 Output Capture RegisterB High Byte"},
 {"OCR3AL","Timer/Counter3 Output Capture RegisterA Low Byte"},
 {"OCR3AH","Timer/Counter3 Output Capture RegisterA High Byte"},
 {"TCNT3L","Timer/Counter3 Low Byte"},
 {"TCNT3H","Timer/Counter3 High Byte"},
 {"TCCR3C","Timer/Counter3 Control Register C"},
 {"TCCR3B","Timer/Counter3 Control Register B"},
 {"TCCR3A","Timer/Counter3 Control Register A"},
 {"OCR2  ","Timer/Counter2 Output Compare Register"},
 {"TCNT2 ","Timer/Counter2 (8-bit)"},
 {"TCCR2 ","Timer/Counter2 Control Register"},
 {"ICR1L ","Timer/Counter1 Input Capture Register Low Byte"},
 {"ICR1H ","Timer/Counter1 Input Capture Register High Byte"},
 {"OCR1CL","Timer/Counter1 Output Capture RegisterC Low Byte"},
 {"OCR1CH","Timer/Counter1 Output Capture RegisterC High Byte"},
 {"OCR1BL","Timer/Counter1 Output Capture RegisterB Low Byte"},
 {"OCR1BH","Timer/Counter1 Output Capture RegisterB High Byte"},
 {"OCR1AL","Timer/Counter1 Output Capture RegisterA Low Byte"},
 {"OCR1AH","Timer/Counter1 Output Capture RegisterA High Byte"},
 {"TCNT1L","Timer/Counter1 Low Byte"},
 {"TCNT1H","Timer/Counter1 High Byte"},
 {"TCCR1C","Timer/Counter1 Control Register C"},
 {"TCCR1B","Timer/Counter1 Control Register B"},
 {"TCCR1A","Timer/Counter1 Control Register A"},
 {"OCR0  ","Timer/Counter0 Output Compare Register"},
 {"TCNT0 ","Timer/Counter0 (8-bit)"},
 {"TCCR0 ","Timer/Counter0 Control Register"},
 {"ASSR  ","Asynchronous mode Status Register"},
 {"SFIOR ","Special Function IO Register"},
 {"CLKPR ","Clock Prescale Register"},

 {"MCUSR ","MCU general Status Regsiter"},
 {"MCUCR ","MCU general Control Regsiter"},
 {"EMCUCR","Extended MCU general Control Register"},

 {"SPMCR ","Store Program Memory Control Register"},
 {"TIFR  ","Timer/Counter Interrupt Flag Register"},
 {"TIMSK ","Timer/Counter Interrupt Mask Register"},
 {"ETIMSK","Extended Timer/Counter Interrupt Mask Register"},
 {"ETIFR ","Extended Timer/Counter Interrupt Flag Register"},
 {"GIFR  ","General Interrupt Flag Register"},
 {"GIMSK ","General Interrupt Mask Register"},
 {"GICR  ","General Interrupt Control Register"},
 {"PCMSK0","Pin Change Mask Register 0"},
 {"PCMSK1","Pin Change Mask Register 1"},
 {"SPL   ","Stack Pointer Low"},
 {"SPH   ","Stack Pointer High"},
 {"SREG  ","Status Register"},
 {"RAMPZ ","RAM Page Z Select Register"},
 {"XDIV  ","XTAL Divide Control Registe"},

 {"EICR  ","External Interrupt Control Register"},
 {"EICRB ","External Interrupt Control Register B"},
 {"EICRA ","External Interrupt Control Register A"},
 {"EIMSK ","External Interrupt Mask Register"},
 {"EIFR  ","External Interrupt Flag Register"},

 {"ADMUX ","ADC Multiplexer Select Register"},
 {"ADCSR ","ADC Control and Status Register"},
 {"ADCSRA","ADC Control and Status Register A"},
 {"ADCH  ","ADC Data Register High"},
 {"ADCL  ","ADC Data Register Low"},
 
 {"TWCR  ","2-wire Serial Interface Control Register"},
 {"TWDR  ","2-wire Serial Interface Data Register"},
 {"TWAR  ","2-wire Serial Interface (Slave) Address Register"},
 {"TWSR  ","2-wire Serial Interface Status Register"},
 {"TWBR  ","2-wire Serial Interface Bit Rate Register"},

 {"OSCCAL","Oscillator Calibration Register"},
 {"OCDR  ",  "On-Chip Debug Related Register"},

 {"XMCRB ","External Memory Control Register B"},
 {"XMCRA ","External Memory Control Register A"},
 {NULL,NULL},
};


IO_port16 port16_wr[]={
  {TCNT1H,HIGH,&temp_reg_t1},
  {TCNT1L,LOW, &temp_reg_t1},
  {OCR1AH,HIGH,&temp_reg_t1},
  {OCR1AL,LOW, &temp_reg_t1},
  {OCR1BH,HIGH,&temp_reg_t1},
  {OCR1BL,LOW, &temp_reg_t1},
  {OCR1CH,HIGH,&temp_reg_t1},
  {OCR1CL,LOW, &temp_reg_t1},
  {TCNT3H,HIGH,&temp_reg_t3},
  {TCNT3L,LOW, &temp_reg_t3},
  {OCR3AH,HIGH,&temp_reg_t3},
  {OCR3AL,LOW, &temp_reg_t3},
  {OCR3BH,HIGH,&temp_reg_t3},
  {OCR3BL,LOW, &temp_reg_t3},
  {OCR3CH,HIGH,&temp_reg_t3},
  {OCR3CL,LOW, &temp_reg_t3},
  {-1,-1}
};

IO_port16 port16_rd[]={
  {TCNT1H,HIGH,&temp_reg_t1},
  {TCNT1L,LOW, &temp_reg_t1},
  {ICR1H, HIGH,&temp_reg_t1},
  {ICR1L, LOW, &temp_reg_t1},
  {TCNT3H,HIGH,&temp_reg_t3},
  {TCNT3L,LOW, &temp_reg_t3},
  {ICR3H, HIGH,&temp_reg_t3},
  {ICR3L, LOW, &temp_reg_t3},
  {-1,-1}
};


