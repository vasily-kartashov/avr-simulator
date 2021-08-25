/*
 File        : menu_memory.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : This is the file to create windows of memory.

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
#include <stdlib.h>
#include <libintl.h>
#include <ctype.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/XmAll.h>
#include "types.h"
#include "global.h"

#define MAX_IO_PORT_LINES 30

int type_gpr_window=HEX,
  nlines_io=10,            /* number of lines in io_window               */
  address_io=0,            /* address of start point of io_window        */
  address_io_old=0,         /* need only for tip window in io_window      */
  type_micro_old,          /* need only for tip windows                  */    
  height_min_io_window,    /* height of io_window without port lines     */
  io_window_step;          /* number of incr. pixels of io_window        */
  
Widget *field=NULL,        /* edit field in gpr_window                */
  vscr_io_bar,             /* scrollbar in io_window                  */
  *label_io=NULL,          /* array of labels in io_window            */
  **bit_button=NULL,       /* array of bit buttons                    */
  *label_io_val=NULL,      /* array of values in io_window            */
  *label_io_addr=NULL,     /* aray of address labels on io_window     */
  dataform_io;             /* form io_widget to place data            */

char **gpr_tip_str=NULL;
Pixel red_pixel;

char *memory_type_str[]={
  N_("Registers"),N_("I/O registers"),N_("Ext I/O Registers"),N_("SRAM"),
  N_("Data"),N_("EEPROM"),N_("FLASH")};

struct MemoryEditWindowData{
  Widget address_widget,text_widget;
  int data_type;
  uint32_t address;
} mewd;

void update_gpr_window();
void update_io_window();
void full_update_io_window();
void switch_button_gpr();
void close_button_gpr();
void close_button_io();
void scroll_bar_io_callback();
void get_focus_field_callback();
void lose_focus_field_callback();
void field_changed_callback(); 
void set_size_io_window();
void bit_button_callback();
void resize_io();
void create_io_ports_line();
void init_memory_window(void);

void init_windows_memory(void)
{
  int n;

  /* allocate memory for 'mem_file_changed' */
  if(mem_file_changed!=NULL) free(mem_file_changed);
  size_mem_file=(controllers[type_micro].size_gpr+
		 controllers[type_micro].size_io_reg+
		 controllers[type_micro].size_sram)/8+1;
  
  if((mem_file_changed=malloc(size_mem_file))==NULL)
    {
      puts("init_windows_memory(): can't allocate memory for 'mem_file_changed'");
      exit(1);
     }
  memset(mem_file_changed,0,size_mem_file);

  /* allocate memory for 'eeprom_file_changed' */
  if(eeprom_changed!=NULL) free(eeprom_changed);
  size_eeprom=controllers[type_micro].size_eeprom;
  
  if((eeprom_changed=malloc(size_eeprom))==NULL)
    {
      puts("init_windows_memory(): can't allocate memory for 'eeprom_changed'");
      exit(1);
    }
  memset(eeprom_changed,0,size_eeprom);

  if(io_window)
    {
      /* set parameters of scrollbar of io window */
      for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++);
      XtVaSetValues(vscr_io_bar,XmNmaximum,n,XmNsliderSize,nlines_io,NULL);
    }
  init_memory_window();
}

char *conv_to_bin(int number,char *buf,Boolean space)
{
  int n,max;
  
  if(buf==NULL)
    {
      puts("conv_to_bin(): buf==NULL");
      return NULL;
    }
  max=space?9:8;
  for(n=0;n<max;n++)
    {
      if(n==4&&max==9) {buf[n]=' ';n++;}
      if(number%2)
	buf[max-1-n]='1';
      else
	buf[max-1-n]='0';
      number/=2;
    }
  buf[n]=0;
  return buf;
}

/* GPR window */
void create_gpr_window(void)
{  
  char buf[5];
  Widget button,shell,frame,gpr_column,column;
  int n,num,m,max=controllers[type_micro].size_gpr;
  XmString string;
  Arg args[19];
  XColor color;
  XtWidgetGeometry size;

 if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "red",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=BlackPixel(display,scr_num);
    }  
  red_pixel=color.pixel;

  if((field=malloc(max*sizeof(Widget)))==NULL)
    {
      puts("create_gpr_window(): can't allocate memory for 'field'");
      exit(1);
    } 
  if((gpr_tip_str=malloc(max*sizeof(char *)))==NULL)
    {
      puts("create_gpr_window(): can't allocate memory for 'gpr_tip_str'");
      exit(1);
    }
  
  for(n=0;n<max;n++)
    {
      if((gpr_tip_str[n]=malloc(10*sizeof(char)))==NULL)
	{
	  printf("create_gpr_window(): \
can't allocate memory for 'gpr_tip_str[%d]'",n);
	  exit(1);
	}
      gpr_tip_str[n][0]=0;
    }


  shell=XtVaCreateWidget("Shell",xmDialogShellWidgetClass,toplevel,
			 XmNtitle,_("General Registers"),
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,
			 XmNmappedWhenManaged,False,NULL);
  gpr_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
			      XmNautoUnmanage,False,
			      XmNdialogStyle,XmDIALOG_MODELESS,
			      XmNresizePolicy,XmRESIZE_NONE,
			      XmNfractionBase,5,NULL);

  frame=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,gpr_window,
				XmNtopAttachment,XmATTACH_FORM,
				XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,5,
				XmNrightAttachment,XmATTACH_FORM,
				XmNrightOffset,5,NULL);
  gpr_column=XtVaCreateManagedWidget("form",xmRowColumnWidgetClass,frame,
				     XmNorientation,XmVERTICAL,
				     XmNnumColumns,max/8,
				     XmNpacking,XmPACK_COLUMN,
				     XmNmarginWidth,3,XmNmarginHeight,3,NULL);

  XtSetArg(args[m=0],XmNleftAttachment,XmATTACH_WIDGET);m++;
  XtSetArg(args[m],XmNtopOffset,6);m++;
  XtSetArg(args[m],XmNleftOffset,0);m++;
  XtSetArg(args[m],XmNcolumns,3);m++;
  XtSetArg(args[m],XmNmaxLength,2);m++;
  XtSetArg(args[m],XmNvalue,"0");m++;
  XtSetArg(args[m],XmNcursorPositionVisible,False);m++;
  
  for(n=0;n<max;n++)
    {
      num=m;
      sprintf(buf,"r%-2d:",n);
      column=XtVaCreateManagedWidget("column",xmRowColumnWidgetClass,gpr_column,
				     XmNorientation,XmHORIZONTAL,
				     XmNmarginWidth,0,XmNmarginHeight,0,NULL);
      string=XmStringGenerate(buf, XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaCreateManagedWidget("r",xmLabelWidgetClass,column,XmNlabelString,string,
			      NULL);
      XmStringFree(string);
      
      field[n]=XtCreateManagedWidget("gpr_field",xmTextFieldWidgetClass,column,
				     args,num);
      xs_widget_tip(field[n],gpr_tip_str[n]);
      XtAddCallback(field[n],XmNfocusCallback,(XtCallbackProc)
		    get_focus_field_callback,NULL);
      XtAddCallback(field[n],XmNlosingFocusCallback,(XtCallbackProc)
		    lose_focus_field_callback,NULL);
    }

  string=XmStringGenerate(_("Hex"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Dec",xmPushButtonGadgetClass,
				 gpr_window,XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,6,XmNtopWidget,frame,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,XmNlabelString,string,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,2,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,6,NULL);
  
  XtAddCallback(button,
		XmNactivateCallback,(XtCallbackProc)switch_button_gpr,NULL);
  XmStringFree(string);
  
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 gpr_window,XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,6,XmNtopWidget,frame,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,3,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,4,XmNlabelString,string,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,6,NULL);
  
  XtAddCallback(button,
		XmNactivateCallback,(XtCallbackProc)close_button_gpr,NULL);
  XmStringFree(string);

  XtManageChild(gpr_window);
  XtManageChild(shell);

  /* set size */
  XtQueryGeometry(shell,NULL,&size);
  XtVaSetValues(shell,XmNminWidth,size.width,XmNmaxWidth,size.width,
		XmNminHeight,size.height,XmNmaxHeight,size.height,
		XmNmappedWhenManaged,True,NULL);

}

void update_gpr_window(void)
{
  int n;
  char buf[4];
    
  if(!gpr_window) return;
  if(!XtIsManaged(gpr_window)) return;

  for(n=0;n<controllers[type_micro].size_gpr;n++)
    {
      if(type_gpr_window==HEX) sprintf(buf,"%X",gpr_pointer[n]);
      else sprintf(buf,"%u",gpr_pointer[n]);
      
      delete_tip(field[n],gpr_tip_str[n]);
      xs_widget_tip(field[n],
		    conv_to_bin((int)gpr_pointer[n],gpr_tip_str[n],True));
      
      if(mem_file_changed[n/8]&(1<<n%8))
	XtVaSetValues(field[n],XmNvalue,buf,XmNforeground,red_pixel,NULL);
      else
	XtVaSetValues(field[n],XmNvalue,buf,XmNforeground,
		      BlackPixel(display,scr_num),NULL);

    }
}

void switch_button_gpr(Widget w,XtPointer client_data,XtPointer call_data)
{
  XmString string;
  int n;
  char buf[4];

  if(type_gpr_window==HEX) 
    {
      type_gpr_window=DEC;
      string=XmStringGenerate(_("Dec"),XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(w,XmNlabelString,string,NULL);
      XmStringFree(string);
      for(n=0;n<controllers[type_micro].size_gpr;n++)
	XtVaSetValues(field[n],XmNmaxLength,3,NULL);
    }
  else
    {
      type_gpr_window=HEX;
      string=XmStringGenerate(_("Hex"),XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(w,XmNlabelString,string,NULL);
      XmStringFree(string);
      for(n=0;n<controllers[type_micro].size_gpr;n++)
	XtVaSetValues(field[n],XmNmaxLength,2,NULL);
    }
  
  /* update */
  for(n=0;n<controllers[type_micro].size_gpr;n++)
    {
      if(type_gpr_window==HEX)
	sprintf(buf,"%X",gpr_pointer[n]);
      else sprintf(buf,"%u",gpr_pointer[n]);
      XtVaSetValues(field[n],XmNvalue,buf,NULL); 
    }
}

void field_changed_callback(Widget w,XtPointer client_data,
			    XtPointer call_data)
{
  char *buf,buf2[10];
  int n;

  buf=XmTextFieldGetString(w);
  n=0;
  while(n<controllers[type_micro].size_gpr&&w!=field[n]) n++;
  if(type_gpr_window==DEC)
    gpr_pointer[n]=strtoul(buf,NULL,10);
  else
    {
      sprintf(buf2,"0x%s",buf);
      gpr_pointer[n]=strtoul(buf2,NULL,16);
    }
  delete_tip(field[n],gpr_tip_str[n]);
  xs_widget_tip(field[n],conv_to_bin((int)gpr_pointer[n],gpr_tip_str[n],True));
  XtFree(buf);
  if(n>25) update_main(); 
}

void close_button_gpr(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtVaSetValues(FindMenuButton(memory_menu,"General Registers"),
		XmNset,XmUNSET,NULL);
  XtUnmanageChild(gpr_window);
}

/* allow visible cursor */
void get_focus_field_callback (Widget w,XtPointer client_data,
			       XtPointer call_data)
{
  XtVaSetValues(w,XmNcursorPositionVisible,True,NULL);
  XtAddCallback(w,XmNvalueChangedCallback,(XtCallbackProc)
		field_changed_callback,NULL);
}

/* don't allow visible cursor*/
void lose_focus_field_callback (Widget w,XtPointer client_data,
			       XtPointer call_data)
{
  XtVaSetValues(w,XmNcursorPositionVisible,False,NULL);
  XtRemoveCallback(w,XmNvalueChangedCallback,(XtCallbackProc)
		   field_changed_callback,NULL);
}

/* I/O window */
void create_io_ports_window(void)
{
  Widget scroll,temp,shell,frame,io_form,*widgetlist;
  XmString string;
  XtWidgetGeometry size;
  int n,num_ports,n_wid;
  Dimension height_label;

  address_io_old=address_io;
  for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++);
  num_ports=n;

  shell=XtVaCreateWidget("IO_WINDOW",xmDialogShellWidgetClass,toplevel,
			 XmNtitle,_("I/O Registers"),
			 XmNkeyboardFocusPolicy,XmPOINTER,
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,
			 XmNmappedWhenManaged,False,NULL);
  io_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
			     XmNautoUnmanage,False,
			     XmNdialogStyle,XmDIALOG_MODELESS,
			     XmNfractionBase,3,NULL);

  frame=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,io_window,
				XmNtopAttachment,XmATTACH_FORM,
				XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,5,
				XmNrightAttachment,XmATTACH_FORM,
				XmNrightOffset,5,NULL);
  io_form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame,NULL);
  
  string=XmStringGenerate(_("Name  Addr  Bit values   Hex"),
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  temp=XtVaCreateManagedWidget("label",xmLabelGadgetClass,io_form,
			       XmNlabelString,string,XmNtopOffset,5,
			       XmNleftAttachment,XmATTACH_FORM,
			       XmNleftOffset,5,NULL);
  XmStringFree(string);
  
  temp=XtVaCreateManagedWidget("separator",xmSeparatorGadgetClass,io_form,
			       XmNtopAttachment,XmATTACH_WIDGET,
			       XmNtopWidget,temp,
			       XmNleftAttachment,XmATTACH_FORM,
			       XmNrightAttachment,XmATTACH_FORM,NULL);
  /* create scroll window */
  scroll=XtVaCreateManagedWidget("Scroll",xmScrolledWindowWidgetClass,
				 io_form,
				 XmNscrollingPolicy,XmAPPLICATION_DEFINED,
				 XmNscrollBarDisplayPolicy,XmSTATIC,
				 XmNvisualPolicy,XmVARIABLE,
				 XmNleftAttachment,XmATTACH_FORM,
				 XmNleftOffset,3,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNrightOffset,2,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopWidget,temp,
				 XmNtopOffset,2,
				 XmNbottomAttachment,XmATTACH_FORM,
				 NULL);
  
  dataform_io=XtVaCreateManagedWidget("form area",xmFormWidgetClass,scroll,
				      NULL);
  
  vscr_io_bar=XtVaCreateManagedWidget("scr_bar",xmScrollBarWidgetClass,
				      scroll,XmNorientation,XmVERTICAL,
				      XmNincrement,1,XmNminimum,0,
				      XmNvalue,0,
				      XmNmaximum,num_ports,
				      XmNsliderSize,nlines_io,
				      NULL);
  XtAddCallback(vscr_io_bar,XmNvalueChangedCallback,
		(XtCallbackProc)scroll_bar_io_callback,NULL);
  XtAddCallback(vscr_io_bar,XmNdragCallback,
		(XtCallbackProc)scroll_bar_io_callback,NULL);
  XmScrolledWindowSetAreas(scroll,NULL,vscr_io_bar,dataform_io);
 
  /* allocate memory for labels and bit buttons */
  if(nlines_io>num_ports) nlines_io=num_ports;
  if((label_io=malloc(MAX_IO_PORT_LINES*sizeof(Widget)))==NULL)
    {
      puts("create_io_ports_window(): can't allocate memory for 'label_io'");
      exit(1);
    }
  memset(label_io,0,MAX_IO_PORT_LINES*sizeof(Widget));

  if((bit_button=malloc(MAX_IO_PORT_LINES*sizeof(Widget *)))==NULL)
    {
      puts("create_io_ports_window(): \
can't allocate memory for 'bit_button'");
      exit(1);
    }
  for(n=0;n<MAX_IO_PORT_LINES;n++)
    if((bit_button[n]=malloc(8*sizeof(Widget)))==NULL)
      {
      printf("create_io_ports_window(): \
can't allocate memory for 'bit_button[%d]'",n);
      exit(1);
      }
  if((label_io_val=malloc(MAX_IO_PORT_LINES*sizeof(Widget)))==NULL)
    {
      puts("create_io_ports_window(): \
can't allocate memory for 'label_io_val'");
      exit(1);
    }
  if((label_io_addr=malloc(MAX_IO_PORT_LINES*sizeof(Widget)))==NULL)
    {
      puts("create_io_ports_window(): \
can't allocate memory for 'label_io_addr'");
      exit(1);
    }

  /* create name labels of ports and bit buttons */
  if((widgetlist=malloc(nlines_io*11*sizeof(Widget)))==NULL)
    {
      puts("create_io_ports_window(): \
can't allocate memory for 'widgetlist'");
      exit(1);
    }
  n_wid=0;
  for(n=0;n<nlines_io;n++) create_io_ports_line(n,widgetlist,&n_wid);
  XtManageChildren(widgetlist,n_wid);
  free(widgetlist);

  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  temp=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
			       io_window,
			       XmNrightAttachment,XmATTACH_POSITION,
			       XmNrightPosition,2,XmNlabelString,string,
			       XmNleftAttachment,XmATTACH_POSITION,
			       XmNleftPosition,1,
			       XmNbottomAttachment,XmATTACH_FORM,
			       XmNbottomOffset,6,NULL);
  
  XtAddCallback(temp,
		XmNactivateCallback,(XtCallbackProc)close_button_io,NULL);
  XmStringFree(string);
  XtVaSetValues(frame,XmNbottomAttachment,XmATTACH_WIDGET,
		XmNbottomWidget,temp,XmNbottomOffset,6,NULL);
  XtAddEventHandler(shell,StructureNotifyMask,False,resize_io,NULL);

  XtManageChild(io_window);
  XtManageChild(shell);
  
  /* set size */
  XtQueryGeometry(shell,NULL,&size);
  XtVaGetValues(label_io[0],XmNheight,&height_label,NULL);
  io_window_step=height_label+2;
  height_min_io_window=size.height-nlines_io*io_window_step;

  XtVaSetValues(shell,XmNminWidth,size.width,XmNmaxWidth,size.width,
		XmNheightInc,io_window_step,XmNbaseHeight,size.height,
		XmNmappedWhenManaged,True,NULL);
}


void create_io_ports_line(int n,Widget *widgetlist,int *num_wid)
{
  int i,n_wid,n_args;
  char buf[10];
  XmString string;
  Arg args[10];

  n_args=0;n_wid=*num_wid;
  /* create name labels */
  string=XmStringGenerate(descr_io_ports[controllers[type_micro].io_registers[n].code].name,
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  
  XtSetArg(args[n_args],XmNlabelString,string);n_args++;
  XtSetArg(args[n_args],XmNtopOffset,2);n_args++;
  XtSetArg(args[n_args],XmNleftAttachment,XmATTACH_FORM);n_args++;
  
  if(n==0) 
    { XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_FORM);n_args++;}
  else
    {
      XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_WIDGET);n_args++;
      XtSetArg(args[n_args],XmNtopWidget,label_io[n-1]);n_args++;
    }
  
  widgetlist[n_wid++]=
    label_io[n]=XtCreateWidget("label",xmLabelWidgetClass,
			       dataform_io,args,n_args);
  XmStringFree(string);
  xs_widget_tip(label_io[n],descr_io_ports[controllers[type_micro].io_registers[n+address_io].code].fullname);
  
  /* create address labels */
  sprintf(buf,"%Xh",controllers[type_micro].io_registers[n].address);
  fill_zero(buf,3);
  string=XmStringGenerate(buf,
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  n_args=0;
  XtSetArg(args[n_args],XmNlabelString,string);n_args++;
  XtSetArg(args[n_args],XmNtopOffset,2);n_args++;
  XtSetArg(args[n_args],XmNleftAttachment,XmATTACH_WIDGET);n_args++;
  XtSetArg(args[n_args],XmNleftWidget,label_io[n]);n_args++;

  if(n==0)
    {
      XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_FORM);n_args++;
    }
  else
    {
      XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_WIDGET);n_args++;
      XtSetArg(args[n_args],XmNtopWidget,label_io_addr[n-1]);n_args++;
      /*  if(n==nlines_io) */
/*  	{ */
/*  	  XtVaSetValues(label_io[n],XmNbottomAttachment,XmATTACH_FORM, */
/*  			NULL); */
/*  	  XtVaSetValues(label_io_addr[n],XmNbottomAttachment,XmATTACH_FORM, */
/*  			NULL); */
/*  	} */
    }

  widgetlist[n_wid++]=
    label_io_addr[n]=XtCreateWidget("label",xmLabelGadgetClass,dataform_io,
				    args,n_args);
  XmStringFree(string);
  
  /* create bit button */
  n_args=0;
  XtSetArg(args[n_args],XmNtopOffset,0);n_args++;
  XtSetArg(args[n_args],XmNhighlightOnEnter,True);n_args++;
  XtSetArg(args[n_args],XmNshadowThickness,0);n_args++;

  for(i=0;i<8;i++)
    {
      n_args=4;
      if(i==0)
	{
	  XtSetArg(args[n_args],XmNleftAttachment,XmATTACH_WIDGET);n_args++;
	  XtSetArg(args[n_args],XmNleftWidget,label_io_addr[n]);n_args++;
	  XtSetArg(args[n_args],XmNleftOffset,5);n_args++;
	}
      else
	{
	  XtSetArg(args[n_args],XmNleftAttachment,XmATTACH_WIDGET);n_args++;
	  XtSetArg(args[n_args],XmNleftWidget,bit_button[n][i-1]);n_args++;
	}
      if(n==0)
	{XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_FORM);n_args++;}
      else
	{
	  XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_WIDGET);n_args++;
	  XtSetArg(args[n_args],XmNtopWidget,bit_button[n-1][i]);n_args++;
	}

      widgetlist[n_wid++]=
	bit_button[n][i]=XtCreateWidget("0",xmPushButtonGadgetClass,
					  dataform_io,args,n_args);
      
      if(!((controllers[type_micro].io_registers[n].read_mask|
	    controllers[type_micro].io_registers[n].write_mask)
	   &(1<<(7-i))))
	XtSetSensitive(bit_button[n][i],False);
      
      XtAddCallback(bit_button[n][i],
		    XmNactivateCallback,
		    (XtCallbackProc)bit_button_callback,NULL);
    }

  /* create labels of values */
  string=XmStringGenerate("00h",XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  n_args=0;
  XtSetArg(args[n_args],XmNlabelString,string);n_args++;
  XtSetArg(args[n_args],XmNtopOffset,2);n_args++;
  XtSetArg(args[n_args],XmNleftAttachment,XmATTACH_WIDGET);n_args++;
  XtSetArg(args[n_args],XmNleftWidget,bit_button[n][7]);n_args++;
  XtSetArg(args[n_args],XmNleftOffset,10);n_args++;

 
  if(n==0)
    {
      XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_FORM);n_args++;
      XtSetArg(args[n_args],XmNtopOffset,1);n_args++;
    }
  else
    {
      XtSetArg(args[n_args],XmNtopAttachment,XmATTACH_WIDGET);n_args++;
      XtSetArg(args[n_args],XmNtopWidget,label_io_val[n-1]);n_args++;
    }
  if(n==nlines_io)
    {XtSetArg(args[n_args],XmNbottomAttachment,XmATTACH_FORM);n_args++;}
  
  widgetlist[n_wid++]=
    label_io_val[n]=XtCreateWidget("label",xmLabelGadgetClass,dataform_io,
				   args,n_args);
  XmStringFree(string);
  
  *num_wid=n_wid;
}

void resize_io(Widget w,char *m,XConfigureEvent *event,
	       Boolean *continueDispatch)
{
  int old,num_ports,n,n_wid;
  Widget *widgetlist;
  
  if(event->type==ConfigureNotify)
    if(event->height!=height_min_io_window+nlines_io*io_window_step)
      {
	for(n=0;controllers[type_micro].io_registers[n].code!=-1;n++);
	num_ports=n;
	old=nlines_io;
	nlines_io=(event->height-height_min_io_window)/io_window_step;
	
	if(nlines_io>num_ports)
	  {
	    nlines_io=num_ports;
	    if(nlines_io==old) return;
	  }
	
	if(nlines_io>old)
	  { /* increase io_lines */

	    if((widgetlist=malloc(nlines_io*11*sizeof(Widget)))==NULL)
	      {
		puts("resize_io(): can't allocate memory for 'widgetlist'");
		exit(1);
	      }
	    n_wid=0;
	    for(n=old;n<nlines_io;n++)
	      {
		
		if(label_io[n]==NULL)
		  create_io_ports_line(n,widgetlist,&n_wid);
	      }
	    if(n_wid) XtManageChildren(widgetlist,n_wid);
	    free(widgetlist);
	  }
	
	XtVaSetValues(vscr_io_bar,XmNmaximum,num_ports,XmNsliderSize,
		      nlines_io,NULL);
	
	if(address_io+nlines_io>num_ports)
	  {
	    address_io_old=address_io;
	    address_io=num_ports-nlines_io;
	    XtVaSetValues(vscr_io_bar,XmNvalue,num_ports-nlines_io,NULL);
	  }
	
	full_update_io_window();
      }
}

void update_io_window(void)
{
  int i,n,address;
  XmString string;
  char buf[10];
    
  if(!io_window) return;
  if(!XtIsManaged(io_window)) return;
  
  for(n=0;n<nlines_io;n++)
    {
      address=controllers[type_micro].size_gpr+
	controllers[type_micro].io_registers[address_io+n].address;
      
      if(!(mem_file_changed[address/8]&(1<<address%8))) continue;
      
      for(i=0;i<8;i++)
	{
	  if(gpr_pointer[address]&(1<<(7-i)))
	    string=XmStringGenerate("1",XmFONTLIST_DEFAULT_TAG,
				    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  else
	    string=XmStringGenerate("0",XmFONTLIST_DEFAULT_TAG,
				    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  XtVaSetValues(bit_button[n][i],XmNlabelString,string,NULL);
	  XmStringFree(string);
	}
      
      sprintf(buf,"%X",gpr_pointer[address]);
      fill_zero(buf,2);
      buf[2]='h';buf[3]=0;
      string=XmStringGenerate(buf,XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(label_io_val[n],XmNlabelString,string,NULL);
      XmStringFree(string);
    }
}

void full_update_io_window(void)
{
  int i,n;
  XmString string;
  char buf[10];

  if(!io_window) return;
  if(!XtIsManaged(io_window)) return;
  

  for(n=0;n<nlines_io;n++)
    {
      string=XmStringGenerate(descr_io_ports[controllers[type_micro].io_registers[n+address_io].code].name,
			      XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(label_io[n],XmNlabelString,string,NULL);
      XmStringFree(string);
      
      delete_tip(label_io[n],descr_io_ports[controllers[type_micro_old].io_registers[n+address_io_old].code].fullname);
      xs_widget_tip(label_io[n],descr_io_ports[controllers[type_micro].io_registers[n+address_io].code].fullname);

      sprintf(buf,"%Xh",
	      controllers[type_micro].io_registers[n+address_io].address);
      fill_zero(buf,3);
      string=XmStringGenerate(buf,XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(label_io_addr[n],XmNlabelString,string,NULL);
      XmStringFree(string);
      for(i=0;i<8;i++)
	{
	  if((controllers[type_micro].io_registers[address_io+n].read_mask|
		controllers[type_micro].io_registers[address_io+n].write_mask)
	       &(1<<(7-i)))
	    XtSetSensitive(bit_button[n][i],True);
	  else 
	    XtSetSensitive(bit_button[n][i],False);
	  
	  if(io_pointer[controllers[type_micro].io_registers[address_io+n].address]&(1<<(7-i)))
	    string=XmStringGenerate("1",XmFONTLIST_DEFAULT_TAG,
				    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  else
	    string=XmStringGenerate("0",XmFONTLIST_DEFAULT_TAG,
				    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
	  XtVaSetValues(bit_button[n][i],XmNlabelString,string,NULL);
	  XmStringFree(string);
	}
      
      sprintf(buf,"%X",io_pointer[controllers[type_micro].io_registers[address_io+n].address]);
      fill_zero(buf,2);
      buf[2]='h';buf[3]=0;
      string=XmStringGenerate(buf,XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(label_io_val[n],XmNlabelString,string,NULL);
      XmStringFree(string);
    }
  address_io_old=address_io;
  type_micro_old=type_micro;
}

void scroll_bar_io_callback(Widget w,XtPointer client_data,
			    XtPointer call_data)
{
  address_io=((XmScrollBarCallbackStruct *)call_data)->value;
  full_update_io_window();
}

void bit_button_callback(Widget w,XtPointer client_data,XtPointer call_data)
{
  int n,m;
  char buf[5];
  XmString string;

  n=m=0;
  while(w!=bit_button[n][m])
    if(++m==8) {m=0;n++;}
  if(io_pointer[controllers[type_micro].io_registers[address_io+n].address]
     &(1<<(7-m)))
    {
      io_pointer[controllers[type_micro].io_registers[address_io+n].address]=
	io_pointer[controllers[type_micro].io_registers[address_io+n].address]
	&~(1<<(7-m));
      string=XmStringGenerate("0",XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(bit_button[n][m],XmNlabelString,string,NULL);
      XmStringFree(string);
    }
  else
    {
      io_pointer[controllers[type_micro].io_registers[address_io+n].address]=
	io_pointer[controllers[type_micro].io_registers[address_io+n].address]
	|(1<<(7-m));
      string=XmStringGenerate("1",XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      XtVaSetValues(bit_button[n][m],XmNlabelString,string,NULL);
      XmStringFree(string);
    }
  sprintf(buf,"%X",io_pointer[controllers[type_micro].io_registers[address_io+n].address]);
  fill_zero(buf,2);
  buf[2]='h';buf[3]=0;
  string=XmStringGenerate(buf,XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(label_io_val[n],XmNlabelString,string,NULL);
  XmStringFree(string);
  if(controllers[type_micro].io_registers[address_io+n].code==SREG) 
    update_main();
}

void set_size_io_window(Widget w,XtPointer client_data,XtPointer call_data)
{
  Dimension width,height,height_label;

  XtRemoveCallback(w,XmNpopupCallback,(XtCallbackProc)set_size_io_window,
		   NULL);

  XtVaGetValues(label_io[0],XmNheight,&height_label,NULL);
  io_window_step=height_label+2;
  
  XtVaGetValues(w,XmNwidth,&width,XmNheight,&height,NULL);
  height_min_io_window=height-nlines_io*io_window_step;
  XtVaSetValues(w,XmNmaxWidth,width,XmNminWidth,width,XmNminHeight,
		height+(4-nlines_io)*io_window_step,
		XmNheightInc,io_window_step,XmNwidth,width,
		XmNmaxHeight,
		height_min_io_window+MAX_IO_PORT_LINES*io_window_step,NULL);
}

void close_button_io(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtVaSetValues(FindMenuButton(memory_menu,"I/O Registers"),
		XmNset,XmUNSET,NULL);
  XtUnmanageChild(io_window);
  return;
}


/* Memory window ----------------------------------------------------------- */
void close_button_memory(Widget w,XtPointer client_data,XtPointer call_data)
{
  XtVaSetValues(FindMenuButton(memory_menu,"Memory"),XmNset,XmUNSET,NULL);
  XtUnmanageChild(memory_window);
}

void show_edit_window_callback(Widget w,XtPointer client_data,
			      XmDrawingAreaCallbackStruct *call_data)
{
  XButtonEvent *event;
  XmString string;
  char buf[20];
  
  if(call_data->event->type!=ButtonPress) return;

  event=&call_data->event->xbutton;
  if(event->button!=Button1) return;
  
  if(mwda.memory_type==M_FLASH) return;

  mewd.address=event->y/mwda.font_height*16+event->x/mwda.font_width/4+
    mwda.data_offset;
  if(mewd.address>=mwda.data_size) return;
  
  sprintf(buf,"%X",mewd.address);
  fill_zero(buf,6);
  string=XmStringCreateLtoR(buf,XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(mewd.address_widget,XmNlabelString,string,NULL);
  XmStringFree(string);

  
  switch(mewd.data_type){
  case HEX: sprintf(buf,"%X",mwda.pdata[mewd.address]);break;
  case DEC: sprintf(buf,"%u",mwda.pdata[mewd.address]);break;
  case OCT: sprintf(buf,"%o",mwda.pdata[mewd.address]);break;
  case BIN: conv_to_bin(mwda.pdata[mewd.address],buf,False);break;
  }
  XtVaSetValues(mewd.text_widget,XmNvalue,buf);
  
  XtManageChild(edit_memory_window);
}

void change_edit_window_type_callback(Widget w,int type,XtPointer call_data)
{
  char buf[20],*str;
  uint8_t val,base;

  str=XmTextFieldGetString(mewd.text_widget);
  switch(mewd.data_type){
  case HEX: base=16;break;
  case DEC: base=10;break;
  case OCT: base=8;break;
  case BIN: base=2;break;
  }
  val=(uint8_t)strtoul(str,NULL,base);

  switch(type){
  case HEX: 
    mewd.data_type=HEX;sprintf(buf,"%X",val);break;
  case DEC:
    mewd.data_type=DEC;sprintf(buf,"%u",val);break;
  case OCT:
    mewd.data_type=OCT;sprintf(buf,"%o",val);break;
  case BIN:
    mewd.data_type=BIN;conv_to_bin(val,buf,False);break;
  }
  
  XtVaSetValues(mewd.text_widget,XmNvalue,buf,NULL);
}

void refresh_memory_window(Widget w,XtPointer client_data, XtPointer call_data)
{
  XEvent *event;
  Pixmap pixmap;
  
  event=((XmDrawingAreaCallbackStruct *)call_data)->event;
  if(event->type==Expose)  
    {   
      if(event->xexpose.window==XtWindow(mwda.label_widget)) pixmap=mwda.label;
      else 
	if(event->xexpose.window==XtWindow(mwda.address_widget)) 
	  pixmap=mwda.address;
	else
	  if(event->xexpose.window==XtWindow(mwda.data_widget)) 
	    pixmap=mwda.data;
	  else
	    if(event->xexpose.window==XtWindow(mwda.chars_widget))
	      pixmap=mwda.chars;
	    else return;
      XCopyArea(display,pixmap,event->xexpose.window,mwda.gc,event->xexpose.x,
		event->xexpose.y,event->xexpose.width,
		event->xexpose.height,event->xexpose.x,event->xexpose.y);
    };  
}

void draw_memory_window(void)
{
  static char *label=" Address   00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F        chars";
  char buf[10];
  int n,i,index;
  GC gc;

  XDrawImageString(display,mwda.label,mwda.label_gc,0,mwda.font_height-1,label,
		   strlen(label));
  
  XFillRectangle(display,mwda.address,mwda.address_gc,0,0,mwda.font_width*8,
		 mwda.font_height*mwda.num_lines+3);
  XFillRectangle(display,mwda.data,mwda.data_gc,
		 0,0,mwda.font_width*65,mwda.font_height*mwda.num_lines+3);
  XFillRectangle(display,mwda.chars,mwda.chars_gc,
		 0,0,mwda.font_width*18,mwda.font_height*mwda.num_lines+3);
  for(n=mwda.cur_line;n<mwda.cur_line+mwda.num_lines&&n<mwda.max_lines;n++)
    {
      sprintf(buf,"%X",n*16+mwda.data_offset);
      fill_zero(buf,6);
      XDrawString(display,mwda.address,mwda.gc,mwda.font_width,
		  mwda.font_height*(n+1-mwda.cur_line),buf,strlen(buf));
      for(i=0;i<16;i++)
	{  
	  if((index=n*16+i+mwda.data_offset)>=mwda.data_size) break;
	  switch(mwda.data_type){
	  case HEX: 
	    sprintf(buf,"%X",mwda.pdata[index]);fill_zero(buf,2);
	    break;
	  case DEC:
	    sprintf(buf,"%u",mwda.pdata[index]);fill_zero(buf,3);
	    break;
	  }
	  
	  gc=mwda.gc;
	  if(mwda.mem_changed!=NULL)
	    if(mwda.mem_changed[(mwda.mem_offset+index)/8]&
	       (1<<(mwda.mem_offset+index)%8)) gc=mwda.gc_new;
	  

	  XDrawString(display,mwda.data,gc,mwda.font_width*
		      (4*i+1+mwda.data_type),
		      mwda.font_height*(n+1-mwda.cur_line),buf,strlen(buf));
	  XDrawString(display,mwda.chars,gc,mwda.font_width*(i+1),
		      mwda.font_height*(n+1-mwda.cur_line),
		      &mwda.pdata[n*16+i],1);
	}
    }
}

void full_update_memory_window(void)
{
  if(!memory_window) return;
  if(!XtIsManaged(memory_window)) return;
  draw_memory_window();
  XCopyArea(display,mwda.label,XtWindow(mwda.label_widget),mwda.label_gc,
	    0,0,mwda.width,mwda.font_height,0,0);
  XCopyArea(display,mwda.address,XtWindow(mwda.address_widget),mwda.address_gc,
	    0,0,mwda.font_width*8,mwda.font_height*mwda.num_lines+3,0,0);
  XCopyArea(display,mwda.data,XtWindow(mwda.data_widget),mwda.data_gc,
	    0,0,mwda.font_width*65,mwda.font_height*mwda.num_lines+3,0,0);
  XCopyArea(display,mwda.chars,XtWindow(mwda.chars_widget),mwda.chars_gc,
	    0,0,mwda.font_width*18,mwda.font_height*mwda.num_lines+3,0,0);
}

void aply_edit_window_callback(Widget w,XtPointer client_data,
			       XtPointer call_data)
{
  char *str;
  int base;

  str=XmTextFieldGetString(mewd.text_widget);
  switch(mewd.data_type){
  case HEX: base=16;break;
  case DEC: base=10;break;
  case OCT: base=8;break;
  case BIN: base=2;break;
  }
  
  mwda.pdata[mewd.address]=(uint8_t)strtoul(str,NULL,base);
  mwda.mem_changed[(mwda.mem_offset+mewd.address)/8]|=
    1<<(mwda.mem_offset+mewd.address)%8;
  XtUnmanageChild(edit_memory_window);
  draw_memory_window();
  full_update_memory_window();
}

void verify_text_ew_callback(Widget w,XtPointer client_data,
			     XmTextVerifyCallbackStruct *call_data)
{
  char new_str[80],*str,*ins=call_data->text->ptr;
  int size=call_data->text->length,n;

  if(call_data->startPos!=call_data->endPos) return;

  switch(mewd.data_type){
  case HEX:
    for(n=0;n<size;n++) if(!isxdigit(ins[n])) 
      {call_data->doit=False;return;}
    break;
  case DEC:
    for(n=0;n<size;n++) if(!isdigit(ins[n])) 
      {call_data->doit=False;return;}
    break;
  case OCT:
    for(n=0;n<size;n++) if(ins[n]<'0'||ins[n]>'7') 
      {call_data->doit=False;return;}
    break;
  case BIN:
    for(n=0;n<size;n++) if(ins[n]!='0'&&ins[n]!='1') 
      {call_data->doit=False;return;}
    break;
  }
  
  str=XmTextFieldGetString(w);
  strncpy(new_str,str,call_data->currInsert);
  strncpy(&new_str[call_data->currInsert],ins,size);
  strncpy(&new_str[call_data->currInsert+size],
	  &str[call_data->currInsert],strlen(str)-call_data->currInsert);
  new_str[strlen(str)+size]=0;
  
  switch(mewd.data_type){
  case HEX: n=strtoul(new_str,NULL,16);break;
  case DEC: n=strtoul(new_str,NULL,10);break;
  case OCT: n=strtoul(new_str,NULL,8);break;
  case BIN: n=strtoul(new_str,NULL,2);break;
  }
  if(n>255) call_data->doit=False;
}

void memory_scroll_bar_callback(Widget w,XtPointer client_data,
				XtPointer call_data)
{
  char buf[10];

  XtVaGetValues(mwda.vscr_bar,XmNvalue,&mwda.cur_line,NULL);
  draw_memory_window();
  full_update_memory_window();
  
  sprintf(buf,"%X",mwda.cur_line*16);
  fill_zero(buf,6);
  XtVaSetValues(mwda.address_text_widget,XmNvalue,buf,NULL);
}

void memory_resize_callback(Widget w,XtPointer client_data,
			    XmDrawingAreaCallbackStruct *call_data)
{
  Dimension height;
  if(call_data->reason!=XmCR_RESIZE) return;

  XtVaGetValues(mwda.address_widget,XmNheight,&height,NULL);
  mwda.num_lines=height/mwda.font_height;

  if(mwda.num_lines>mwda.max_lines)
    {
      if(mwda.max_lines)
	XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.max_lines,
		      XmNmaximum,mwda.max_lines,NULL);
      else XtVaSetValues(mwda.vscr_bar,XmNsliderSize,1,
			 XmNmaximum,1,NULL);
    }
  else
    XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.num_lines,
		  XmNmaximum,mwda.max_lines,NULL);  

  XFreePixmap(display,mwda.label);
  XFreePixmap(display,mwda.address);
  XFreePixmap(display,mwda.data);
  XFreePixmap(display,mwda.chars);
  mwda.label=CreatePixmap(XtWindow(toplevel),mwda.width,
    			  mwda.font_height+3,mwda.label_bg);
  mwda.address=CreatePixmap(XtWindow(toplevel),mwda.font_width*8,
    			    mwda.font_height*mwda.num_lines+3,mwda.address_bg);
  mwda.data=CreatePixmap(XtWindow(toplevel),mwda.font_width*65,
    			 mwda.font_height*mwda.num_lines+3,mwda.data_bg);
  mwda.chars=CreatePixmap(XtWindow(toplevel),mwda.font_width*18,
    			  mwda.font_height*mwda.num_lines+3,mwda.chars_bg); 
  draw_memory_window();
}

void init_memory_window(void)
{
  if(!memory_window) return;

  switch(mwda.memory_type){
  case M_REGISTERS:
    mwda.pdata=gpr_pointer;
    mwda.data_size=controllers[type_micro].size_gpr;
    mwda.data_offset=0;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=mem_file_changed;
    mwda.mem_offset=0;
    break;
  case M_IO_REGISTERS:
    mwda.pdata=io_pointer;
    mwda.data_offset=0;
    mwda.data_size=controllers[type_micro].size_io_reg;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=mem_file_changed;
    mwda.mem_offset=controllers[type_micro].size_gpr;
    break;
  case M_EX_IO_REGISTERS:
    break;
  case M_SRAM:
    mwda.pdata=sram_pointer;
    mwda.data_offset=0;
    mwda.data_size=controllers[type_micro].size_sram;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=mem_file_changed;
    mwda.mem_offset=controllers[type_micro].size_io_reg+
      controllers[type_micro].size_gpr;
    break;
  case M_DATA:
    mwda.pdata=gpr_pointer;
    mwda.data_offset=0;
    mwda.data_size=controllers[type_micro].size_gpr+
      controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=mem_file_changed;
    mwda.mem_offset=0;
    break;    
  case M_EEPROM:
    mwda.pdata=eeprom_pointer;
    mwda.data_offset=0;
    mwda.data_size=controllers[type_micro].size_eeprom;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=eeprom_changed;
    mwda.mem_offset=0;
    break;
  case M_FLASH:
    mwda.pdata=flash_pointer;
    mwda.data_offset=0;
    mwda.data_size=controllers[type_micro].size_flash*2;
    mwda.max_lines=mwda.data_size/16;
    mwda.mem_changed=NULL;
    mwda.mem_offset=0;
  }
  mwda.cur_line=0;

  if(mwda.num_lines>mwda.max_lines)
    {
      if(mwda.max_lines)
	XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.max_lines,
		      XmNmaximum,mwda.max_lines,NULL);
      else XtVaSetValues(mwda.vscr_bar,XmNsliderSize,1,
			 XmNmaximum,1,NULL);
    }
  else
    XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.num_lines,
		  XmNmaximum,mwda.max_lines,NULL);  
}

void combo_mem_callback(Widget w,XtPointer client_data,
			XmComboBoxCallbackStruct *call_data)
{
  mwda.memory_type=call_data->item_position;
  init_memory_window();
  draw_memory_window();
  full_update_memory_window();
}

void switch_memory_data_type_callback(Widget w,XtPointer client_data,
				      XtPointer call_data)
{
  XmString string;

  if(mwda.data_type==HEX)
    {
      mwda.data_type=DEC;
      string=XmStringCreateLtoR("DEC",XmSTRING_DEFAULT_CHARSET);
    }
  else
    {
      mwda.data_type=HEX;
      string=XmStringCreateLtoR("HEX",XmSTRING_DEFAULT_CHARSET);
    }

  draw_memory_window();
  full_update_memory_window();
  XtVaSetValues(w,XmNlabelString,string,NULL);
  XmStringFree(string);
}

void address_textfield_callback(Widget w,XtPointer client_data,
				XtPointer call_data)
{
  char *str=XmTextFieldGetString(w);
  int address;

  address=strtoul(str,NULL,16);
  mwda.data_offset=address%16;
  mwda.cur_line=address/16;
  draw_memory_window();
  full_update_memory_window();
}

void create_edit_memory_window(void)
{
  Widget shell,button,frame,radio_box,row;
  XmString string;

  mewd.data_type=HEX;

  shell=XtVaCreateWidget("Edit_Memory_Shell",xmDialogShellWidgetClass,
			 toplevel,XmNtitle,_("Edit memory"),
			 XmNallowShellResize,True,
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,NULL);
  edit_memory_window=
    XtVaCreateWidget("memory_form",xmFormWidgetClass,shell,
		     XmNautoUnmanage,False,XmNdialogStyle,XmDIALOG_MODELESS,
		     XmNfractionBase,10,XmNmarginHeight,5,
		     XmNmarginWidth,5,XmNdialogStyle,
		     XmDIALOG_FULL_APPLICATION_MODAL,NULL);

  row=XtVaCreateWidget("memory_row",xmRowColumnWidgetClass,
		       edit_memory_window,XmNtopAttachment,XmATTACH_FORM,
		       XmNleftAttachment,XmATTACH_FORM,
		       XmNrightAttachment,XmATTACH_FORM,
		       XmNorientation,XmHORIZONTAL,NULL);
  mewd.address_widget=
    XtVaCreateManagedWidget("0",xmLabelGadgetClass,row,NULL);
  mewd.text_widget=
    XtVaCreateManagedWidget("address_textfield",xmTextFieldWidgetClass,
			    row,NULL);
  XtAddCallback(mewd.text_widget,XmNmodifyVerifyCallback,
		(XtCallbackProc)verify_text_ew_callback,NULL);
  
  /* create radio box */
  frame=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,edit_memory_window,
				XmNtopAttachment,XmATTACH_WIDGET,
				XmNtopWidget,row,
				XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNrightAttachment,XmATTACH_FORM,NULL);
  radio_box=XmCreateRadioBox(frame,"radiobox",NULL,0);
  button=XtVaCreateManagedWidget("Hex",xmToggleButtonGadgetClass,radio_box,
				 XmNset,XmSET,NULL); 
  XtAddCallback(button,XmNvalueChangedCallback,
		(XtCallbackProc)change_edit_window_type_callback,
		(XtPointer)HEX);
  button=
    XtVaCreateManagedWidget("Dec",xmToggleButtonGadgetClass,radio_box,NULL); 
  XtAddCallback(button,XmNvalueChangedCallback,
		(XtCallbackProc)change_edit_window_type_callback,
		(XtPointer)DEC);
  button=
    XtVaCreateManagedWidget("Oct",xmToggleButtonGadgetClass,radio_box,NULL);
  XtAddCallback(button,XmNvalueChangedCallback,
		(XtCallbackProc)change_edit_window_type_callback,
		(XtPointer)OCT);
  button=
    XtVaCreateManagedWidget("Bin",xmToggleButtonGadgetClass,radio_box,NULL);
  XtAddCallback(button,XmNvalueChangedCallback,
		(XtCallbackProc)change_edit_window_type_callback,
		(XtPointer)BIN);
  
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 edit_memory_window,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,4,XmNlabelString,string,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,6,NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)unmanage,edit_memory_window);
  XmStringFree(string);

  string=XmStringGenerate(_("Set"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Set",xmPushButtonGadgetClass,
				 edit_memory_window,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,6,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,9,XmNlabelString,string,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,6,NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)aply_edit_window_callback,NULL);
  XmStringFree(string);  

  XtVaSetValues(radio_box,XmNorientation,XmHORIZONTAL,NULL);
  XtVaSetValues(frame,XmNbottomAttachment,XmATTACH_WIDGET,
		XmNbottomWidget,button,XmNbottomOffset,5,NULL);
  XtManageChild(row);
  XtManageChild(radio_box);
}

void create_memory_window(void)
{
  Widget shell,memory_frame,button,scroll,memory_row,combo_mem;
  XmString string,*list;
  XFontStruct *font_struct;
  XGCValues values;
  XColor color;
  XtWidgetGeometry size;
  int size_list,n,size_string,t;

 /* init window */
  mwda.num_lines=20;
  mwda.cur_line=0;
  mwda.data_type=HEX;
  mwda.data_offset=0;
  mwda.data_size=controllers[type_micro].size_gpr+
    controllers[type_micro].size_io_reg+controllers[type_micro].size_sram;
  mwda.pdata=gpr_pointer;
  mwda.memory_type=M_DATA;
  mwda.mem_changed=mem_file_changed;
  mwda.mem_offset=0;
  mwda.max_lines=mwda.data_size/16;

  values.foreground=BlackPixel(display,scr_num);
  values.background=WhitePixel(display,scr_num);
  mwda.gc=XCreateGC(display,XtWindow(toplevel),
		    GCForeground|GCBackground,&values);
  
  if((font_struct=XQueryFont(display,XGContextFromGC(mwda.gc)))==NULL)
    {
      fprintf(stderr,"Can't load font\n");
      exit(1);
    }
  mwda.font_width=font_struct->max_bounds.width;
  mwda.font_height=font_struct->ascent+font_struct->descent;
  XFreeFontInfo(NULL,font_struct,1);
  mwda.width=93*mwda.font_width;
  mwda.height=(mwda.num_lines+3)*mwda.font_height;

 /* allocate color */ 
  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "rgb:A0/A0/A0",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=WhitePixel(display,scr_num);
    }
  values.background=mwda.label_bg=color.pixel;
  mwda.label_gc=XCreateGC(display,XtWindow(toplevel),GCForeground|GCBackground,
			  &values);
  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "rgb:80/80/D0",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=WhitePixel(display,scr_num);
    }
  values.foreground=mwda.address_bg=color.pixel;
  mwda.address_gc=
    XCreateGC(display,XtWindow(toplevel),GCForeground,&values);
  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "rgb:F0/F0/F0",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=WhitePixel(display,scr_num);
    }
  values.foreground=mwda.data_bg=mwda.chars_bg=color.pixel;
  mwda.chars_gc=mwda.data_gc=
    XCreateGC(display,XtWindow(toplevel),GCForeground,&values);

  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "red",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=BlackPixel(display,scr_num);
    }
  values.foreground=color.pixel;
  mwda.gc_new=XCreateGC(display,XtWindow(toplevel),GCForeground,&values);


  /* Create widgets */  
  shell=XtVaCreateManagedWidget("Memory_Shell",xmDialogShellWidgetClass,
				toplevel,XmNtitle,_("Memory"),
				XmNallowShellResize,True,
				XmNmappedWhenManaged,False,NULL);
  memory_window=XtVaCreateWidget("memory_form",xmFormWidgetClass,shell,
				 XmNautoUnmanage,False,
				 XmNdialogStyle,XmDIALOG_MODELESS,
				 XmNfractionBase,3,NULL);
  memory_row=XtVaCreateWidget("memory_row",xmRowColumnWidgetClass,
			      memory_window,XmNtopAttachment,XmATTACH_FORM,
			      XmNtopOffset,1,XmNorientation,XmHORIZONTAL,NULL);

  size_string=1;n=0;
  for(n=0;n<XtNumber(memory_type_str);n++)
    if((t=strlen(memory_type_str[n]))>size_string) size_string=t;
  
  size_list=n;  
  if((list=malloc(size_list*sizeof(XmString)))==NULL)
    {
      puts("create_memory_window(): can't allocate memory for 'list'");
      exit(1);
    }

  for(n=0;n<size_list;n++)
    list[n]=XmStringCreateLtoR(gettext(memory_type_str[n]),
			       XmSTRING_DEFAULT_CHARSET);
  
  combo_mem=
    XtVaCreateManagedWidget("memory_type_box",xmComboBoxWidgetClass,memory_row,
			    XmNcomboBoxType,XmDROP_DOWN_LIST,XmNitems,list,
			    XmNitemCount,size_list,
			    XmNvisibleItemCount,size_list,
			    XmNcolumns,size_string,
			    XmNselectedPosition,M_DATA,NULL);

  XtVaSetValues(XtNameToWidget(combo_mem,"*Text"),XmNshadowThickness,1,NULL);
  for(n=0;n<size_list;n++) XmStringFree(list[n]);
  free(list);
  
  XtAddCallback(combo_mem,XmNselectionCallback,
		(XtCallbackProc)combo_mem_callback,NULL);

  string=XmStringGenerate(_("Address:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaCreateManagedWidget("address_label",xmLabelGadgetClass,memory_row,
			  XmNlabelString,string,NULL);
  XmStringFree(string);

  mwda.address_text_widget=
    XtVaCreateManagedWidget("address_textfield",xmTextFieldWidgetClass,
			    memory_row,XmNcolumns,7,XmNvalue,"000000",NULL);
  XtAddCallback(mwda.address_text_widget,XmNactivateCallback,
		(XtCallbackProc)address_textfield_callback,NULL);

  string=XmStringGenerate(_("Find:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaCreateManagedWidget("find_label",xmLabelGadgetClass,memory_row,
			  XmNlabelString,string,NULL);
  XmStringFree(string);

  XtVaCreateManagedWidget("find_textfield",xmTextFieldWidgetClass,
			  memory_row,XmNcolumns,10,NULL);
  
  mwda.data_type_widget=
    XtVaCreateManagedWidget("HEX",xmPushButtonWidgetClass,
			    memory_row,NULL);
  XtAddCallback(mwda.data_type_widget,XmNactivateCallback,
		(XtCallbackProc)switch_memory_data_type_callback,NULL);

  XtManageChild(memory_row);

  memory_frame=
    XtVaCreateWidget("memory_frame",xmFrameWidgetClass,memory_window,
		     XmNtopAttachment,XmATTACH_WIDGET,XmNtopWidget,memory_row,
		     XmNleftAttachment,XmATTACH_FORM,XmNleftOffset,5,
		     XmNrightAttachment,XmATTACH_FORM,
		     XmNrightOffset,5,XmNmarginWidth,2,
		     XmNmarginHeight,2,NULL);  

  /* create scroll window */
  scroll=
    XtVaCreateWidget("Memory_Scroll",xmScrolledWindowWidgetClass,memory_frame,
		     XmNscrollingPolicy,XmAPPLICATION_DEFINED,
		     XmNscrollBarDisplayPolicy,XmSTATIC,
		     XmNleftAttachment,XmATTACH_FORM,XmNleftOffset,3,
		     XmNrightAttachment,XmATTACH_FORM,XmNrightOffset,2,
		     XmNtopAttachment,XmATTACH_WIDGET,XmNtopWidget,
		     memory_frame,XmNvisualPolicy,XmVARIABLE,
		     XmNbottomAttachment,XmATTACH_FORM,
		     XmNbottomOffset,3,NULL);
  mwda.draw=
    XtVaCreateWidget("DrawingManager",xmFormWidgetClass,scroll,
		     XmNbackground,WhitePixel(display,scr_num),
		     XmNforeground,BlackPixel(display,scr_num),NULL); 
  mwda.label_widget=
    XtVaCreateManagedWidget("Drawing area",xmDrawingAreaWidgetClass,mwda.draw,
			    XmNborderWidth,1,XmNtopAttachment,XmATTACH_FORM,
			    XmNwidth,mwda.width,XmNheight,mwda.font_height+3,
			    XmNbackground,mwda.label_bg,NULL);
  mwda.address_widget=
    XtVaCreateManagedWidget("Drawing area",xmDrawingAreaWidgetClass,mwda.draw,
			    XmNborderWidth,1,XmNtopAttachment,XmATTACH_WIDGET,
			    XmNtopWidget,mwda.label_widget,
			    XmNbottomAttachment,XmATTACH_FORM,
			    XmNwidth,mwda.font_width*8,
			    XmNheight,mwda.font_height*mwda.num_lines+3,
			    XmNx,0,XmNleftAttachment,XmATTACH_NONE,
			    XmNbottomOffset,mwda.font_height/2,
			    XmNtopOffset,mwda.font_height/2,
			    XmNbackground,mwda.address_bg,NULL);
  mwda.data_widget=
    XtVaCreateManagedWidget("Drawing area",xmDrawingAreaWidgetClass,mwda.draw,
			    XmNborderWidth,1,XmNtopAttachment,XmATTACH_WIDGET,
			    XmNtopWidget,mwda.label_widget,
			    XmNbottomAttachment,XmATTACH_FORM,
			    XmNwidth,mwda.font_width*65,
			    XmNheight,mwda.font_height*mwda.num_lines+3,
			    XmNleftAttachment,XmATTACH_NONE,
			    XmNx,mwda.font_width*9,
			    XmNbottomOffset,mwda.font_height/2,
			    XmNtopOffset,mwda.font_height/2,
			    XmNbackground,mwda.data_bg,NULL);
  mwda.chars_widget=
    XtVaCreateManagedWidget("Drawing area",xmDrawingAreaWidgetClass,mwda.draw,
			    XmNborderWidth,1,XmNtopAttachment,XmATTACH_WIDGET,
			    XmNtopWidget,mwda.label_widget,
			    XmNbottomAttachment,XmATTACH_FORM,
			    XmNwidth,mwda.font_width*18,
			    XmNheight,mwda.font_height*mwda.num_lines+3,
			    XmNleftAttachment,XmATTACH_NONE,
			    XmNx,mwda.font_width*75,
			    XmNbottomOffset,mwda.font_height/2,
			    XmNtopOffset,mwda.font_height/2,
			    XmNbackground,mwda.chars_bg,NULL);

  mwda.vscr_bar=
    XtVaCreateManagedWidget("memory_scr_bar",xmScrollBarWidgetClass,scroll,
			    XmNorientation,XmVERTICAL,XmNincrement,1,
			    XmNminimum,0,XmNvalue,0,NULL);

  XtAddCallback(mwda.vscr_bar,XmNvalueChangedCallback,
		(XtCallbackProc)memory_scroll_bar_callback,NULL);
  XtAddCallback(mwda.vscr_bar,XmNdragCallback,
		(XtCallbackProc)memory_scroll_bar_callback,NULL);
  XmScrolledWindowSetAreas(scroll,NULL,mwda.vscr_bar,mwda.draw);
  
  XtManageChild(mwda.draw);
  XtManageChild(scroll);
  XtManageChild(memory_frame);

  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 memory_window,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,2,XmNlabelString,string,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,6,NULL);
  
  XtAddCallback(button,
		XmNactivateCallback,(XtCallbackProc)close_button_memory,NULL);
  XmStringFree(string);
  XtVaSetValues(memory_frame,XmNbottomOffset,6,XmNbottomAttachment,
		XmATTACH_WIDGET,XmNbottomWidget,button,NULL);

  mwda.label=CreatePixmap(XtWindow(toplevel),mwda.width,
			  mwda.font_height+3,mwda.label_bg);
  mwda.address=CreatePixmap(XtWindow(toplevel),mwda.font_width*8,
			    mwda.font_height*mwda.num_lines+3,mwda.address_bg);
  mwda.data=CreatePixmap(XtWindow(toplevel),mwda.font_width*65,
			 mwda.font_height*mwda.num_lines+3,mwda.data_bg);
  mwda.chars=CreatePixmap(XtWindow(toplevel),mwda.font_width*18,
			  mwda.font_height*mwda.num_lines+3,mwda.chars_bg);
  XtAddCallback(mwda.label_widget,XmNexposeCallback,
		(XtCallbackProc)refresh_memory_window,NULL);
  XtAddCallback(mwda.address_widget,XmNexposeCallback,
		(XtCallbackProc)refresh_memory_window,NULL);
  XtAddCallback(mwda.data_widget,XmNexposeCallback,
		(XtCallbackProc)refresh_memory_window,NULL);
  XtAddCallback(mwda.chars_widget,XmNexposeCallback,
		(XtCallbackProc)refresh_memory_window,NULL);
  XtAddCallback(mwda.address_widget,XmNresizeCallback, 
  		(XtCallbackProc)memory_resize_callback,NULL);
  XtAddCallback(mwda.data_widget,XmNinputCallback,
		(XtCallbackProc)show_edit_window_callback,NULL);
  if(mwda.num_lines>mwda.max_lines)
    XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.max_lines,
		  XmNmaximum,mwda.max_lines,NULL);
  else
    XtVaSetValues(mwda.vscr_bar,XmNsliderSize,mwda.num_lines,
		  XmNmaximum,mwda.max_lines,NULL);

  XtManageChild(memory_window);

  /* set size */
  XtQueryGeometry(shell,NULL,&size);
  XtVaSetValues(shell,XmNminWidth,size.width,XmNmaxWidth,size.width,
		XmNbaseHeight,size.height,XmNheightInc,mwda.font_height,
		XmNmappedWhenManaged,True,NULL);

  draw_memory_window();

  create_edit_memory_window();
}


