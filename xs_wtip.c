/* File:        widget_tip.c
 *
 * Description: Code to display a "widget tip" for a specified widget
 *              A widget tip is a window which is pop'ed when the mouse 
 *		moves into the the specified widget. It's
 *              similar to a tooltip/widget_tip/ballon_help ...
 *
 * Author:      George MacDonald
 *
 * Copyright:   GPL - see http://www.gnu.org
 *
 * History:     George MacDonald        2/15/99        Created
 *
 * Changed a little by Sergiy Uvarov 06/26/2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/XmAll.h>

void 		xs_widget_tip();
static void 	widget_tip_enter();
static void 	widget_tip_leave();
static void 	widget_tip_press();
static void 	widget_tip_startup_timeout();
static void 	widget_tip_uptime_timeout();
static Window 	create_widget_tip_window();
void 		destroy_widget_tip();
void 		widget_tip_cancel_timer();
Pixmap 		get_color();
void 		widget_tip_set_font();

Window 		Widget_tip_window=0;
XtIntervalId 	Widget_tip_interval_id=-1;
char 		*Widget_tip_str=NULL;
Position 	Widget_tip_x, Widget_tip_y;
Widget   	Widget_tip_w=NULL;
int		Default_widget_tip_uptime=5000;
int		Default_widget_tip_startup_delay=800;
char 		*Default_widget_tip_fontname="8x13";
int		Widget_tip_uptime;
int		Widget_tip_startup_delay;
char 		*Widget_tip_fontname;
Pixel		Widget_tip_bg=-1;
Pixel		Widget_tip_fg=-1;
extern Display *display;
extern int scr_num;

void xs_widget_tip(Widget w,char *tipStr)
{
  static int first=1;
  
  if (first)
    {
      Widget_tip_uptime=Default_widget_tip_uptime;
      Widget_tip_startup_delay=Default_widget_tip_startup_delay;
      Widget_tip_fontname=Default_widget_tip_fontname;
      Widget_tip_bg=get_color(w,"lightgray");
      Widget_tip_fg=get_color(w,"black");
      
      first=0;
    }
  
  XtAddEventHandler(w,EnterWindowMask,FALSE,widget_tip_enter,tipStr);
  XtAddEventHandler(w,LeaveWindowMask,FALSE,widget_tip_leave,NULL);
  XtAddEventHandler(w,ButtonPressMask,FALSE,widget_tip_press,NULL);
}

void delete_tip(Widget w,char *tipStr)
{
  destroy_widget_tip(w);
  XtRemoveEventHandler(w,EnterWindowMask,FALSE,widget_tip_enter,tipStr);
  XtRemoveEventHandler(w,LeaveWindowMask,FALSE,widget_tip_leave,NULL);
  XtRemoveEventHandler(w,ButtonPressMask,FALSE,widget_tip_press,NULL);
}

void widget_tip_enter(Widget w,char *tipStr,XCrossingEvent *event,
		      Boolean *continueDispatch)
{
  Arg 		wargs[20];
  int 		n;
  XtAppContext 	ctx;
  Position	widget_x, widget_y;
  Position	root_x, root_y;

  ctx=XtWidgetToApplicationContext(w);
  
  *continueDispatch = TRUE;
  
  Widget_tip_w=w;
  
  n=0;
  XtSetArg(wargs[n],XtNwidth, &widget_x);n++;
  XtSetArg(wargs[n],XtNheight,&widget_y);n++;
  XtGetValues(w,wargs,n);
  
  XtTranslateCoords(w,widget_x,widget_y,&root_x,&root_y);
  
  Widget_tip_x=root_x-(widget_x/3);
  Widget_tip_y=root_y+6;
  
  if (tipStr==NULL) 
    return ;
  else	/* Create a widget tip? */
    {  
      if (Widget_tip_window!=0) /* Already got one up!? */
	{
	  if (strcmp(Widget_tip_str,tipStr)!=0)
	    destroy_widget_tip(w);
	  else
	    return;
	}
      
      if(Widget_tip_str==NULL)
	/*Widget_tip_str=(char *)strdup(tipStr);*/
	Widget_tip_str=tipStr;
      
      Widget_tip_interval_id=
	XtAppAddTimeOut(ctx,Widget_tip_startup_delay,
			widget_tip_startup_timeout,w);
    }
  
  *continueDispatch = FALSE;
  
  return;
}

void widget_tip_press(Widget w,char *tipStr,XButtonPressedEvent *event,
		      Boolean *continueDispatch)
{
  destroy_widget_tip(w);
  
  *continueDispatch = TRUE;
}


void widget_tip_leave(Widget w,char *tipStr,XCrossingEvent *event,
		      Boolean *continueDispatch)
{
  XtAppContext 	ctx;

  ctx=XtWidgetToApplicationContext(w);
  
  *continueDispatch=TRUE;
  
  Widget_tip_w=w;
  Widget_tip_x=event->x_root; 
  Widget_tip_y=event->y_root;  
  
  if(tipStr==NULL)
    {
      if(event->detail==NotifyNonlinear)
	{
	  destroy_widget_tip(w);
	}
      else
	destroy_widget_tip(w);
      
      Widget_tip_w = NULL;
    }
  else	/* Eh? */
    {
      
      return;
    }
  
  *continueDispatch = FALSE;
  
  return;
}

void widget_tip_startup_timeout(Widget w,XtIntervalId id)  

{
  XtAppContext 	ctx;
  
  ctx=XtWidgetToApplicationContext(w);
   
  if(Widget_tip_window==0)
    { 
      if(Widget_tip_w==NULL||Widget_tip_str==NULL) return;
      Widget_tip_window=
	create_widget_tip_window(Widget_tip_w,Widget_tip_str, 
				 Widget_tip_x,Widget_tip_y);
      Widget_tip_interval_id=
	XtAppAddTimeOut(ctx,Widget_tip_uptime,widget_tip_uptime_timeout,w);
    }
}

void widget_tip_uptime_timeout(Widget w,XtIntervalId id)  
{
  
  if(Widget_tip_window!=0)
    {
      Widget_tip_interval_id=-1;
      destroy_widget_tip(w);
    }
}

void destroy_widget_tip(Widget w)
{
  
  if(Widget_tip_window!=0)
    {
      XDestroyWindow(display,Widget_tip_window);
      Widget_tip_window=0;
    }
  
  if(Widget_tip_str!=NULL)
    {
      /*free(Widget_tip_str);*/
      Widget_tip_str=NULL;
    }
  
  widget_tip_cancel_timer();
}

void widget_tip_cancel_timer()
{
  
  if(Widget_tip_interval_id!=-1)
    {
      XtRemoveTimeOut( Widget_tip_interval_id );
      Widget_tip_interval_id=-1;
    }
}

Window create_widget_tip_window(Widget w,char *str,int x,int y)
{
  static int first=1;
  static int screen;
  static Screen *screen_ptr=NULL;
  static char *window_name = "Widget_tip";
  static XFontStruct *font_info;
  static int depth;
  static Visual *visual;
  static unsigned int class;
  static unsigned long valuemask;
  static unsigned long GCvaluemask=0;
  static XSetWindowAttributes setwinattr;
  static XGCValues values;
  static int border_width;
  XSizeHints 	size_hints;
  int  		str_x, str_y;
  unsigned int 	width, height;
  int 		len;
  GC 		gc;		/* Need to free this */
  Window	win;
  int           win_x, win_y;
  
  if(first)
    {
      border_width=1;
      screen=DefaultScreen(display);
      screen_ptr=ScreenOfDisplay(display,screen);

      if((font_info=XLoadQueryFont(display,Widget_tip_fontname))==NULL)
	{
	  fprintf( stderr, "Can't open %s font!!!!\n",Widget_tip_fontname);
	  return 0;
	}
      
      depth=CopyFromParent;
      class=CopyFromParent;
      visual=CopyFromParent;
      
      valuemask=CWBackPixel|CWBorderPixel|CWOverrideRedirect;
      
      setwinattr.background_pixel=Widget_tip_bg;
      setwinattr.border_pixel=BlackPixel(display,screen);
      setwinattr.override_redirect=True;
      
      first = 0;
    }
  
  len=strlen(str);
  width=XTextWidth(font_info,str,len)+6;
  height=font_info->max_bounds.ascent+font_info->max_bounds.descent+4;
  
  str_x=3;
  str_y=font_info->max_bounds.ascent+2;
   
  win_x=x;
  if((win_x+width)>WidthOfScreen(screen_ptr))
    {
      win_x=((int)WidthOfScreen(screen_ptr))-width;
      if(win_x<0)
	win_x=0;
    }
  
  win_y=y;
  if(win_y+(height*2)>HeightOfScreen(screen_ptr))
    {
      win_y=((int)HeightOfScreen(screen_ptr))-(4*height);
      if (win_y<0)
	win_y=0;
    }
  
  win=XCreateWindow(display,RootWindow(display,screen),win_x,win_y,width,
		    height,border_width,depth,class,visual,
		    valuemask,&setwinattr );
  
  gc=XCreateGC(display,win,GCvaluemask,&values );
  
  XSetFont(display,gc,font_info->fid);
  
  XSetForeground(display,gc,Widget_tip_fg);
  
  size_hints.flags=PPosition|PSize|PMinSize;
  size_hints.x=win_x;
  size_hints.y=win_y;
  size_hints.width=width;
  size_hints.height=height;
  size_hints.min_width=width;
  size_hints.min_height=height;
  
  XSetStandardProperties(display,win,window_name,NULL,0,NULL,0,&size_hints);
  
  XMapWindow(display,win);
  XDrawString(display,win,gc,str_x,str_y,str,len);
  XFlush(display);
  XFreeGC(display,gc);
  
  return win;
}

/* These must be called before first creation of a widget tip, otherwise
 * the default values will be used
 */
Pixel get_color(Widget w,char *colorName)
{
  Colormap cmap;
  XColor   color;
  XColor   exact;
  
  cmap=DefaultColormap(display,scr_num);

  /* Lookup Pixel colors based on character string symbolic names */
  if(XAllocNamedColor(display,cmap,colorName,&color,&exact))
    return color.pixel;
  else
    {
      fprintf(stderr, 
	      "Warning: Couldn't allocate color %s\n",colorName);
      return BlackPixel(display,scr_num);
    }
}

void widget_tip_set_font(char *fontName)
{ 
  /*Widget_tip_fontname=(char *)strdup(fontName);*/
}
