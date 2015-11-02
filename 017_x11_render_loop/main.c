// clang -o test main.c -std=c99 -lX11

// problems:
// * load system fonts?
// * tearing vsyc refresh rate
// * window close does not quit / server times out

#include <X11/Xlib.h>
#include <stdio.h>
#include <unistd.h> // sleep
#include <stdbool.h>
#include <string.h>

void draw_frame (Display* display, Window window, GC graphics_context) {
	if (!display) {
		fprintf (stderr, "ERROR: lost display\n");
		return;
	}
	
	XClearWindow (display, window);
	
	XDrawLine (display, window, graphics_context, 10, 10, 1014, 758);

	int x = 30, y = 10;
	char txt[] = "Hello X11!";
	int len = strlen (txt);
	XDrawString (display, window, graphics_context, x, y, txt, len);
	x = 30, y = 20;
	char txtb[] = "Here is a lengthy treatise on fonts.";
	len = strlen (txtb);
	XDrawString (display, window, graphics_context, x, y, txtb, len);
	
	//XFlush (display); // dispatches the command queue
}

void event_loop (Display* display, Window window, GC graphics_context) {
	while (true) {
		if (!display) {
			fprintf (stderr, "ERROR: lost display\n");
			return;
		}
	
		XEvent event;
		XNextEvent (display, &event);
		if (event.type == Expose) { // was MapNotify
			draw_frame (display, window, graphics_context);
		}
	}
}

int main () {
	printf ("X11 demo\n");

	Display* display = NULL;
	GC graphics_context;
	Window window;
	int black_colour, white_colour;

	{ // create window and display combo
		printf ("init window...\n");
		display = XOpenDisplay (NULL);
		if (!display) {
			fprintf (stderr, "ERROR: opening display\n");
			return 1;
		}
		black_colour = BlackPixel (display, DefaultScreen (display));
		white_colour = WhitePixel (display, DefaultScreen (display));
		// note: can use CopyFromParent instead of colour index
		// note: there is also XCreateWindow with more params
		// simplewindow also clears the window to background colour
		// 0,0 is request position, other 0 is border (not used)
		window = XCreateSimpleWindow (display, DefaultRootWindow (display), 0, 0,
			1024, 768, 0, black_colour, black_colour);
		// get MapNotify events etc.
		XSelectInput (display, window, ExposureMask); // was StructureNotifyMask
		// put window on screen command
		XMapWindow (display, window);
		// create graphics context
		graphics_context = XCreateGC (display, window, 0, 0);
	} // endinitwindow

	{ // draw
		// set drawing colour
		XSetForeground (display, graphics_context, white_colour);
		

		

		// note: can also unload a font
		const char* font_name = "-*-Monospace-*-10-*";
		XFontStruct* font = XLoadQueryFont (display, font_name);
		if (!font) {
			fprintf (stderr, "ERROR: could not load font %s\n", font_name);
			font = XLoadQueryFont (display, "fixed");
			if (!font) {
				fprintf (stderr, "ERROR: default font fixed did not load\n");
				return 1;
			}
		}
		XSetFont (display, graphics_context, font->fid);

		event_loop (display, window, graphics_context);
	} // enddraw

	return 0;
}
