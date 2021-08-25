#include <stdio.h>
#include <libintl.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <Xm/XmAll.h>

#ifdef MAIN
#define EXPORT
#define INIT(x,val) x=val
#else
#define EXPORT extern
#define INIT(x,val) x
#endif

#define _(x) gettext(x)
#define N_(x) (x)

EXPORT uint8_t INIT(*gpr_pointer,NULL),INIT(*io_pointer,NULL),
  INIT(*sram_pointer,NULL),INIT(*eeprom_pointer,NULL),
  INIT(*flash_pointer,NULL);
EXPORT ListCommand *list_commands;
EXPORT int index_pins[MAX_CODE_PINS],/* for more fast searching of pin data  */
  index_ports[MAX_CODE_PORTS], /* for more fast searching of port data   */
  index_flags[MAX_CODE_FLAGS]; /* for more fast searching of flag data   */

EXPORT DrawArea drawarea;     /* here prints dissamblered code           */
EXPORT uint8_t temp_reg_t1;   /* TEMP register for accessing 16-bit reg. */
EXPORT uint8_t temp_reg_t3;   /* TEMP register for accessing 16-bit reg. */
EXPORT int stack_90s1200[3];  /* hardware stack of 90S1200               */

EXPORT Widget toplevel,INIT(gpr_window,NULL),INIT(io_window,NULL),
  INIT(memory_window,NULL),INIT(edit_memory_window,NULL),
  INIT(ports_window,NULL),INIT(debugger_window,NULL),INIT(project_window,NULL),
  INIT(about_window,NULL),INIT(file_selection,NULL);

EXPORT Display *display;
EXPORT int scr_num;
EXPORT MemoryWindowDrawArea mwda;

/* used to detect changing byte of memory */
EXPORT uint8_t INIT(*mem_file_changed,NULL),INIT(*eeprom_changed,NULL);
EXPORT int size_mem_file,size_eeprom;

EXPORT int INIT(pc,-1),       /* pc register                             */
  INIT(num_call,0),           /* number of inner calls                   */
  INIT(type_micro,0),         /* type of micro, default AT90S1200        */
  INIT(frequency,4000000),    /* frequency of MPU                        */
  INIT(debug,FALSE),          /* use for displaying current pixmap       */
  INIT(clocks,0),             /* clocks                                  */
  load_memory,                /* flag for detecting type memory          */
  INIT(type_time,uS),
  INIT(wtd_freq,1000000),     /* WatchDog frequency */
  INIT(wacthdog_timeout,UNPROGRAMMED),
  INIT(multi_step,10),        /* number of command is executed by multi step */
  INIT(auto_step,100),    /* it's time while a command are executed by auto step */
  INIT(debug_flag,0);         /* */

EXPORT char error_message[80],*filename;

EXPORT pthread_t thread;
EXPORT pthread_attr_t attr_thread;

extern Microcontroller controllers[];
extern Descr_io_port descr_io_ports[];
extern IO_port16 port16_wr[];
extern IO_port16 port16_rd[];
extern Command commands[];
extern Widget edit_time;
extern Menu memory_menu[];

/* ??? */
extern int load_memory,debug,type_time;
extern int ports_window_flag;
extern int watchdog_prescaler;
extern Line_port_window data_port_window[6];
extern Data_port_window data_port_log[6],data_port_sti[6];


extern void set_size_static_window(Widget,XtPointer,XtPointer);
extern void manage();
extern void unmanage(Widget w,XtPointer client_data, XtPointer call_data);
extern void fill_zero();
extern void set_size_static_window();
extern void xs_widget_tip();
extern void delete_tip();
extern Pixmap CreatePixmap(Window window,unsigned int width,
			   unsigned int height,Pixel pixel);
extern void use_project_callback();
extern void show_error();

extern void draw();
extern void create_gpr_window();
extern void create_io_ports_window();
extern void update_gpr_window();
extern void update_io_window();
extern void update_main();
extern void update_eeprom_window();
extern void full_update_io_window();
extern void full_update_memory_window(void);
extern void init_memory_window(void);
extern void clean_memory();
extern void hardware();
extern int load_flash();
extern int load_eeprom();
extern void reset();
extern void exec_command();
extern void quit_callback();
extern void create_memory_window();
extern void draw_memory_window();

extern Widget FindMenuButton(Menu *menu,char *name);
extern Widget FindToolbarButton(char *name);

extern int get_io_index();
extern void show_info();

extern void set_sreg_flag(int f);
extern void clear_sreg_flag(int f);
extern int  get_sreg_flag(int f);
extern uint8_t add_port(int p,uint8_t add);
extern uint8_t get_port(int port);
extern void set_port(int p,uint8_t value);
extern void set_port16(int low,int hi,uint16_t value);
extern uint16_t get_port16(int low,int hi);
extern uint16_t add_port16(int low,int hi,uint16_t value);
