project         = avr_simulator
prefix          = /usr/local
objs            = avr_simulator.o func_command.o hardware.o xs_wtip.o\
		buttons_menu.o menu_options.o menu_memory.o help.o 
x_includes      = -I/usr/X11R6/include
motif_includes  = -I/usr/X11R6/include

# X and Motif libraries and locations.
xlib            = -L/usr/X11R6/lib -lXmu -lXp -lXext -lXt  -lX11 
motif           = -L/usr/X11R6/lib -lXm 
thread		= -lpthread

# Using gcc compiler....
CC              =  gcc

DEBUG           = -g
# Our flags to create object files....
CFLAGS          = $(motif_includes) $(x_includes) -Wall -ansi $(DEBUG)

.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: clean locale install uninstall
.IGNORE: clean uninstall

#===========================================================================
$(project) : $(objs)
	@echo "Linking avr_simulator"
	$(CC)  -o $(project) $(objs) $(motif) $(xlib) $(thread)
#===========================================================================

avr_simulator.o: avr_simulator.h types.h commands.h microcontrollers.h avr/*.h

$(objs): global.h types.h
#===========================================================================
clean:
	@echo "Cleaning..."
	rm -f *~ core $(objs) $(project) avr/*~
	@echo "Cleaned"

locale:
	xgettext -j -o $(project).po -k_ *.c

install: uninstall
	install $(project) $(prefix)/bin
	mkdir $(prefix)/share/$(project)
	install -m 644 *.xpm $(prefix)/share/$(project)
uninstall:
	rm -rf $(prefix)/bin/$(project) $(prefix)/share/$(project)
