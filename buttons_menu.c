/*
 File        : buttons_menu.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : Creation of buttons and menu.

 Copyright notice:

 avr_simulator - A GNU/Linux simulator for the Atmel AVR series
 of microcontrollers. Copyright (C) 2001 Sergey Uvarov

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
#include <libintl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <Xm/XmAll.h>
#include "types.h"
#include "global.h"

void reload_flash_callback();
void reload_eeprom_callback();
void reset_callback();
void trace_into_callback();
void step_over_callback();
void step_out_callback();
void multi_step_callback();
void auto_step_callback();
void run_callback();
void stop_callback();
void load_eeprom_callback();
void load_flash_callback();
void toggle_gpr_callback();
void toggle_sram_callback();
void toggle_io_callback();
void toggle_eeprom_callback();
void toggle_memory_callback();
void logging_callback();
void stimuli_callback();

int thread_stop;

Buttons buttons[]={
  {N_("Open"),load_flash_callback,&file_selection,"xpm/load.xpm",N_("Load Flash")},
  {N_("Reload"),reload_flash_callback,NULL,"xpm/reload.xpm",N_("Reload Flash")},
  {"separator",NULL,NULL,NULL,NULL},
  {N_("Trace Into"),trace_into_callback,NULL,"xpm/trace_into.xpm",
   N_("Trace Into")},
  {N_("Step Over"),step_over_callback,NULL,"xpm/step_over.xpm",N_("Step Over")},
  {N_("Step Out"),step_out_callback,NULL,"xpm/step_out.xpm",N_("Step Out")},
  {N_("Multi Step"),multi_step_callback,NULL,"xpm/multi_step.xpm",N_("Multi Step")},
  {N_("Auto Step"),auto_step_callback,NULL,"xpm/auto_step.xpm",N_("Auto Step")},
  {"separator",NULL,NULL,NULL,NULL},
  {N_("Run"),run_callback,NULL,"xpm/run.xpm",N_("Run")},
  {N_("Stop"),stop_callback,NULL,"xpm/stop.xpm",N_("Stop")},
  {"separator",NULL,NULL,NULL,NULL},
  {N_("Reset"),reset_callback,NULL,"xpm/reset.xpm",N_("Reset")},
  {NULL}
};

Menu file_menu[]={
  {BUTTON,N_("Load Flash..."),"F","Alt+L","Alt<Key>l",NULL,
   load_flash_callback,&file_selection},
  {BUTTON,N_("Reload Flash"),"R",NULL,NULL,NULL,reload_flash_callback,NULL},
  {SEPARATOR,"separaor",NULL,NULL,NULL,NULL,0,NULL,NULL},
  {BUTTON,N_("Load EEPROM..."),"E","Alt+E","Alt<Key>e",NULL,
   load_eeprom_callback,&file_selection},
  {BUTTON,N_("Reload EEPROM"),"o",NULL,NULL,NULL,reload_eeprom_callback,NULL},
  {SEPARATOR,"separaor",NULL,NULL,NULL,NULL,NULL,NULL},
  {BUTTON,N_("Quit"),"Q","Alt+Q","Alt<Key>q",NULL,quit_callback,NULL},
  {0,NULL}
};

Menu memory_menu[]={
  {CHECK,N_("General Registers"),"G",NULL,NULL,NULL,toggle_gpr_callback,
   &gpr_window},
  {CHECK,N_("I/O Registers"),"I",NULL,NULL,NULL,toggle_io_callback,
   &io_window},
  {CHECK,N_("Memory"),"M",NULL,NULL,NULL,toggle_memory_callback,
   &memory_window},
  {0,NULL}
};

Menu debug_menu[]={
  {BUTTON,N_("Reset"),"R",NULL,NULL,NULL,reset_callback,NULL},
  {BUTTON,N_("Run"),"u",NULL,NULL,NULL,run_callback,NULL},
  {BUTTON,N_("Stop"),"S",NULL,NULL,NULL,stop_callback,NULL},
  {BUTTON,N_("Trace Into"),"T",NULL,NULL,NULL,trace_into_callback,NULL},
  {BUTTON,N_("Step Over"),"e",NULL,NULL,NULL,step_over_callback,NULL},
  {BUTTON,N_("Step Out"),"O",NULL,NULL,NULL,step_out_callback,NULL},
  {BUTTON,N_("Multi Step"),"M",NULL,NULL,NULL,multi_step_callback,NULL},
  {BUTTON,N_("Auto Step"),"A",NULL,NULL,NULL,auto_step_callback,NULL},
  {0,NULL}
};

Menu options_menu[]={
  {BUTTON,N_("Project..."),"P",NULL,NULL,NULL,manage,&project_window},
  {BUTTON,N_("Debugger..."),"D",NULL,NULL,NULL,manage,&debugger_window},
  {BUTTON,N_("Port Logging..."),"L",NULL,NULL,NULL,logging_callback,NULL},
  {BUTTON,N_("Port Stimuli..."),"S",NULL,NULL,NULL,stimuli_callback,NULL},
  {0,NULL}
};

Menu help_menu[]={
  {BUTTON,N_("About"),"A",NULL,NULL,NULL,manage,&about_window},
  {0,NULL}
};

Menu menu[]={
  {BUTTON,N_("File"),"F",NULL,NULL,(struct _Menu *)file_menu,
   NULL,NULL,(Widget)0,(Widget)0},
  {BUTTON,N_("View"),"V",NULL,NULL,(struct _Menu *)memory_menu,
   NULL,NULL,(Widget)0,(Widget)0},
  {BUTTON,N_("Debug"),"D",NULL,NULL,(struct _Menu *)debug_menu,
   NULL,NULL,(Widget)0,(Widget)0},
  {BUTTON,N_("Options"),"O",NULL,NULL,(struct _Menu *)options_menu,
   NULL,NULL,(Widget)0,(Widget)0},
  {BUTTON,N_("Help"),"H",NULL,NULL,(struct _Menu *)help_menu,
   NULL,NULL,(Widget)0,(Widget)0},
  {0,NULL}
};

void create_buttons(Widget parent)
{
  XmString string;
  int n,num_buf,num=0,string_check=0;
  Arg args[12];
  Pixel background,background_arm;
  

  XtVaGetValues(parent,XmNbackground,&background,XmNtopShadowColor,
		&background_arm,NULL);

  XtSetArg(args[num],XmNtopAttachment,XmATTACH_FORM);num++;
  XtSetArg(args[num],XmNtopOffset,3);num++;
  XtSetArg(args[num],XmNleftOffset,3);num++;
  XtSetArg(args[num],XmNhighlightThickness,1);num++;
  XtSetArg(args[num],XmNshadowThickness,1);num++;
  XtSetArg(args[num],XmNmarginWidth,1);num++;
  XtSetArg(args[num],XmNmarginHeight,1);num++;
  XtSetArg(args[num],XmNhighlightOnEnter,True);num++;
  num_buf=num;
  string_check=0;
  for(n=0;buttons[n].name!=NULL;n++)
    {
      num=num_buf;
      if(!(strcmp(buttons[n].name,"separator")))
	{
	  buttons[n].widget=
	    XtVaCreateManagedWidget(buttons[n].name,xmSeparatorGadgetClass,
				    parent,XmNtopAttachment,XmATTACH_FORM,
				    XmNtopOffset,2,XmNleftOffset,3,
				    XmNleftAttachment,XmATTACH_WIDGET,
				    XmNleftWidget,buttons[n-1].widget,
				    XmNorientation,XmVERTICAL,
				    XmNbottomAttachment,XmATTACH_FORM,
				    XmNbottomOffset,1,
				    NULL);
	  continue;
	}

      if(buttons[n].name_pixmap!=NULL)
	{
	  if((buttons[n].pixmap=XmGetPixmap(XtScreen(parent),
					    buttons[n].name_pixmap,
					    BlackPixel(XtDisplay(parent),0),
					    WhitePixel(XtDisplay(parent),0)
					    ))==XmUNSPECIFIED_PIXMAP) 
	    {
	      fprintf(stderr," Pixmap problem with file %s\n",
		      buttons[n].name_pixmap);
	      string=XmStringGenerate(gettext(buttons[n].name),
				      XmFONTLIST_DEFAULT_TAG,
				      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	      XtSetArg(args[num],XmNlabelString,string);num++;
	      string_check=1;
	    }
	  else
	    {
	      XtSetArg(args[num],XmNlabelType,XmPIXMAP);num++;
	      XtSetArg(args[num],XmNlabelPixmap,buttons[n].pixmap);num++; 
	      string_check=0;
	    }
	}
      else
	{
	  string=XmStringGenerate(gettext(buttons[n].name),
				  XmFONTLIST_DEFAULT_TAG,
				  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  XtSetArg(args[num],XmNlabelString,string);num++;
	  string_check=1;
	}
      
      if(n==0)
	{
	  XtSetArg(args[num],XmNleftAttachment,XmATTACH_FORM);
	  num++;
	}
      else
	{
	  XtSetArg(args[num],XmNleftAttachment,XmATTACH_WIDGET);num++;
	  XtSetArg(args[num],XmNleftWidget,buttons[n-1].widget);num++;
	}
      
      buttons[n].widget=XmCreatePushButton(parent,buttons[n].name,args,num);
      
      XtAddCallback(buttons[n].widget,
		    XmNactivateCallback,(XtCallbackProc)buttons[n].callback,
		    (XtPointer)buttons[n].call_data);
      if(string_check) XmStringFree(string);
      xs_widget_tip(buttons[n].widget,gettext(buttons[n].prompt_str));
      XtManageChild(buttons[n].widget);
    }
  
}


void create_menu(Widget parent,Menu *menu)
{
  int n,num;
  XmString string,string_accel;
  Arg args[10];
  
  for(n=0;menu[n].name!=NULL;n++)
    {
      if(menu[n].type==SEPARATOR)
	{
	  menu[n].button=
	    XtVaCreateManagedWidget(menu[n].name,xmSeparatorGadgetClass,
				    parent,NULL);
	  continue;
	}
      num=0;
      string=XmStringGenerate(gettext(menu[n].name),XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtSetArg(args[num],XmNlabelString,string);num++;
      XtSetArg(args[num],XmNshadowThickness,1);num++;
      if(menu[n].mnemonic!=NULL)
	{
	  XtSetArg(args[num],XmNmnemonic,XStringToKeysym(menu[n].mnemonic));
	  num++;
	}
      if(menu[n].accelerator_text!=NULL)
	{
	  string_accel=XmStringGenerate(menu[n].accelerator_text,
					XmFONTLIST_DEFAULT_TAG,
					XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  XtSetArg(args[num],XmNacceleratorText,string_accel);num++;
	}

      if(menu[n].accelerator!=NULL)
	{
	  XtSetArg(args[num],XmNaccelerator,menu[n].accelerator);num++;
	}
      if(menu[n].type==BUTTON)
	{
	  if(menu[n].sub_menu!=NULL)
	    {
	      menu[n].pulldown=
		XmCreatePulldownMenu(parent,menu[n].name,NULL,0);
	      XtVaSetValues(menu[n].pulldown,XmNshadowThickness,1,NULL);
	      XtSetArg(args[num],XmNsubMenuId,menu[n].pulldown);num++;
	      menu[n].button=
		XtCreateManagedWidget(menu[n].name,xmCascadeButtonWidgetClass,
				      parent,args,num);
	      create_menu(menu[n].pulldown,menu[n].sub_menu);
	    }
	  else
	    {
	      menu[n].button=
		XtCreateManagedWidget(menu[n].name,xmPushButtonWidgetClass,
				      parent,args,num);
	    }
	  if(menu[n].callback!=NULL)
	    XtAddCallback(menu[n].button,XmNactivateCallback,
			  (XtCallbackProc)menu[n].callback,
			  (XtPointer)menu[n].data);
	}
      if(menu[n].type==CHECK)
	{
	  XtSetArg(args[num],XmNindicatorOn,XmINDICATOR_CHECK);num++;
	  menu[n].button=
	    XtCreateManagedWidget(menu[n].name,xmToggleButtonGadgetClass,
				  parent,args,num);
	  if(menu[n].callback!=NULL)
	    XtAddCallback(menu[n].button,XmNvalueChangedCallback,
			  (XtCallbackProc)menu[n].callback,
			  (XtPointer)menu[n].data);
	}

      if(!strcmp(menu[n].name,"Help"))
	XtVaSetValues(parent,XmNmenuHelpWidget,menu[n].button,NULL);
      XmStringFree(string);
      if(menu[n].accelerator_text!=NULL) XmStringFree(string_accel);
    }
}

Widget FindMenuButton(Menu *menu,char *name)
{
  int n;
  Widget result;
  
  for(n=0;menu[n].name!=NULL;n++)
    {
      if(strcmp(menu[n].name,name)) 
	{
	  if(menu[n].sub_menu==NULL) continue;
	  if((result=FindMenuButton(menu[n].sub_menu,name)))
	    return result;
	}
      else return menu[n].button;
    }
  return 0;
}

Widget FindToolbarButton(char *name)
{
  return 0;
}

void SetSensitiveMenu(char *name,Boolean sen)
{
  Widget w=FindMenuButton(menu,name);
  
  if(w) XtSetSensitive(w,sen);
  else fprintf(stderr,"Menu Button '%s' not found\n",name);
}

void SetSensitiveToolbar(char *name,Boolean sen)
{
  int n;

  for(n=0;n<XtNumber(buttons);n++)
    {
      if(strcmp(buttons[n].name,name)) continue;
      XtSetSensitive(buttons[n].widget,sen);
      return;
    }
  fprintf(stderr,"Toolbar Button '%s' not found\n",name);
}

void change_sensitive_run(Boolean sen)
{
  SetSensitiveToolbar("Stop",sen);
  SetSensitiveToolbar("Reload",!sen);
  SetSensitiveToolbar("Trace Into",!sen);
  SetSensitiveToolbar("Step Over",!sen);
  SetSensitiveToolbar("Step Out",!sen);
  SetSensitiveToolbar("Multi Step",!sen);
  SetSensitiveToolbar("Auto Step",!sen);
  SetSensitiveToolbar("Run",!sen);
  SetSensitiveToolbar("Open",!sen);
  SetSensitiveToolbar("Reset",!sen);

  SetSensitiveMenu("Stop",sen);
  SetSensitiveMenu("Trace Into",!sen);
  SetSensitiveMenu("Step Over",!sen);
  SetSensitiveMenu("Step Out",!sen);
  SetSensitiveMenu("Multi Step",!sen);
  SetSensitiveMenu("Auto Step",!sen);
  SetSensitiveMenu("Run",!sen);
  SetSensitiveMenu("Reset",!sen);
  SetSensitiveMenu("File",!sen);
  SetSensitiveMenu("Options",!sen);
}


void reload_flash_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  load_flash(NULL);
}

void reload_eeprom_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  load_eeprom(NULL);
}

void trace_into_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);
  if(pc==-1)
    {
      reset();
    }
  else
    {
      exec_command();
      drawarea.current_line_deb=list_commands[pc].number_line;
      debug=TRUE;
      draw();
      debug=FALSE;
      if(pc>=controllers[type_micro].size_flash) pc=0;
    }

  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();
}

void step_over_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  int n;

  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom); 
  if(pc==-1)
    {
      reset();
    }
  else
    {
      if(strncmp(list_commands[pc].mnemocode,"call",4)&&
	 strncmp(list_commands[pc].mnemocode,"rcall",5)&&
	 strncmp(list_commands[pc].mnemocode,"icall",5)&&
	 strncmp(list_commands[pc].mnemocode,"eicall",6))
	{
	  exec_command();
	}
      else
	{
	  n=0;
	  do{
	    exec_command();
	    n++;
	    if(list_commands[pc].flags&BREAKPOINT) break;
	  }while(strncmp(list_commands[pc].mnemocode,"ret",3)&&n<2000);
	  if(!(list_commands[pc].flags&BREAKPOINT)) exec_command();
	}
      drawarea.current_line_deb=list_commands[pc].number_line;
      debug=TRUE;
      draw();
      debug=FALSE;
      if(pc>=controllers[type_micro].size_flash) pc=0;
    }
  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();
}

void *run_thread(void *foo)
{
  int time,old_time;
  char buf[12];

  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);
  thread_stop=0;
  change_sensitive_run(True);

  do{
    exec_command();
    if(pc>=controllers[type_micro].size_flash) pc=0;
    if(list_commands[pc].flags&BREAKPOINT) break;
    time=(int)(clocks*1000.0/frequency);
    if(time%10==0&&old_time!=time)
      {
	switch(type_time)
	  {
	  case uS: sprintf(buf,"%f:6",1.0/frequency*1E6*clocks);break;
	  case mS: sprintf(buf,"%f:6",1.0/frequency*1E3*clocks);break;
	  case  S: sprintf(buf,"%f:6",1.0/frequency*clocks);break;
	  }	
	XmTextFieldSetString(edit_time,buf);
	XmUpdateDisplay(edit_time);
      }
    old_time=time;
    if(thread_stop) break;
  }while(1);
  
  drawarea.current_line_deb=list_commands[pc].number_line;
  debug=TRUE;
  draw();
  debug=FALSE;

  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();
  
  change_sensitive_run(False);
  return NULL;
}

void run_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  if(pc==-1) reset();
  else
    {
      if(pthread_create(&thread,&attr_thread,run_thread,NULL))
	perror("pthread_create");
    }
}

void stop_callback(Widget w,XtPointer client_data, XtPointer call_data)
{
  thread_stop=1;
}

void load_eeprom_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  load_memory=EEPROM;
  XtManageChild(*((Widget *)client_data));
}

void load_flash_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  load_memory=FLASH;
  XtManageChild(*((Widget *)client_data));
}

void toggle_gpr_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  Widget *window=(Widget *)client_data;
  
  if(((XmToggleButtonCallbackStruct *)call_data)->set==XmSET)
    {
      if(!*window) create_gpr_window();
      else XtManageChild(*window);
      update_gpr_window();
    }
  else XtUnmanageChild(*window);
  return;
}

void toggle_io_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  Widget *window=(Widget *)client_data;
  
  if(((XmToggleButtonCallbackStruct *)call_data)->set==XmSET)
    {
      if(!*window)   create_io_ports_window();
      else XtManageChild(*window);
      full_update_io_window();
    }
  else XtUnmanageChild(*window);
  return;
}

void toggle_memory_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  Widget *window=(Widget *)client_data;
  
  if(((XmToggleButtonCallbackStruct *)call_data)->set==XmSET)
    {
      if(!*window) 
	{
	  create_memory_window();
	}
      else 
	{
	  draw_memory_window();
	  XtManageChild(*window);
	}
    }
  else XtUnmanageChild(*window);
}

void reset_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  reset();
}

void step_out_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  int n;
  
  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);

  if(pc==-1) reset();
  if(!num_call) {trace_into_callback(w,client_data,call_data);return;}
  else
    {
      n=0;
      do{
	exec_command();
	n++;
	if(list_commands[pc].flags&BREAKPOINT) break;
      }while(strncmp(list_commands[pc].mnemocode,"ret",3)&&n<2000);
      
      if(!(list_commands[pc].flags&BREAKPOINT)) exec_command();

      drawarea.current_line_deb=list_commands[pc].number_line;
      debug=TRUE;
      draw();
      debug=FALSE;
      if(pc>=controllers[type_micro].size_flash) pc=0;
    }
  
  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();
}

void multi_step_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  int n;
  
  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);

  if(pc==-1) reset();
  else
    {
      n=0;
      do{
	exec_command();
	if(pc>=controllers[type_micro].size_flash) pc=0;
	n++;
	if(list_commands[pc].flags&BREAKPOINT) break;
      }while(n<multi_step);
      
      drawarea.current_line_deb=list_commands[pc].number_line;
      debug=TRUE;
      draw();
      debug=FALSE;
    }
  
  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();
}

void *auto_step_thread(void *foo)
{
  int n;
  int time,old_time;
  char buf[12];

  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);

  thread_stop=0;
  change_sensitive_run(True);
  
  n=clocks;
  do{
    exec_command();
    if(pc>=controllers[type_micro].size_flash) pc=0;
    if(list_commands[pc].flags&BREAKPOINT) break;
    time=(clocks-n)*1000/frequency;
    if(time%10==0&&old_time!=time)
      {
	switch(type_time)
	  {
	  case uS: sprintf(buf,"%f:6",1.0/frequency*1E6*clocks);break;
	  case mS: sprintf(buf,"%f:6",1.0/frequency*1E3*clocks);break;
	  case  S: sprintf(buf,"%f:6",1.0/frequency*clocks);break;
	  }
	XmTextFieldSetString(edit_time,buf);
      }
    old_time=time;
    if(thread_stop) break;
  }while(auto_step>time);
  
  drawarea.current_line_deb=list_commands[pc].number_line;
  debug=TRUE;
  draw();
  debug=FALSE;

  update_gpr_window();
  update_io_window(); 
  update_main();
  full_update_memory_window();

  change_sensitive_run(False);
 
  return NULL;
}

void auto_step_callback(Widget w,XtPointer client_data,XtPointer call_data)
{   
  memset(mem_file_changed,0,size_mem_file);
  memset(eeprom_changed,0,size_eeprom);

  if(pc==-1) reset();
  else
    {
      if(pthread_create(&thread,&attr_thread,auto_step_thread,NULL))
	perror("pthread_create");
    }
}

void logging_callback()
{
  int n;

  ports_window_flag=0;
  for(n=0;n<6;n++)
    if(data_port_log[n].state!=-1)
      {
	XtSetSensitive(data_port_window[n].button1,True);
	XtVaSetValues(data_port_window[n].button1,XmNset,
		      data_port_log[n].state,NULL);
	XmTextFieldSetString(data_port_window[n].data,
			     data_port_log[n].filename);
	if(data_port_log[n].state==XmSET)
	  {
	    XtSetSensitive(data_port_window[n].data,True);
	    XtSetSensitive(data_port_window[n].button2,True);
	  }
	else
	  {
	    XtSetSensitive(data_port_window[n].data,False);
	    XtSetSensitive(data_port_window[n].button2,False);
	  }
      }
    else
      {
	XtSetSensitive(data_port_window[n].button1,False);
	XtSetSensitive(data_port_window[n].data,False);
	XtSetSensitive(data_port_window[n].button2,False);
      }
  XtVaSetValues(XtParent(ports_window),XmNtitle,_("Logging ports"),NULL);
  XtManageChild(ports_window);
}

void stimuli_callback()
{
  int n;

  ports_window_flag=1;
  for(n=0;n<6;n++)
    if(data_port_sti[n].state!=-1)
      {
	XtSetSensitive(data_port_window[n].button1,True);
	XtVaSetValues(data_port_window[n].button1,XmNset,
		      data_port_sti[n].state,NULL);
	XmTextFieldSetString(data_port_window[n].data,
			     data_port_sti[n].filename);
	if(data_port_sti[n].state==XmSET)
	  {
	    XtSetSensitive(data_port_window[n].data,True);
	    XtSetSensitive(data_port_window[n].button2,True);
	  }
	else
	  {
	    XtSetSensitive(data_port_window[n].data,False);
	    XtSetSensitive(data_port_window[n].button2,False);
	  }
      }
    else
      {
	XtSetSensitive(data_port_window[n].button1,False);
	XtSetSensitive(data_port_window[n].data,False);
	XtSetSensitive(data_port_window[n].button2,False);
      }
  XtVaSetValues(XtParent(ports_window),XmNtitle,_("Stimuli ports"),NULL);
  XtManageChild(ports_window);
}

