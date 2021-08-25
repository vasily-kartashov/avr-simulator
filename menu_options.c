/*
 File        : menu_options.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : creating option menu.

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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libintl.h>
#include <Xm/XmAll.h>
#include "types.h"
#include "global.h"

void use_debugger_callback();
void close_debugger_callback();
void use_ports_callback();
void close_ports_callback();
void toggle_ports_callback();
void popup_file_selection_callback();
void watchdog_unprog_callback();
void watchdog_prog_callback();
void close_project_callback();

int  check_port_file();
void init_ports_window();
void file_selection_ok_port();
void create_project_selection();

extern Ports ports[];

Widget freq_field,             /* field where you enter frequency         */
  watchdog_freq_field,         /* field where you enter watchdog frequency*/
  combo_proc,                  /* widgets for choose processor            */  
  file_selection_port,
  prog_option_menu,            /* option menu for set reset delay */
  prog_button[2];              /* buttons from previous menu */
  
int  ports_window_flag,  /* show what buttons was pressed logging or stimuli */
  number_edit_line,   /* in port logging and stimuli window */
  wacthdog_timeout_temp=UNPROGRAMMED;

Line_port_window data_port_window[6];
Data_port_window data_port_log[6],data_port_sti[6];


void create_ports_window(void)
{
  Widget shell,frame,button,form,separator;
  XmString string;
  int n;
  char buf[10];
  

  /* create Selection File */
  string=XmStringGenerate(_("Selection of file"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  file_selection_port=XmCreateFileSelectionDialog(toplevel,
						  "selection_file",NULL,0);
  XtVaSetValues(file_selection_port,XmNdialogTitle,string,
		XmNheight,400,XmNwidth,400,
		XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL,NULL);
  XmStringFree(string);

  XtAddCallback(file_selection_port,XmNcancelCallback,
                (XtCallbackProc)unmanage,
                (XtPointer)&file_selection_port);
  XtAddCallback(file_selection_port,XmNokCallback,
                (XtCallbackProc)file_selection_ok_port,NULL);
  XtUnmanageChild(XmFileSelectionBoxGetChild(file_selection_port,
                                             XmDIALOG_HELP_BUTTON));
  /* end creation of file selection */


  shell=XtVaCreateWidget("Shell",xmDialogShellWidgetClass,toplevel,
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,
			 NULL);
  ports_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
				XmNautoUnmanage,False,XmNdialogStyle,
				XmDIALOG_FULL_APPLICATION_MODAL,
				XmNresizePolicy,XmRESIZE_NONE,
				XmNfractionBase,5,NULL);

  frame=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,ports_window,
				XmNtopAttachment,XmATTACH_FORM,
				XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,5,
				XmNrightAttachment,XmATTACH_FORM,
				XmNrightOffset,5,
				NULL);
  
  form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame,NULL);
  
  for(n=0;n<6;n++)
    {
      sprintf(buf,"%s %c",_("Port"),'A'+n);
      string=XmStringGenerate(buf,XmFONTLIST_DEFAULT_TAG,
			      XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
      data_port_window[n].button1=
	XtVaCreateManagedWidget(" ",xmToggleButtonGadgetClass,
				form,XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,4,XmNtopOffset,4,
				XmNlabelString,string,NULL);
      XmStringFree(string);
     XtAddCallback(data_port_window[n].button1,XmNvalueChangedCallback,
		   toggle_ports_callback,NULL);
      data_port_window[n].data=
	XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,form,
				XmNtopOffset,4,
				XmNleftAttachment,XmATTACH_WIDGET,
				XmNleftWidget,data_port_window[n].button1,
				XmNleftOffset,4,
				XmNcolumns,50,XmNmaxLength,15,
				XmNcursorPositionVisible,False,NULL);
     
      data_port_window[n].button2=
	XtVaCreateManagedWidget("...",xmPushButtonGadgetClass,form,
				XmNtopOffset,4,
				XmNrightAttachment,XmATTACH_FORM,
				XmNrightOffset,4,NULL);
      
      XtAddCallback(data_port_window[n].button2,XmNactivateCallback,
		    (XtCallbackProc)popup_file_selection_callback,NULL);
      XtVaSetValues(data_port_window[n].data,XmNrightAttachment,
		    XmATTACH_WIDGET,XmNrightOffset,4,
		    XmNrightWidget,data_port_window[n].button2,NULL);
      if(n==0)
	{
	  XtVaSetValues(data_port_window[n].button1,
			XmNtopAttachment,XmATTACH_FORM,NULL);
	  XtVaSetValues(data_port_window[n].data,
			XmNtopAttachment,XmATTACH_FORM,NULL);
	  XtVaSetValues(data_port_window[n].button2,
			XmNtopAttachment,XmATTACH_FORM,NULL);
	}
      else 
	{
	  XtVaSetValues(data_port_window[n].button1,XmNtopAttachment,
			XmATTACH_WIDGET,XmNtopWidget,separator,NULL);
	  XtVaSetValues(data_port_window[n].data,XmNtopAttachment,
			XmATTACH_WIDGET,XmNtopWidget,separator,NULL);
	  XtVaSetValues(data_port_window[n].button2,XmNtopAttachment,
			XmATTACH_WIDGET,XmNtopWidget,separator,NULL);
	}
      if(n!=5)
	    separator=
	      XtVaCreateManagedWidget("separator",xmSeparatorGadgetClass,form,
				      XmNtopAttachment,XmATTACH_WIDGET,
				      XmNtopWidget,data_port_window[n].button1,
				      XmNtopOffset,4,    
				      XmNleftAttachment,XmATTACH_FORM,
				      XmNrightAttachment,XmATTACH_FORM,
				      NULL);
    }
  XtVaSetValues(data_port_window[5].button1,XmNbottomOffset,4,
		XmNbottomAttachment,XmATTACH_FORM,NULL);
  XtVaSetValues(data_port_window[5].data,XmNbottomOffset,4,XmNbottomAttachment,
		XmATTACH_FORM,NULL);
  XtVaSetValues(data_port_window[5].button2,XmNbottomOffset,4,
		XmNbottomAttachment,XmATTACH_FORM,NULL);

  string=XmStringGenerate(_("OK"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("OK",xmPushButtonGadgetClass,
				 ports_window,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,frame,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,XmNlabelString,string,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,2,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)use_ports_callback,ports_window);
  XmStringFree(string);
  
  string=XmStringGenerate(_("Cancel"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 ports_window,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,form,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,4,XmNlabelString,string,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,3,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);
  
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)close_ports_callback,ports_window);
  XmStringFree(string);
  XtAddCallback(shell,XmNpopupCallback,(XtCallbackProc)set_size_static_window,
		NULL);
}

void file_selection_ok_port(Widget w,Widget *client_data,
			    XmFileSelectionBoxCallbackStruct *call_data)
{
  Data_port_window *data;
  char *filename;

  if(ports_window_flag) data=data_port_sti;
  else data=data_port_log;

  XtUnmanageChild(file_selection_port);
  filename=XmCvtXmStringToCT(call_data->value);
  XmTextFieldSetString(data_port_window[number_edit_line].data,filename);
  XtFree(filename);
}

void popup_file_selection_callback(Widget w,Widget *client_data,
				   XtPointer call_data)
{
  int n;
  XmString string;
  
  n=0;
  while(w!=data_port_window[n].button2) n++;
  number_edit_line=n;
  XtManageChild(file_selection_port);
  
  if(ports_window_flag)
    string=XmStringGenerate("*.sti",XmFONTLIST_DEFAULT_TAG,
			    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  else
    string=XmStringGenerate("*.log",XmFONTLIST_DEFAULT_TAG,
			    XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XtVaSetValues(file_selection_port,XmNpattern,string,NULL);
  XmStringFree(string);
  
}

void close_ports_callback(Widget w,Widget *client_data,XtPointer call_data)
{
  int n;
  Data_port_window *data;

  if(ports_window_flag) data=data_port_sti;
  else data=data_port_log;
  XtUnmanageChild(*client_data);
  for(n=0;n<6;n++)
    {
      XtVaSetValues(data_port_window[n].button1,XmNset,data[n].state,NULL);
      XmTextFieldSetString(data_port_window[n].data,data[n].filename);
      if(data[n].state==XmUNSET)
	{
	  XtSetSensitive(data_port_window[n].data,False);
	  XtSetSensitive(data_port_window[n].button2,False);
	}
      else
	{
	  XtSetSensitive(data_port_window[n].data,True);
	  XtSetSensitive(data_port_window[n].button2,True);
	}
    }
}

void use_ports_callback(Widget w,Widget *client_data,XtPointer call_data)
{
  int n;
  Data_port_window *data;

  XtUnmanageChild(*client_data);
  if(ports_window_flag) data=data_port_sti;
  else data=data_port_log;

  for(n=0;n<6;n++)
    if(data[n].state!=-1)
      {
	XtVaGetValues(data_port_window[n].button1,XmNset,&data[n].state,NULL);
	XtFree(data[n].filename);
	data[n].filename=XmTextFieldGetString(data_port_window[n].data);
	if(data[n].state==XmSET&&ports_window_flag==1)
	  if(check_port_file(data[n].filename))
	    {
	      XtFree(data[n].filename);
	      data[n].filename=XtMalloc(1);
	      data[n].filename[0]=0;
	      show_error();
	    }
      }

}

void toggle_ports_callback(Widget w,Widget *client_data,
			   XmToggleButtonCallbackStruct *call_data)
{
  int n;
  
  n=0;
  while(data_port_window[n].button1!=w) n++;
  if(call_data->set==XmSET)
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

void create_debugger_window(void)
{
  Widget shell,frame1,frame2,frame3,label,form,option_form,button;
  static Widget data[3];
  XmString string;
  char number[10];

  shell=XtVaCreateWidget("Shell",xmDialogShellWidgetClass,toplevel,
			 XmNtitle,_("Debugger's options"),
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,NULL);
  debugger_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
				   XmNautoUnmanage,False,
				   XmNdialogStyle,
				   XmDIALOG_FULL_APPLICATION_MODAL,
				   XmNresizePolicy,XmRESIZE_NONE,
				   XmNfractionBase,5,NULL);

  option_form=XtVaCreateManagedWidget("form",xmFormWidgetClass,debugger_window,
				      XmNtopAttachment,XmATTACH_FORM,
				      XmNleftAttachment,XmATTACH_FORM,
				      XmNrightAttachment,XmATTACH_FORM,NULL);
  
  frame1=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,option_form,
				 XmNtopAttachment,XmATTACH_FORM,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_FORM,
				 XmNleftOffset,5,
				 XmNmarginWidth,2,XmNmarginHeight,2,NULL);
  
  string=XmStringGenerate(_("Multi Step"),XmFONTLIST_DEFAULT_TAG,          
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  XtVaCreateManagedWidget("label",xmLabelGadgetClass,frame1,
                          XmNlabelString,string,
                          XmNframeChildType,XmFRAME_TITLE_CHILD,
                          XmNchildHorizontalAlignment,XmALIGNMENT_CENTER,
                          XmNchildVerticalAlignment,XmALIGNMENT_CENTER,
                          NULL);
  XmStringFree(string);
  
  form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame1,NULL);
  string=XmStringGenerate(_("Number of Single Steps"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  label=XtVaCreateManagedWidget("label",xmLabelGadgetClass,form,
				XmNlabelString,string,XmNbottomOffset,4,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_FORM,
				XmNbottomAttachment,XmATTACH_FORM,NULL);
  XmStringFree(string);

  sprintf(number,"%d",multi_step);
  data[0]=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,form,
				  XmNtopAttachment,XmATTACH_FORM,
				  XmNbottomAttachment,XmATTACH_FORM,
				  /*  XmNrightAttachment,XmATTACH_FORM, */
				  XmNleftAttachment,XmATTACH_WIDGET,
				  XmNleftWidget,label,XmNvalue,number,
				  XmNleftOffset,4,XmNrightOffset,4,
				  XmNbottomOffset,4,
				  XmNcolumns,3,XmNmaxLength,3,
				  XmNcursorPositionVisible,False,
				  NULL);


  frame2=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,option_form,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopWidget,frame1,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_FORM,
				 XmNleftOffset,5,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,5,
				 XmNmarginWidth,2,XmNmarginHeight,2,NULL);
  
  string=XmStringGenerate(_("Auto Step"),XmFONTLIST_DEFAULT_TAG,          
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  XtVaCreateManagedWidget("label",xmLabelGadgetClass,frame2,
                          XmNlabelString,string,
                          XmNframeChildType,XmFRAME_TITLE_CHILD,
                          XmNchildHorizontalAlignment,XmALIGNMENT_CENTER,
                          XmNchildVerticalAlignment,XmALIGNMENT_CENTER,
                          NULL);
  XmStringFree(string);

  form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame2,NULL);
  string=XmStringGenerate(_("Single Step delay (ms)"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  label=XtVaCreateManagedWidget("label",xmLabelGadgetClass,form,
				XmNlabelString,string,XmNbottomOffset,4,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_FORM,
				XmNbottomAttachment,XmATTACH_FORM,NULL);
  XmStringFree(string);

  sprintf(number,"%d",auto_step);
  data[1]=XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,form,
				  XmNtopAttachment,XmATTACH_FORM,
				  XmNbottomAttachment,XmATTACH_FORM,
				  /*  XmNrightAttachment,XmATTACH_FORM, */
				  XmNleftAttachment,XmATTACH_WIDGET,
				  XmNleftWidget,label,XmNvalue,number,
				  XmNleftOffset,4,XmNrightOffset,4,
				  XmNbottomOffset,4,
				  XmNcolumns,3,XmNmaxLength,3,
				  XmNcursorPositionVisible,False,
				  NULL);

  frame3=XtVaCreateManagedWidget("Frame",xmFrameWidgetClass,option_form,
				 XmNtopAttachment,XmATTACH_FORM,
				 XmNtopOffset,5,
				 XmNleftAttachment,XmATTACH_WIDGET,
				 XmNleftWidget,frame1,
				 XmNleftOffset,5,
				 XmNrightAttachment,XmATTACH_FORM,
				 XmNrightOffset,5,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,5,
				 XmNmarginWidth,2,XmNmarginHeight,2,NULL);
  
  string=XmStringGenerate(_("Generic"),XmFONTLIST_DEFAULT_TAG,          
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  XtVaCreateManagedWidget("label",xmLabelGadgetClass,frame3,
                          XmNlabelString,string,
                          XmNframeChildType,XmFRAME_TITLE_CHILD,
                          XmNchildHorizontalAlignment,XmALIGNMENT_CENTER,
                          XmNchildVerticalAlignment,XmALIGNMENT_CENTER,
                          NULL);
  XmStringFree(string);

  form=XtVaCreateManagedWidget("form",xmFormWidgetClass,frame3,NULL);
  
  string=XmStringGenerate(_("Break enabled if interrupt occur"),
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE); 
  data[2]=XtVaCreateManagedWidget("toggle_debugger",xmToggleButtonGadgetClass,
				  form,XmNtopAttachment,XmATTACH_FORM,
				  XmNleftAttachment,XmATTACH_FORM,
				  XmNrightAttachment,XmATTACH_FORM,
				  XmNlabelString,string,
				  NULL);
  XmStringFree(string);
  
  string=XmStringGenerate(_("OK"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("OK",xmPushButtonGadgetClass,
				 debugger_window,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,option_form,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,XmNlabelString,string,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,2,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)use_debugger_callback,data);
  XmStringFree(string);
  
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 debugger_window,
				 XmNtopAttachment,XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,option_form,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,4,XmNlabelString,string,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,3,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);

  XtVaSetValues(frame2,XmNrightAttachment,XmATTACH_WIDGET,
		XmNrightWidget,frame3,XmNrightOffset,5,NULL);
  
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)close_debugger_callback,data);
  XmStringFree(string);
  XtAddCallback(shell,XmNpopupCallback,(XtCallbackProc)set_size_static_window,
		NULL);
}

void use_debugger_callback(Widget w,Widget *client_data,XtPointer call_data)
{
  char *string;
  unsigned int number;
  unsigned char set;

  string=XmTextFieldGetString(client_data[0]);
  if((number=strtoul(string,NULL,10)))
    multi_step=number;
  XtFree(string);
  string=XmTextFieldGetString(client_data[1]);
  if((number=strtoul(string,NULL,10)))
    auto_step=number;
  XtFree(string);
  XtVaGetValues(client_data[2],XmNset,&set,NULL);
  if(set==XmSET) debug_flag=debug_flag|DEBUG_INTERRUPT;
  else debug_flag=debug_flag&(~DEBUG_INTERRUPT);
    
  XtUnmanageChild(debugger_window);
}

void close_debugger_callback(Widget w,Widget *client_data,XtPointer call_data)
{
  char string[10];

  sprintf(string,"%d",multi_step);
  XtVaSetValues(client_data[0],XmNvalue,string,NULL);
  
  sprintf(string,"%d",auto_step);
  XtVaSetValues(client_data[1],XmNvalue,string,NULL);

  
  if(debug_flag&DEBUG_INTERRUPT)
    XtVaSetValues(client_data[2],XmNset,XmSET,NULL);
  else 
    XtVaSetValues(client_data[2],XmNset,XmUNSET,NULL);
    
  XtUnmanageChild(debugger_window);
}

void create_project_selection(void)
{
  int n,size_list,size_string,t;
  XmString *list,string;
  char buf[30];
  Widget shell,rowcolumn,label,button,frame,menu;

  /* create field for choosen processor */
  shell=XtVaCreateWidget("project",xmDialogShellWidgetClass,toplevel,
			 XmNtitle,_("Project"),
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,NULL);
  project_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
				  XmNautoUnmanage,False,XmNdialogStyle,
				  XmDIALOG_FULL_APPLICATION_MODAL,
				  XmNresizePolicy,XmRESIZE_NONE,
				  XmNfractionBase,5,NULL);
  
  frame=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,project_window,
				XmNtopAttachment,XmATTACH_FORM,
				XmNtopOffset,5,
				XmNleftAttachment,XmATTACH_FORM,
				XmNleftOffset,5,
				XmNrightAttachment,XmATTACH_FORM,
				XmNrightOffset,5,NULL);
  rowcolumn=
    XtVaCreateManagedWidget("Rowcolumn",xmRowColumnWidgetClass,frame, 
			    XmNpacking,XmPACK_COLUMN,      
			    XmNorientation,XmHORIZONTAL,XmNisAligned,True,
			    XmNentryAlignment,XmALIGNMENT_BEGINNING,
			    XmNspacing,3,XmNentryVerticalAlignment,
			    XmALIGNMENT_CENTER,XmNnumColumns,4,NULL);  

  size_string=1;n=0;
  while(controllers[n].name!=(char *)NULL) 
    {
      if((t=strlen(controllers[n].name))>size_string) size_string=t;
      n++;
    }
  size_list=n;
  
  if((list=malloc(size_list*sizeof(XmString)))==NULL)
    {
      puts("create_project_selection(): can't allocate memory for 'list'");
      exit(1);
    }

  for(n=0;n<size_list;n++)
    list[n]=XmStringCreateLtoR(controllers[n].name,XmSTRING_DEFAULT_CHARSET);
  

  string=XmStringGenerate(_("Processor:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("processor_label",xmLabelGadgetClass,
				rowcolumn,XmNlabelString,string,NULL);
  XmStringFree(string);

  combo_proc=XtVaCreateManagedWidget("processor_box",xmComboBoxWidgetClass,
				     rowcolumn,
				     XmNcomboBoxType,XmDROP_DOWN_LIST,
				     XmNitems,list,XmNitemCount,size_list,
				     XmNvisibleItemCount,10,
				     XmNcolumns,size_string,	       
				     NULL);

  XtVaSetValues(XtNameToWidget(combo_proc,"*Text"),XmNshadowThickness,1,NULL);

  for(n=0;n<size_list;n++) XmStringFree(list[n]);
  free(list);
  /* end field for choosen processor*/

  /* setting frequency */

  string=XmStringGenerate(_("Frequency:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("freq_label",xmLabelGadgetClass,rowcolumn,
				XmNlabelString,string,NULL);
  XmStringFree(string);

  sprintf(buf,"%d",frequency);
  freq_field=XtVaCreateManagedWidget("freq_field",xmTextFieldWidgetClass,
				     rowcolumn,XmNcolumns,8,XmNmaxLength,8,
				     XmNvalue,buf,
				     XmNcursorPositionVisible,True,NULL);
  /* end of setting frequency */

  /* setting watchdog frequency */

  string=XmStringGenerate(_("Watchdog frequency:"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("watchdog_freq_label",xmLabelGadgetClass,
				rowcolumn,XmNlabelString,string,NULL);
  XmStringFree(string);
  
  sprintf(buf,"%d",wtd_freq);
  watchdog_freq_field=
    XtVaCreateManagedWidget("field",xmTextFieldWidgetClass,
			    rowcolumn,XmNcolumns,8,XmNmaxLength,8,XmNvalue,buf,
			    XmNcursorPositionVisible,True,NULL);
  /* end of setting watchdog frequency */

  /* choosing Reset Delay Time-Out Period */
  string=XmStringGenerate(_("Reset Delay Time-Out Period:"),
			  XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  label=XtVaCreateManagedWidget("watchdog_freq_label",xmLabelGadgetClass,
				rowcolumn,XmNlabelString,string,NULL);
  XmStringFree(string);

  prog_option_menu=XmCreateOptionMenu(rowcolumn,"option_menu",NULL,0);
  menu=XmCreatePulldownMenu(prog_option_menu,"option",NULL,0);
  
  string=XmStringGenerate(_("Unprogrammed"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  prog_button[UNPROGRAMMED]=
    XtVaCreateManagedWidget("unprogrammed",xmPushButtonGadgetClass,
			    menu,XmNlabelString,string,NULL);
  XtAddCallback(prog_button[UNPROGRAMMED],XmNactivateCallback,
		(XtCallbackProc)watchdog_unprog_callback,NULL);
  XmStringFree(string);
  string=XmStringGenerate(_("Programmed"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  prog_button[PROGRAMMED]=
    XtVaCreateManagedWidget("programmed",xmPushButtonGadgetClass,
			    menu,XmNlabelString,string,NULL);
  XtAddCallback(prog_button[PROGRAMMED],XmNactivateCallback,
		(XtCallbackProc)watchdog_prog_callback,NULL);
  XmStringFree(string);
 
  
  XtVaSetValues(prog_option_menu,XmNsubMenuId,menu,NULL);
  XtManageChild(prog_option_menu);
  /* end of choosing Reset Delay Time-Out Period */

  
  string=XmStringGenerate(_("OK"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("OK",xmPushButtonGadgetClass,
				 project_window,XmNtopAttachment,
				 XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,label,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,1,XmNlabelString,string,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,2,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)use_project_callback,NULL);
  XmStringFree(string);
  
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,
				 project_window,XmNtopAttachment,
				 XmATTACH_WIDGET,
				 XmNtopOffset,7,XmNtopWidget,
				 watchdog_freq_field,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,4,XmNlabelString,string,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,3,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNbottomOffset,7,NULL);
  
  XtAddCallback(button,XmNactivateCallback,
		(XtCallbackProc)close_project_callback,NULL);
  XmStringFree(string);
  XtAddCallback(shell,XmNpopupCallback,(XtCallbackProc)set_size_static_window,
		NULL);

}

void watchdog_unprog_callback(Widget w,XtPointer client,XtPointer call)
{
  wacthdog_timeout_temp=UNPROGRAMMED;
}

void watchdog_prog_callback(Widget w,XtPointer data,XtPointer call)
{
  wacthdog_timeout_temp=PROGRAMMED;
}

void close_project_callback(Widget w,XtPointer data,XtPointer call)
{
  char buf[10];
 
  XtVaSetValues(combo_proc,XmNselectedPosition,type_micro,NULL);
  sprintf(buf,"%d",frequency);
  XmTextFieldSetString(freq_field,buf);
  sprintf(buf,"%d",wtd_freq);
  XmTextFieldSetString(watchdog_freq_field,buf);
  XtVaSetValues(prog_option_menu,XmNmenuHistory,prog_button[wacthdog_timeout],
		NULL);
  XtUnmanageChild(project_window);
}

/* check format of file */
int check_port_file(char *name)
{
  FILE *file;
  int temp1,temp2,result;

  if((file=fopen(name,"r"))==NULL)
    {
      sprintf(error_message,"open file '%s': %s",name,strerror(errno));
      return 1;
    }
  while((result=fscanf(file,"%d:%x",&temp1,&temp2))==2);
  if(result!=EOF)
    {
      sprintf(error_message,"file '%s' has bad format",name);
      return 1;
    }

  fclose(file);
  return 0;
}

