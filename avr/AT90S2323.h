int at90s2323_int[]={RESET,INT0,TIMER0_OVF,-1};

IO_port at90s2323_io[]={
  {SREG, 0x3F,0xFF,0xFF,0},
  {SPL,  0x3D,0xFF,0xFF,0},
  {GIMSK,0x3B,0x40,0x40,0},
  {GIFR, 0x3A,0x40,0x40,0},
  {TIMSK,0x39,0x02,0x02,0},
  {TIFR, 0x38,0x02,0x02,0},
  {MCUCR,0x35,0x33,0x33,0},
  {MCUSR,0x34,0x03,0x03,0},
  {TCCR0,0x33,0x07,0x07,0},
  {TCNT0,0x32,0xFF,0xFF,0},
  {WDTCR,0x21,0x1F,0x1F,0},
  {EEARL,0x1E,0x7F,0x7F,0},
  {EEDR, 0x1D,0xFF,0xFF,0},
  {EECR, 0x1C,0x07,0x07,0},
  {PORTB,0x18,0x07,0x07,0},
  {DDRB, 0x17,0x07,0x07,0},
  {PINB, 0x16,0x00,0x07,0},
  {-1,0,0,0}
};

Pin at90s2323_pins[]={
  {T0_PIN,  0x18,0x16,1<<2},
  {INT0_PIN,0x18,0x16,1<<1},
  {-1,0,0,0}
};

Flag at90s2323_flags[]={
  {INT0_FLAG, GIMSK, 1<<6},

  {INTF0_FLAG,GIFR,  1<<6},

  {TOIE0_FLAG,TIMSK, 1<<1},

  {TOV0_FLAG, TIFR,  1<<1},

  {ISC00_FLAG,MCUCR, 1<<0},
  {ISC01_FLAG,MCUCR, 1<<1},
  {SM0_FLAG,  MCUCR, 1<<4},
  {SE_FLAG,   MCUCR, 1<<5},

  {PORF_FLAG, MCUSR, 1<<0},
  {EXTRF_FLAG,MCUSR, 1<<1},

  {CS00_FLAG, TCCR0, 1<<0},
  {CS01_FLAG, TCCR0, 1<<1},
  {CS02_FLAG, TCCR0, 1<<2},

  {WDP0_FLAG, WDTCR, 1<<0},
  {WDP1_FLAG, WDTCR, 1<<1},
  {WDP2_FLAG, WDTCR, 1<<2},
  {WDE_FLAG,  WDTCR, 1<<3},
  {WDTOE_FLAG,WDTCR, 1<<4},

  {EERE_FLAG, EECR,  1<<0},
  {EEWE_FLAG, EECR,  1<<1},
  {EEMWE_FLAG,EECR,  1<<2},
  {-1,-1}
};
