/* 
 
 Author: Cody Wright          
 Start Date: October 12, 2018  
 3480 Computer Graphics       
 projectMain.cpp              

 Compile Instructions:         
      $ make clean               
      $ make                     
      $ ./project                

*/

/* Dreamcast Intro */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xdbe.h>
#include <X11/Xutil.h>

using namespace std;

//Variable types...
typedef double Flt;
typedef Flt Vec[3];
#define PI 3.14159265358979
#define SWAP(x,y) (x)^=(y);(y)^=(x);(x)^=(y)
#define rnd() ((Flt)rand() / (Flt)RAND_MAX)
//constant definitions...
const int MAXOBJECTS =  264;
enum {
	TYPE_NONE=0,
	TYPE_DISK,
	TYPE_RING,
	TYPE_SPHERE,
	TYPE_TRIANGLE,
	TYPE_CYLINDER,
	ORTHO,
	PERSPECTIVE,
	SURF_NONE,
	SURF_CHECKER,
	SURF_BARY,
    SURF_SPHERICAL,
    SURF_CYLINDRICAL,
    SURF_DISK,
    SURF_TRIANGLE,
    SURF_TRIANGLE_R,
    SURF_TRIANGLE_E,
    SURF_TRIANGLE_A,
    SURF_TRIANGLE_M,
    SURF_TRIANGLE_C,
    SURF_TRIANGLE_S,
    SURF_TRIANGLE_T,
    SURF_TRIANGLE_BALL
};

//Ray tracing structures
struct Ray {
	//Ray origin and direction
	Vec o, d;
};

struct Hit {
	//t     = distance to hit point
	//p     = hit point
	//norm  = normal of surface hit
	//color = color of surface hit
	Flt t;
	Vec p, norm, color;
};

struct Object {
	int type;
	Vec center;
	Vec norm;
	Vec tri[3];
    Vec tri_img[3];
	Flt radius, radius2;
    Flt apex;
	Vec color;
	int surface;
	bool specular;
	Vec spec;
};

class Image {
public:
	int width, height;
	unsigned char *data;
	~Image() { delete [] data; }
	Image() { }
	Image(const char *fname) {
		if (fname[0] == '\0')
			return;
		int ppmFlag = 0;
		char name[40];
		strcpy(name, fname);
		int slen = strlen(name);
		char ppmname[80];
		if (strncmp(name+(slen-4), ".ppm", 4) == 0)
			ppmFlag = 1;
		if (ppmFlag) {
			strcpy(ppmname, name);
		} else {
			name[slen-4] = '\0';
			sprintf(ppmname,"%s.ppm", name);
			char ts[100];
			sprintf(ts, "convert %s %s", fname, ppmname);
			system(ts);
		}
		FILE *fpi = fopen(ppmname, "r");
		if (fpi) {
			char line[200];
			fgets(line, 200, fpi);
			fgets(line, 200, fpi);
			while (line[0] == '#')
				fgets(line, 200, fpi);
			sscanf(line, "%i %i", &width, &height);
			fgets(line, 200, fpi);
			//get pixel data
			int n = width * height * 3;			
			data = new unsigned char[n];			
			for (int i=0; i<n; i++)
				data[i] = fgetc(fpi);
			fclose(fpi);
		} else {
			printf("ERROR opening image: %s\n",ppmname);
			exit(0);
		}
		if (!ppmFlag)
			unlink(ppmname);
	}
	//overloaded assignment operator.
	void operator = (const Image &i) {
		width = i.width;
		height = i.height;
		data = new unsigned char[width*height*3];
		unsigned char *p = data;
		unsigned char *p2 = i.data;
		for (int i=0; i<height*width*3; i++) {
			*p = *p2;
			++p;
			++p2;
		}
	}
} img[9] = {"D.jpg", "r.jpg", "e.jpg", "a.jpg",
            "m.jpg", "c.jpg", "s.jpg", "t.jpg", "ball.jpg"};

class Global {
	public:
		int xres, yres;
        // ray tracing
		int mode;
		Object object[MAXOBJECTS];
		int nobjects;
		Vec ambient;
		Vec lightPos;
		Vec diffuse;
		int cornell;
		Vec from, at, up;
		Flt angle;
        bool background;
        Flt bx;
        Flt by;
        Flt gravity;
        Flt acceleration;
        Flt xforce;
        Flt yforce;
        Flt floor;
        Flt wall;
        Flt anti_gravity;
        int letters;
        int d;
        Flt dy;
        Flt dc;
        Flt dc1;
        int r;
        Flt ry;
        Flt rc;
        Flt rc1;
        int e;
        Flt ey;
        Flt ec;
        Flt ec1;
        int a1;
        Flt a1y;
        Flt a1c;
        Flt a1c1;
        int m;
        Flt my;
        Flt mc;
        Flt mc1;
        int c;
        Flt cy;
        Flt cc;
        Flt cc1;
        int a2;
        Flt a2y;
        Flt a2c;
        Flt a2c1;
        int s;
        Flt sy;
        Flt sc;
        Flt sc1;
        int t;
        Flt ty;
        Flt tc;
        Flt tc1;
        int disk_x;
        int disk_y;
        int pixel_count;
        int color_map[446] = {};
        int appature;
		
        Global() {
			srand((unsigned)time(NULL));
			xres = 800, yres = 600;
			mode = 0;
			nobjects = 0;
			cornell = 0;
			void vecMake(Flt a, Flt b, Flt c, Vec v);
			vecMake(0.0, 0.0, 0.0, ambient);
			vecMake(0.99, 0.99, 0.99, diffuse);
			vecMake(-10.0, 4000.0, -500.0, lightPos);
			//
			vecMake(0.0, 0.0,  1000.0, from);
			vecMake(0.0, 0.0, -1000.0, at);
			vecMake(0.0, 1.0,  0.0,    up);
			angle = 45.0;
            background = false;
            bx = 0.0;
            by = 0.0;
            gravity = 0;
            acceleration = 6;
            xforce = 50;
            yforce = -144;
            floor = -1296.0;
            wall = 1550;
            letters = 0;
		}
} g;

class X11_wrapper {
	private:
		Display *dpy;
		Window win;
		GC gc;
	public:
		X11_wrapper() {
			//constructor
			if (!(dpy=XOpenDisplay(NULL))) {
				fprintf(stderr, "ERROR: could not open display\n"); fflush(stderr);
				exit(EXIT_FAILURE);
			}
			int scr = DefaultScreen(dpy);
			win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 1, 1,
					g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
			XStoreName(dpy, win, "3480 ray tracer. M-menu. ");
			gc = XCreateGC(dpy, win, 0, NULL);
			XMapWindow(dpy, win);
			XSelectInput(dpy, win, ExposureMask | StructureNotifyMask |
					PointerMotionMask | ButtonPressMask |
					ButtonReleaseMask | KeyPressMask);
            //backBuffer = XdbeAllocateBackBufferName(dpy, win, XdbeUndefined);
		}
		~X11_wrapper() {
			XDestroyWindow(dpy, win);
			XCloseDisplay(dpy);
		}
		
       void setTitle(){
			char ts[256];
			sprintf(ts,"3480 ray tracer. M-menu. %i x %i.",g.xres, g.yres);
		}
		void getWindowAttributes(int *width, int *height) {
			XWindowAttributes gwa;
			XGetWindowAttributes(dpy, win, &gwa);
			*width = gwa.width;
			*height = gwa.height;
		}
		XImage *getImage(int width, int height) {
			//XImage *image = XGetImage(dpy, win,
			//            0, 0 , width, height, AllPlanes, ZPixmap);
			return XGetImage(dpy, win, 0, 0, width, height, AllPlanes, ZPixmap);
		}
		void checkResize(XEvent *e) {
			//ConfigureNotify is sent when window size changes.
			if (e->type != ConfigureNotify)
				return;
			XConfigureEvent xce = e->xconfigure;
			g.xres = xce.width;
			g.yres = xce.height;
			//printf("%ix%i\n", g.xres, g.yres); fflush(stdout);
		}
		void clearScreen() {
			//XClearWindow(dpy, win);
			setColor3i(0,0,0);
			XFillRectangle(dpy, win, gc, 0, 0, g.xres, g.yres);
		}

		bool getXPending() {
			return (XPending(dpy));
		}
		void getXNextEvent(XEvent *e) {
			XNextEvent(dpy, e);
		}
		void setColor3i(int r, int g, int b) {
			unsigned long cref = (r<<16) + (g<<8) + b;
			XSetForeground(dpy, gc, cref);
		}
		unsigned long rgbToLong(Vec rgb) {
			//Convert rgb[3] into an integer
			const float range = 255.999;
			int i;
			unsigned long cref = 0;
			for (i=0; i<3; i++) {
				//Don't let a color get too bright.
				if (rgb[i] > 1.0)
					rgb[i] = 1.0;
				//Shift left 8 bits
				cref <<= 8;
				//Put next color component in low-order byte
				cref += (int)(rgb[i]*range);
			}
			return cref;
		}
		void drawPixel(int x, int y, Vec rgb) {
			unsigned long cref = rgbToLong(rgb);
			XSetForeground(dpy, gc, cref);
			XDrawPoint(dpy, win, gc, x, y);
		}
		void drawText(int x, int y, const char *text) {
			XDrawString(dpy, win, gc, x, y, text, strlen(text));
		}
} x11;

// ray tracing
void init(void);
void checkResize(XEvent *e);
void checkMouse(XEvent *e);
int checkKeys(XEvent *e);
void physics();
void render(int projection);
//vector functions...
void vecZero(Vec v);
void vecMake(Flt a, Flt b, Flt c, Vec v);
void vecCopy(Vec source, Vec dest);
void vecSub(Vec v0, Vec v1, Vec dest);
void vecNormalize(Vec v);
Flt vecDotProduct(Vec v0, Vec v1);
Flt vecLength(Vec v);


int main(void)
{
	srand((unsigned)time(NULL));
	init();
	x11.clearScreen();
	int done=0;
	while (!done) {
		while (x11.getXPending()) {
			XEvent e;
			x11.getXNextEvent(&e);
			x11.checkResize(&e);
			checkMouse(&e);
			done = checkKeys(&e);
			//render();
		}
	}
	return 0;
}

void takeScreenshot(const char *filename, int reset)
{
	//This function will capture your current X11 window,
	//and save it to a PPM P6 image file.
	//File names are generated sequentially.
	static int picnum = 0;
	int x,y;
	int width, height;
	x11.getWindowAttributes(&width, &height);
	if (reset)
		picnum = 0;
	XImage *image = x11.getImage(width, height);
	//
	//If filename argument is empty, generate a sequential filename...
	char ts[256] = "";
	strcpy(ts, filename);
	if (ts[0] == '\0') {
		sprintf(ts,"./lab5%02i.ppm", picnum);
		picnum++;
	}
	FILE *fpo = fopen(ts, "w");
	if (fpo) {
		fprintf(fpo, "P6\n%i %i\n255\n", width, height);
		for (y=0; y<height; y++) {
			for (x=0; x<width; x++) {
				unsigned long pixel = XGetPixel(image, x, y);
				fputc(((pixel & 0x00ff0000)>>16), fpo);
				fputc(((pixel & 0x0000ff00)>> 8), fpo);
				fputc(((pixel & 0x000000ff)    ), fpo);
			}
		}
		fclose(fpo);
	}
	XFree(image);
}

void init_cornell()
{
}

void init(void)
{
	if (g.cornell) {
		init_cornell();
		return;
	}
    g.background = true;
	//Setup some objects
	Object *o;
	g.nobjects=0;
	//
	// triangle cover
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(   -1500,   -475, 100.0, o->tri[0]);
	vecMake(    1500,   -475, 100.0, o->tri[1]);
	vecMake(       0,  -1200, 100.0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	Vec norm;
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(1,1,1, o->color);
	o->surface = SURF_NONE;
    o->specular = true;
	vecMake(1,1,1, o->spec);
	g.nobjects++;
	//-------------------------------------------------------------
	/*/ red ball
	o = &g.object[g.nobjects];
	o->type = TYPE_DISK;
	vecMake(-990+g.bx, 795+g.by, 0.0, o->center);
	vecMake(0.0, 0.0, 1.0, o->norm);
	o->radius = 30.0;
	vecMake(200,0,0, o->color);
	vecNormalize(o->norm);
    o->surface = SURF_DISK;
	g.nobjects++;
	*/
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   60, 60, 0, o->tri_img[1]);
	vecMake(   60,   0, 0, o->tri_img[2]);
	
    vecMake(   -1020+g.bx, 825+g.by , 0, o->tri[0]);
	vecMake(   -960+g.bx,  825+g.by, 0, o->tri[1]);
	vecMake(   -1020+g.bx, 765+g.by, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(200,0, 0, o->color);
	o->surface = SURF_TRIANGLE_BALL;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(   0,  60, 0, o->tri_img[0]);
	vecMake(   60,  60, 0, o->tri_img[1]);
	vecMake(   60, 0, 0, o->tri_img[2]);
	
    vecMake(  -960+g.bx,  825+g.by, 0, o->tri[0]);
	vecMake(   -960+g.bx,  765+g.by, 0, o->tri[1]);
	vecMake(   -1020+g.bx,  765+g.by, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(200,0,0, o->color);
	o->surface = SURF_TRIANGLE_BALL;
	g.nobjects++;
	/*
    // red ball
	o = &g.object[g.nobjects];
	o->type = TYPE_DISK;
	vecMake(-790, 595, 0.0, o->center);
	vecMake(0.0, 0.0, 1.0, o->norm);
	o->radius = 30.0;
	vecMake(200,0,0, o->color);
	vecNormalize(o->norm);
    o->surface = SURF_DISK;
	g.nobjects++;*/
	
	/*/TEST DISK
	o = &g.object[g.nobjects];
	o->type = TYPE_DISK;
	vecMake(0.0, 0.0, 0.0, o->center);
	vecMake(0.0, 0.0, 1.0, o->norm);
	vecMake(.7,.7,.1, o->color);
	o->radius = 100.0;
	o->specular = true;
	vecMake(.2, .2, .2, o->spec);
	o->surface = SURF_NONE;
	o->surface = SURF_DISK;
	vecNormalize(o->norm);
	g.nobjects++;*/

    //TEST TRIANGLES
/*	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   60, 60, 0, o->tri_img[1]);
	vecMake(   60,   0, 0, o->tri_img[2]);

	vecMake(     0+g.bx,   0+g.by, 0, o->tri[0]); //a
	vecMake(    60+g.bx,   0+g.by, 0, o->tri[1]); //c
	vecMake(     0+g.bx,  60+g.by, 0, o->tri[2]); //b
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(200,0,0, o->color);
	o->surface = SURF_TRIANGLE_BALL;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(   0,  60, 0, o->tri_img[0]);
	vecMake(   60,  60, 0, o->tri_img[1]);
	vecMake(   60, 0, 0, o->tri_img[2]);
	
    vecMake(     60+g.bx,  0+g.by, 0, o->tri[0]); //c
	vecMake(    60+g.bx,   60+g.by, 0, o->tri[1]);
	vecMake(    0+g.bx,  60+g.by, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(200,0,0, o->color);
	o->surface = SURF_TRIANGLE_BALL;
	g.nobjects++;
	*/
	//Letter D 
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   200, 200, 0, o->tri_img[1]);
	vecMake(   200,   0, 0, o->tri_img[2]);
	
    vecMake(   -725,   -730+g.dy+g.dc1, 0, o->tri[0]);
	vecMake(    -525,   -730+g.dy+g.dc1, 0, o->tri[1]);
	vecMake(    -725,  -530+g.dy+g.dc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(1,1,1, o->color);
	o->surface = SURF_TRIANGLE;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 200, 0, o->tri_img[0]);
	vecMake(   200, 200, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   -725,  -530+g.dy+g.dc, 0, o->tri[0]);
	vecMake(   -525,  -730+g.dy+g.dc1, 0, o->tri[1]);
	vecMake(   -525,  -530+g.dy+g.dc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE;
	g.nobjects++;
	
    // Letter R
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   -525,   -730+g.ry+g.rc1, 0, o->tri[0]);
	vecMake(    -375,   -730+g.ry+g.rc1, 0, o->tri[1]);
	vecMake(    -525,  -580+g.ry+g.rc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.2,.2,.2, o->color);
	o->surface = SURF_TRIANGLE_R;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   -525,  -580+g.ry+g.rc, 0, o->tri[0]);
	vecMake(   -375,  -730+g.ry+g.rc1, 0, o->tri[1]);
	vecMake(   -375,  -580+g.ry+g.rc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.2,.2,.2, o->color);
	o->surface = SURF_TRIANGLE_R;
	g.nobjects++;
	
    // Letter E
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   -375,   -730+g.ey+g.ec1, 0, o->tri[0]);
	vecMake(    -225,   -730+g.ey+g.ec1, 0, o->tri[1]);
	vecMake(    -375,  -580+g.ey+g.ec, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_E;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   -375,  -580+g.ey+g.ec, 0, o->tri[0]);
	vecMake(   -225,  -730+g.ey+g.ec1, 0, o->tri[1]);
	vecMake(   -225,  -580+g.ey+g.ec, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_E;
	g.nobjects++;
	
    // Letter A
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   -225,   -730+g.a1y+g.a1c1, 0, o->tri[0]);
	vecMake(    -75,   -730+g.a1y+g.a1c1, 0, o->tri[1]);
	vecMake(    -225,  -580+g.a1y+g.a1c, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_A;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   -225,  -580+g.a1y+g.a1c, 0, o->tri[0]);
	vecMake(   -75,  -730+g.a1y+g.a1c1, 0, o->tri[1]);
	vecMake(   -75,  -580+g.a1y+g.a1c, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_A;
	g.nobjects++;
	
    // Letter M
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   -75,   -730+g.my+g.mc1, 0, o->tri[0]);
	vecMake(    75,   -730+g.my+g.mc1, 0, o->tri[1]);
	vecMake(    -75,  -580+g.my+g.mc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_M;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   -75,  -580+g.my+g.mc, 0, o->tri[0]);
	vecMake(   75,  -730+g.my+g.mc1, 0, o->tri[1]);
	vecMake(   75,  -580+g.my+g.mc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_M;
	g.nobjects++;
	
    // Letter C
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   75,   -730+g.cy+g.cc1, 0, o->tri[0]);
	vecMake(    225,   -730+g.cy+g.cc1, 0, o->tri[1]);
	vecMake(    75,  -580+g.cy+ g.cc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_C;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   75,  -580+g.cy+g.cc, 0, o->tri[0]);
	vecMake(   225,  -730+g.cy+g.cc1, 0, o->tri[1]);
	vecMake(   225,  -580+g.cy+g.cc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_C;
	g.nobjects++;
	
    // Letter A
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
    vecMake(   225,   -730+g.a2y+g.a2c1, 0, o->tri[0]);
	vecMake(    375,   -730+g.a2y+g.a2c1, 0, o->tri[1]);
	vecMake(    225,  -580+g.a2y+g.a2c, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_A;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
    vecMake(   225,  -580+g.a2y+g.a2c, 0, o->tri[0]);
	vecMake(   375,  -730+g.a2y+g.a2c1, 0, o->tri[1]);
	vecMake(   375,  -580+g.a2y+g.a2c, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_A;
	g.nobjects++;
	
    // Letter S
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(   150,   0, 0, o->tri_img[2]);
	
	vecMake(   375,   -730+g.sy+g.sc1, 0, o->tri[0]);
	vecMake(    525,   -730+g.sy+g.sc1, 0, o->tri[1]);
	vecMake(    375,  -580+g.sy+g.sc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_S;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 150, 0, o->tri_img[0]);
	vecMake(   150, 150, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
	vecMake(   375,  -580+g.sy+g.sc, 0, o->tri[0]);
	vecMake(   525,  -730+g.sy+g.sc1, 0, o->tri[1]);
	vecMake(   525,  -580+g.sy+g.sc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.1,.1,.1, o->color);
	o->surface = SURF_TRIANGLE_S;
	g.nobjects++;
	
    // Letter T
	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0,   0, 0, o->tri_img[0]);
	vecMake(   200, 200, 0, o->tri_img[1]);
	vecMake(   200,   0, 0, o->tri_img[2]);
	
	vecMake(   525,   -730+g.ty+g.tc1, 0, o->tri[0]);
	vecMake(    725,   -730+g.ty+g.tc1, 0, o->tri[1]);
	vecMake(    525,  -530+g.ty+ g.tc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_T;
	g.nobjects++;

	o = &g.object[g.nobjects];
	o->type = TYPE_TRIANGLE;
	vecMake(     0, 200, 0, o->tri_img[0]);
	vecMake(   200, 200, 0, o->tri_img[1]);
	vecMake(     0,   0, 0, o->tri_img[2]);
	
	vecMake(   525,  -530+g.ty+g.tc, 0, o->tri[0]);
	vecMake(   725,  -730+g.ty+g.tc1, 0, o->tri[1]);
	vecMake(   725,  -530+g.ty+g.tc, 0, o->tri[2]);
	void getTriangleNormal(Vec tri[3], Vec norm);
	getTriangleNormal(o->tri, norm);
	vecCopy(norm, o->norm);
	vecMake(.9,.9,.9, o->color);
	o->surface = SURF_TRIANGLE_T;
	g.nobjects++;
	
    //
    g.appature = 0;
	vecMake(1.1, 1.1, 1.1, g.ambient);
	vecMake(0.9, 0.9, 0.9, g.diffuse);
	vecMake(0, 0, 5000.0, g.lightPos);
	//
	vecMake(0.0, 0.0, 1000.0, g.from);
	vecMake(0.0, 0.0, -1000.0, g.at);
	vecMake(0.0, 1.0,  0.0, g.up);
	g.angle = 90.0;
}

Flt vecDotProduct(Vec v0, Vec v1)
{
	return (v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2]);
}

void vecCrossProduct(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[1]*v1[2] - v1[1]*v0[2];
	dest[1] = v0[2]*v1[0] - v1[2]*v0[0];
	dest[2] = v0[0]*v1[1] - v1[0]*v0[1];
}

void vecZero(Vec v)
{
	v[0] = v[1] = v[2] = 0.0;
}

void vecMake(Flt a, Flt b, Flt c, Vec v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void vecCopy(Vec source, Vec dest)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}

Flt vecLength(Vec vec)
{
	return sqrt(vecDotProduct(vec, vec));
}

void vecSub(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] - v1[0];
	dest[1] = v0[1] - v1[1];
	dest[2] = v0[2] - v1[2];
}

void vecNormalize(Vec vec)
{
	//code to normalize a vector.
	Flt len = vecLength(vec);
	if (len == 0.0) {
		vecMake(1,0,0,vec);
		return;
	}
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

void getTriangleNormal(Vec tri[3], Vec norm)
{
	Vec v0,v1;
	vecSub(tri[1], tri[0], v0);
	vecSub(tri[2], tri[0], v1);
	vecCrossProduct(v0, v1, norm);
	vecNormalize(norm);
}

void checkMouse(XEvent *e)
{
	if (e->type == ButtonPress) {
		//No mouse in this program.
	}
}

void showMenu()
{
	int y = 20;
	int inc = 16;
	x11.setColor3i(255, 255, 255);
	x11.drawText(10, y, "Menu");
	y += inc;
	x11.setColor3i(255, 0, 0);
	x11.drawText(10, y, "R - Render");
	y += inc;
	x11.drawText(10, y, "A - Animate");
}

void maintain_letters() 
{
    if (g.d == 1 && g.dy < 200.0) {
        g.dy += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.dy > 200.0 && g.d < 8) {
        if (g.d == 1) {
            g.dc = 50.0;
            g.dc1 = 25.0;
        } else if (g.d == 2) {
            g.dc = 100.0;
        } else if (g.d == 3) {
            g.dc = 50.0;
        } else if (g.d == 4) {
            g.dc = -50.0;
            g.dc1 = -25.0;
        } else if (g.d == 5) {
            g.dc = -100.0;
            g.dc1 = 0.0;
        } else if (g.d == 6) {
            g.dc = -50.0;
        } else if (g.d == 7) {
            g.dc = 0.0;
        }
        g.d++;
    }

    if (g.r == 1 && g.ry < 200.0) {
        g.ry += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.ry > 200.0 && g.r < 8) {
        if (g.r == 1) {
            g.rc = 50.0;
            g.rc1 = 25.0;
        } else if (g.r == 2) {
            g.rc = 100.0;
        } else if (g.r == 3) {
            g.rc = 50.0;
        } else if (g.r == 4) {
            g.rc = -50.0;
            g.rc1 = -25.0;
        } else if (g.r == 5) {
            g.rc = -100.0;
        } else if (g.r == 6) {
            g.rc = -50.0;
            g.rc1 = 0.0;
        } else if (g.r == 7) {
            g.rc = 0.0;
        }
        g.r++;
    }

    if (g.e == 1 && g.ey < 200.0) {
        g.ey += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.ey > 200.0 && g.e < 8) {
        if (g.e == 1) {
            g.ec = 50.0;
            g.ec1 = 25.0;
        } else if (g.e == 2) {
            g.ec = 100.0;
        } else if (g.e == 3) {
            g.ec = 50.0;
        } else if (g.e == 4) {
            g.ec = -50.0;
            g.ec1 = -25.0;
        } else if (g.e == 5) {
            g.ec = -100.0;
        } else if (g.e == 6) {
            g.ec = -50.0;
            g.ec1 = 0.0;
        } else if (g.e == 7) {
            g.ec = 0.0;
        }
        g.e++;
    }   

    if (g.a1 == 1 && g.a1y < 200.0) {
        g.a1y += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.a1y > 200.0 && g.a1 < 8) {
        if (g.a1 == 1) {
            g.a1c = 50.0;
            g.a1c1 = 25.0;
        } else if (g.a1 == 2) {
            g.a1c = 100.0;
        } else if (g.a1 == 3) {
            g.a1c = 50.0;
        } else if (g.a1 == 4) {
            g.a1c = -50.0;
            g.a1c1 = -25.0;
        } else if (g.a1 == 5) {
            g.a1c = -100.0;
        } else if (g.a1 == 6) {
            g.a1c = -50.0;
            g.a1c1 = 0.0;
        } else if (g.a1 == 7) {
            g.a1c = 0.0;
        }
        g.a1++;
    } 

    if (g.m == 1 && g.my < 200.0) {
        g.my += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.my > 200.0 && g.m < 8) {
        if (g.m == 1) {
            g.mc = 50.0;
            g.mc1 = 25.0;
        } else if (g.m == 2) {
            g.mc = 100.0;
        } else if (g.m == 3) {
            g.mc = 50.0;
        } else if (g.m == 4) {
            g.mc = -50.0;
            g.mc1 = -25.0;
        } else if (g.m == 5) {
            g.mc = -100.0;
        } else if (g.m == 6) {
            g.mc = 50.0;
            g.mc1 = 0.0;
        } else if (g.m == 7) {
            g.mc = 0.0;
        }
        g.m++;
    }

    if (g.c == 1 && g.cy < 200.0) {
        g.cy += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.cy > 200.0 && g.c < 8) {
        if (g.c == 1) {
            g.cc = 50.0;
            g.cc1 = 25.0;
        } else if (g.c == 2) {
            g.cc = 100.0;
        } else if (g.c == 3) {
            g.cc = 50.0;
        } else if (g.c == 4) {
            g.cc = -50.0;
            g.cc1 = -25.0;
        } else if (g.c == 5) {
            g.cc = -100.0;
        } else if (g.c == 6) {
            g.cc = -50.0;
            g.cc1 = 0.0;
        } else if (g.c == 7) {
            g.cc = 0.0;
        }
        g.c++;
    }

    if (g.a2 == 1 && g.a2y < 200.0) {
        g.a2y += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.a2y > 200.0 && g.a2 < 8) {
        if (g.a2 == 1) {
            g.a2c = 50.0;
            g.a2c1 = 25.0;
        } else if (g.a2 == 2) {
            g.a2c = 100.0;
        } else if (g.a2 == 3) {
            g.a2c = 50.0;
        } else if (g.a2 == 4) {
            g.a2c = -50.0;
            g.a2c1 = -25.0;
        } else if (g.a2 == 5) {
            g.a2c = -100.0;
        } else if (g.a2 == 6) {
            g.a2c = -50.0;
            g.a2c1 = 0.0;
        } else if (g.a2 == 7) {
            g.a2c = 0.0;
        }
        g.a2++;
    }

    if (g.s == 1 && g.sy < 200.0) {
        g.sy += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.sy > 200.0 && g.s < 8) {
        if (g.s == 1) {
            g.sc = 50.0;
            g.sc = 25.0;
        } else if (g.s == 2) {
            g.sc = 100.0;
        } else if (g.s == 3) {
            g.sc = 50.0;
        } else if (g.s == 4) {
            g.sc = -50.0;
            g.sc = -25.0;
        } else if (g.s == 5) {
            g.sc = -100.0;
        } else if (g.s == 6) {
            g.sc = -50.0;
            g.sc = 0.0;
        } else if (g.s == 7) {
            g.sc = 0.0;
        }
        g.s++;
    }

    if (g.t == 1 && g.ty < 200.0) {
        g.ty += g.anti_gravity;
        g.anti_gravity += (g.acceleration * g.acceleration);
    } else if (g.ty > 200.0 && g.t < 8) {
        if (g.t == 1) {
            g.tc = 50.0;
            g.tc1 = 25.0;
        } else if (g.t == 2) {
            g.tc = 100.0;
        } else if (g.t == 3) {
            g.tc = 50.0;
        } else if (g.t == 4) {
            g.tc = -50.0;
            g.tc1 = -25.0;
        } else if (g.t == 5) {
            g.tc = -100.0;
        } else if (g.t == 6) {
            g.tc = -50.0;
            g.tc1 = 0.0;
        } else if (g.t == 7) {
            g.tc = 0.0;
        }
        g.t++;
    }    
}

void swirl()
{
    int frame = 0;
    int finish = 0;
    while (finish == 0) {
        init();
        render(ORTHO);
        takeScreenshot("", 0);
        frame++;
        maintain_letters();
        if (g.by < -100) {
            g.by += 100;
            g.bx += g.xforce; 
        }
    }
}

void physics()
{
    int frame = 0;
    int break_physics = 0;
    // g.gravity is the velocity of the ball
    // g.gravity simulates gravity for a ball dropping
    while (break_physics == 0) { 
        init();
	    render(ORTHO);
        frame++;
        takeScreenshot("", 0);
        g.by -= g.gravity;
        //cout << g.by << endl;
        g.gravity += (g.acceleration * g.acceleration);
        //cout << g.gravity << endl;
        if (g.xforce < 0.0) {
            g.xforce = 0.0;
        } else {
            g.bx += g.xforce; 
            g.xforce /= 1.04;
        }
        if (g.by == g.floor) {
            g.gravity = g.yforce;
            g.xforce = 19.0;
            g.letters++;
            //cout << g.letters << endl;
            if (g.letters == 1) {
                g.d = 1;
            } else if (g.letters == 2) {
                g.anti_gravity = 0;
                g.r = 1;
            } else if (g.letters == 3) {
                g.anti_gravity = 0;
                g.e = 1;
            } else if (g.letters == 4) {
                g.anti_gravity = 0;
                g.a1 = 1;
            } else if (g.letters == 5) {
                g.anti_gravity = 0;
                g.m = 1;
            } else if (g.letters == 6) {
                g.anti_gravity = 0;
                g.c = 1;
            } else if (g.letters == 7) {
                g.anti_gravity = 0;
                g.a2 = 1;
            } else if (g.letters == 8) {
                g.anti_gravity = 0;
                g.s = 1;
            } else if (g.letters == 9) {
                g.anti_gravity = 0;
                g.t = 1;
            }
            //cout << g.gravity << endl;
        }
        maintain_letters();
        if (g.bx > g.wall) {
            g.gravity = 0;
            g.xforce = 0;
            break_physics = 1;
            // start the swirly animation
            // call break physics
            // begin art project
        }
    }
    swirl();
}

int checkKeys(XEvent *e)
{
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_a) {
            physics();
			g.bx = 0.0;
			g.by = 0.0;
			return 0;
		}
		//----------------------------------------
		//----------------------------------------
		if (key == XK_r) {
			init();
			render(ORTHO);
			return 0;
		}
		//if (key == XK_p) {
		//	init();
		//	render(PERSPECTIVE);
		//	return 0;
		//}
		if (key == XK_m) {
			showMenu();
			return 0;
		}
		if (key == XK_Escape) {
			if (g.mode) {
				g.mode = 0;
				x11.clearScreen();
				return 0;
			}
			return 1;
		}
	}
	return 0; 
}

Flt getLength(Vec p1, Vec p2)
{
	Flt d0 = p2[0] - p1[0];
	Flt d1 = p2[1] - p1[1];
	Flt d2 = p2[2] - p1[2];
	Flt len = sqrt(d0*d0 + d1*d1 + d2*d2);
	return len;	
}

Flt getArea(Vec v0, Vec v1, Vec v2)
{
	Flt a = getLength(v1, v0);
	Flt b = getLength(v2, v1);
	Flt c = getLength(v0, v2);
	Flt s = (a+b+c) / 2.0;
	return (sqrt(s * (s-a) * (s-b) * (s-c)));
}

bool getBarycentric(Vec tri[3], Hit *hit, Flt *u, Flt *v)
{
	Flt area = getArea(tri[0], tri[1], tri[2]);
	if (area == 0.0) {
		*u = *v = 0.0;
		return false;
	}
	Vec pt;
	vecCopy(hit->p, pt);
	Flt area1 = getArea(tri[0], tri[1], pt);
	Flt area2 = getArea(tri[2], tri[0], pt);
	*u = area1 / area;
	*v = area2 / area;
	return (*u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0);
}

bool pointInTriangle(Vec tri[3], Vec p, Flt *u, Flt *v)
{
	//source: http://blogs.msdn.com/b/rezanour/archive/2011/08/07/
	//
	//This function determines if point p is inside triangle tri.
	//   step 1: 3D half-space tests
	//   step 2: find barycentric coordinates
	//
	Vec cross0, cross1, cross2;
	Vec ba, ca, pa;
	//setup sub-triangles
	vecSub(tri[1], tri[0], ba);
	vecSub(tri[2], tri[0], ca);
	vecSub(p, tri[0], pa);
	//This is a half-space test
	vecCrossProduct(ca, pa, cross1);
	vecCrossProduct(ca, ba, cross0);
	if (vecDotProduct(cross0, cross1) < 0.0)
		return 0;
	//This is a half-space test
	vecCrossProduct(ba,pa,cross2);
	vecCrossProduct(ba,ca,cross0);
	if (vecDotProduct(cross0, cross2) < 0.0)
		return 0;
	//Point is within 2 half-spaces
	//Get area proportions
	//Area is actually length/2, but we just care about the relationship.
	Flt areaABC = vecLength(cross0);
	Flt areaV = vecLength(cross1);
	Flt areaU = vecLength(cross2);
	*u = areaU / areaABC;
	*v = areaV / areaABC;
	//return true if valid barycentric coordinates.
	return (*u >= 0.0 && *v >= 0.0 && *u + *v <= 1.0);
}

int rayPlaneIntersect(Vec center, Vec normal, Ray *ray, Hit *hit)
{
	//http://mathworld.wolfram.com/Plane.html
	//
	//Where does the ray intersect the plane?
	//
	//plane equation:
	//
	// (p - center) . normal = 0.0
	//
	//where:
	//center = known point on plane
	//normal = normal vector of plane
	//
	//ray equation:
	//
	// O + tD
	//
	//where:
	//   ray origin = O
	//   ray direction = D
	//
	//Substitute ray equation into plane equation,
	//and solve for t
	//
	// (O + t * D - center) . normal = 0.0               (substitution)
	// 
	// (t * D + O - center) . normal = 0.0               (commutative)
	// 
	// t * (D . normal) + (O - center) . normal = 0.0    (distributive)
	// 
	// t * (D . normal) = - (O - center) . normal        (subtraction)
	// 
	// t = - ((O - center) . normal) / (D . normal)      (division)
	// 
	// notes: period is dot product
	//        solve for t
	//        populate hit->t and hit->p[] below...
	//        remember the unary minus sign above
	Vec v0;
	v0[0] = ray->o[0] - center[0];
	v0[1] = ray->o[1] - center[1];
	v0[2] = ray->o[2] - center[2];
	Flt dot1 = vecDotProduct(v0, normal);
	if (dot1 == 0.0)
		return 0;
	Flt dot2 = vecDotProduct(ray->d, normal);
	if (dot2 == 0.0)
		return 0;
	hit->t = -dot1 / dot2;
	if (hit->t < 0.0)
		return 0;
	hit->p[0] = ray->o[0] + hit->t * ray->d[0];
	hit->p[1] = ray->o[1] + hit->t * ray->d[1];
	hit->p[2] = ray->o[2] + hit->t * ray->d[2];
	return 1;
}

int rayDiskIntersect(Object *o, Ray *ray, Hit *hit)
{
	//Does the ray intersect the disk's plane?
	if (rayPlaneIntersect(o->center, o->norm, ray, hit)) {
		//Yes.

		//printf("hit ");
		//Check that the hit point is within the disk radius
		//...
		//--------------------------------------------------------------
		//This is temporary code so you will see disks on the screen.
		//This must be changed for the assignment.
		//Use the point hit instead of the ray origin
        // get the focal length
        // focus on the letter to blur out the ball
        //

		Flt d0, d1, d2, dist;

		d0 = (o->center[0] - hit->p[0]);
		d1 = (o->center[1] - hit->p[1]);
		d2 = (o->center[2] - hit->p[2]);
        dist = sqrt(d0*d0 + d1*d1 + d2*d2);
		if (dist <= o->radius) {
	         //Hit is within radius
			 return 1;
		}
	}
	return 0;
}

int rayRingIntersect(Object *o, Ray *ray, Hit *hit)
{
	if (rayPlaneIntersect(o->center, o->norm, ray, hit)) {
		Flt d0 = o->center[0] - hit->p[0];
		Flt d1 = o->center[1] - hit->p[1];
		Flt d2 = o->center[2] - hit->p[2];
		Flt dist = sqrt(d0*d0 + d1*d1 + d2*d2);
		if (dist >= o->radius && dist <= o->radius2) {
			return 1;
		}
	}
	return 0;
}

int rayTriangleIntersect(Object *o, Ray *ray, Hit *hit)
{
	if (rayPlaneIntersect(o->tri[0], o->norm, ray, hit)) {
		Flt u,v;
		if (pointInTriangle(o->tri, hit->p, &u, &v)) {
			Flt w = 1.0 - u - v;
			if (o->surface == SURF_BARY) {
				o->color[0] = u;
				o->color[1] = v;
				o->color[2] = w;
			}
			return 1;
		}
	}
	return 0;
}

void sphereNormal(Vec hitPoint, Vec center, Vec norm)
{
	//Calculate normal at hit point of sphere
	norm[0] = hitPoint[0] - center[0];
	norm[1] = hitPoint[1] - center[1];
	norm[2] = hitPoint[2] - center[2];
	vecNormalize(norm);
}

int raySphereIntersect(Object *o, Ray *ray, Hit *hit)
{
	//Log("raySphereIntersect()...\n");
	//Determine if and where a ray intersects a sphere.
	//
	// sphere equation:
	// (p - c) * (p - c) = r * r
	//
	// where:
	// p = point on sphere surface
	// c = center of sphere
	//
	// ray equation:
	// o + t*d
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (o + t*d - c) * (o + t*d - c) - r * r = 0
	//
	// we want it in this form:
	// a*t*t + b*t + c = 0
	//
	// (o + d*t - c)
	// (o + d*t - c)
	// -------------
	// o*o + o*d*t - o*c + o*d*t + d*t*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + o*o + o*d*t - o*c + o*d*t - d*t*c - o*c + c*d*t + c*c
	// d*t*d*t + 2(o*d*t) - 2(c*d*t) + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + o*o - o*c - o*c + c*c
	// d*t*d*t + 2(o-c)*d*t + (o-c)*(o-c)
	//
	// t*t*d*d + t*2*(o-c)*d + (o-c)*(o-c) - r*r
	//
	// a = dx*dx + dy*dy + dz*dz
	// b = 2(ox-cx)*dx + 2(oy-cy)*dy + 2(oz-cz)*dz
	// c = (ox-cx)*(ox-cx) + (oy-cy)*(oy-cy) + (oz-cz)*(oz-cz) - r*r
	//
	// now put it in quadratic form:
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//
	//1. a, b, and c are given to you just above.
	//2. Create variables named a,b,c, and assign the values you see above.
	//3. Look how a,b,c are used in the quadratic equation.
	//4. Make your code solve for t.
	//5. Remember, a quadratic can have 0, 1, or 2 solutions.
	//
	//Your code goes here...
	//
	//I'll start you out with a and b
	//You try to do c
	//
	Flt a = ray->d[0]*ray->d[0] + ray->d[1]*ray->d[1] + ray->d[2]*ray->d[2];
	Flt b = 2.0*(ray->o[0]-o->center[0])*ray->d[0] +
		2.0*(ray->o[1]-o->center[1])*ray->d[1] +
		2.0*(ray->o[2]-o->center[2])*ray->d[2];
	Flt c = (ray->o[0]-o->center[0])*(ray->o[0]-o->center[0]) +
		(ray->o[1]-o->center[1])*(ray->o[1]-o->center[1]) +
		(ray->o[2]-o->center[2])*(ray->o[2]-o->center[2]) -
		o->radius*o->radius;
	Flt t0,t1;
	//discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	//
	if (t0 > 0.0) {
		hit->p[0] = ray->o[0] + ray->d[0] * t0;
		hit->p[1] = ray->o[1] + ray->d[1] * t0;
		hit->p[2] = ray->o[2] + ray->d[2] * t0;
		sphereNormal(hit->p, o->center, hit->norm);
		hit->t = t0;
		return 1;
	}
	if (t1 > 0.0) {
		hit->p[0] = ray->o[0] + ray->d[0] * t1;
		hit->p[1] = ray->o[1] + ray->d[1] * t1;
		hit->p[2] = ray->o[2] + ray->d[2] * t1;
		sphereNormal(hit->p, o->center, hit->norm);
		hit->t = t1;
		return 1;
	}
	return 0;
}

void cylinderNormal(Vec p, Vec c, Vec norm) {
	//Center of cylinder is at the origin
	vecMake(p[0], 0.0, p[2], norm);
	vecNormalize(norm);
}

int rayCylinderIntersect(Object *o, Ray *ray, Hit *hit)
{
	//http://voices.yahoo.com/
	//developing-equation-cone-simplest-case-2522846.html?cat=17
	// x2 + z2 = a*y2
	//----------------------------------
	//for cylinder centered at origin...
	//----------------------------------
	//
	// cylinder equation
	// x2 + z2 = r2
	//
	// where:
	//   x = x component of point on cylinder surface
	//   z = z component of point on cylinder surface
	//   r = radius
	//
	// ray equation:
	// o + t*d
	//
	// where:
	//   o = ray origin
	//   d = ray direction
	//   t = distance along ray, or scalar
	//
	// substitute ray equation into sphere equation
	//
	// (ox+t*dx)2 + (oz+t*dz)2 - r*r = 0
	//
	// where:
	//   ox = x component of ray origin
	//   oz = z component of ray origin
	//   dx = x component of ray direction
	//   dz = z component of ray direction
	//
	// ox + t*dx
	// ox + t*dx
	// --------------------------------
	// ox*ox + 2(ox * t*dx) + t*t*dx*dx
	//
	// add in the z components...
	//
	//ox*ox + 2(ox * t*dx) + t*t*dx*dx + oz*oz + 2(oz * t*dz) + t*t*dz*dz - r*r
	//
	//Goal is to solve for t using the quadratic equation...
	// t = (-b +/- sqrt(b*b - 4ac)) / 2a
	//
	//t*t*dx*dx + t*t*dz*dz + ox*ox + 2(ox * t*dx) + oz*oz + 2(oz * t*dz) - r*r
	//t*t*dx*dx + t*t*dz*dz + 2(t*ox*dx) + 2(t*oz*dz) + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + 2t*ox*dx + 2t*oz*dz + ox*ox + oz*oz - r*r
	//t*t*dx*dx + t*t*dz*dz + t*2*ox*dx + t*2*oz*dz + ox*ox + oz*oz - r*r
	//a = dx*dx + dz*dz
	//b = 2*ox*dx + 2*oz*dz
	//c = ox*ox + oz*oz - r*r
	//
	Ray r;
	vecCopy(ray->o, r.o);
	vecCopy(ray->d, r.d);
	//transformVec(o->invmat, r.o);
	//transformVec(o->invmat, r.d);
	//translateVec(o->translate, r.o);

	//now put a,b,c into C source code...
	Flt a = r.d[0] * r.d[0] + r.d[2] * r.d[2];
	Flt b = 2.0 * r.o[0] * r.d[0] + 2.0 * r.o[2] * r.d[2];
	Flt c = r.o[0]*r.o[0] + r.o[2]*r.o[2] - o->radius * o->radius;
	//
	Flt t0,t1;
	//disc:  discriminant
	Flt disc = b * b - 4.0 * a * c;
	if (disc < 0.0) return 0;
	disc = sqrt(disc);
	t0 = (-b - disc) / (2.0*a);
	t1 = (-b + disc) / (2.0*a);
	if (t0 >= 0.0) {
		//possible hit
		hit->p[0] = r.o[0] + r.d[0] * t0;
		hit->p[1] = r.o[1] + r.d[1] * t0;
		hit->p[2] = r.o[2] + r.d[2] * t0;
		if (hit->p[1] >= 0.0 && hit->p[1] <= o->apex) {
			hit->t = t0;
			return 1;
		}
	}
	if (t1 >= 0.0) {
		hit->p[0] = r.o[0] + r.d[0] * t1;
		hit->p[1] = r.o[1] + r.d[1] * t1;
		hit->p[2] = r.o[2] + r.d[2] * t1;
		if (hit->p[1] >= 0.0 && hit->p[1] <= o->apex) {
			hit->t = t1;
			return 1;
		}
	}
	return 0;
}

inline void nudge_ray_forward(Ray *r)
{
	r->o[0] += r->d[0] * 0.0001;
	r->o[1] += r->d[1] * 0.0001;
	r->o[2] += r->d[2] * 0.0001;
}

bool getShadow(Vec pt)
{
	//Trace a ray through the scene.
	Vec lightDir;
	vecSub(g.lightPos, pt, lightDir);
	Flt distanceToLight = vecLength(lightDir);
	vecNormalize(lightDir);
	Ray ray;
	vecCopy(pt,       ray.o);
	vecCopy(lightDir, ray.d);
	nudge_ray_forward(&ray);
	Hit hit;
	Object *o;
	for (int i=0; i<g.nobjects; i++) {
		o = &g.object[i];
		switch (o->type) {
			case TYPE_DISK:
				if (rayDiskIntersect(o, &ray, &hit)) {
					if (hit.t < distanceToLight)
						return true;
				}
				break;
			case TYPE_RING:
				if (rayRingIntersect(o, &ray, &hit)) {
					if (hit.t < distanceToLight)
						return true;
				}
				break;
			case TYPE_TRIANGLE:
				if (rayTriangleIntersect(o, &ray, &hit)) {
					if (hit.t < distanceToLight)
						return true;
				}
				break;
			case TYPE_SPHERE:
				if (raySphereIntersect(o, &ray, &hit)) {
					if (hit.t < distanceToLight)
						return true;
				}
				break;
		}
	}
	return false;
}

void reflect(Vec I, Vec N, Vec R)
{
	//I = incident vector
	//N = the surface normal
	//R = reflected ray
	Flt dot = -vecDotProduct(I, N);
	Flt len = 2.0 * dot;
	R[0] = len * N[0] + I[0];
	R[1] = len * N[1] + I[1];
	R[2] = len * N[2] + I[2];
}

void trace(Ray *ray, Vec rgb, Flt weight, int level)
{
	//Trace a ray through the scene.
	if (level > 5) {
		return;
	}
	if (weight < 0.1) {
		return;
	}
	int i;
	Hit hit, closehit;
	Object *o;
	int h = -1;
	closehit.t = 9e9;
	for (i=0; i<g.nobjects; i++) {
		o = &g.object[i];
		switch (o->type) {
			case TYPE_DISK:
				if (rayDiskIntersect(o, ray, &hit)) {
					if (hit.t < closehit.t) {
						closehit.t = hit.t;
						vecCopy(hit.p, closehit.p);
						vecCopy(o->color, closehit.color);
						vecCopy(o->norm, closehit.norm);
						h=i;
					}
				}
				break;
			case TYPE_RING:
				if (rayRingIntersect(o, ray, &hit)) {
					if (hit.t < closehit.t) {
						closehit.t = hit.t;
						vecCopy(hit.p, closehit.p);
						vecCopy(o->color, closehit.color);
						vecCopy(o->norm, closehit.norm);
						h=i;
					}
				}
				break;
			case TYPE_TRIANGLE:
				if (rayTriangleIntersect(o, ray, &hit)) {
					if (hit.t < closehit.t) {
						closehit.t = hit.t;
						vecCopy(hit.p, closehit.p);
						vecCopy(o->color, closehit.color);
						vecCopy(o->norm, closehit.norm);
						h=i;
					}
				}
				break;
			case TYPE_SPHERE:
				if (raySphereIntersect(o, ray, &hit)) {
					if (hit.t < closehit.t) {
						closehit.t = hit.t;
						vecCopy(hit.p, closehit.p);
						vecCopy(o->color, closehit.color);
						sphereNormal(closehit.p, o->center, closehit.norm);
						h=i;
					}
				}
				break;
			case TYPE_CYLINDER:
				if (rayCylinderIntersect(o, ray, &hit)) {
					if (hit.t < closehit.t) {
						closehit.t = hit.t;
						vecCopy(hit.p, closehit.p);
						vecCopy(o->color, closehit.color);
						cylinderNormal(closehit.p, o->center, closehit.norm);
						h=i;
					}
				}
		}
	}
	if (h < 0) {
		//ray did not hit an object, set the color to white.
        if (g.background) {
	        rgb[0] = 1.0;
	        rgb[1] = 1.0;
		    rgb[2] = 1.0;
        }
		return;
	}
	//Set the color of the pixel to the color of the object.
	o = &g.object[h];
	if (o->surface == SURF_CHECKER) {
		int x = 10000.0 + (closehit.p[0]/2) / 37.0;
		int z = 10000.0 + (closehit.p[2]/2) / 37.0;
		x = x % 2;
		z = z % 2;
		if (x==z) {
			closehit.color[0] = 0.9;
			closehit.color[1] = 0.1;
			closehit.color[2] = 0.1;
		} else {
			closehit.color[0] = 0.9;
			closehit.color[1] = 0.9;
			closehit.color[2] = 0.9;
		}
	}
	if (o->surface == SURF_TRIANGLE) {
		Image *im = &img[0]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 200 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_R) {
		Image *im = &img[1]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_E) {
		Image *im = &img[2]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_A) {
		Image *im = &img[3]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_M) {
		Image *im = &img[4]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_C) {
		Image *im = &img[5]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_S) {
		Image *im = &img[6]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 150 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
	if (o->surface == SURF_TRIANGLE_T) {
		Image *im = &img[7]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 200 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } 
        }
    }
    if (o->surface == SURF_TRIANGLE_BALL) {
		Image *im = &img[8]; 
        Flt u, v, w;
        if (getBarycentric(o->tri, &hit, &u, &v)) {
            w = 1.0 - u - v;
            //multiply 3 x's by u,v,w
            Flt x1 = o->tri_img[0][0] * u;
            Flt x2 = o->tri_img[1][0] * v;
            Flt x3 = o->tri_img[2][0] * w;
            //multiply 3 y's by u,v,w
            Flt y1 = o->tri_img[0][1] * u;
            Flt y2 = o->tri_img[1][1] * v;
            Flt y3 = o->tri_img[2][1] * w;
            
            Flt xtotal = x1+x2+x3;
            Flt ytotal = y1+y2+y3;
            
            int x = xtotal;
            int y = ytotal;

		    int offset =  x * 60 * 3 + y * 3;
		    unsigned char *p = im->data + offset;
		    closehit.color[0] = (Flt)*(p+0) / 255.0;
            //cout << closehit.color[0] << "   ";
		    closehit.color[1] = (Flt)*(p+1) / 255.0;
            //cout << closehit.color[1] << "   ";
		    closehit.color[2] = (Flt)*(p+2) / 255.0;
            //cout << closehit.color[2] << endl;
            /*if (closehit.color[2] >= .9 && closehit.color[1] >= .9
                    && closehit.color[0] >= .9) {
		        closehit.color[0] = 255;
		        closehit.color[1] = 255;
		        closehit.color[2] = 255;      
            } */
        }
    }

	if (o->surface == SURF_DISK) {
        // get focal length
        // put each pixel in a for loop

    }
	if (o->surface == SURF_SPHERICAL) {
		Image *im = &img[0];
		Flt tx,ty;
		Flt angle1 = atan2(closehit.norm[2], closehit.norm[0]);
		angle1 = PI - angle1;
		int w = im->width;
		int h = im->height;
		tx = ((Flt)w * (angle1 / (PI*2.0)));
		Flt angle2 = asin(closehit.norm[1]);
		Flt m2 = (Flt)(h/2);
		if (closehit.norm[1] < 0.0) {
			ty = m2 + (m2 * (-angle2 / (PI/2.0)));
		} else {
			ty = m2 - (m2 * (angle2 / (PI/2.0)));
		}
		int j = (int)floor(tx + 0.0001);
		int i = (int)floor(ty + 0.0001);
		int offset = i * w * 3 + j * 3;
		unsigned char *p = im->data + offset;
		//
		closehit.color[0] = (Flt)*(p+0) / 255.0;
		closehit.color[1] = (Flt)*(p+1) / 255.0;
		closehit.color[2] = (Flt)*(p+2) / 255.0;
        if (closehit.color[2] > closehit.color[1]) {
            o->specular = true;
        } else {
            o->specular = false;
        }
	}
	if (o->surface == SURF_CYLINDRICAL) {
		Image *im = &img[1];
		Flt tx,ty;
		Flt angle1 = atan2(closehit.norm[2], closehit.norm[0]);
		angle1 = PI - angle1;
		int w = im->width;
		int h = im->height;
		tx = ((Flt)w * (angle1 / (PI*2.0)));
		Flt angle2 = asin(closehit.norm[1]);
		Flt m2 = (Flt)(h/2);
		if (closehit.norm[1] < 0.0) {
			ty = m2 + (m2 * (-angle2 / (PI/2.0)));
		} else {
			ty = m2 - (m2 * (angle2 / (PI/2.0)));
		}
	    ty = h * (1- (closehit.p[1]/500));
		int j = (int)floor(tx + 0.0001);
		int i = (int)floor(ty + 0.0001);
		int offset = i * w * 3 + j * 3;
		unsigned char *p = im->data + offset;
		//
		closehit.color[0] = (Flt)*(p+0) / 255.0;
		closehit.color[1] = (Flt)*(p+1) / 255.0;
		closehit.color[2] = (Flt)*(p+2) / 255.0;
	}
	if (o->specular) {
		//Build a reflected ray
		Vec rcol = {0,0,0};
		Ray tray;
		#define DISPERSION
#ifdef DISPERSION
		Vec tcol = {0,0,0};
		int n = 0;
		for (int i=0; i<10; i++) {
			reflect(ray->d, closehit.norm, tray.d);
			vecCopy(closehit.p, tray.o);
			nudge_ray_forward(&tray);
			tray.d[0] += rnd() * .1 - .05;
			tray.d[1] += rnd() * .1 - .05;
			tray.d[2] += rnd() * .1 - .05;
			vecZero(rcol);
			trace(&tray, rcol, weight*0.5, level+1);
			tcol[0] += rcol[0];
			tcol[1] += rcol[1];
			tcol[2] += rcol[2];
			++n;
		}
		Flt fn = 1.0 / (Flt)n;
		rcol[0] = tcol[0] * fn;
		rcol[1] = tcol[1] * fn;
		rcol[2] = tcol[2] * fn;
#else //DISPERSION
		reflect(ray->d, closehit.norm, tray.d);
		vecCopy(closehit.p, tray.o);
		nudge_ray_forward(&tray);
		vecZero(rcol);
		trace(&tray, rcol, weight*0.5, level+1);
#endif //DISPERSION
		//
		rgb[0] += rcol[0] * o->spec[0] * weight;
		rgb[1] += rcol[1] * o->spec[1] * weight;
		rgb[2] += rcol[2] * o->spec[2] * weight;
		//
		//Look for specular highlight...
		//http://en.wikipedia.org/wiki/Specular_highlight
		//Blinn Phong lighting model
		Vec lightDir, halfway;
		lightDir[0] = g.lightPos[0] - closehit.p[0];
		lightDir[1] = g.lightPos[1] - closehit.p[1];
		lightDir[2] = g.lightPos[2] - closehit.p[2];
		halfway[0] = (lightDir[0] - ray->d[0]) * 0.5;
		halfway[1] = (lightDir[1] - ray->d[1]) * 0.5;
		halfway[2] = (lightDir[2] - ray->d[2]) * 0.5;
		vecNormalize(halfway);
		Flt dot = vecDotProduct(halfway, closehit.norm);
		if (dot > 0.0) {
			dot = pow(dot, 320.0);
			rgb[0] += 1.0 * dot;
			rgb[1] += 1.0 * dot;
			rgb[2] += 1.0 * dot;
		}
	}
	//
	bool inShad = false;
	inShad = getShadow(closehit.p);
	Flt dot = 0.0;
	if (!inShad) {
		//get light contribution. shoot a vector to the light.
		Vec v;
		vecSub(g.lightPos, closehit.p, v);
		vecNormalize(v);
		vecNormalize(closehit.norm);
		dot = vecDotProduct(v, closehit.norm);
		if (dot < 0.0)
			dot = 0.0;
	}
	//The ray hit an object.
	for (int i=0; i<3; i++) {
		rgb[i] += closehit.color[i] * dot * g.diffuse[i];
		rgb[i] += closehit.color[i] * g.ambient[i];
	}
	return;
}

void setupRay(Vec eye, Vec pixel, Ray *ray)
{
	//Make a ray from eye through a screen pixel
	vecCopy(eye, ray->o);
	vecSub(pixel, eye, ray->d);
	vecNormalize(ray->d);
}

void render(int projection)
{
    //show_sphere();
    //show_lines();
	void castRaysFromCamera();
	castRaysFromCamera();
	return;
	//
	//below is old render code. Not used.
	//Camera was not adjustable.
	//
	//Starting position of pixels is bottom-left corner of screen.
	//Pixel at origin (0,0) is in the middle of the screen.
	/*
	   Flt xStart = -g.xres / 2.0;
	   Flt yStart = -g.yres / 2.0;
	   Flt xStep = 1.0;
	   Flt yStep = 1.0;
	   Vec eye, pixel, rgb;
	   Ray ray;
	//Assume orthographic projection.
	//Set eye in front of each pixel.
	//Make a ray straight through each screen pixel.
	pixel[1] = yStart;
	for (int i=g.yres-1; i>=0; i--) {
	pixel[0] = xStart;
	for (int j=0; j<g.xres; j++) {
	vecCopy(pixel, eye);
	if (projection == PERSPECTIVE) {
	//Place code here for perspective projection
	eye[0] = 0.0;
	eye[1] = 100.0;
	}
	//Move the eye back from the screen.
	eye[2] = 1000.0;
	//Cast the ray through the pixel.
	setupRay(eye, pixel, &ray);
	vecZero(rgb);
	trace(&ray, rgb, 1.0, 1);
	x11.drawPixel(j, i, rgb);
	pixel[0] += xStep;
	}
	pixel[1] += yStep;
	}
	*/
}

#define vecComb(A,a,B,b,c) (c)[0] = (A) * (a)[0] + (B) * (b)[0]; \
									(c)[1] = (A) * (a)[1] + (B) * (b)[1]; \
									(c)[2] = (A) * (a)[2] + (B) * (b)[2]

void vecAdd(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] + v1[0];
	dest[1] = v0[1] + v1[1];
	dest[2] = v0[2] + v1[2];
}

void vecRand(Vec v0)
{
    int x = rand() % 10 + 1;
    int y = rand() % 10 + 1;
    int z = rand() % 10 + 1;
	
    v0[0] += x;
	v0[1] += y;
    v0[2] += z;
}


Flt degreesToRadians(Flt angle) {
	return (angle / 360.0) * (3.141592653589793 * 2.0);
}

void castRaysFromCamera()
{
	//This function casts rays through every pixel.
	//The camera may be set anywhere looking anywhere.
	Flt fyres = (Flt)g.yres;
	Flt fxres = (Flt)g.xres;
	Flt ty = 1.0 / (fyres - 1.0);
	Flt tx = 1.0 / (fxres - 1.0);
	int px = 1;
	int py = 1;
	//
	Vec from = {0.0, 0.0, 1000.0};
	Vec at = {0.0, 0.0, -1000.0};
	Vec up = {0.0, 1.0, 0.0};
	Flt angle = 15.0;
	vecCopy(g.from, from);
	vecCopy(g.at, at);
	vecCopy(g.up, up);
	angle = g.angle;
	vecNormalize(up);
	//
	Flt viewAnglex, aspectRatio;
	Flt frustumheight, frustumwidth;
	Vec rgb, eye, dir, left, out;
	vecSub(at, from, out);
	vecNormalize(out);
	aspectRatio = fxres / fyres;
	//-------------------------------------------------------------------------
	viewAnglex = degreesToRadians(angle * 0.5);
	frustumwidth = tan(viewAnglex);
	frustumheight = frustumwidth / aspectRatio;
	//frustumwidth is actually half the distance across screen
	//compute the left and up vectors...
	vecCrossProduct(out, up, left);
	//printf("left: %lf %lf %lf\n",left[0],left[1],left[2]);
	vecNormalize(left);
	vecCrossProduct(left, out, up);
	//printf("up: %lf %lf %lf\n",up[0],up[1],up[2]);
	//
	Ray ray;
	vecCopy(from, eye);
	//Trace every pixel...
	//
	for (int i=g.yres-1; i>=0; i--) {
		py = i;
		for (int j=0; j<g.xres; j++) {
			px = j;
			//Start the color at black
			//Start the ray origin at the eye
			vecZero(rgb);
			vecCopy(eye, ray.o);
			//
			//Activate one of the following...
			//#define EXPANDED_CALCULATION
			//#define COMPRESSED_CALCULATION
#define OPTIMIZED_CALCULATION
			//
			//=========================
#ifdef EXPANDED_CALCULATION
			//Build a vector from screen center to the current pixel
			//
			//Calculate distance across g.screen.
			//Subtract 1 because we are starting in the middle
			//of the first pixel, and ending in the middle of the
			//the last pixel. Our distance across the screen is
			//one less than the distance across all pixels.
			//This seems to work well.
			Flt ty = fyres - 1.0;
			Flt tx = fxres - 1.0;
			//Proportion of screen width & height
			//This is half the screen width & height
			Flt xprop = (Flt)px/tx;
			Flt yprop = (Flt)py/ty;
			//Position within vector
			//Multiply by 2 and subtract 1,
			//so that we move from negative to positive.
			Flt xpos = 2.0 * xprop - 1.0;
			Flt ypos = 2.0 * yprop - 1.0;
			//Position in frustum
			Flt xfrust =  frustumwidth  * xpos;
			Flt yfrust = -frustumheight * ypos;
			//Multiply by left and up vectors
			Vec h, v;
			h[0] = xfrust * left[0];
			h[1] = xfrust * left[1];
			h[2] = xfrust * left[2];
			v[0] = yfrust * up[0];
			v[1] = yfrust * up[1];
			v[2] = yfrust * up[2];
			//Add the vectors together to get a direction vector.
			//This is the direction from the center of the screen
			//to the pixel that we want to color.
			vecAdd(v, h, dir);
			//The magnitude of the dir vector will lead us directly
			//to the correct pixel.
#endif //EXPANDED_CALCULATION
			//===========================
			//===========================
#ifdef COMPRESSED_CALCULATION
			vecComb(-frustumheight * (2.0 *
						(Flt)py/(fyres-1.0) - 1.0), up,
					frustumwidth  * (2.0 *
						(Flt)px/(fxres-1.0) - 1.0), left,
					dir);
#endif //COMPRESSED_CALCULATION
			//=============================
			//==========================
#ifdef OPTIMIZED_CALCULATION
			//Define these temp variables above (outside the loops)
			//(already done)
			//Flt ty = 1.0 / (g.screen.fyres - 1.0);
			//Flt tx = 1.0 / (g.screen.fxres - 1.0);
            //shoot a ray to disk
            //one of the rays hits the dge blurry edge
            //focus halfway
            //shoot multiple rays to the halfway point
            //start from mutliple positions 
            //evertthing passes through 1 point ray direction random starting point 
            //save focal length 
            //ray is a point x,y,z 
            //get random point - point = new ray direction 
            //for loop for multiple starting locations 
            //origin + direction * focal length 
            //new ray = focal point - start 
            //do multiple average
            //if (appature) do a loop 
            //O +dt where t is the focal length 
			vecComb(-frustumheight * (2.0 * (Flt)py*ty - 1.0), up,
					frustumwidth  * (2.0 * (Flt)px*tx - 1.0), left,
					dir);
#endif //OPTIMIZED_CALCULATION
			//============================
			//
			//vecCopy(dir, savedir);
			//
            if (g.appature == 1) {
                Vec focal_length;
	            Vec mid_point;
                Vec average_dir;
                Vec average_rgb;
                
                // set mid point to ray origin / 2
                mid_point[0] = ray.o[0]/2;
                mid_point[1] = ray.o[1]/2;
                mid_point[2] = ray.o[2]/2;

                for (int i = 0; i < 15; i++) {
                    // set rand to ray origin
                    Vec rand;
                    rand[0] = ray.o[0];
                    rand[1] = ray.o[1];
                    rand[2] = ray.o[2];
                    // add a random vector;
                    vecRand(rand);
                    // focal length = random point - midpoint
                    vecSub(rand, mid_point, focal_length);
                    // accumulate the focal lengths
                    vecAdd(average_dir, focal_length, average_dir);
                    // accumulate the pixels
                    vecAdd(average_rgb, rgb, average_rgb);
                    vecAdd(dir, out, ray.d);
			        vecNormalize(ray.d);
                    trace(&ray, rgb, 1.0, 1);
                }
                // take the average of the focal lengths
                average_dir[0] /= 15;
                average_dir[1] /= 15;
                average_dir[2] /= 15;
                // take the average of the pixels
                average_rgb[0] /= 15;
                average_rgb[1] /= 15;
                average_rgb[2] /= 15;
			    
			    x11.drawPixel(j, i, average_rgb);
            } else {
			    vecAdd(dir, out, ray.d);
			    vecNormalize(ray.d);
			    trace(&ray, rgb, 1.0, 1);
			    x11.drawPixel(j, i, rgb);
            }
		}
	}
}
