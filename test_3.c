/************************************************************/
/* M. Herrmann, August 2015                                 */
/************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <malloc.h>

#define GRAPHICS 1

#define DISTY   2   /* 0 trajectory, 1 part of trajectory, 2 moving */

#define NR 50      /* number of robots */
#define L 10        /* number of positions: 0: center, 1: wall, 2: corner*/
#define TT0 40000000L
#define TM 46000000L /* Seems like the amount of run time which will terminate the graphics */
#define TRUE    1
#define FALSE   0

#define NUMBER_OF_COLORS        39
#define BLACK                   0
#define DIM_GREY                1
#define GREY_69                 2
#define GREY                    3
#define LIGHT_GREY              4
#define WHITE                   5
#define BLUE                    6
#define BLUE_CYAN               7
#define CYAN                    8
#define CYAN_GREEN              9
#define GREEN                   10
#define GREEN_YELLOW            11
#define YELLOW                  12
#define YELLOW_RED              13
#define RED                     14
#define MAGENTA                 15
#define LIME_GREEN              16
#define BROWN                   17
#define MAROON                  18
#define GOLD                    19
#define AQUAMARINE              20
#define FIREBRICK               21
#define GOLDENROD               22
#define BLUE_VIOLET             23
#define CADET_BLUE              24
#define CORAL                   25
#define CORNFLOWER_BLUE         26
#define DARK_GREEN              27
#define DARK_OLIVE_GREEN        28
#define PEACH_PUFF              29
#define PAPAYA_WHIP             30
#define BISQUE                  31
#define AZURE                   32
#define LAVENDER                33
#define MISTY_ROSE              34
#define MEDIUM_BLUE             35
#define NAVY_BLUE               36
#define PALE_TURQUOISE          37
#define SEA_GREEN               38

#define PLATE_UP                0
#define PLATE_DOWN              1
#define PLATE_LOCKED            2
#define WINDOW_W                700 /* Height and Weight of the Window */ 
#define WINDOW_H                600
#define FONT1                   "8x13bold" /* default font */ 
#define FONT2                   "8x13"     /* if FONT1 doesn't exist */
#define Color(c)               XSetForeground(display,gc,color[c].pixel)
#define DrawText(x,y,t)        XDrawString(display,window,gc,x,y,t,strlen(t))
#define FillRectangle(x,y,w,h) XFillRectangle(display,window,gc,x,y,w,h)
#define DrawRectangle(x,y,w,h) XDrawRectangle(display,window,gc,x,y,w,h)
#define DrawLine(x1,y1,x2,y2)  XDrawLine(display,window,gc,x1,y1,x2,y2)
#define DrawArc(x,y,w,h,a,b)   XDrawArc(display,window,gc,x,y,w,h,a,b)
#define TEXT_BUF_SIZE           64


#define QUIT_BUTTON                     0
#define DISPLAY_RATE_UP_BUTTON          1
#define DISPLAY_RATE_DOWN_BUTTON        2
#define RUN_BUTTON                      3

#define P_D 0.01        /* robot radius or rather step length*/

#define RR1 0.02        /* robot diameter */
#define RR2 0.015       /* minimal wall distance */
#define FEEL (4.0*RR1*RR1)

typedef unsigned char      boolean;

long t,display_rate;
double bou;
double p1_x,p1_y,p2_x,p2_y;
double sp;

double p_d;
double rr1;
double rr2;

double sc_x;
double sc_y;
double scx0;
double scy0;

// Adjust the size of the window and size of the robot
double rw1,rw2;

double w,a1,a2,aw,ac;

FILE *pf[NR],*mf[NR];

/*Robot paramters
robot position: p_x, p_y
robot velocity: v_x, v_y
robot best local position: pbx, pby
robot best global position: gbx, gby
values of best local/global position: gf, pf */

struct Robot {
    int l_st,l_st_o;
    int imax,imax_o;
    double v_x,v_y;
    double p_x,p_y;
    double p_x1,p_y1;
    double p_xo,p_yo;
    double pbx,pby;
    double gbx,gby;
    double gf,pf;
};

#if GRAPHICS
struct Context {
    struct Button    *Buttons;
    char             Comment[TEXT_BUF_SIZE];
};

struct Button {
    char            *Text;
    u_char          State,Value;
    short int       X,Y,Width,Height;
    struct Button   *Next;
};

Display         *display;
Window          window;
GC              gc;
XColor          color[NUMBER_OF_COLORS];
Font            font;
struct Context  *context;

#endif

struct Robot **robots;

#if GRAPHICS
void NetDisplay() {
    char x_text[50]="";

    Color(WHITE);
    sprintf(x_text,"%ld     ",t);
    DrawText(WINDOW_W-70,WINDOW_H-110,x_text);

    Color(GREEN);
    DrawRectangle(1,1,WINDOW_H-1,WINDOW_H-2);

    Color(RED);
    DrawRectangle((WINDOW_H)/3,(WINDOW_H-20)/2,(WINDOW_H)/3,4);   // horizontal 
    DrawRectangle((WINDOW_H)/3,1,4,(WINDOW_H-20)/2); // vertical
    DrawRectangle((WINDOW_H*2)/3,1,4,(WINDOW_H-20)/2); // vertical 

    Color(YELLOW);
    DrawRectangle(((WINDOW_H+6)*2)/3,1,(WINDOW_H-14)/3,(WINDOW_H)/2);
}


/* takes a robot number (only matter for colour), and a robot struct pointer */
void RobotDisplay(int r,struct Robot *robot) {

    int x1,x2,y1,y2;

    x2=(int)((double)WINDOW_H*robot->p_x);
    y2=(int)((double)WINDOW_H*robot->p_y);
    x1=(int)((double)WINDOW_H*robot->p_x1);
    y1=(int)((double)WINDOW_H*robot->p_y1);

#if DISTY*(DISTY-1)
    Color(BLACK);
    DrawArc(x1-rw2,y1-rw2,rw1,rw1,0,23040);
#endif
    Color(1+(5+0)%38);

  //Drwa them (int x,  int y,  int width,  int height,  int startAngle,  int sweepAngle)
    DrawArc(x2-rw2,y2-rw2,rw1,rw1,0,23040);
    /*DrawLine(x1,y1,x2,y2);*/
    robot->p_x1=robot->p_x;
    robot->p_y1=robot->p_y;
}

//ignore
void DrawPlate(int x,int y,int w,int h,u_char c,u_char state) {
    h--; w--;
    Color(c);
    FillRectangle(x+1,y+1,w-1,h-1);
    if (state == PLATE_DOWN) Color(BLACK);
    else Color(WHITE);
    DrawLine(x,y,x,y+h-1);
    DrawLine(x,y,x+w-1,y);
    if (state == PLATE_DOWN) Color(WHITE);
    else Color(BLACK);
    DrawLine(x+1,y+h,x+w,y+h);
    DrawLine(x+w,y+1,x+w,y+h);
}

//ignore
void DrawButton(struct Button *but) {
    short int x,y;
    u_char    c;

    if (but->State == PLATE_UP) {
        x = but->X + 4; 
        y = but->Y + 13; 
        c = GREY;
    }
    else {
        x = but->X + 5; 
        y = but->Y + 14; 
        c = GREY_69;
    }
    DrawPlate(but->X,but->Y,but->Width,but->Height,c,but->State);
    Color(BLACK);
    DrawText(x,y,but->Text);
    XSync(display,FALSE);
}

void DrawWindow(struct Context *context) {

    struct Button *but;
    int r;

    XSync(display,1);

    /*DrawPlate(0,0,WINDOW_W-1,WINDOW_H-1,WHITE,PLATE_UP);
    DrawPlate(0,0,WINDOW_W-1,WINDOW_H-1,BLACK,PLATE_UP);*/
    Color(BLACK);
    FillRectangle(WINDOW_H,WINDOW_H-120,WINDOW_W-1,WINDOW_H-1);

    NetDisplay();

  //NR: Number of robots, r: robot number (do it NR times)
    for (r=0;r<NR;r++) {
        RobotDisplay(r,robots[r]);
    }

    but = context->Buttons;;
    while (but) {
        DrawButton(but);
        but = but->Next;
    }

    XSync(display,0);
}

struct Button *PressButton(struct Context *context)
{
  XEvent               report;
  int                  mouse_x,mouse_y;
  struct Button        *but,*ret=NULL;
  static struct Button *pressed_but=NULL;
  u_char               redraw = FALSE;

  while (ret==NULL)
  {
    XNextEvent(display,&report);
    switch(report.type)
    {
      case Expose:
       redraw = TRUE;
       break;
      case ButtonPress:
       if (report.xbutton.button == Button1)
       {
         mouse_x = report.xbutton.x;
         mouse_y = report.xbutton.y;
         but = context->Buttons;
         while(but)
         {
           if ((mouse_x >= but->X)&&(mouse_x < but->X + but->Width)&&
               (mouse_y >= but->Y)&&(mouse_y < but->Y + but->Height))
           {
             but->State = PLATE_DOWN;
             DrawButton(but);
             pressed_but = but;
           }
           but = but->Next;
         }
       }
       break;
      case ButtonRelease:
       if ((report.xbutton.button == Button1)&&(pressed_but))
       {
         mouse_x = report.xbutton.x;
         mouse_y = report.xbutton.y;
         but = pressed_but;
         pressed_but = NULL;
         if (but->State == PLATE_DOWN) {
          if ((mouse_x >= but->X)&&(mouse_x < but->X + but->Width)&&
              (mouse_y >= but->Y)&&(mouse_y < but->Y + but->Height)) ret = but;
          else
          {
            but->State = PLATE_UP;
            DrawButton(but);
          }
    }
       }
       break;
      case MotionNotify:
       if (pressed_but)
       {
         mouse_x = report.xmotion.x;
         mouse_y = report.xmotion.y;
         but = pressed_but;
         if (but->State == PLATE_DOWN)
         {
           if (!((mouse_x >= but->X)&&(mouse_x < but->X + but->Width)&&
                 (mouse_y >= but->Y)&&(mouse_y < but->Y + but->Height)))
           {
             but->State = PLATE_UP;
             DrawButton(but);
           }
         }
         else
         {
           if ((mouse_x >= but->X)&&(mouse_x < but->X + but->Width)&&
                (mouse_y >= but->Y)&&(mouse_y < but->Y + but->Height))
           {
             but->State = PLATE_DOWN;
             DrawButton(but);
           }
         }
       }
       break;
    }
    if (redraw == TRUE)
    {
      DrawWindow(context);
      redraw = FALSE;
    }
  }
  return(ret);
}

boolean UnpressButton(struct Context *context,struct Button *cancelbutton)
{
  XEvent               report;
  int                  mouse_x,mouse_y;
  u_char               ret,redraw=FALSE;

  ret = FALSE;
  while (XPending(display))
  {
    XNextEvent(display,&report);
    switch(report.type)
    {
      case Expose:
       redraw = TRUE;
       break;
      case ButtonRelease:
       if (report.xbutton.button == Button1)
       {
         mouse_x = report.xbutton.x;
         mouse_y = report.xbutton.y;
         if ((mouse_x >= cancelbutton->X)&&
             (mouse_x <  cancelbutton->X + cancelbutton->Width)&&
             (mouse_y >= cancelbutton->Y)&&
             (mouse_y <  cancelbutton->Y + cancelbutton->Height)) ret = TRUE;
       }
       break;
    } 
  }
  if (redraw == TRUE) DrawWindow(context);
  return(ret);
}

struct Button *CreateButton(u_char value,char *text,int x,int y)
{
  struct Button *but;

  but         = (struct Button *)malloc(sizeof(struct Button));
  but->Value  = value;
  but->X      = x - 4 - strlen(text)*4;
  but->Y      = y;
  but->Text   = (char *)malloc(TEXT_BUF_SIZE);
  but->Width  = strlen(text)*8 + 8;
  but->Height = 18;
  but->State  = FALSE;
  but->Next   = NULL;
  strcpy(but->Text,text);
  return(but);
}


void OpenGraphics() {
  Colormap map;
  char     *disp = NULL;
  XColor   exact;
  int c;

  if (!(display = XOpenDisplay(disp)))
  {
    perror("Cannot open display\n");
    exit(-1);
  }

  map = XDefaultColormap(display,DefaultScreen(display));
  
    XAllocNamedColor(display,map,"black",&color[BLACK],&exact);
    XAllocNamedColor(display,map,"grey41",&color[DIM_GREY],&exact);
    XAllocNamedColor(display,map,"grey69",&color[GREY_69],&exact);
    XAllocNamedColor(display,map,"grey75",&color[GREY],&exact);
    XAllocNamedColor(display,map,"grey82",&color[LIGHT_GREY],&exact);
    XAllocNamedColor(display,map,"white",&color[WHITE],&exact);
    XAllocNamedColor(display,map,"red",&color[RED],&exact);
    XAllocNamedColor(display,map,"green",&color[GREEN],&exact);
    XAllocNamedColor(display,map,"blue",&color[BLUE],&exact);
    XAllocNamedColor(display,map,"cyan",&color[CYAN],&exact);
    XAllocNamedColor(display,map,"magenta",&color[MAGENTA],&exact);
    XAllocNamedColor(display,map,"yellow",&color[YELLOW],&exact);
    XAllocNamedColor(display,map,"lime green",&color[LIME_GREEN],&exact);  
    XAllocNamedColor(display,map,"#00b0f0",&color[BLUE_CYAN],&exact);
    XAllocNamedColor(display,map,"#00f0b0",&color[CYAN_GREEN],&exact);
    XAllocNamedColor(display,map,"#b0f000",&color[GREEN_YELLOW],&exact);
    XAllocNamedColor(display,map,"#f0b000",&color[YELLOW_RED],&exact);
    XAllocNamedColor(display,map,"brown",&color[BROWN],&exact);
    XAllocNamedColor(display,map,"maroon",&color[MAROON],&exact);
    XAllocNamedColor(display,map,"gold",&color[GOLD],&exact);
    XAllocNamedColor(display,map,"aquamarine",&color[AQUAMARINE],&exact);
    XAllocNamedColor(display,map,"firebrick",&color[FIREBRICK],&exact);
    XAllocNamedColor(display,map,"goldenrod",&color[GOLDENROD],&exact);
    XAllocNamedColor(display,map,"blue violet",&color[BLUE_VIOLET],&exact);
    XAllocNamedColor(display,map,"cadet blue",&color[CADET_BLUE],&exact);
    XAllocNamedColor(display,map,"coral",&color[CORAL],&exact);
    XAllocNamedColor(display,map,"cornflower blue",&color[CORNFLOWER_BLUE],
                     &exact);
    XAllocNamedColor(display,map,"dark green",&color[DARK_GREEN],&exact);
    XAllocNamedColor(display,map,"dark olive green",&color[DARK_OLIVE_GREEN],
                     &exact);
    XAllocNamedColor(display,map,"peach puff",&color[PEACH_PUFF],&exact);
    XAllocNamedColor(display,map,"papaya whip",&color[PAPAYA_WHIP],&exact);
    XAllocNamedColor(display,map,"bisque",&color[BISQUE],&exact);
    XAllocNamedColor(display,map,"azure",&color[AZURE],&exact);
    XAllocNamedColor(display,map,"lavender",&color[LAVENDER],&exact);
    XAllocNamedColor(display,map,"misty rose",&color[MISTY_ROSE],&exact);
    XAllocNamedColor(display,map,"medium blue",&color[MEDIUM_BLUE],&exact);
    XAllocNamedColor(display,map,"navy blue",&color[NAVY_BLUE],&exact);
    XAllocNamedColor(display,map,"pale turquoise",&color[PALE_TURQUOISE],
                     &exact);
    XAllocNamedColor(display,map,"sea green",&color[SEA_GREEN],&exact);


  XListFonts(display,FONT1,1,&c);
  if (c) font=XLoadFont(display,FONT1);
  else   font=XLoadFont(display,FONT2);
  window = XCreateSimpleWindow(display,
                               RootWindow(display,DefaultScreen(display)),
                               0,0,WINDOW_W,WINDOW_H,0,
                               WhitePixel(display,DefaultScreen(display)),
                               BlackPixel(display,DefaultScreen(display)));
  XChangeProperty(display,window,XA_WM_NAME,XA_STRING,8,PropModeReplace,
                  (unsigned char *)
                 "Graphics",47);
  XChangeProperty(display,window,XA_WM_ICON_NAME,XA_STRING,8,PropModeReplace,
                  (unsigned char *)"Neural Map Simulator",17);
  XSelectInput(display,window,ExposureMask|KeyPressMask|ButtonPressMask|
                              ButtonReleaseMask|PointerMotionMask);
  XMapWindow(display,window);
  gc = XCreateGC(display,window,0,NULL);
  XSetBackground(display,gc,color[GREY].pixel);
  XSetFont(display,gc,font);
}

void CloseGraphics() {
    XCloseDisplay(display);
}
#endif

void InitGlobal() {
    t=0L;
    p_d=P_D;
    bou=-0.21;
    rr1=RR1;
    rr2=RR2;

    sc_x=(double)WINDOW_H;
    sc_y=(double)WINDOW_H;
    scx0=(double)WINDOW_H;
    scy0=(double)WINDOW_H;
    rw1=WINDOW_H*RR1;
    rw2=WINDOW_H*RR2;

    w=0.9;
    a1=1.5;
    a2=1.5;

    aw=0.9;
    ac=0.01;

    sp=0.0001;
}


//Care about this
void InitRobot(int r,struct Robot *robot) {

    robot->imax=0;
    robot->l_st=0;

    /*robot->p_x=0.5+0.5*(drand48()-0.5);
    robot->p_y=0.5+0.5*(drand48()-0.5);*/

    if (NR<4) {
         robot->p_x=1.5*rr2+(1.0-rr1)*rr1*(0.5+(double)(r%NR))/((double)NR*rr1);
         robot->p_y=0.5;
    }
    else {
        robot->p_x=1.5*rr2+(1.0-rr1)*rr1*(0.5+(double)(r%(int)(sqrt((double)NR))))/((double)((int)(sqrt((double)NR)))*rr1);
        if ((int)(sqrt((double)NR))*(int)(sqrt((double)NR))!=NR) { 
            robot->p_y=1.5*rr2+(1.0-rr1)*rr1*(0.5+(double)(r/((int)(sqrt((double)NR)))))/((double)((int)(sqrt((double)NR)))*rr1)*(1.0-1.5/((double)((int)(sqrt((double)NR)))));
        }
        else {
            robot->p_y=1.5*rr2+(1.0-rr1)*rr1*(0.5+(double)(r/((int)(sqrt((double)NR)))))/((double)((int)(sqrt((double)NR)))*rr1);
        }
    }

  //Update Robot position x_1
    robot->p_x1=robot->p_x+0.0*(drand48()-0.5);
    robot->p_y1=robot->p_y+0.0*(drand48()-0.5);

  //Initalize Robot velocity
    robot->v_x=0.1*(drand48()-0.5);
    robot->v_y=0.1*(drand48()-0.5);

  //Best local and global position is the inital position
    robot->pbx=robot->p_x;
    robot->pby=robot->p_y;

    robot->gbx=robot->p_x;
    robot->gby=robot->p_y;

  //Update Robot position
    robot->p_x=0.5+0.25*(robot->p_x-1.9);
    robot->p_y=0.5+0.65*(robot->p_y-0.5);
}


// Arguments: Robot number, and the pointer to the robot. For example (7, robot[7])

void RunRobot(int my_r,struct Robot *robot) {
  int r,rg,mrad=my_r;
  double rad,radmin=1.0e10;

  /********************************************************************/
  /****************   R O B O T    D Y N A M I C S  *******************/
  /********************************************************************/
  for (r=0;r<NR;r++) {
    rg=(int)(drand48()*(double)NR);
    if (((robots[rg]->p_x>0.66) && (robots[rg]->p_y<0.5)) && ((robot->p_x<0.66) || (robot->p_y>0.5))) {
      robot->gbx=robots[rg]->p_x;
      robot->gby=robots[rg]->p_y;
      break;
    }
    else {
      robot->gbx=robot->p_x;
      robot->gby=robot->p_y;
    }
  }
  if (robot->p_x<0.33) {
    robot->pbx=robot->p_x;
    robot->pby=robot->p_y;
  }
  else {
    robot->pbx=robot->p_x;
    robot->pby=robot->p_y;
  }
  int oo=0;
  for (r=0;r<NR;r++) {
    if (r!=my_r) {
      rad=(robot->p_x-robots[r]->p_x)*(robot->p_x-robots[r]->p_x)+(robot->p_y-robots[r]->p_y)*(robot->p_y-robots[r]->p_y);
      if (rad<radmin) {
        radmin=rad;
        mrad=r;
      }

      if (rad<FEEL) {
        robot->l_st+=1;
      }
    }
  }
  //printf("%g\n",radmin);

  if ((robot->p_x<bou)||(robot->p_x>1.0-bou)||(robot->p_y<bou)||(robot->p_y>1.0-bou)) robot->l_st+=2;
  else if (((robot->p_x<bou)&&(robot->p_y<bou))||((robot->p_x<bou)&&(robot->p_y>1.0-bou))||((robot->p_x>1.0-bou)&&(robot->p_y<bou))||((robot->p_x>1.0-bou)&&(robot->p_y>1.0-bou))) robot->l_st+=4;     


  robot->v_x=w*robot->v_x+a1*drand48()*(robot->pbx-robot->p_x)+a2*drand48()*(robot->gbx-robot->p_x)+ac*(robot->p_x-robots[mrad]->p_x)/(radmin+0.0001);
  robot->v_y=w*robot->v_y+a1*drand48()*(robot->pby-robot->p_y)+a2*drand48()*(robot->gby-robot->p_y)+ac*(robot->p_y-robots[mrad]->p_y)/(radmin+0.0001);

  if (robot->p_x<rr2) robot->v_x-=aw*(robot->p_x-rr2);
  if (robot->p_x>1.0-rr2) robot->v_x+=aw*(robot->p_x-(1.0-rr2));
  if (robot->p_y<rr2) robot->v_y-=aw*(robot->p_y-rr2);
  if (robot->p_y>1.0-rr2) robot->v_y+=aw*(robot->p_y-(1.0-rr2));

  robot->p_xo=robot->p_x;
  robot->p_yo=robot->p_y;

  robot->p_x+=sp*robot->v_x;
  if ((fabs(robot->p_y-0.0)<0.5)&&(((robot->p_xo>0.3)&&(robot->p_x<0.36))||((robot->p_xo<0.3)&&(robot->p_x>0.36)))) {
  robot->p_x=robot->p_xo;
  } 
  if ((fabs(robot->p_y-0.0)<0.5)&&(((robot->p_xo<0.62)&&(robot->p_x>0.66))||((robot->p_xo>0.62)&&(robot->p_x<0.66)))) {
  robot->p_x=robot->p_xo;
  } 
  if ((fabs(robot->p_y-0.0)<0.5)&&(((robot->p_xo<0.66)&&(robot->p_x>0.62))||((robot->p_xo>0.66)&&(robot->p_x<0.62)))) {
  robot->p_x=robot->p_xo;
  } 

  robot->p_y+=sp*robot->v_y;
  if ((fabs(robot->p_x-0.5)<0.167)&&(((robot->p_yo>0.48)&&(robot->p_y<0.52))||((robot->p_yo<0.48)&&(robot->p_y>0.52)))) {
  robot->p_y=robot->p_yo; 
  }
  if ((fabs(robot->p_x-0.5)<0.167)&&(((robot->p_yo<0.52)&&(robot->p_y>0.48))||((robot->p_yo>0.52)&&(robot->p_y<0.48)))) {
  robot->p_y=robot->p_yo; 
  }

  oo=0;
  for (r=0;r<NR;r++) {
    if (r!=my_r) {
      rad=(robot->p_x-robots[r]->p_x)*(robot->p_x-robots[r]->p_x)+(robot->p_y-robots[r]->p_y)*(robot->p_y-robots[r]->p_y);
      if (rad<rr1*rr1) {
        robot->p_x=robot->p_xo;
        robot->p_y=robot->p_yo;
        oo=1;
        break;
      }
    }
  }
  if (oo==0) {
    if (robot->p_x<rr2) robot->p_x=rr2;
    if (robot->p_x>1.0-rr2) robot->p_x=1.0-rr2;
    if (robot->p_y<rr2) robot->p_y=rr2;
    if (robot->p_y>1.0-rr2) robot->p_y=1.0-rr2;
  }

  robot->l_st_o=robot->l_st;
  robot->l_st=0;
}

void InitFiles() {
    int r;
    char ffn[10];
    for (r=0;r<NR;r++) {
        sprintf(ffn,"pf%d",r);
        if((pf[r]=fopen(ffn,"wt"))==NULL) printf("couldn't open output file\n");
        sprintf(ffn,"mf%d",r);
        if((mf[r]=fopen(ffn,"wt"))==NULL) printf("couldn't open output file\n");
    }
}

void EvalRobot(long t, int r,struct Robot *robot) {

    /********************************************************************/
    /********************   E V A L U A T I O N   ***********************/
    /********************************************************************/

    if (t>=TT0) fprintf(pf[r],"%ld %g %g\n",t,robot->p_x,robot->p_y);
}

int main() {

    int r;

#if GRAPHICS
    struct Button *but,*button;
    boolean quit=FALSE;
    context = (struct Context *)malloc(sizeof(struct Context));

    but= context->Buttons= CreateButton(RUN_BUTTON,"RUN/STOP",WINDOW_W-50,WINDOW_H-90);
    but= but->Next=CreateButton(DISPLAY_RATE_UP_BUTTON,"+",WINDOW_W-40,WINDOW_H-60);
    but= but->Next=CreateButton(DISPLAY_RATE_DOWN_BUTTON,"-",WINDOW_W-60,WINDOW_H-60);
    but= but->Next=CreateButton(QUIT_BUTTON,"QUIT",WINDOW_W-50,WINDOW_H-30);

    display_rate=1L;

    OpenGraphics();
#endif

    robots = (struct Robot **)malloc(NR*sizeof(struct Robot*));

    for (r=0;r<NR;r++) {
        robots[r] = (struct Robot *)malloc(sizeof(struct Robot));
    }

    /*InitFiles();*/
    InitGlobal();

    for (r=0;r<NR;r++) {
        InitRobot(r,robots[r]);
    }

#if GRAPHICS
    while(quit == FALSE) {
        button=PressButton(context);
        switch(button->Value) {
        case QUIT_BUTTON:  
            quit = TRUE;
            break;
        case DISPLAY_RATE_UP_BUTTON:
            display_rate*=10L;
            if (display_rate>100000L) display_rate=100000L;
            display_rate-=display_rate%10L;
            if (display_rate<1L) display_rate=1L;
            DrawWindow(context);
            break;
        case DISPLAY_RATE_DOWN_BUTTON:
            if (display_rate>9L) display_rate/=10L;
            display_rate-=display_rate%10L;
            if (display_rate<1L) display_rate=1L;
            DrawWindow(context);
            break;

        case RUN_BUTTON:
#endif
            do { 

                for (r=0;r<NR;r++) {
                    RunRobot(r,robots[r]);
                }
                t++;
                if (t>TM) {
#if GRAPHICS
                    CloseGraphics();
#endif
                    return(1);
                    exit(1);
                }

#if GRAPHICS
                if (t%display_rate==0L) {
                    DrawWindow(context);
#if DISTY*(DISTY-2)
                    if (t%(250L*display_rate)==0L) {
                        DrawPlate(0,0,WINDOW_W-1,WINDOW_H-1,BLACK,PLATE_UP);
                    }
#endif
                }
#endif
            }
#if GRAPHICS
            while (UnpressButton(context,button)==FALSE); 
            break;
#else
            while (TRUE); 
#endif
#if GRAPHICS
        }
        button->State = PLATE_UP;
    }

    CloseGraphics();
#endif
    return(1);
}
