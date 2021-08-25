/*
 File        : avr_simulator.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : This is the main file.

 Copyright notice:

 avr_simulator - A GNU/Linux simulator for the Atmel AVR series
 of microcontrollers. Copyright (C) 2001 Sergiy Uvarov

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 Sergiy Uvarov

 E-mail : colonel@ff.dsu.dp.ua
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>
#include <signal.h>
#include <Xm/XmAll.h>
#include <X11/Xmu/Editres.h>
/*#include <X11/xpm.h> */

#define AVR_SIMULATOR
#define MAIN
#include "types.h"
#include "avr_simulator.h"
#include "global.h"
#include "microcontrollers.h"
#include "commands.h"

/*  #include "auto_step.xpm" */
/*  #include "breakpoint.xpm" */
/*  #include "current.xpm" */
/*  #include "empty.xpm" */
/*  #include "load.xpm" */
/*  #include "multi_step.xpm" */
/*  #include "reload.xpm" */
/*  #include "reset.xpm" */
/*  #include "run.xpm" */
/*  #include "step_out.xpm" */
/*  #include "step_over.xpm" */
/*  #include "stop.xpm" */
/*  #include "trace_into.xpm" */


void init();
int create_listing();
int read_word_gen(int *command,int *address,FILE *file);
int read_word_intel(int *command,int *address,FILE *file);
int read_word_motorola(int *command,int *address,FILE *file);
void fill_zero(char *string,int number_sym);
int load_flash(char *name);
int load_eeprom(char *name);
void draw();

void create_main();
void update_main();
Pixmap CreatePixmap(Window window,unsigned int width,unsigned int height,
		    Pixel pixel);
void show_error();
void exec_command();
void show_info();

void reset();
void manage();
void unmanage();
void file_selection_ok();
void refresh();
void resize();

void use_project_callback();
void scroll_bar_callback();
void time_button_s_callback();
void time_button_us_callback();
void time_button_ms_callback();
void file_selection_gen();
void file_selection_intel();
void file_selection_motorola();
void set_size_static_window();
void info_close_callback();
void drawarea_input_callback();

int (*read_word)(int *,int *,FILE *)=read_word_gen;

char *flags[]={"I:","T:","H:","S:","V:","N:","Z:","C:"};

Widget error_dialog,form,separator,frame,option_menu,
  menu_fileselection,info_dialog,main_window;

Widget label_freq,             /* widget for displaying frequency of MPU  */
  edit_pc,                     /* edit field for displaying PC register   */
  edit_X,                      /* edit field for displaying X register    */
  edit_Y,                      /* edit fieldfor displaying Y register     */
  edit_Z,                      /* edit field for displaying Z register    */
  label_sreg,                  /* label for displaying SREG register      */
  edit_time,                   /* edit field for displaying TIME          */
  edit_clock;                  /* edit field for displaying CLOCKS        */

static String fallbacks[]={
  "*background: rgb:D0/E0/C0",
  "*XmPushButton.armColor: rgb:A0/A0/A0",
  "*XmPushButton.highlightThickness: 1",
  "*XmPushButton.shadowThickness: 1",
  "*XmTextField.highlightThickness: 1",
  "*XmTextField.shadowThickness: 1",
  "*XmCascadeButtonGadget.shadowThickness: 1",
  "*XmCascadeButtonGadget.highlightThickness: 1", 
  "*XmPushButtonGadget.shadowThickness: 1",
  "*XmPushButtonGadget.highlightThickness: 1", 
  "*XmToggleButtonGadget.highlightThickness: 0",
  "*XmToggleButtonGadget.shadowThickness: 0",
  "*XmToggleButtonGadget.indicatorOn: INDICATOR_CHECK",
  "*option.shadowThickness: 1",                    
  "*XmScrolledWindow*shadowThickness: 1",
  "*Scroll*shadowThickness: 0",
  "*Scroll.XmScrollBar.shadowThickness: 1",
  "*XmComboBox.shadowThickness: 1",
  "*XmComboBox.highlightThickness: 1",
  "*XmComboBox.marginHeight: 1",
  "*XmComboBox.marginWidth: 2",
  "*XmTextField.background: rgb:C0/D0/D0",
  "*XmTextField.marginHeight:2",
  "*XmTextField.marginWidth: 3",
  "*XmList.background: rgb:C0/D0/D0",
  "*option_menu.marginHeight: 0",
  "*option_menu.marginWidth: 0",
  "*memory_label.shadowThickness: 1",
  "*memory_label.highlightThickness: 0",
  "*memory_label.background: rgb:A8/B8/B8",
  "*memory_field.shadowThickness: 1",
  "*BulletinBoard.marginHeight: 0",
  "*BulletinBoard.marginWidth: 0",
  "*about_label.background: rgb:F0/F0/F0",
  NULL
};


int main(int argc, char *args[])
{
  Widget menu_bar,button,scroll,form_buttons;
  XtAppContext context;
  XmString string;

  XInitThreads();
  XtSetLanguageProc(NULL,NULL,NULL);
  textdomain("avr_simulator");

  toplevel=XtVaOpenApplication(&context,"Avr_simulator",NULL,0,&argc,args, 
                               fallbacks,sessionShellWidgetClass,NULL);
 
  display=XtDisplay(toplevel);
  scr_num=DefaultScreen(display);

  XtAddEventHandler(toplevel,(EventMask)0,True,
		    (XtEventHandler)_XEditResCheckMessages,0);

  XtVaSetValues(toplevel,XmNallowShellResize,True,
		XmNkeyboardFocusPolicy,XmPOINTER,NULL);
  main_window=XtVaCreateManagedWidget("main_window",xmMainWindowWidgetClass,
				      toplevel,NULL);
  
  /* create menu */ 
  menu_bar=XmCreateMenuBar(main_window,"menu",NULL,0);
  create_menu(menu_bar,menu,5);
  form=XtVaCreateWidget("form",xmFormWidgetClass,main_window,NULL);
  XtVaSetValues(main_window,XmNmenuBar,menu_bar,XmNworkWindow,form,NULL);
  XtManageChild(menu_bar);
  /* end menu */

  /* create Selection File */
  string=XmStringGenerate(_("Selection of file"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  file_selection=XmCreateFileSelectionDialog(toplevel,"selection_file",NULL,0);
  XtVaSetValues(file_selection,XmNdialogTitle,string,
		XmNheight,400,XmNwidth,400,
		XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL,NULL);
  XmStringFree(string);
  string=XmStringGenerate("*.gen",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(file_selection,XmNpattern,string,NULL);
  XmStringFree(string);
  
  XtAddCallback(file_selection,XmNcancelCallback,
                (XtCallbackProc)unmanage,
                (XtPointer)&file_selection);
  XtAddCallback(file_selection,XmNokCallback,
                (XtCallbackProc)file_selection_ok,
                (XtPointer)&file_selection);
  XtUnmanageChild(XmFileSelectionBoxGetChild(file_selection,
                                             XmDIALOG_HELP_BUTTON));
  /* option menu */
  
  option_menu=XmCreateOptionMenu(file_selection,"option_menu",NULL,0);
  menu_fileselection=XmCreatePulldownMenu(option_menu,"option",NULL,0);
  
  string=XmStringGenerate(_("generic"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("generic",xmPushButtonGadgetClass,
                                 menu_fileselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,(XtCallbackProc)file_selection_gen,
		(XtPointer)&file_selection);
  XmStringFree(string);
  string=XmStringGenerate(_("motorola"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("motorola",xmPushButtonGadgetClass,
                                 menu_fileselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)file_selection_motorola,
		(XtPointer)&file_selection);
  XmStringFree(string);
  string=XmStringGenerate(_("intel"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("intel",xmPushButtonGadgetClass,
                                 menu_fileselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)file_selection_intel,
		(XtPointer)&file_selection);
  XmStringFree(string);
  string=XmStringCreateLocalized(_("Type of file:"));
  XtVaSetValues(option_menu,XmNsubMenuId,menu_fileselection,XmNlabelString,
		string,
                XmNtopAttachment,XmATTACH_FORM,XmNtopOffset,10,
                XmNleftAttachment,XmATTACH_FORM,XmNleftOffset,10,
                NULL);
  XmStringFree(string);
  XtManageChild(option_menu);
  /* end Selection File */
  
  form_buttons=
    XtVaCreateManagedWidget("form",xmFormWidgetClass,form,
			    XmNtopAttachment,XmATTACH_FORM,
			    XmNleftAttachment,XmATTACH_FORM,
			    XmNrightAttachment,XmATTACH_FORM,
			   /*   XmNheight,30, */NULL);
			    
  create_buttons(form_buttons);
    
  separator=XtVaCreateManagedWidget("Separator",xmSeparatorWidgetClass,form,
				    XmNtopAttachment,XmATTACH_WIDGET,
				    XmNtopWidget,form_buttons,
				    XmNtopOffset,3,XmNleftOffset,2,
				    XmNleftAttachment,XmATTACH_FORM,
				    XmNrightAttachment,XmATTACH_FORM,
				    XmNrightOffset,2,NULL);
  
  /* window for text of program */
  frame=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,form,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopWidget,separator,XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,5,
				XmNbottomAttachment,XmATTACH_FORM,
				XmNbottomOffset,5,
				XmNmarginWidth,2,XmNmarginHeight,2,
				NULL);
  scroll=XtVaCreateManagedWidget("Scroll",xmScrolledWindowWidgetClass,frame,
				 XmNscrollingPolicy,XmAPPLICATION_DEFINED,
				 XmNscrollBarDisplayPolicy,XmSTATIC,
				 NULL);
  drawarea.draw=XtVaCreateManagedWidget("Drawing area",
					xmDrawingAreaWidgetClass,scroll,
					XmNbackground,
					WhitePixel(display,scr_num),
					XmNforeground,
					BlackPixel(display,scr_num),NULL);
  drawarea.v_scrb=XtVaCreateManagedWidget("scr_bar",xmScrollBarWidgetClass,
					  scroll,XmNorientation,XmVERTICAL,
					  XmNincrement,1,XmNminimum,0,
					  XmNvalue,0,XmNsliderSize,1,
					  XmNmaximum,1,NULL);
  XtAddCallback(drawarea.v_scrb,XmNvalueChangedCallback,
		(XtCallbackProc)scroll_bar_callback,NULL);
  XtAddCallback(drawarea.v_scrb,XmNdragCallback,
		(XtCallbackProc)scroll_bar_callback,NULL);
  XtAddCallback(drawarea.draw,XmNinputCallback,
		(XtCallbackProc)drawarea_input_callback,NULL);
  
  XmScrolledWindowSetAreas(scroll,NULL,drawarea.v_scrb,drawarea.draw);
  /* end of window */

  /* create error message window */
  error_dialog=XmCreateErrorDialog(toplevel,"Error message",NULL,0);
  string=XmStringGenerate(_("Error message"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(error_dialog,XmNdialogTitle,string,NULL);
  XmStringFree(string);
  XtUnmanageChild(XmMessageBoxGetChild(error_dialog,
					     XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(error_dialog,
				       XmDIALOG_CANCEL_BUTTON));
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(error_dialog,XmNokLabelString,string,
		XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL,NULL);
  XmStringFree(string);
  /* end of error message window */

  /* create info message window */
  info_dialog=XmCreateInformationDialog(toplevel,"Info_dialog",NULL,0);
  string=XmStringGenerate(_("Info message"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(info_dialog,XmNdialogTitle,string,NULL);
  XmStringFree(string);
  XtUnmanageChild(XmMessageBoxGetChild(info_dialog,
					     XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(info_dialog,
				       XmDIALOG_OK_BUTTON));
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(info_dialog,XmNcancelLabelString,string,
		XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL,NULL);
  XmStringFree(string);
  XtAddCallback(info_dialog,XmNcancelCallback,
		(XtCallbackProc)info_close_callback,NULL);
  /* end of info message window */

  create_project_selection();

  /* create main status */
  create_main();

  create_debugger_window();

  create_ports_window();

  create_about();

  XtManageChild(form);
  XtRealizeWidget(toplevel);

  init();
  XtAppMainLoop(context);
  return 0;
}

void set_new_proccessor()
{
  int n;
  
  /* allocate data memory */
  free(gpr_pointer);
  if((gpr_pointer=malloc(controllers[type_micro].size_gpr+
			 controllers[type_micro].size_io_reg+
			 controllers[type_micro].size_sram))==NULL)
    {
      puts("use_project_callback(): can't allocate memory for 'gpr_pointer'");
      exit(1);
    }
  io_pointer=&gpr_pointer[controllers[type_micro].size_gpr];
  sram_pointer=&gpr_pointer[controllers[type_micro].size_gpr+
			    controllers[type_micro].size_io_reg];
 
  /* allocate EEPROM memory*/
  free(eeprom_pointer);
  if((eeprom_pointer=malloc(controllers[type_micro].size_eeprom))==NULL)
    {
      puts("use_project_callback(): \
can't allocate memory for 'eeprom_pointer'");
      exit(1);
    }

  /* allocate FLASH memory*/
  free(flash_pointer);
  if((flash_pointer=malloc(controllers[type_micro].size_flash*2))==NULL)
    {
      puts("use_project_callback(): \
can't allocate memory for 'flash_pointer'");
      exit(1);
    }
  clean_memory();

  /* for more fast searching data of ports */
  for(n=0;n<MAX_CODE_PORTS;n++) index_ports[n]=-1;
  for(n=0;n<MAX_CODE_PINS;n++) index_pins[n]=-1;
  for(n=0;n<MAX_CODE_FLAGS;n++) index_flags[n]=-1;

  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++)
    index_ports[controllers[type_micro].io_registers[n].code]=n;
  for(n=0;controllers[type_micro].pins[n].code!=-1;n++)
    index_pins[controllers[type_micro].pins[n].code]=n;
  for(n=0;controllers[type_micro].flags[n].flag_code!=-1;n++)
    index_flags[controllers[type_micro].flags[n].flag_code]=n;

  hardware_init();
  pc=-1;
}

void init(void)
{
  XFontStruct *font_struct;
  XGCValues values;
  XtWidgetGeometry toplevel_size;
  XColor color;

  drawarea.current_line_deb=-1;
  drawarea.num_commands=0;

  set_new_proccessor();

  init_windows_memory();

  full_update_io_window();

  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "rgb:F0/F0/F0",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=WhitePixel(display,scr_num);
    }
  drawarea.background=color.pixel;

  drawarea.window=XtWindow(drawarea.draw);
  values.foreground=BlackPixel(display,scr_num);
  drawarea.gc=XCreateGC(display,drawarea.window,GCForeground,&values);

  values.foreground=drawarea.background;
  drawarea.gc2=XCreateGC(display,drawarea.window,GCForeground,&values);

  if((font_struct=XQueryFont(display,XGContextFromGC(drawarea.gc)))==NULL)
    puts("font error.");
  drawarea.font_width=font_struct->max_bounds.width;
  drawarea.font_height=font_struct->ascent+font_struct->descent;

  XFreeFontInfo(NULL,font_struct,0);
  drawarea.width=29*drawarea.font_width;
  drawarea.height=drawarea.font_height*20;
  XtVaSetValues(drawarea.draw,XmNwidth,drawarea.width,
		XmNheight,drawarea.height,NULL);
  
  /* configure toplevel */
  XtQueryGeometry(toplevel,NULL,&toplevel_size);
  XtVaSetValues(toplevel,XmNheightInc,drawarea.font_height,
		XmNminHeight,toplevel_size.height-16*drawarea.font_width,
		XmNminWidth,toplevel_size.width,
		XmNbaseHeight,toplevel_size.height,
		XmNallowShellResize,False,NULL);
  
  drawarea.pixmap=CreatePixmap(drawarea.window,drawarea.width,drawarea.height,
			       drawarea.background);
  XtAddCallback(drawarea.draw,XmNexposeCallback,(XtCallbackProc)refresh,
		NULL);
  XtAddCallback(drawarea.draw,XmNresizeCallback,(XtCallbackProc)resize,
		NULL);

  /*pixmaps */  
/*    if((errno=XpmCreatePixmapFromData(display,window,breakpoint_xpm, */
/*  				    &temp_p,NULL,NULL))) */
/*      fprintf(stderr,"%s\n",XpmGetErrorString(errno)); */

/*    XCopyArea(display,temp_p,stop_pixmap,temp_gc,0,0,1,1,0,0); */
/*    if((errno=XpmCreatePixmapFromData(display,window,current_xpm, */
/*  				    &current_pixmap,NULL,NULL))) */
/*      fprintf(stderr,"%s\n",XpmGetErrorString(errno)); */

  if((drawarea.stop_pixmap=
      XmGetPixmap(XtScreen(toplevel),"xpm/breakpoint.xpm",
		  BlackPixel(display,scr_num),drawarea.background
		  ))==XmUNSPECIFIED_PIXMAP) 
    fprintf(stderr," Pixmap problem with file breakpoint.xpm\n");
  if((drawarea.current_pixmap=
      XmGetPixmap(XtScreen(toplevel),"xpm/current.xpm",
		  BlackPixel(display,scr_num),drawarea.background
		  ))==XmUNSPECIFIED_PIXMAP) 
    fprintf(stderr," Pixmap problem with file current.xpm\n");
  
  if(pthread_attr_init(&attr_thread))
    {perror("pthread_attr_init:");exit(1);}
  if(pthread_attr_setdetachstate(&attr_thread,PTHREAD_CREATE_DETACHED))
    {perror("pthread_attr_setdetachstate:");exit(1);}

  SetSensitiveToolbar("Reload",False);
  SetSensitiveToolbar("Trace Into",False);
  SetSensitiveToolbar("Step Over",False);
  SetSensitiveToolbar("Step Out",False);
  SetSensitiveToolbar("Multi Step",False);
  SetSensitiveToolbar("Auto Step",False);
  SetSensitiveToolbar("Run",False);
  SetSensitiveToolbar("Stop",False);
  SetSensitiveToolbar("Reset",False);
  SetSensitiveMenu("Debug",False);
}

/* -1 command not found */
int create_listing()
{
  int com,address,found,n,num_line;

  drawarea.num_commands=num_line=0;
  for(address=0;address<controllers[type_micro].size_flash;address++)
    {
      list_commands[address].function_command=NULL;
      list_commands[address].number_line=-1;
      list_commands[address].code=0xff;
    }
  for(address=num_line=0;address<controllers[type_micro].size_flash;address++)
    {
      n=-1;found=FALSE;
      
      com=((uint16_t*)(flash_pointer))[address];

      while(commands[++n].mnemocode[0]!=0)
	{
	  if((com&commands[n].mask)==commands[n].code)
	    {
	      drawarea.num_commands++;
	      drawarea.display_lines[num_line]=address;
	      list_commands[address].number_line=num_line++;
	      list_commands[address].flags=0;
	      list_commands[address].code=com;
	      
	      strcpy(list_commands[address].mnemocode,
		     commands[n].mnemocode);
	      list_commands[address].function_command=commands[n].command;
	      list_commands[address].num_words=commands[n].num_words;
	      if(commands[n].num_words==2)
		{
		  com=((uint16_t*)(flash_pointer))[++address];
		  list_commands[address].code=com;
		  list_commands[address-1].arg1=
		    commands[n].arg1(list_commands[address-1].mnemocode,
				     list_commands[address-1].code,address-1);
		  strcat(list_commands[address-1].mnemocode,",");
		  list_commands[address-1].arg2=
		    commands[n].arg2(list_commands[address-1].mnemocode,
				     list_commands[address-1].code,address-1);
		}
	      else
		{
		  list_commands[address].arg1=
		    commands[n].arg1(list_commands[address].mnemocode,
				     com,address);
		  strcat(list_commands[address].mnemocode,",");
		  list_commands[address].arg2=
		    commands[n].arg2(list_commands[address].mnemocode,
				     com,address);
		}
	      found=TRUE;
	      break;
	    }
	}
    }

  if(drawarea.num_commands==0)
    {
      sprintf(error_message,gettext(_("No commands")));
      return 1;
    }

  return 0;
}

/* returned value 2 is 'OK', 1 is 'ERROR' */
int read_word_gen(int *command,int *address,FILE *file)
{
  return fscanf(file,"%x:%x",address,command);
}

int read_word_intel(int *command,int *address,FILE *file)
{
  char buf[256];
  int size_buf,n;
  static char word[7]="0x0000";
  static int addr=0,position=0,size_buf2=0,buf2[128];
  
  if(position<size_buf2)
    {
      *command=buf2[position++];
      *address=addr++;
      return 2;
    }
  else
    {
      if(fgets(buf,256,file)==NULL)
	return EOF;
      size_buf=strlen(buf);
      if(buf[size_buf-1]=='\n')
	buf[--size_buf]=0;
      if(buf[size_buf-1]=='\r')
	buf[--size_buf]=0;
      if(buf[0]==':'&&size_buf>=11)
	{
	  word[2]=buf[1];word[3]=buf[2];word[4]=0;
	  size_buf2=strtoul(word,NULL,16)/2;
	  if(size_buf2*4!=strlen(buf)-11)
	    return 1;
	  strncpy(&word[2],&buf[3],4);
	  addr=strtoul(word,NULL,16)/2;
	  position=0;
	  if(buf[8]=='0')
	    {
	      for(n=0;n<size_buf2;n++)
		{
		  strncpy(&word[4],&buf[n*4+9],2);
		  strncpy(&word[2],&buf[n*4+11],2);
		  buf2[n]=strtoul(word,NULL,16);
		}
	      return read_word_intel(command,address,file);
	    }
	  else
	    {
	      size_buf2=position=0;
	      return read_word_intel(command,address,file);
	    }
	}
      return 1;
    }
}

int read_word_motorola(int *command,int *address,FILE *file)
{
  char buf[256];
  int size_buf,n;
  static char word[7]="0x0000";
  static int addr=0,position=0,size_buf2=0,buf2[128];
  
  if(position<size_buf2)
    {
      *command=buf2[position++];
      *address=addr++;
      return 2;
    }
  else
    {
      if(fgets(buf,256,file)==NULL)
	return EOF;
      size_buf=strlen(buf);
      if(buf[size_buf-1]=='\n')
	buf[--size_buf]=0;
      if(buf[size_buf-1]=='\r')
	buf[--size_buf]=0;
      if((!strncmp(buf,"S1",2))&&size_buf>10)
	{
	  word[2]=buf[2];word[3]=buf[3];word[4]=0;
	  size_buf2=strtoul(word,NULL,16);
	  if(size_buf2*2!=strlen(buf)-4)
	    return 1;
	  strncpy(&word[2],&buf[4],4);
	  addr=strtoul(word,NULL,16)/2;
	  position=0;size_buf2=(size_buf2-3)/2;
	  for(n=0;n<size_buf2;n++)
	    {
	      strncpy(&word[4],&buf[n*4+8],2);
	      strncpy(&word[2],&buf[n*4+10],2);
	      buf2[n]=strtoul(word,NULL,16);
	    }
	  return read_word_motorola(command,address,file);
	}
      else
	return read_word_motorola(command,address,file);
    }
}


void fill_zero(char *string,int number_sym)
{
  char buf[10];
  int n;
  
  if(number_sym<strlen(string)) return;
  strcpy(buf,string);
  for(n=0;n<number_sym-strlen(buf);n++) string[n]='0';
  strcpy(&string[n],buf);
  return;
}

void manage(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtManageChild(*((Widget *)client_data));
}

void unmanage(Widget w,XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(*((Widget *)client_data));
}

void quit_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  XFreeGC(display,drawarea.gc);
  XFreePixmap(display,drawarea.pixmap);
  XtFree(filename);

  exit(0);
};

void file_selection_ok(Widget w,XtPointer client_data, XtPointer call_data)
{
  char *filename;
  
  XtUnmanageChild(*((Widget *)client_data));
  filename=XmCvtXmStringToCT(((XmFileSelectionBoxCallbackStruct *)
			      call_data)->value);
  switch(load_memory){
  case FLASH:  
    if(load_flash(filename)) show_error();
    break;
  case EEPROM: 
    if(load_eeprom(filename)) show_error();
  }
  XtFree(filename);
}

void resize(Widget w,XtPointer client_data, XtPointer call_data)
{
  XtVaGetValues(drawarea.draw,XmNheight,&drawarea.height,NULL);
  drawarea.pixmap=CreatePixmap(drawarea.window,drawarea.width,drawarea.height,
			       drawarea.background);
  draw();

  if(drawarea.num_commands>drawarea.height/drawarea.font_height)
    XtVaSetValues(drawarea.v_scrb,XmNsliderSize,
		  drawarea.height/drawarea.font_height,
		  XmNmaximum,drawarea.num_commands,NULL);
  else
    XtVaSetValues(drawarea.v_scrb,XmNminimum,0,XmNvalue,0,XmNsliderSize,1,
		  XmNmaximum,1,NULL);
}

void refresh(Widget w,XtPointer client_data, XtPointer call_data)
{
  XEvent *event;
  
  event=((XmDrawingAreaCallbackStruct *)call_data)->event;
  if(event->type==Expose)  
      {    
        if(event->xexpose.window==drawarea.window) 
          XCopyArea(display,drawarea.pixmap,drawarea.window,drawarea.gc,
		    event->xexpose.x,event->xexpose.y,event->xexpose.width,
                    event->xexpose.height,event->xexpose.x,event->xexpose.y); 
      }     
  
}

int load_flash(char *name)
{
  static char *filename;
  static FILE *file;
  int com,address;

  if(name!=NULL)
    { /* new file */
      if((filename=malloc(strlen(name)+1))==NULL)
	{
	  puts("load_flash(): can't allocate memory for 'name'");
	  exit(1);
	}
      strcpy(filename,name);
    }
  if(filename==NULL) return 0;
  if(file!=NULL) fclose(file);

  if((file=fopen(filename,"r"))==NULL) 
    {
      sprintf(error_message,"open file: %s",strerror(errno));
      return 1;
    }
  
  drawarea.current_line_deb=pc=-1;
  free(list_commands);
  if((list_commands=
      malloc(controllers[type_micro].size_flash*sizeof(ListCommand)))==NULL)
    {
      puts("load_flash(): can't allocate memory for 'list_commands'");
      exit(1);
    }
  
  free(drawarea.display_lines);
  if((drawarea.display_lines=
      malloc(controllers[type_micro].size_flash*sizeof(int)))==NULL)
    {
      puts("load_flash(): \
can't allocate memory for 'drawarea.display_lines'");
      exit(1);
    }

  /* clean flash memory */
  memset(flash_pointer,0xFF,controllers[type_micro].size_flash*2);

  while(read_word(&com,&address,file)==2)
    {
      if(address>=controllers[type_micro].size_flash)
	{
	  sprintf(error_message,
		  _("Size of progarm is bigger then size of flash"));
	  return 1;
	}
      flash_pointer[address*2]=(uint8_t)com;
      flash_pointer[address*2+1]=(uint8_t)(com>>8);
    }

  if(create_listing()) 
    {
      drawarea.current_line_deb=-1;
      drawarea.num_commands=0;
      XtVaSetValues(drawarea.v_scrb,XmNsliderSize,1,
		    XmNmaximum,drawarea.num_commands+1,
		    XmNvalue,0,NULL);
      draw(); /* clean screen */
      SetSensitiveMenu("Debug",False);
      return 1;
    }
  SetSensitiveMenu("Debug",True);

  /* set size of slider in drawarea */
  if(drawarea.height/drawarea.font_height<=drawarea.num_commands)
    XtVaSetValues(drawarea.v_scrb,XmNsliderSize,
		  drawarea.height/drawarea.font_height,
		  XmNmaximum,drawarea.num_commands,
		  XmNvalue,0,NULL);
  else
    XtVaSetValues(drawarea.v_scrb,XmNvalue,0,XmNsliderSize,1,XmNmaximum,1,
		  NULL);

  clean_memory();

  draw();

  full_update_memory_window();

  change_sensitive_run(False);

  return 0;
}

int load_eeprom(char *name)
{
  static char *filename;
  static FILE *file;
  int address,data,error;
  
  if(name!=NULL)
    { /* new file */
      free(filename);
      if((filename=malloc(strlen(name)+1))==NULL)
	{
	  puts("load_eeprom(): can't allocate memory for 'filename'");
	  exit(1);
	}
      strcpy(filename,name);
    }
  if(filename==NULL) return 0;

  if(file!=NULL) fclose(file);

  if((file=fopen(filename,"r"))==NULL) 
    {
      sprintf(error_message,"open file: %s",strerror(errno));
      return 1;
    }

  while((error=read_word(&data,&address,file))==2)
    {
      if(address>=controllers[type_micro].size_eeprom)
	{
	  sprintf(error_message,
		  _("Size of file is bigger then size of eeprom"));
	  return 1;
	}
      eeprom_pointer[address]=data;
    }
  if(error==1) return 1;
  return 0;
}

void scroll_bar_callback (Widget w,XtPointer client_data, XtPointer call_data)
{
  draw();
  return;
}

void drawarea_input_callback(Widget w,XtPointer client_data,
			     XtPointer call_data)
{
  XmDrawingAreaCallbackStruct *data;
  int num_line,address,cur_line;
  
  if(list_commands==NULL) return;
  data=(XmDrawingAreaCallbackStruct *)call_data;
  if(data->reason==XmCR_INPUT)
    if(data->event->type==ButtonPress)
      {
	if(data->event->xbutton.x>drawarea.font_width*8&&
	   data->event->xbutton.x<drawarea.font_width*12&&
	   data->event->xbutton.button==Button1)
	  {
	    XtVaGetValues(drawarea.v_scrb,XmNvalue,&num_line,NULL);
	    if((cur_line=num_line+data->event->xbutton.y/
		drawarea.font_height)>=drawarea.num_commands)
	      return;
	    address=drawarea.display_lines[cur_line];
	    if(list_commands[address].flags&BREAKPOINT)
	      list_commands[address].flags=
		list_commands[address].flags&(~BREAKPOINT);
	    else list_commands[address].flags=
		   list_commands[address].flags|BREAKPOINT;
	    draw();
	  }
      }
    
}

void draw(void)
{
  int value,num_line,n;
  char line[30],buf[10];
  
  XFillRectangle(display,drawarea.pixmap,drawarea.gc2,0,0,
		 drawarea.width,drawarea.height);
  
  /* Get number of first displaying line */
  XtVaGetValues(drawarea.v_scrb,XmNvalue,&value,NULL);
  /* if current debug line not displaying then to change first */
  /* displaying line */
  if(debug==TRUE&&drawarea.current_line_deb!=-1)
    {
      if(drawarea.current_line_deb<value||
	 drawarea.current_line_deb>value+drawarea.height/drawarea.font_height)
	{
	  if(drawarea.current_line_deb+drawarea.height/drawarea.font_height>
	     drawarea.num_commands-1)
	    value=drawarea.num_commands-drawarea.height/drawarea.font_height;
	  else
	    value=drawarea.current_line_deb;
	}
      
      if(drawarea.current_line_deb>
	 value+drawarea.height/drawarea.font_height-5&&
	 drawarea.current_line_deb<=value+drawarea.height/drawarea.font_height)
	{	  
	  if(drawarea.num_commands<
	     drawarea.current_line_deb+drawarea.height/drawarea.font_height/2)
	    {
	      if(drawarea.num_commands-drawarea.height/drawarea.font_height-1>
		 value)
		value=drawarea.num_commands-
		  drawarea.height/drawarea.font_height-1;
	    }
	  else
	    value=drawarea.current_line_deb-
	      drawarea.height/drawarea.font_height/2;
	}
    }
  XtVaSetValues(drawarea.v_scrb,XmNvalue,value,NULL);
  
  
  /* print debuging lines */
  for(n=value,num_line=0;num_line<drawarea.height/drawarea.font_height;n++)
    {
      if(n>=drawarea.num_commands) break;
      sprintf(buf,"%X",drawarea.display_lines[n]);
      fill_zero(buf,5);
      sprintf(line,"%s      %s",buf,
	      list_commands[drawarea.display_lines[n]].mnemocode);
      XDrawString(display,drawarea.pixmap,drawarea.gc,6,
		  num_line*drawarea.font_height+10,
		  line,strlen(line));
      /* display stop_pixmap */
      if(list_commands[drawarea.display_lines[n]].flags&BREAKPOINT)
	XCopyArea(display,drawarea.stop_pixmap,drawarea.pixmap,drawarea.gc,0,0,
		  15,15,drawarea.font_width*8+4,
		  (num_line-1)*drawarea.font_height+13);
      num_line++;
    }
  
  /* displaying current_pixmap */
  if(pc!=-1&&drawarea.current_line_deb>=value&&
     drawarea.current_line_deb<=drawarea.height/drawarea.font_height+value&&
     drawarea.current_line_deb!=-1)
    {
      XCopyArea(display,drawarea.current_pixmap,drawarea.pixmap,drawarea.gc,0,0,
		15,15,drawarea.font_width*6,
		(drawarea.current_line_deb-value)*drawarea.font_height);
    }
  XCopyArea(display,drawarea.pixmap,drawarea.window,drawarea.gc,0,0,drawarea.width,
	    drawarea.height,0,0);
}

Pixmap CreatePixmap(Window window,unsigned int width,unsigned int height,
		    Pixel pixel)
{
  Pixmap pixmap;
  char *data;
  
  if((data=malloc(width*height))==NULL)
    {
      puts("CreatePixmap(): can't allocate memory for 'data'");
      exit(1);
    }
  memset(data,0,width*height);    
  pixmap=XCreatePixmapFromBitmapData(display,window,data,width,
				     height,WhitePixel(display,scr_num),
				     pixel,DefaultDepth(display,scr_num));
  free(data);
  return pixmap;
}



void create_main(void)
{
  char buf[10];
  XmString string;
  Widget label,frame1,frame1_form,menu_timeselection,option_menu,button;
  

  frame1=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,form,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopWidget,separator,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,frame,
				 XmNleftOffset,5,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNrightOffset,5,
				 XmNmarginWidth,6,XmNmarginHeight,4,NULL);
  frame1_form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame1,NULL);
  string=XmStringGenerate("PC:",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label pc",xmLabelGadgetClass,frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_FORM,
				XmNtopOffset,2,
				XmNbottomAttachment,XmATTACH_FORM,
				XmNbottomOffset,2,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);
  
  sprintf(buf,"%X",pc);
  fill_zero(buf,8);
  edit_pc=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,frame1_form,
				  XmNtopAttachment,XmATTACH_FORM,
				  XmNleftAttachment,XmATTACH_WIDGET,
				  XmNleftWidget,label,
				  XmNleftOffset,0,
				  XmNcolumns,6,XmNmaxLength,6,
				  XmNvalue,buf,XmNeditable,False,
				  XmNcursorPositionVisible,False,NULL);
  
  
  string=XmStringGenerate("X:",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label X",xmLabelGadgetClass,frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget,edit_pc,
				XmNtopOffset,2,XmNleftOffset,10,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);
  
  edit_X=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,frame1_form,
				 XmNtopAttachment,XmATTACH_FORM,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,label,
				 XmNleftOffset,0,
				 XmNcolumns,4,XmNmaxLength,4,
				 XmNvalue,"0000",
				 XmNcursorPositionVisible,False,NULL);
  
  string=XmStringGenerate("Y:",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label Y",xmLabelGadgetClass,frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget,edit_X,
				XmNtopOffset,2,XmNleftOffset,10,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);
  
  edit_Y=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,frame1_form,
				 XmNtopAttachment,XmATTACH_FORM,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,label,
				 XmNleftOffset,0,
				 XmNcolumns,4,XmNmaxLength,4,
				 XmNvalue,"0000",
				 XmNcursorPositionVisible,False,NULL);
  
  string=XmStringGenerate("Z:",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label Z",xmLabelGadgetClass,frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget,edit_Y,
				XmNtopOffset,2,XmNleftOffset,10,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);

  edit_Z=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,frame1_form,
				 XmNtopAttachment,XmATTACH_FORM,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,label,
				 XmNleftOffset,0,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNcolumns,4,XmNmaxLength,4,
				 XmNvalue,"0000",
				 XmNcursorPositionVisible,False,NULL);

  frame1=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,form,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopWidget,frame1,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,frame,
				 XmNleftOffset,5,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNrightOffset,5,
				 XmNmarginWidth,3,XmNmarginHeight,2,NULL);

  string=XmStringGenerate(_("Status Register"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaCreateManagedWidget("label",xmLabelGadgetClass,frame1,
                          XmNlabelString,string,
                          XmNframeChildType,XmFRAME_TITLE_CHILD,
                          XmNchildHorizontalAlignment,XmALIGNMENT_BEGINNING,
                          XmNchildVerticalAlignment,XmALIGNMENT_CENTER,
                          NULL);
  XmStringFree(string);

  string=XmStringGenerate("I:0 T:0 H:0 S:0 V:0 N:0 Z:0 C:0",
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label_sreg=XtVaCreateManagedWidget("label sreg",xmLabelGadgetClass,frame1,
				     XmNlabelString,string,NULL);
  XmStringFree(string);

  
  frame1=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,form,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopWidget,frame1,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,frame,
				 XmNleftOffset,5,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNrightOffset,5,
				 XmNmarginWidth,6,XmNmarginHeight,0,NULL);
  frame1_form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame1,NULL);
  
  string=XmStringGenerate(_("Elapsed time:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label sreg",xmLabelGadgetClass,
				frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_FORM,
				XmNbottomAttachment,XmATTACH_FORM,
				XmNtopOffset,1,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);

  edit_time=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,frame1_form,
				    XmNtopAttachment,XmATTACH_FORM,
				    XmNtopOffset,5,XmNbottomOffset,5,
				    XmNbottomAttachment,XmATTACH_FORM,
				    XmNleftAttachment,XmATTACH_WIDGET,
				    XmNleftWidget,label,XmNleftOffset,0,
				    XmNcolumns,6,XmNmaxLength,6,
				    XmNvalue,"0,0000",XmNeditable,False,
				    XmNcursorPositionVisible,False,NULL);
  
  /* option menu */
  
  option_menu=XmCreateOptionMenu(frame1_form,"option_menu_time",NULL,0);
  menu_timeselection=XmCreatePulldownMenu(option_menu,"option",NULL,0);
  
  string=XmStringGenerate(_("uS"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("uS",xmPushButtonGadgetClass,
				 menu_timeselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)time_button_us_callback,NULL);
  XmStringFree(string);
  string=XmStringGenerate(_("mS"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("mS",xmPushButtonGadgetClass,
                                 menu_timeselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)time_button_ms_callback,NULL);
  XmStringFree(string);
  string=XmStringGenerate(_("S"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("S",xmPushButtonGadgetClass,
                                 menu_timeselection,XmNlabelString,string,
				 NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)time_button_s_callback,NULL);
  XmStringFree(string);

  string=XmStringGenerate("",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(option_menu,XmNsubMenuId,menu_timeselection,
		XmNlabelString,string,XmNtopAttachment,XmATTACH_FORM,
		XmNtopOffset,2,XmNbottomOffset,2,
		XmNleftAttachment,XmATTACH_WIDGET,
		XmNleftWidget,edit_time,XmNleftOffset,3,XmNlabelString,string,
		XmNbottomAttachment,XmATTACH_FORM,
		XmNmarginHeight,0,NULL);
  XtVaSetValues(menu_timeselection,XmNshadowThickness,1,NULL);
  XtManageChild(option_menu);
  XmStringFree(string);
  
  /* end option menu */
  
  string=XmStringGenerate(_("Clock:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("label clock",xmLabelGadgetClass,
				frame1_form,
				XmNtopAttachment,XmATTACH_FORM,
				XmNbottomAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget,option_menu,
				XmNtopOffset,1,XmNleftOffset,6,
				XmNlabelString,string,
				NULL);
  XmStringFree(string);

  edit_clock=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,
				     frame1_form,
				     XmNtopAttachment,XmATTACH_FORM,
				     XmNtopOffset,5,XmNbottomOffset,5,
				     XmNbottomAttachment,XmATTACH_FORM,
				     XmNleftAttachment,XmATTACH_WIDGET,
				     XmNleftWidget,label,XmNleftOffset,0,
				     XmNrightAttachment,XmATTACH_FORM,
				     XmNcolumns,9,XmNmaxLength,9,
				     XmNvalue,"000000000",XmNeditable,False,
				     XmNcursorPositionVisible,False,NULL);
}

void time_button_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  XmString string;
  
  if(type_time==uS)
    {
      type_time=mS;
      string=XmStringGenerate(_("mS"),XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
    }
  else
    {
      type_time=uS;
      string=XmStringGenerate(_("uS"),XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      
    }
  XtVaSetValues(w,XmNlabelString,string,NULL);
  XmStringFree(string);
  update_main();
}

void time_button_s_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  type_time=S;
  update_main();
}

void time_button_us_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  type_time=uS;
  update_main();
}

void time_button_ms_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  type_time=mS;
  update_main();
}

void update_main(void)
{
  char buf[10],buf2[60];
  XmString string;
  int n,sreg;
  
  sprintf(buf,"%X",pc);
  if(pc==-1) strcpy(buf,"FFFFFF");
  fill_zero(buf,6);
  XmTextFieldSetString(edit_pc,buf);

  sprintf(buf,"%X",gpr_pointer[26]);
  fill_zero(buf,2);
  sprintf(buf2,"%X%s",gpr_pointer[27],buf);
  fill_zero(buf2,4);
  XmTextFieldSetString(edit_X,buf2);

  sprintf(buf,"%X",gpr_pointer[28]);
  fill_zero(buf,2);
  sprintf(buf2,"%X%s",gpr_pointer[29],buf);
  fill_zero(buf2,4);
  XmTextFieldSetString(edit_Y,buf2);

  sprintf(buf,"%X",gpr_pointer[30]);
  fill_zero(buf,2);
  sprintf(buf2,"%X%s",gpr_pointer[31],buf);
  fill_zero(buf2,4);
  XmTextFieldSetString(edit_Z,buf2);
  
  buf2[0]=0;
  sreg=get_port(SREG);
  for(n=0;n<8;n++)
    {
      strcat(buf2,flags[n]);
      if(sreg&(1<<(7-n)))
	strcat(buf2,"1 ");
      else strcat(buf2,"0 ");
    }
  string=XmStringGenerate(buf2,XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(label_sreg,XmNlabelString,string,NULL);
  XmStringFree(string);

  switch(type_time)
    {
    case uS: sprintf(buf2,"%f:6",1.0/frequency*1E6*clocks);break;
    case mS: sprintf(buf2,"%f:6",1.0/frequency*1E3*clocks);break;
    case  S: sprintf(buf2,"%f:6",1.0/frequency*clocks);break;
    }
  
  XmTextFieldSetString(edit_time,buf2);

  sprintf(buf2,"%d",clocks);
  fill_zero(buf2,9);
  XmTextFieldSetString(edit_clock,buf2);
}

void set_size_static_window(Widget w,XtPointer client_data,XtPointer call_data)
{
  Dimension width,height;
  
  XtRemoveCallback(w,XmNpopupCallback,(XtCallbackProc)set_size_static_window,
		   NULL);
  XtVaGetValues(w,XmNwidth,&width,XmNheight,&height,NULL);
  XtVaSetValues(w,XmNmaxWidth,width,XmNminWidth,width,XmNminHeight,height,
		XmNmaxHeight,height,NULL);
}

void file_selection_gen(Widget w,XtPointer client_data,XtPointer call_data)
{
  XmString string;
  
  string=XmStringGenerate("*.gen",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(*(Widget *)client_data,XmNpattern,string,NULL);
  XmStringFree(string);
  read_word=read_word_gen;
}

void file_selection_intel(Widget w,XtPointer client_data,XtPointer call_data)
{
  XmString string;
  
  string=XmStringGenerate("*.hex",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(*(Widget *)client_data,XmNpattern,string,NULL);
  XmStringFree(string);
  read_word=read_word_intel;
}

void file_selection_motorola(Widget w,XtPointer client_data,
			     XtPointer call_data)
{
  XmString string;
  
  string=XmStringGenerate("*.s",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(*(Widget *)client_data,XmNpattern,string,NULL);
  XmStringFree(string);
  read_word=read_word_motorola;
}

void show_error(void)
{
  XmString string;
  
  string=XmStringGenerate(error_message,XmFONTLIST_DEFAULT_TAG,          
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);            
  XtVaSetValues(error_dialog,XmNmessageString,string,NULL);                 
  XmStringFree(string);                                                     
  XtManageChild(error_dialog);
}

void reset(void)
{
 clean_memory();
 memset(mem_file_changed,0,size_mem_file);
 memset(eeprom_changed,0,size_eeprom);
 
 drawarea.current_line_deb=pc=clocks=num_call=watchdog_prescaler=0;
 SetSensitiveToolbar("Stop",False);
 SetSensitiveMenu("Stop",False);

 debug=TRUE;
 draw();
 debug=FALSE;

 hardware_init();
 update_gpr_window();
 full_update_io_window();
 full_update_memory_window();
 update_main();
}

void exec_command(void)
{
  static int clock_old=0;
  int temp;

  if(!(strncmp(list_commands[pc].mnemocode,"call",4)&&
       strncmp(list_commands[pc].mnemocode,"rcall",5)&&
       strncmp(list_commands[pc].mnemocode,"icall",5)&&
       strncmp(list_commands[pc].mnemocode,"eicall",6)))
    num_call++;
  if(!(strncmp(list_commands[pc].mnemocode,"ret",3)&&
       strncmp(list_commands[pc].mnemocode,"reti",4))) num_call--;

  clock_old=clocks; /* for normal halt in EEPROM access*/
  if(!interrupt()) 
    {
      if(list_commands[pc].function_command!=NULL)
	list_commands[pc].function_command();
      else
	{
	  show_info(_("Can't execute command!!!"));
	  thread_stop=1;
	}
    }
  else if(debug_flag&DEBUG_INTERRUPT) 
    {
      show_info(_("Interrupt"));
      thread_stop=1;
    }

  temp=clocks;
  for(clocks=clock_old;clocks<temp;clocks++)
    hardware();
  clock_old=clocks;
}

void use_project_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  char *buf;
  int n;

  wacthdog_timeout=wacthdog_timeout_temp;
  buf=XmTextFieldGetString(freq_field);
  if((n=strtoul(buf,NULL,10))==0)
    return;
  frequency=n;
  XtFree(buf);
  buf=XmTextFieldGetString(watchdog_freq_field);
  if((n=strtoul(buf,NULL,10))==0)
    return;
  wtd_freq=n;
  XtFree(buf);

  XtVaGetValues(combo_proc,XmNselectedPosition,&type_micro,NULL);
  XtUnmanageChild(project_window);

  set_new_proccessor();
  
/*    if(load_flash(NULL)) {pixmap=CreatePixmap();show_error();} */
/*    if(load_eeprom(NULL)) show_error(); */
  
  SetSensitiveToolbar("Stop",False);
  SetSensitiveMenu("Stop",False);

  init_windows_memory();

  update_gpr_window();
  full_update_io_window();
  full_update_memory_window();
  update_main();
}

void show_info(char *message)
{
  XmString string;

  string=XmStringGenerate(_(message),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(info_dialog,XmNmessageString,string,NULL);
  XmStringFree(string);
  XtManageChild(info_dialog);
}

void info_close_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  pthread_kill(thread,SIGCONT);
}

