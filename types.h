#include <inttypes.h>

#define UNPROGRAMMED 0
#define PROGRAMMED 1

/* return port address */
#define IO_ADDRESS(port) io_port[index_ports[port]].address
#define ISPORT(port) (index_ports[port]!=-1)
#define ISFLAG(flag) (index_flags[flag]!=-1)
#define ISPIN(pin)  (index_pins[pin]!=-1)

/* structure of AVR command */
typedef struct {
  char mnemocode[16]; 
  int code;
  int mask;
  unsigned char num_words;
  int (*arg1)(char *mnemocode,int command,int address);
  int (*arg2)(char *mnemocode,int command,int address);
  void (*command)(void);
} Command;

/* structure of listing */
typedef struct {
  char mnemocode[20];
  char comment[6];
  void (*function_command)(void);
  int arg1,arg2;
  int flags;
  int number_line;
  int code;
  unsigned char num_words;
} ListCommand;

/* I/O ports */
typedef struct {
  int code,address,write_mask,read_mask,init_value;
} IO_port;

enum{LOW,HIGH};
/* 16-bit I/O ports */
typedef struct {
  int code,type;
  uint8_t *temp;
} IO_port16;

/* structure of pin's definition */
typedef struct {
  int code,data_address,pins_address,mask;
} Pin;

/* used in load and save ports functions */
typedef struct {
  int data,direct,pins;
} Ports;

/* structure of flag mask */
typedef struct {
  int flag_code,port_code,flag_mask;
} Flag;

/* structure of microcontrollers */
typedef struct {
  char *name;
  int size_gpr,size_io_reg,size_sram;
  int size_flash; /* size in words*/
  int size_eeprom;
  int *interrupts;
  IO_port *io_registers;
  Pin *pins;
  Flag *flags;
} Microcontroller;

/* definition of I/O ports */
typedef struct {
  char *name;
  char *fullname;
} Descr_io_port;

/* definition of button */
typedef struct {
  char *name;                 /* Label                          */
  void (*callback)();         /* callback function              */
  void *call_data;            /* call_data                      */
  char *name_pixmap;          /* filename of pixmap             */
  char *prompt_str;           /* */
  Widget widget;
  Pixmap pixmap;
} Buttons;

/* The following structure is used for menu construction */
enum{BUTTON=0,CHECK,RADIO,SEPARATOR};
typedef struct _Menu{
  int     type;
  char    *name;          /* Used as the menu item name             */
  char    *mnemonic;      /* If non-null Use first char as mnemonic */
  char    *accelerator_text;
  char    *accelerator;   /* An accelerator                         */
  struct _Menu  *sub_menu;/* sub menu                               */
  void    (*callback)();  /* Function to call when selected         */
  void    *data;          /* Call data for callback function        */
  Widget  button;
  Widget  pulldown;
} Menu;

/* struct of scroll window */
typedef struct {
  Widget draw,v_scrb;
  Window window;
  Pixmap pixmap,stop_pixmap,current_pixmap;
  int font_width,font_height;
  GC gc,gc2;
  int num_commands,current_line_deb;
  Dimension height,width;
  int *display_lines;     /* address of command */
  Pixel background; 
} DrawArea;

/* widgets of port_window */
typedef struct{
  Widget button1,data,button2;
} Line_port_window;

/* widget's data of port_window */
typedef struct{
  char state;
  char *filename;
  FILE *file;
} Data_port_window;

typedef struct{
  Widget draw,vscr_bar,label_widget,address_widget,data_widget,chars_widget,
    address_text_widget,data_type_widget;
  Dimension height,width;
  Pixmap label,address,data,chars;
  GC gc,gc_new,label_gc,address_gc,data_gc,chars_gc;
  int font_width,font_height;
  Window window,label_window,address_window,data_window,chars_window;
  int num_lines,cur_line,max_lines;
  uint8_t *pdata;          /* data to display */
  int data_size;           /* size of date */
  int data_type;           /* HEX or DEC */
  int data_offset;
  int memory_type;
  Pixel label_bg,address_bg,data_bg,chars_bg;
  uint8_t *mem_changed;
  uint32_t mem_offset;
} MemoryWindowDrawArea;

enum{DEC=0,HEX=1,OCT,BIN};
/* memory's types */
enum{M_REGISTERS,M_IO_REGISTERS,M_EX_IO_REGISTERS,M_SRAM,M_DATA,M_EEPROM,
       M_FLASH};


#define uS  0
#define mS  1
#define S   2

#define BREAKPOINT 1
#define FLASH 0
#define EEPROM 1

/* debugger's flags */
#define DEBUG_INTERRUPT 1

/* interrupts definition */

enum{
  RESET,           /* Hardware Pin,Power-on Reset and Watchdog Reset */
    INT0,            /* External Interrupt Request 0      */
    INT1,            /* External Interrupt Request 1      */
    INT2,            /* External Interrupt Request 2      */
    INT3,            /* External Interrupt Request 3      */
    INT4,            /* External Interrupt Request 4      */
    INT5,            /* External Interrupt Request 5      */
    INT6,            /* External Interrupt Request 6      */
    INT7,            /* External Interrupt Request 7      */
    PCINT0,          /* Pin Change Interrupt Request 0    */
    PCINT1,          /* Pin Change Interrupt Request 1    */
    TIMER3_CAPT,     /* Timer/Counter3 Capture Event      */
    TIMER3_COMPA,    /* Timer/Counter3 Campare Match A    */
    TIMER3_COMPB,    /* Timer/Counter3 Campare Match B    */
    TIMER3_COMPC,    /* Timer/Counter3 Campare Match C    */
    TIMER3_OVF,      /* Timer/Counter3 Overflow           */
    TIMER2_COMP,     /* Timer/Counter2 Campare Match      */
    TIMER2_OVF,      /* Timer/Counter2 Overflow           */
    TIMER1_CAPT,     /* Timer/Counter1 Capture Event      */
    TIMER1_COMPA,    /* Timer/Counter1 Campare Match A    */
    TIMER1_COMPB,    /* Timer/Counter1 Campare Match B    */
    TIMER1_COMPC,    /* Timer/Counter1 Campare Match C    */
    TIMER1_OVF,      /* Timer/Counter1 Overflow           */
    TIMER0_COMP,     /* Timer/Counter0 Campare Match      */
    TIMER0_OVF,      /* Timer/Counter0 Overflow           */
    SPI_STC,         /* SPI Serial Transfer Complete      */
    UART0_RX,        /* UART0, RX Complete                */
    UART0_UDRE,      /* UART0, Data Register Empty        */
    UART0_TX,        /* UART0, TX Complete                */
    UART1_RX,        /* UART1, RX Complete                */
    UART1_UDRE,      /* UART1, Data Register Empty        */
    UART1_TX,        /* UART1, TX Complete                */
    ADC,             /* ADC Conversion Complete           */
    EE_READY,        /* EEPROM Ready                      */
    ANALOG_COMP,     /* Analog Comparator                 */
    TWSI,            /* 2-wire Serial Interface           */
    SPM_RDY,         /* Store Program Memory Ready        */
    MAX_CODE_INT
    };
    

/* definition of I/O ports */
enum{ 
  UBRR1L=0,       /* UART1 Baud Rate Register Low      */
    UBRR1H,       /* UART1 Baud Rate Register High     */
    UCSR1C,       /* UART1 Control Register and Status Register */
    UCSR1B,       /* UART1 Control Register            */
    UCSR1A,       /* UART1 Status Register             */
    UDR1,         /* UART1 I/O Data Register Low       */
    UBRR0L,       /* UART0 Baud Rate Register          */
    UBRR0H,       /* UART0 Baud Register High          */
    UCSR0C,       /* UART0 Control Register and Status Register */
    UCSR0B,       /* UART0 Control Register            */
    UCSR0A,       /* UART0 Status Register             */
    UDR0,         /* UART0 I/O Data Register           */
    UBRRHI,       /* UART Baud Register High           */

    PING,         /* Input Pins, Port G                */
    DDRG,         /* Data Direction Register, Port G   */
    PORTG,        /* Data Register, Port G             */
    PINF,         /* Input Pins, Port F                */
    DDRF,         /* Data Direction Register, Port F   */
    PORTF,        /* Data Register, Port F             */
    PINE,         /* Input Pins, Port E                */
    DDRE,         /* Data Direction Register, Port E   */
    PORTE,        /* Data Register, Port E             */
    PIND,         /* Input Pins, Port D                */
    DDRD,         /* Data Direction Register, Port D   */
    PORTD,        /* Data Register, Port D             */
    PINC,         /* Input Pins, Port C                */
    DDRC,         /* Data Direction Register, Port C   */
    PORTC,        /* Data Register, Port C             */
    PINB,         /* Input Pins, Port B                */
    DDRB,         /* Data Direction Register, Port B   */
    PORTB,        /* Data Register, Port B             */
    PINA,         /* Input Pins, Port A                */
    DDRA,         /* Data Direction Register, Port A   */
    PORTA,        /* Data Register, Port A             */
    
    ACSR,         /* Analog Comparator Control and Status Register */
   
    SPCR,         /* SPI Control Register              */
    SPSR,         /* SPI Status Register               */
    SPDR,         /* SPI I/O Data Register             */
   
    EECR,         /* EEPROM Control Register           */
    EEDR,         /* EEPROM Data Register              */
    EEARL,        /* EEPROM Address Register Low       */
    EEARH,        /* EEPROM Address Register High      */

    WDTCR,        /* Watchdog Timer Control Register   */

    ICR3L,        /* Timer/Counter3 Input Capture Register Low Byte */
    ICR3H,        /* Timer/Counter3 Input Capture Register High Byte */
    OCR3CL,       /* Timer/Counter3 Output Capture RegisterC Low Byte */
    OCR3CH,       /* Timer/Counter3 Output Capture RegisterC High Byte */
    OCR3BL,       /* Timer/Counter3 Output Capture RegisterB Low Byte */
    OCR3BH,       /* Timer/Counter3 Output Capture RegisterB High Byte */
    OCR3AL,       /* Timer/Counter3 Output Capture RegisterA Low Byte */
    OCR3AH,       /* Timer/Counter3 Output Capture RegisterA High Byte */
    TCNT3L,       /* Timer/Counter3 Low Byte                */
    TCNT3H,       /* Timer/Counter3 High Byte               */
    TCCR3C,       /* Timer/Counter3 Control Register C      */
    TCCR3B,       /* Timer/Counter3 Control Register B      */
    TCCR3A,       /* Timer/Counter3 Control Register A      */
    OCR2,         /* Timer/Counter2 Output Compare Register */
    TCNT2,        /* Timer/Counter2 (8-bit)            */
    TCCR2,        /* Timer/Counter2 Control Register   */
    ICR1L,        /* Timer/Counter1 Input Capture Register Low Byte */
    ICR1H,        /* Timer/Counter1 Input Capture Register High Byte */
    OCR1CL,       /* Timer/Counter1 Output Capture RegisterC Low Byte */
    OCR1CH,       /* Timer/Counter1 Output Capture RegisterC High Byte */
    OCR1BL,       /* Timer/Counter1 Output Capture RegisterB Low Byte */
    OCR1BH,       /* Timer/Counter1 Output Capture RegisterB High Byte */
    OCR1AL,       /* Timer/Counter1 Output Capture RegisterA Low Byte */
    OCR1AH,       /* Timer/Counter1 Output Capture RegisterA High Byte */
    TCNT1L,       /* Timer/Counter1 Low Byte                */
    TCNT1H,       /* Timer/Counter1 High Byte               */
    TCCR1C,       /* Timer/Counter1 Control Register C      */
    TCCR1B,       /* Timer/Counter1 Control Register B      */
    TCCR1A,       /* Timer/Counter1 Control Register A      */
    OCR0,         /* Timer/Counter0 Output Compare Register */
    TCNT0,        /* Timer/Counter0 (8-bit)                 */
    TCCR0,        /* Timer/Counter0 Control Register        */
    ASSR,         /* Asynchronous mode Status Register      */
    SFIOR,        /* Special Function IO Register           */
    CLKPR,        /* Clock Prescale Register */

    MCUSR,        /* MCU general Status Regsiter            */
    MCUCR,        /* MCU general Control Regsiter           */
    EMCUCR,       /* Extended MCU general Control Register  */

    SPMCR,        /* Store Program Memory Control Register  */
    TIFR ,        /* Timer/Counter Interrupt Flag Register  */
    TIMSK,        /* Timer/Counter Interrupt Mask Register  */
    ETIMSK,       /* Extended Timer/Counter Interrupt Mask Register */
    ETIFR,        /* Extended Timer/Counter Interrupt Flag Register */
    GIFR,         /* General Interrupt Flag Register        */
    GIMSK,        /* General Interrupt Mask Register        */
    GICR,         /* General Interrupt Control Register     */
    PCMSK0,       /* Pin Change Mask Register 0             */
    PCMSK1,       /* Pin Change Mask Register 1             */
    SPL,          /* Stack Pointer Low                      */
    SPH,          /* Stack Pointer High                     */
    SREG,         /* Status Register                        */
    RAMPZ,        /* RAM Page Z Select Register             */
    XDIV,         /* XTAL Divide Control Register           */

    EICR,         /* External Interrupt Control Register    */
    EICRB,        /* External Interrupt Control Register B  */
    EICRA,        /* External Interrupt Control Register A  */
    EIMSK,        /* External Interrupt Mask Register       */
    EIFR,         /* External Interrupt Flag Register       */

    ADMUX,        /* ADC Multiplexer Select Register        */
    ADCSR,        /* ADC Control and Status Register        */
    ADCSRA,       /* ADC Control and Status Register A      */
    ADCH,         /* ADC Data Register High                 */
    ADCL,         /* ADC Data Register Low                  */

    TWCR,         /* 2-wire Serial Interface Control Register */
    TWDR,         /* 2-wire Serial Interface Data Register  */
    TWAR,         /* 2-wire Serial Interface (Slave) Address Register */
    TWSR,         /* 2-wire Serial Interface Status Register */
    TWBR,         /* 2-wire Serial Interface Bit Rate Register */

    OSCCAL,       /* Oscillator Calibration Register        */
    OCDR,         /* On-Chip Debug Related Register         */

    XMCRB,        /* External Memory Control Register B     */
    XMCRA,        /* External Memory Control Register A     */
    MAX_CODE_PORTS,
    };

/* Flags in SREG */
#define FLAG_C 1<<0
#define FLAG_Z 1<<1
#define FLAG_N 1<<2
#define FLAG_V 1<<3
#define FLAG_S 1<<4
#define FLAG_H 1<<5
#define FLAG_T 1<<6
#define FLAG_I 1<<7

/* pins code definition */
enum{
  /* External Interrupts */
  INT0_PIN=1,INT1_PIN,INT2_PIN,INT3_PIN,INT4_PIN,INT5_PIN,INT6_PIN,INT7_PIN,
    PCINT0_PIN,PCINT1_PIN,PCINT2_PIN,PCINT3_PIN,PCINT4_PIN,PCINT5_PIN,
    PCINT6_PIN,PCINT7_PIN,PCINT8_PIN,PCINT9_PIN,PCINT10_PIN,PCINT11_PIN,
    PCINT12_PIN,PCINT13_PIN,PCINT14_PIN,PCINT15_PIN,
    AIN0_PIN,AIN1_PIN,  /* Analog Comparator */
    T0_PIN,T1_PIN,T2_PIN,T3_PIN,
    RXD0_PIN,TXD0_PIN,RXD1_PIN,TXD1_PIN,
    IC1_PIN,IC3_PIN,
    OC0_PIN,OC1A_PIN,OC1B_PIN,OC1C_PIN,OC2_PIN,OC3A_PIN,OC3B_PIN,OC3C_PIN,
    TOSC1_PIN,TOSC2_PIN,
    PWM0_PIN,PWM1A_PIN,PWM1B_PIN,PWM2_PIN,
    MOSI_PIN,MISO_PIN,
    SS_PIN,SCK_PIN,
    ADC0_PIN,ADC1_PIN,ADC2_PIN,ADC3_PIN,ADC4_PIN,ADC5_PIN,ADC6_PIN,ADC7_PIN,
    ADC8_PIN,ADC9_PIN,
    WR_PIN,RD_PIN,
    SDA_PIN,SCL_PIN,
    ALE_PIN,
    TCK_PIN,TMS_PIN,TDO_PIN,TDI_PIN,
    PDI_PIN,PDO_PIN,
    MAX_CODE_PINS};

/* FLAGS code definition */
enum{
  INT0_FLAG=0, /* External Interrupt Enable */
    INT1_FLAG,
    INT2_FLAG,
    INT3_FLAG,
    INT4_FLAG,
    INT5_FLAG,
    INT6_FLAG,
    INT7_FLAG,
    IVCE_FLAG, /* GICR */
    IVSEL_FLAG,
    PCIE0_FLAG,
    PCIE1_FLAG,

    INTF0_FLAG, /* External Interrupt Flag */
    INTF1_FLAG,
    INTF2_FLAG,
    INTF3_FLAG,
    INTF4_FLAG,
    INTF5_FLAG,
    INTF6_FLAG,
    INTF7_FLAG,
    PCIF0_FLAG,
    PCIF1_FLAG,
    
    ISC00_FLAG, /* External Interrupt Control */
    ISC01_FLAG,
    ISC10_FLAG,
    ISC11_FLAG,
    ISC20_FLAG,
    ISC21_FLAG,
    ISC30_FLAG,
    ISC31_FLAG,
    ISC40_FLAG, 
    ISC41_FLAG,
    ISC50_FLAG,
    ISC51_FLAG,
    ISC60_FLAG,
    ISC61_FLAG,
    ISC70_FLAG,
    ISC71_FLAG,

    PCINT0_FLAG,
    PCINT1_FLAG,
    PCINT2_FLAG,
    PCINT3_FLAG,
    PCINT4_FLAG,
    PCINT5_FLAG,
    PCINT6_FLAG,
    PCINT7_FLAG,
    PCINT8_FLAG,
    PCINT9_FLAG,
    PCINT10_FLAG,
    PCINT11_FLAG,
    PCINT12_FLAG,
    PCINT13_FLAG,
    PCINT14_FLAG,
    PCINT15_FLAG,

    TOIE0_FLAG,  /* Timer interrupt enabling flags */
    OCIE0_FLAG,
    TICIE1_FLAG,
    OCIE1A_FLAG,
    OCIE1B_FLAG,
    OCIE1C_FLAG,
    TOIE1_FLAG,
    TOIE2_FLAG,
    OCIE2_FLAG,
    TOIE3_FLAG,
    OCIE3C_FLAG,
    OCIE3B_FLAG,
    OCIE3A_FLAG,
    TICIE3_FLAG,

    TOV0_FLAG,
    OCF0_FLAG,
    TOV1_FLAG,
    ICF1_FLAG,
    OCF1A_FLAG,
    OCF1B_FLAG,
    OCF1C_FLAG,
    TOV2_FLAG,
    OCF2_FLAG,
    TOV3_FLAG,
    OCF3A_FLAG,
    OCF3B_FLAG,
    OCF3C_FLAG,
    ICF3_FLAG,

    CLKPS0_FLAG,
    CLKPS1_FLAG,
    CLKPS2_FLAG,
    CLKPS3_FLAG,
    CLKPCE_FLAG,

    SPMEN_FLAG, /* SPMCR */
    PGERS_FLAG,
    PGWRT_FLAG,
    BLBSET_FLAG,
    LBSET_FLAG,
    ASRE_FLAG,
    ASB_FLAG,
    RWWSRE_FLAG,
    RWWSB_FLAG,
    SPMIE_FLAG,

    TWIE_FLAG, /* TWCR */
    TWEN_FLAG,
    TWWC_FLAG,
    TWSTO_FLAG,
    TWSTA_FLAG,
    TWEA_FLAG,
    TWINT_FLAG,

    SM0_FLAG, /* MCUCR */
    SM1_FLAG,
    SM2_FLAG,
    SE_FLAG,
    SRW_FLAG,
    SRE_FLAG,

    PORF_FLAG, /* MCUSR */
    EXTRF_FLAG,
    BORF_FLAG,
    WDRF_FLAG,
    JTRF_FLAG,
    ISC2_FLAG,
    JTD_FLAG,

    CS00_FLAG, /* TCCR0 */
    CS01_FLAG,
    CS02_FLAG,
    CTC0_FLAG,
    COM00_FLAG,
    COM01_FLAG,
    PWM0_FLAG,
    FOC0_FLAG,
    WGM00_FLAG,
    WGM01_FLAG,

    PSR10_FLAG, /* SFIOR */
    PSR0_FLAG,
    PSR2_FLAG,
    PSR321_FLAG,
    PSR310_FLAG,
    PUD_FLAG,
    ACME_FLAG,
    ADHSM_FLAG,
    ADTS0_FLAG,
    ADTS1_FLAG,
    ADTS2_FLAG,
    TSM_FLAG,

    PWM10_FLAG, /* TCCR1A */
    PWM11_FLAG,
    FOC1A_FLAG,
    FOC1B_FLAG,
    FOC1C_FLAG,
    COM1C0_FLAG,
    COM1C1_FLAG,
    COM1B0_FLAG,
    COM1B1_FLAG,
    COM1A0_FLAG,
    COM1A1_FLAG,
    WGM10_FLAG,
    WGM11_FLAG,

    CS10_FLAG,  /* TCCR1B */
    CS11_FLAG,
    CS12_FLAG,
    CTC1_FLAG,
    ICES1_FLAG,
    ICNC1_FLAG,
    WGM12_FLAG,
    WGM13_FLAG,

    CS20_FLAG, /* TCCR2 */
    CS21_FLAG,
    CS22_FLAG,
    CTC2_FLAG,
    COM20_FLAG,
    COM21_FLAG,
    PWM2_FLAG,
    FOC2_FLAG,
    WGM21_FLAG,
    WGM20_FLAG,

    PWM30_FLAG, /* TCCR3A */
    PWM31_FLAG,
    FOC3A_FLAG,
    FOC3B_FLAG,
    FOC3C_FLAG,
    COM3C0_FLAG,
    COM3C1_FLAG,
    COM3B0_FLAG,
    COM3B1_FLAG,
    COM3A0_FLAG,
    COM3A1_FLAG,
    WGM30_FLAG,
    WGM31_FLAG,

    CS30_FLAG,  /* TCCR3B */
    CS31_FLAG,
    CS32_FLAG,
    CTC3_FLAG,
    ICES3_FLAG,
    ICNC3_FLAG,
    WGM32_FLAG,
    WGM33_FLAG, 
    

    TCR2UB_FLAG, /* ASSR */
    OCR2UB_FLAG,
    TCN2UB_FLAG,
    AS2_FLAG,
    TCR0UB_FLAG,
    OCR0UB_FLAG,
    TCN0UB_FLAG,
    AS0_FLAG,

    WDP0_FLAG, /* WDTCR */
    WDP1_FLAG,
    WDP2_FLAG,
    WDE_FLAG,
    WDTOE_FLAG,
    WDCE_FLAG,
    
    EERE_FLAG, /* EECR */
    EEWE_FLAG,
    EEMWE_FLAG,
    EERIE_FLAG,

    SPI2X_FLAG,/* SPSR */
    WCOL_FLAG, 
    SPIF_FLAG,
    
    SPR0_FLAG, /* SPCR */
    SPR1_FLAG,
    CPHA_FLAG,
    CPOL_FLAG,
    MSTR_FLAG,
    DORD_FLAG,
    SPE_FLAG,
    SPIE_FLAG,

    MPCM0_FLAG, /* UCSR0A */
    U2X0_FLAG,
    PE0_FLAG,
    OR0_FLAG,
    DOR0_FLAG,
    FE0_FALG,
    UDRE0_FLAG,
    TXC0_FLAG,
    RXC0_FLAG,

    MPCM1_FLAG, /* UCSR1A */
    U2X1_FLAG,
    PE1_FLAG,
    OR1_FLAG,
    DOR1_FLAG,
    FE1_FALG,
    UDRE1_FLAG,
    TXC1_FLAG,
    RXC1_FLAG,

    TXB80_FLAG, /* UCSR0B */
    RXB80_FLAG,
    CHR90_FLAG,
    UCSZ02_FLAG,
    TXEN0_FLAG,
    RXEN0_FLAG,
    UDRIE0_FLAG,
    TXCIE0_FLAG,
    RXCIE0_FLAG,

    TXB81_FLAG, /* UCSR1B */
    RXB81_FLAG,
    CHR91_FLAG,
    UCSZ12_FLAG,
    TXEN1_FLAG,
    RXEN1_FLAG,
    UDRIE1_FLAG,
    TXCIE1_FLAG,
    RXCIE1_FLAG,

    UCPOL0_FLAG, /* UCSR0C */
    UCSZ00_FLAG,
    UCSZ01_FLAG,
    USBS0_FLAG,
    UPM00_FLAG,
    UPM01_FLAG,
    UMSEL0_FLAG,
    URSEL0_FLAG,

    UCPOL1_FLAG, /* UCSR1C */
    UCSZ10_FLAG,
    UCSZ11_FLAG,
    USBS1_FLAG,
    UPM10_FLAG,
    UPM11_FLAG,
    UMSEL1_FLAG,
    URSEL1_FLAG,

    ACIS0_FLAG, /* ACSR */
    ACIS1_FLAG,
    ACIC_FLAG,
    ACIE_FLAG,
    ACI_FLAG,
    ACO_FLAG,
    AINBG_FLAG,
    ACBG_FLAG,
    ACD_FLAG,

    MUX0_FLAG, /* ADMUX */
    MUX1_FLAG,
    MUX2_FLAG,
    MUX3_FLAG,
    MUX4_FLAG,
    ADLAR_FLAG,
    REFS0_FLAG,
    REFS1_FLAG,
    ADCBG_FLAG,

    ADPS0_FLAG, /* ADCSR */
    ADPS1_FLAG,
    ADPS2_FLAG,
    ADIE_FLAG,
    ADIF_FLAG,
    ADFR_FLAG,
    ADSC_FLAG,
    ADEN_FLAG,
    ADATE_FLAG,

    TWGCE_FLAG, /* TWAR */

    SRW11_FLAG, /* XMCRA */
    SRW00_FLAG,
    SRW01_FLAG,
    SRW10_FLAG,
    SRL0_FLAG,
    SRL1_FLAG,
    SRL2_FLAG,

    XMM0_FLAG, /* XMCRB */
    XMM1_FLAG,
    XMM2_FLAG,
    XMBK_FLAG,
    MAX_CODE_FLAGS};
