#include <cstdio>
#include "hidapi.h"
#include <csignal>
#include <windows.h>

typedef enum OUTPUT{ROTZ_NEG = -6, ROTY_NEG, ROTX_NEG, Z_NEG, Y_NEG, X_NEG, ZERO, X_POS, Y_POS, Z_POS, ROTX_POS, ROTY_POS, ROTZ_POS} MVMD;
enum MODE{JOINT, TOOL};

void sighandler(int signum)
{
	printf("\nQuit by user.\n");
	exit(1);
}

inline short sgn(short num)
{
	return num > 0 ? 1 : ((num < 0) ? -1 : 0);
}

MVMD getmax(short input[6])
{
	short maxval = 30;
	int maxix = 0;
	if(input[0] | input[1] | input[2] | input[3] | input[4] | input[5])
	{
		for(int i = 0; i != 6; ++i)
		{
			short sgnipt = sgn(input[i]);
			short tmpabs = input[i] * sgnipt;
			if(tmpabs > maxval)
			{
				maxval = tmpabs;
				maxix = (i + 1) * sgnipt;
			}
		}
	}
	else
		return ZERO;
	return (MVMD)(ZERO + maxix);
}

int main()
{
	OUTPUT mode;
	signal(SIGINT, sighandler);
	hid_device *mydev;
	unsigned char buf[256];
	signed short input[6];
	MODE md = JOINT;
	MVMD movemd = ZERO;
	int sgn = 0;
	if(hid_init())
		return -1;
	mydev = hid_open(0x256f, 0xc62f, NULL);
	if(mydev != NULL)
	{
	    printf("Mice found.\n");
	}
	else
	{
		fprintf(stderr, "No device found.\n");
		return 1;
	}
	int res = 0;
	while(res == 0) 
	{
		res = hid_read(mydev, buf, sizeof(buf));
		if(res == 0)
			printf("Waiting...\n");
		if(res < 0)
			printf("Unable to read data.\n");
		Sleep(500);
	}
	while(1)
	{
		while(res = hid_read(mydev, buf, sizeof(buf)))
		{
			int axis = 0;
			switch(buf[0])
			{
				case 1:
					axis = 0;
				case 2:
					input[axis++] = buf[1] | ((buf[2]) << 8);
					input[axis++] = buf[3] | ((buf[4]) << 8);
					input[axis++] = buf[5] | ((buf[6]) << 8);
					input[axis++] = buf[7] | ((buf[8]) << 8);
					input[axis++] = buf[9] | ((buf[10]) << 8);
					input[axis] = buf[11] | ((buf[12]) << 8);
					break;
				case 3:
					if(buf[1] == 2)	
						md = (MODE)(md ^ 1);	//switch mode
					break;
			}
			Sleep(10);
			printf("\n\n\n**********\n");
			printf("Current Mode is ");
			printf(md == 0 ? "JOINT:" : "TOOL:");
			printf("\n");
			printf("Moving Mode is %d\n", getmax(input));
			for(axis = 0; axis != 6; ++axis)
			{
				printf("DOF[%d]: %d", axis + 1, input[axis]);
				printf(axis != 5 ? " " : "");
			}
			printf("\n**********");
		}
	}
	return 0;
}
