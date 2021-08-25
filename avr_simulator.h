extern void xs_widget_tip(Widget w, char *tipStr);
extern void create_buttons(Widget parent);
extern void load_eeprom_callback();
extern void load_flash_callback();
extern void create_menu();
extern void create_ports_window();
extern void create_debugger_window();
extern void init_ports_window();
extern int interrupt();
extern void create_project_selection();
extern void create_about();
extern void create_gpr_window();
extern void create_memory_window();
extern void create_io_ports_window();
extern void update_gpr_window();
extern void full_update_io_window();
extern void full_update_memory_window(void);
extern void init_windows_memory();
extern void init_memory_window(void);
extern void hardware_init(void);
extern uint8_t get_port(int port);

extern void SetSensitiveMenu(char *name,Boolean sen);
extern void SetSensitiveToolbar(char *name,Boolean sen);
extern void change_sensitive_run(Boolean sen);

extern Menu menu[];
extern Widget freq_field,watchdog_freq_field,combo_proc,project_window;
extern int wtd_freq,type_micro_old,wacthdog_timeout_temp,wacthdog_timeout;
extern int watchdog_prescaler;
extern unsigned char *mem_file_changed;
extern int debug_flag,thread_stop;

extern void adc_command(void);
extern void add_command(void);
extern void adiw_command(void);
extern void and_command(void);
extern void andi_command(void);
extern void asr_command(void);
extern void bclr_command(void);
extern void bld_command(void);
extern void brbc_command(void);
extern void brbs_command(void);
extern void bset_command(void);
extern void bst_command(void);
extern void call_command(void);
extern void cbi_command(void);
extern void com_command(void);
extern void cp_command(void);
extern void cpc_command(void);
extern void cpi_command(void);
extern void cpse_command(void);
extern void dec_command(void);
extern void elpm1_command(void);
extern void elpm2_command(void);
extern void elpm3_command(void);
extern void eor_command(void);
extern void fmul_command(void);
extern void fmuls_command(void);
extern void fmulsu_command(void);
extern void icall_command(void);
extern void ijmp_command(void);
extern void in_command(void);
extern void inc_command(void);
extern void jmp_command(void);
extern void ld_x_command(void);
extern void ld_x_plus_command(void);
extern void ld_minus_x_command(void);
extern void ld_y_command(void);
extern void ld_y_plus_command(void);
extern void ld_minus_y_command(void);
extern void ld_z_command(void);
extern void ld_z_plus_command(void);
extern void ld_minus_z_command(void);
extern void ldd_y_command(void);
extern void ldd_z_command(void);
extern void ldi_command(void);
extern void lds_command(void);
extern void lpm_command(void);
extern void lpm_z_command(void);
extern void lpm_z_plus_command(void);
extern void lsr_command(void);
extern void mov_command(void);
extern void movw_command(void);
extern void mul_command(void);
extern void muls_command(void);
extern void mulsu_command(void);
extern void neg_command(void);
extern void nop_command(void);
extern void or_command(void);
extern void ori_command(void);
extern void out_command(void);
extern void pop_command(void);
extern void push_command(void);
extern void rcall_command(void);
extern void ret_command(void);
extern void reti_command(void);
extern void rjmp_command(void);
extern void ror_command(void);
extern void sbc_command(void);
extern void sbci_command(void);
extern void sbi_command(void);
extern void sbic_command(void);
extern void sbis_command(void);
extern void sbiw_command(void);
extern void sbrc_command(void);
extern void sbrs_command(void);
extern void sleep_command(void);
extern void st_x_command(void);
extern void st_x_plus_command(void);
extern void st_minus_x_command(void);
extern void st_y_command(void);
extern void st_y_plus_command(void);
extern void st_minus_y_command(void);
extern void std_y_commad(void);
extern void st_z_command(void);
extern void st_z_plus_command(void);
extern void st_minus_z_command(void);
extern void std_z_commad(void);
extern void sts_command(void);
extern void sub_command(void);
extern void subi_command(void);
extern void swap_command(void);
extern void wdr_command(void);

extern int reg_d5(char *,int,int);       /* 0000000d dddd0000 destantion  */
extern int reg_s5(char *,int,int);       /* 000000d0 0000dddd source      */ 
extern int const_6(char *,int,int);      /* 00000000 dd00dddd const       */
extern int bit_sreg(char *,int,int);     /* 00000000 0ddd0000             */
extern int bit_sreg2(char *,int,int);    /* 00000000 00000ddd             */
extern int bit_num2(char *,int,int);     /* 00000000 00000ddd bld         */
extern int reg_d4(char *,int,int);       /* 00000000 dddd0000 destantion  */
extern int reg_s4(char *,int,int);       /* 00000000 0000dddd source      */
extern int none(char *,int,int);
extern int none_last(char *,int,int);
extern int addr_shift(char *,int,int);   /* 000000dd ddddd000             */
extern int port(char *,int,int);         /* 00000000 ddddd000 port I/O low*/
extern int port_all(char *,int,int);     /* 00000dd0 0000dddd port I/O all*/
extern int byte(char *,int,int);         /* 0000dddd 0000dddd             */
extern int indir_addr(char *,int,int);   /* 00000000 000000dd             */
extern int indir_shift(char * ,int,int); /* 00d0dd00 00000ddd ldd         */
extern int addr_shift12(char *,int,int); /* 0000dddd dddddddd rcall,rjmp  */
extern int reg_d2(char *,int,int);       /* 00000000 00dd0000             */
extern int addr22(char *,int,int);       /* for 22bit PC                  */
extern int reg_s3(char *,int,int);
extern int reg_d3(char *,int,int);
extern int put_z(char *,int,int);
extern int put_zplus(char *,int,int);
extern int word_d(char *,int,int);
extern int word_s(char *,int,int);
extern int longcall(char *,int,int);
extern int word(char *,int,int);

extern void hardware();
extern void clean_memory();
