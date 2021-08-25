extern int watchdog_prescaler;
extern unsigned char *mem_file_changed;


extern void fill_zero(char *,int);
extern void set_sreg_flag(int f);
extern void clear_sreg_flag(int f);
extern int  get_sreg_flag(int f);
extern uint8_t add_port(int p,uint8_t add);
extern uint8_t get_port(int port);
extern void set_port(int p,uint8_t value);
extern void set_port16(int low,int hi,uint16_t value);
extern uint16_t get_port16(int low,int hi);
extern uint16_t add_port16(int low,int hi,uint16_t value);
