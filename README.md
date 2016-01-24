# Robot-Shared-Control

################
#####Draw the envrionment: void NetDisplay()
################

define DrawRectangle(x,y,w,h). W: Width, H: Height, X : is normal, Y: top to bottom

The area is 600 x 600

Colour can be chosen

######################
###### Robot Initalization: void InitRobot(int r,struct Robot *robot)
######################

Robot x_1 and y_1 is only used for drawing. No need for dunamics. More explanation later
	robot->p_x1=robot->p_x+0.0*(drand48()-0.5);
	robot->p_y1=robot->p_y+0.0*(drand48()-0.5);

Inilize Robot velocity
	robot->v_x=0.1*(drand48()-0.5);
	robot->v_y=0.1*(drand48()-0.5);

Best local and global position is the inital position
	robot->pbx=robot->p_x;
	robot->pby=robot->p_y;

	robot->gbx=robot->p_x;
	robot->gby=robot->p_y;

This fomulae is uesed for the initla position of the robot. 
y = a + b*(x-c);  a: constant    b: How speared particles should be    c: Where should the particles be initlaized in both axis
	robot->p_x=0.5+0.25*(robot->p_x-0.5);
	robot->p_y=0.5+0.25*(robot->p_y-0.5);


##################
##### Robot Dynamics: void RunRobot(int my_r,struct Robot *robot)
#####################


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



1) How the robot moves?

update velocity:

	robot->v_x=w*robot->v_x+a1*drand48()*(robot->pbx-robot->p_x)+a2*drand48()*(robot->gbx-robot->p_x); //+ac*(robot->p_x-robots[mrad]->p_x)/(radmin+0.0001);
	robot->v_y=w*robot->v_y+a1*drand48()*(robot->pby-robot->p_y)+a2*drand48()*(robot->gby-robot->p_y); //+ac*(robot->p_y-robots[mrad]->p_y)/(radmin+0.0001);

V_x = w*V_x + a1*(random)*(P_x - x) + a2*(random)*(G_x - x) + a3*(x - R_x)/(radmin+0.0001)
V_y = w*V_y + a1*(random)*(P_y - y) + a2*(random)*(G_y - y) + a3*(y - R_y)/(radmin+0.0001)

w=0.9   a1=a2=1.5 	ac= 0.01 	R_x,y= x and y of the closest robot 	rad = (R_x - R2_x)^2 + (R_y - R2_y)^2 	radmin = minimum differences between the two robot


update position:
robot->p_x+=sp*robot->v_x;
robot->p_y+=sp*robot->v_y;

sp=0.0001

2) How the robot avoids Obstacles?

robot->p_x+=sp*robot->v_x;
if ((fabs(robot->p_y-0.5)>0.1)&&(((robot->p_xo<0.52)&&(robot->p_x>0.48))||((robot->p_xo>0.52)&&(robot->p_x<0.48))))
robot->p_x=robot->p_xo;

x = x + sp*V_x
if |y - 0.5| > 0.1 and ( (x_0 < 0.52 and x > 0.48) or (x_0 > 0.52 and x < 0.48) )  ===> x = x_0

x : new position 	x_0: old position

3) How the robot avoids other robots?

The last part of velocity updation goes away from the closest one. Also:

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

What does this do? I dont know.

if (oo==0) {
	if (robot->p_x<rr2) robot->p_x=rr2;
	if (robot->p_x>1.0-rr2) robot->p_x=1.0-rr2;
	if (robot->p_y<rr2) robot->p_y=rr2;
	if (robot->p_y>1.0-rr2) robot->p_y=1.0-rr2;
}

rr1 = /* minimal wall distance */ 0.015

This is for avoiding walls. The walls of the arena.  arena in robot dynamics is 1x1

4) How the robot updates glabal and local best position?

Take a random robot see if it finds the global position. if you have not found that position, update yours.
for (r=0;r<NR;r++) {
	rg=(int)(drand48()*(double)NR);
	if ((robots[rg]->p_x<0.49)&&(robot->p_x>0.49)) {
		robot->gbx=robots[rg]->p_x;
		robot->gby=robots[rg]->p_y;
		break;
	}
	else {
		robot->gbx=robot->p_x;
		robot->gby=robot->p_y;
	}
}

This needs a little fixing I think.
if (robot->p_x<0.49) {
	robot->pbx=robot->p_x;
	robot->pby=robot->p_y;
}
else {
	robot->pbx=robot->p_x;
	robot->pby=robot->p_y;
}