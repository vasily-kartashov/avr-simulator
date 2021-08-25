/*
 File        : help.c

 Author      : Sergiy Uvarov - Copyright (C) 2001

 Description : Creating help menu.

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
#include <libintl.h>
#include <Xm/XmAll.h>
#include "types.h"
#include "global.h"

void create_about(void)
{
  Widget shell,frame,button,label;
  XmString strings[10];
  Pixel pixel;
  int n;
  Arg args[10];
  XmRenderTable render_table;
  XmRendition renditions[10];
  XmString string;
  XColor color;
  
  shell=XtVaCreateWidget("Shell",xmDialogShellWidgetClass,toplevel,
			 XmNtitle,_("About"),
			 XmNmwmFunctions,MWM_FUNC_MOVE|MWM_FUNC_CLOSE,
			 XmNmwmDecorations,MWM_DECOR_BORDER|MWM_DECOR_TITLE,NULL);
  about_window=XtVaCreateWidget("form",xmFormWidgetClass,shell,
				XmNdialogStyle,XmDIALOG_MODELESS,
				XmNresizePolicy,XmRESIZE_NONE,
				XmNmarginHeight,10,XmNmarginWidth,10,NULL);
   
  frame=XtVaCreateManagedWidget("frame",xmFrameWidgetClass,about_window,
				XmNtopAttachment,XmATTACH_FORM,
				XmNleftAttachment,XmATTACH_FORM,
				XmNrightAttachment,XmATTACH_FORM,NULL);
  
  if(!XAllocNamedColor(display,DefaultColormap(display,scr_num), 
                       "blue",&color,&color))
    {                                             
      puts("Can't allocate color.");
      color.pixel=BlackPixel(display,scr_num);
    }
  pixel=color.pixel;
  
  n=0;
  XtSetArg(args[n],XmNrenditionForeground,pixel);n++;
  /*  XtSetArg(args[n],XmNfont,"misc");n++; */
  XtSetArg(args[n],XmNfontName,"fixed");n++;
  XtSetArg(args[n],XmNfontType,XmFONT_IS_FONT);n++;
  renditions[0]=XmRenditionCreate(frame,(XmStringTag)"Big_rendition",args,n);
  render_table=XmRenderTableAddRenditions(NULL,renditions,1,XmMERGE_REPLACE);

  strings[0]=XmStringGenerate( "                  AVR Simulator V1.3.1\n\n",NULL,
			       XmCHARSET_TEXT,"Big_rendition");
  strings[1]=XmStringGenerate(" 
 AVR Simulator - A GNU/Linux simulator for the Atmel AVR series  
 of microcontrollers. 

 This program was developed as the graduated work. 
 Copyright (C) 2001 Sergiy Uvarov          

 This program is free software; you can redistribute it and/or  
 modify it under the terms of the GNU General Public License    
 as published by the Free Software Foundation; either version 2 
 of the License, or (at your option) any later version.         
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public
 License  along with this program; if not, write to the Free
 Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 MA 02111-1307, USA.
 
 Sergiy Uvarov

 Web site: http://avr.sourceforge.net 
 E-mail  : colonel@ff.dsu.dp.ua\n",
			     XmFONTLIST_DEFAULT_TAG,
			     XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  XmStringConcatAndFree(strings[0],strings[1]);
  label=XtVaCreateManagedWidget("about_label",xmLabelGadgetClass,frame,
				XmNrenderTable,render_table,
				XmNlabelString,strings[0],
				XmNalignment,XmALIGNMENT_BEGINNING,NULL);

  
  XmStringFree(strings[0]);

  
  string=XmStringGenerate(_("Close"),XmFONTLIST_DEFAULT_TAG,
			  XmCHARSET_TEXT,_MOTIF_DEFAULT_LOCALE);
  button=XtVaCreateManagedWidget("Close",xmPushButtonGadgetClass,about_window,
				 XmNbottomAttachment,XmATTACH_FORM,
				 XmNleftAttachment,XmATTACH_POSITION,
				 XmNleftPosition,40,
				 XmNrightAttachment,XmATTACH_POSITION,
				 XmNrightPosition,60,
				 XmNlabelString,string,NULL);
  XmStringFree(string);

  XtVaSetValues(frame,XmNbottomAttachment,XmATTACH_WIDGET,
		XmNbottomWidget,button,XmNbottomOffset,10,NULL);
  
  XtAddCallback(shell,XmNpopupCallback,(XtCallbackProc)set_size_static_window,NULL);
}

