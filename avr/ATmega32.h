int atmega32_int[]={
  RESET,INT0,INT1,INT2,TIMER2_COMP,TIMER2_OVF,TIMER1_CAPT,TIMER1_COMPA,
  TIMER1_COMPB,TIMER1_OVF,TIMER0_COMP,TIMER0_OVF,SPI_STC,UART0_RX,UART0_UDRE,
  UART0_TX,ADC,EE_READY,ANALOG_COMP,TWSI,SPM_RDY,-1};

IO_port atmega32_io[]={
  {SREG,  0x3F,0xFF,0xFF,0},
  {SPH,   0x3E,0xFF,0xFF,0},
  {SPL,   0x3D,0xFF,0xFF,0},
  {OCR0,  0x3C,0xFF,0xFF,0},
  {GICR,  0x3B,0xE3,0xE3,0},
  {GIFR,  0x3A,0xE0,0xE0,0},
  {TIMSK, 0x39,0xFF,0xFF,0},
  {TIFR,  0x38,0xFF,0xFF,0},
  {SPMCR, 0x37,0x9F,0xDF,0},
  {TWCR,  0x36,0xF5,0xFD,0},
  {MCUCR, 0x35,0xFF,0xFF,0},
  {MCUSR, 0x34,0xDF,0xDF,0},
  {TCCR0, 0x33,0xFF,0xEF,0},
  {TCNT0, 0x32,0xFF,0xFF,0},
  {OSCCAL,0x31,0xFF,0xFF,0},
  {OCDR,  0x31,0xFF,0xFF,0},
  {SFIOR, 0x30,0xFF,0xFF,0},
  {TCCR1A,0x2F,0xFF,0xF3,0},
  {TCCR1B,0x2E,0xDF,0xDF,0},
  {TCNT1H,0x2D,0xFF,0xFF,0},
  {TCNT1L,0x2C,0xFF,0xFF,0},
  {OCR1AH,0x2B,0xFF,0xFF,0},
  {OCR1AL,0x2A,0xFF,0xFF,0},
  {OCR1BH,0x29,0xFF,0xFF,0},
  {OCR1BL,0x28,0xFF,0xFF,0},
  {ICR1H, 0x27,0x00,0xFF,0},
  {ICR1L, 0x26,0x00,0xFF,0},
  {TCCR2, 0x25,0xFF,0x7F,0},
  {TCNT2, 0x24,0xFF,0xFF,0},
  {OCR2,  0x23,0xFF,0xFF,0},
  {ASSR,  0x22,0x08,0x0F,0},
  {WDTCR, 0x21,0x1F,0x1F,0},
  {UBRRHI,0x20,0x8F,0x8F,0},
  {UCSR0C,0x20,0xFF,0xFF,0x86},
  {EEARH, 0x1F,0x03,0x03,0},
  {EEARL, 0x1E,0xFF,0xFF,0},
  {EEDR,  0x1D,0xFF,0xFF,0},
  {EECR,  0x1C,0x0F,0x0F,0},
  {PORTA, 0x1B,0xFF,0xFF,0},
  {DDRA,  0x1A,0xFF,0xFF,0},
  {PINA,  0x19,0x00,0xFF,0},
  {PORTB, 0x18,0xFF,0xFF,0},
  {DDRB,  0x17,0xFF,0xFF,0},
  {PINB,  0x16,0x00,0xFF,0},
  {PORTC, 0x15,0xFF,0xFF,0},
  {DDRC,  0x14,0xFF,0xFF,0},
  {PINC,  0x13,0x00,0x7F,0},
  {PORTD, 0x12,0xFF,0xFF,0},
  {DDRD,  0x11,0xFF,0xFF,0},
  {PIND,  0x10,0,0xFF,0},
  {SPDR,  0x0F,0xFF,0xFF,0},
  {SPSR,  0x0E,0x01,0xC1,0},
  {SPCR,  0x0D,0xFF,0xFF,0},
  {UDR0,  0x0C,0xFF,0xFF,0},
  {UCSR0A,0x0B,0x43,0xFF,0x20},
  {UCSR0B,0x0A,0xFD,0xFF,0x0},
  {UBRR0L,0x09,0xFF,0xFF,0},
  {ACSR,  0x08,0xDF,0xFF,0},
  {ADMUX, 0x07,0xFF,0xFF,0},
  {ADCSR, 0x06,0xFF,0xFF,0},
  {ADCH,  0x05,0x00,0xFF,0},
  {ADCL,  0x04,0x00,0xFF,0},
  {TWDR,  0x03,0xFF,0xFF,0xFF},
  {TWAR,  0x02,0xFF,0xFF,0xFE},
  {TWSR,  0x01,0x03,0xFB,0xF8},
  {TWBR,  0x00,0xFF,0xFF,0},
  {-1,0,0,0},
};

Pin atmega32_pins[]={
  {T0_PIN,  0x18,0x16,1<<0},
  {T1_PIN,  0x18,0x16,1<<1},
  {INT2_PIN,0x18,0x16,1<<2},{AIN0_PIN,0x18,0x16,1<<2},
  {OC0_PIN, 0x18,0x16,1<<3},{AIN1_PIN,0x18,0x16,1<<3},
  {SS_PIN,  0x18,0x16,1<<4},
  {MOSI_PIN,0x18,0x16,1<<5},
  {MISO_PIN,0x18,0x16,1<<6},
  {SCK_PIN, 0x18,0x16,1<<7},

  {RXD0_PIN,0x12,0x10,1<<0},
  {TXD0_PIN,0x12,0x10,1<<1},
  {INT0_PIN,0x12,0x10,1<<2},
  {INT1_PIN,0x12,0x10,1<<3},
  {OC1B_PIN,0x12,0x10,1<<4},
  {OC1A_PIN,0x12,0x10,1<<5},
  {IC1_PIN, 0x12,0x10,1<<6},
  {OC2_PIN, 0x12,0x10,1<<7},

  {SCL_PIN, 0x15,0x13,1<<0},
  {SDA_PIN, 0x15,0x13,1<<1},
  {TCK_PIN, 0x15,0x13,1<<2},
  {TMS_PIN, 0x15,0x13,1<<3},
  {TDO_PIN, 0x15,0x13,1<<4},
  {TDI_PIN, 0x15,0x13,1<<5},
  {TOSC1_PIN,0x15,0x13,1<<6},
  {TOSC2_PIN,0x15,0x13,1<<7},

  {ADC0_PIN,0x1B,0x19,1<<0},
  {ADC1_PIN,0x1B,0x19,1<<1},
  {ADC2_PIN,0x1B,0x19,1<<2},
  {ADC3_PIN,0x1B,0x19,1<<3},
  {ADC4_PIN,0x1B,0x19,1<<4},
  {ADC5_PIN,0x1B,0x19,1<<5},
  {ADC4_PIN,0x1B,0x19,1<<6},
  {ADC5_PIN,0x1B,0x19,1<<7},
  {-1,0,0,0}
};

Flag atmega32_flags[]={
  {IVCE_FLAG,  GICR,  1<<0},
  {IVSEL_FLAG, GICR,  1<<1},
  {INT2_FLAG,  GICR,  1<<5},
  {INT0_FLAG,  GICR,  1<<6},
  {INT1_FLAG,  GICR,  1<<7},

  {INTF2_FLAG, GIFR,  1<<5},
  {INTF0_FLAG, GIFR,  1<<6},
  {INTF1_FLAG, GIFR,  1<<7},

  {TOIE0_FLAG, TIMSK, 1<<0},
  {OCIE0_FLAG, TIMSK, 1<<1},
  {TOIE1_FLAG, TIMSK, 1<<2},
  {OCIE1B_FLAG,TIMSK, 1<<3},
  {OCIE1A_FLAG,TIMSK, 1<<4},
  {TICIE1_FLAG,TIMSK, 1<<5},
  {TOIE2_FLAG, TIMSK, 1<<6},
  {OCIE2_FLAG, TIMSK, 1<<7},
  
  {TOV0_FLAG,  TIFR,  1<<0},
  {OCF0_FLAG,  TIFR,  1<<1},
  {TOV1_FLAG,  TIFR,  1<<2},
  {OCF1B_FLAG, TIFR,  1<<3},
  {OCF1A_FLAG, TIFR,  1<<4},
  {ICF1_FLAG,  TIFR,  1<<5},
  {TOV2_FLAG,  TIFR,  1<<6},
  {OCF2_FLAG,  TIFR,  1<<7},

  {SPMEN_FLAG, SPMCR, 1<<0},
  {PGERS_FLAG, SPMCR, 1<<1},
  {PGWRT_FLAG, SPMCR, 1<<2},
  {BLBSET_FLAG,SPMCR, 1<<3},
  {RWWSRE_FLAG,SPMCR, 1<<4},
  {RWWSB_FLAG, SPMCR, 1<<6},
  {SPMIE_FLAG, SPMCR, 1<<7},
  
  {TWIE_FLAG,  TWCR,  1<<0},
  {TWEN_FLAG,  TWCR,  1<<2},
  {TWWC_FLAG,  TWCR,  1<<3},
  {TWSTO_FLAG, TWCR,  1<<4},
  {TWSTA_FLAG, TWCR,  1<<5},
  {TWEA_FLAG,  TWCR,  1<<6},
  {TWINT_FLAG, TWCR,  1<<7},

  {ISC00_FLAG, MCUCR, 1<<0},
  {ISC01_FLAG, MCUCR, 1<<1},
  {ISC10_FLAG, MCUCR, 1<<2},
  {ISC11_FLAG, MCUCR, 1<<3},
  {SM0_FLAG,   MCUCR, 1<<4},
  {SM1_FLAG,   MCUCR, 1<<5},
  {SM2_FLAG,   MCUCR, 1<<6},
  {SE_FLAG,    MCUCR, 1<<7},
 
  {PORF_FLAG,  MCUSR, 1<<0},   
  {EXTRF_FLAG, MCUSR, 1<<1},    
  {BORF_FLAG,  MCUSR, 1<<2},
  {WDRF_FLAG,  MCUSR, 1<<3},
  {JTRF_FLAG,  MCUSR, 1<<4},
  {ISC2_FLAG,  MCUSR, 1<<6},
  {JTD_FLAG,   MCUSR, 1<<7},

  {CS00_FLAG,  TCCR0, 1<<0},
  {CS01_FLAG,  TCCR0, 1<<1},
  {CS02_FLAG,  TCCR0, 1<<2},
  {WGM01_FLAG, TCCR0, 1<<3},
  {COM00_FLAG, TCCR0, 1<<4},
  {COM01_FLAG, TCCR0, 1<<5},
  {WGM00_FLAG, TCCR0, 1<<6},
  {FOC0_FLAG,  TCCR0, 1<<7},

  {PSR10_FLAG, SFIOR, 1<<0},
  {PSR2_FLAG,  SFIOR, 1<<1},
  {PUD_FLAG,   SFIOR, 1<<2},
  {ACME_FLAG,  SFIOR, 1<<3},
  {ADHSM_FLAG, SFIOR, 1<<4},
  {ADTS0_FLAG, SFIOR, 1<<5},
  {ADTS1_FLAG, SFIOR, 1<<6},
  {ADTS2_FLAG, SFIOR, 1<<7},

  {WGM10_FLAG, TCCR1A,1<<0},
  {WGM11_FLAG, TCCR1A,1<<1},
  {FOC1B_FLAG, TCCR1A,1<<2},
  {FOC1A_FLAG, TCCR1A,1<<3},
  {COM1B0_FLAG,TCCR1A,1<<4},
  {COM1B1_FLAG,TCCR1A,1<<5},
  {COM1A0_FLAG,TCCR1A,1<<6},
  {COM1A1_FLAG,TCCR1A,1<<7},

  {CS10_FLAG,  TCCR1B,1<<0},
  {CS11_FLAG,  TCCR1B,1<<1},
  {CS12_FLAG,  TCCR1B,1<<2},
  {WGM12_FLAG, TCCR1B,1<<3},
  {WGM13_FLAG, TCCR1B,1<<4},
  {ICES1_FLAG, TCCR1B,1<<6},
  {ICNC1_FLAG, TCCR1B,1<<7},

  {CS20_FLAG,  TCCR2, 1<<0},
  {CS21_FLAG,  TCCR2, 1<<1},
  {CS22_FLAG,  TCCR2, 1<<2},
  {WGM21_FLAG, TCCR2, 1<<3},
  {COM20_FLAG, TCCR2, 1<<4},
  {COM21_FLAG, TCCR2, 1<<5},
  {WGM20_FLAG, TCCR2, 1<<6},
  {FOC2_FLAG,  TCCR2, 1<<7},

  {TCR2UB_FLAG,ASSR,  1<<0},
  {OCR2UB_FLAG,ASSR,  1<<1},
  {TCN2UB_FLAG,ASSR,  1<<2},
  {AS2_FLAG,   ASSR,  1<<3},

  {WDP0_FLAG,  WDTCR, 1<<0},
  {WDP1_FLAG,  WDTCR, 1<<1},
  {WDP2_FLAG,  WDTCR, 1<<2},
  {WDE_FLAG,   WDTCR, 1<<3},
  {WDTOE_FLAG, WDTCR, 1<<4},

  {EERE_FLAG,  EECR,  1<<0},
  {EEWE_FLAG,  EECR,  1<<1},
  {EEMWE_FLAG, EECR,  1<<2},
  {EERIE_FLAG, EECR,  1<<3},

  {SPI2X_FLAG, SPSR,  1<<0},
  {WCOL_FLAG,  SPSR,  1<<6},
  {SPIF_FLAG,  SPSR,  1<<7},

  {SPR0_FLAG,  SPCR,  1<<0},
  {SPR1_FLAG,  SPCR,  1<<1},
  {CPHA_FLAG,  SPCR,  1<<2},
  {CPOL_FLAG,  SPCR,  1<<3},
  {MSTR_FLAG,  SPCR,  1<<4},
  {DORD_FLAG,  SPCR,  1<<5},
  {SPE_FLAG,   SPCR,  1<<6},
  {SPIE_FLAG,  SPCR,  1<<7},

  {MPCM0_FLAG,  UCSR0A,1<<0},
  {U2X0_FLAG,   UCSR0A,1<<1},
  {PE0_FLAG,    UCSR0A,1<<2},
  {DOR0_FLAG,   UCSR0A,1<<3},
  {FE0_FALG,    UCSR0A,1<<4},
  {UDRE0_FLAG,  UCSR0A,1<<5},
  {TXC0_FLAG,   UCSR0A,1<<6},
  {RXC0_FLAG,   UCSR0A,1<<7},

  {TXB80_FLAG,  UCSR0B,1<<0},
  {RXB80_FLAG,  UCSR0B,1<<1},
  {UCSZ02_FLAG, UCSR0B,1<<2},
  {TXEN0_FLAG,  UCSR0B,1<<3},
  {RXEN0_FLAG,  UCSR0B,1<<4},
  {UDRIE0_FLAG, UCSR0B,1<<5},
  {TXCIE0_FLAG, UCSR0B,1<<6},
  {RXCIE0_FLAG, UCSR0B,1<<7},

  {UCPOL0_FLAG, UCSR0C,1<<0},
  {UCSZ00_FLAG, UCSR0C,1<<1},
  {UCSZ01_FLAG, UCSR0C,1<<2},
  {USBS0_FLAG,  UCSR0C,1<<3},
  {UPM00_FLAG,  UCSR0C,1<<4},
  {UPM01_FLAG,  UCSR0C,1<<5},
  {UMSEL0_FLAG, UCSR0C,1<<6},
  {URSEL0_FLAG, UCSR0C,1<<7},

  {ACIS0_FLAG, ACSR,  1<<0},
  {ACIS1_FLAG, ACSR,  1<<1},
  {ACIC_FLAG,  ACSR,  1<<2},
  {ACIE_FLAG,  ACSR,  1<<3},
  {ACI_FLAG,   ACSR,  1<<4},
  {ACO_FLAG,   ACSR,  1<<5},
  {ACBG_FLAG,  ACSR,  1<<6},
  {ACD_FLAG,   ACSR,  1<<7},

  {MUX0_FLAG,  ADMUX, 1<<0},
  {MUX1_FLAG,  ADMUX, 1<<1},
  {MUX2_FLAG,  ADMUX, 1<<2},
  {MUX3_FLAG,  ADMUX, 1<<3},
  {MUX4_FLAG,  ADMUX, 1<<4},
  {ADLAR_FLAG, ADMUX, 1<<5},
  {REFS0_FLAG, ADMUX, 1<<6},
  {REFS1_FLAG, ADMUX, 1<<7},

  {ADPS0_FLAG, ADCSR, 1<<0},
  {ADPS1_FLAG, ADCSR, 1<<1},
  {ADPS2_FLAG, ADCSR, 1<<2},
  {ADIE_FLAG,  ADCSR, 1<<3},
  {ADIF_FLAG,  ADCSR, 1<<4},
  {ADATE_FLAG, ADCSR, 1<<5},
  {ADSC_FLAG,  ADCSR, 1<<6},
  {ADEN_FLAG,  ADCSR, 1<<7},
  
  {TWGCE_FLAG, TWAR,  1<<0},
  {-1,-1}
};
