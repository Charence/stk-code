#include "bsn_client.hpp"
#include <stdio.h>
#include "SDL_thread.h"
#include "SDL_net.h"
#include <string.h>

static int repeat1 = 0;
//static int repeat2 = 0;
static IPaddress ipaddress1;
//static IPaddress ipaddress2;
static TCPsocket tcpsock1;
//static TCPsocket tcpsock2;
static SDL_Thread *thread1;
//static SDL_Thread *thread2;

#define MAXSAMP 3
static int sample1[MAXSAMP];
//static int sample2[MAXSAMP];
static int center1[MAXSAMP];
//static int center2[MAXSAMP];
static int changed1 = 0;
//static int changed2 = 0;

static int axis[MAXSAMP];
static float scale[MAXSAMP];

#define MAXLEN 1024
static char buffer1[MAXLEN];
static char buffer2[MAXLEN];

#define NET_SENSOR_DATA 0
#define NET_SET_CONFIG 1
#define NET_EXIT 3

static int bsn_loop1 (void *unused) {
  int k0, n, i;
  if(SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
	return -2;
  }
  if (SDLNet_ResolveHost(&ipaddress1, "localhost", 9001) == -1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return -3;
  }
  tcpsock1=SDLNet_TCP_Open(&ipaddress1);
  if (!tcpsock1) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return -4;
  }

  repeat1 = 1;

  while (repeat1) {
    int result = SDLNet_TCP_Recv(tcpsock1,buffer1,8);
    if (result<=0) break;
    k0 = *((int*)buffer1);
    n = *((int*)(buffer1+4));
    n -= 8;
    if (k0==NET_EXIT) break;
    if (k0==NET_SENSOR_DATA) {
      if (n>=12) {
		result = SDLNet_TCP_Recv(tcpsock1,buffer1,12);
		n -= 12;
      }
      else {
		result = SDLNet_TCP_Recv(tcpsock1,buffer1,n);
		n = 0;
      }
      i = 0;
      while (n) {
		if (n>=2) {
		  result = SDLNet_TCP_Recv(tcpsock1,buffer1,2);
		  n -= 2;
		  if (i<MAXSAMP)
			sample1[i++] = *((short int*)buffer1);
		}
		else {
		  result = SDLNet_TCP_Recv(tcpsock1,buffer1,n);
		  n = 0;
		}
      }
      changed1 = 1;      
    }
    else {
      while (n) {
		if (n>MAXLEN) {
		  result = SDLNet_TCP_Recv(tcpsock1,buffer1,MAXLEN);
		  n -= MAXLEN;
		}
		else {
		  result = SDLNet_TCP_Recv(tcpsock1,buffer1,n);
		  n = 0;
		}
      }
    }
  }
  SDLNet_TCP_Close(tcpsock1);
  return 0;
}

/*static int bsn_loop2(void *unused) {
	int k0, n, i;
	if (SDLNet_Init() == -1) {
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		return -2;
	}
	if ( SDLNet_ResolveHost(&ipaddress2, "localhost", 9002) == -1) {
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		return -3;
	}
	tcpsock2 = SDLNet_TCP_Open(&ipaddress2);
	if (!tcpsock2) {
		printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		return -4;
	}

	repeat2 = 1;

	while (repeat2) {
		int result = SDLNet_TCP_Recv(tcpsock2, buffer2, 8);
		if (result <= 0) break;
		k0 = *((int*)buffer2);
		n = *((int*)(buffer2 + 4));
		n -= 8;
		if (k0 == NET_EXIT) break;
		if (k0 == NET_SENSOR_DATA) {
			if (n >= 12) {
				result = SDLNet_TCP_Recv(tcpsock2, buffer2, 12);
				n -= 12;
			}
			else {
				result = SDLNet_TCP_Recv(tcpsock2, buffer2, n);
				n = 0;
			}
			i = 0;
			while (n) {
				if (n >= 2) {
					result = SDLNet_TCP_Recv(tcpsock2, buffer2, 2);
					n -= 2;
					if (i<MAXSAMP)
						sample2[i++] = *((short int*)buffer2);
				}
				else {
					result = SDLNet_TCP_Recv(tcpsock2, buffer2, n);
					n = 0;
				}
			}
			changed2 = 1;
		}
		else {
			while (n) {
				if (n>MAXLEN) {
					result = SDLNet_TCP_Recv(tcpsock2, buffer2, MAXLEN);
					n -= MAXLEN;
				}
				else {
					result = SDLNet_TCP_Recv(tcpsock2, buffer2, n);
					n = 0;
				}
			}
		}
	}
	SDLNet_TCP_Close(tcpsock2);
	return 0;
}*/

int bsn_init (char *config) {
	if (config) {
		FILE *f = fopen(config,"r");
		if (f) {
			fscanf(f,"%d%d%d",axis,axis+1,axis+2);
			fscanf(f,"%f%f%f",scale,scale+1,scale+2);
			fscanf(f,"%d%d%d",center1,center1+1,center1+2);
			fclose(f);
			/*center2[0] = center1[0];
			center2[1] = center1[1];
			center2[2] = center1[2];*/
		}
	}
	else {
	  axis[0] = 0;
	  axis[1] = 1;
	  axis[2] = 2;
	  scale[0] = 0.005;
	  scale[1] = 0.005;
	  scale[2] = 0.005;
	  center1[0] = 3500;
	  center1[1] = 2800;
	  center1[2] = 3500;
	  /*center2[0] = 3500;
	  center2[1] = 2800;
	  center2[2] = 3500;*/
	}

	thread1 = SDL_CreateThread(bsn_loop1,"BSN_Thread", NULL);
	//thread2 = SDL_CreateThread(bsn_loop2, "BSN_Thread", NULL);
	if (thread1 == NULL /*|| thread2 == NULL*/) {
		fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

int bsn_halt () {
  repeat1 = 0;
  SDL_WaitThread(thread1, NULL);
  /*repeat2 = 0;
  SDL_WaitThread(thread2, NULL);*/
  return 0;
}

int bsn_state1 (float *xp, float *yp, float *zp) {
  int result = changed1;
  *xp = (sample1[axis[0]] - center1[axis[0]])*scale[0];
  *yp = (sample1[axis[1]] - center1[axis[1]])*scale[1];
  *zp = (sample1[axis[2]] - center1[axis[2]])*scale[2];
  changed1 = 0;
  return result;
}

/*int bsn_state2(float *xp, float *yp, float *zp) {
	int result = changed2;
	*xp = (sample2[axis[0]] - center2[axis[0]])*scale[0];
	*yp = (sample2[axis[1]] - center2[axis[1]])*scale[1];
	*zp = (sample2[axis[2]] - center2[axis[2]])*scale[2];
	changed2 = 0;
	return result;
}*/

int bsn_set_center1() {
	int i;
	for (i = 0; i<MAXSAMP; i++)
		center1[i] = sample1[i];
	return 0;
}

/*int bsn_set_center2() {
	int i;
	for (i = 0; i<MAXSAMP; i++)
		center2[i] = sample2[i];
	return 0;
}*/

static int contestant_id = 0;

int bsn_start_stream () {
	// create a UDPsocket on any available port (client)
	UDPsocket udpsock;
	int channel;
	IPaddress ipaddress;
	UDPpacket *packet;
	int numsent;

	udpsock=SDLNet_UDP_Open(0);
	if(!udpsock) {
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return -2;
	}

	SDLNet_ResolveHost(&ipaddress, "localhost", 5555);

	channel = SDLNet_UDP_Bind(udpsock, -1, &ipaddress);
	if(channel==-1) {
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		return -3;
	}

	// create a new UDPpacket to hold 1024 bytes of data
	packet=SDLNet_AllocPacket(512);
	if(!packet) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return -4;
	}

	{
		FILE * f = fopen("contestant_id.txt","r");
		fscanf(f,"%d",&contestant_id);
		fclose(f);
		contestant_id++;
		f = fopen("contestant_id.txt","w");
		fprintf(f,"%d\n",contestant_id);
		fclose(f);
	}

	sprintf((char*)packet->data, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><start component=\"gateway\" SessionIsUser=\"yes\" configFile=\"c:/StreamingInfo.xml\" sessionID=\"CID_%06d\"/>\n", contestant_id);
	packet->len = strlen((char*)packet->data) + 1;

	numsent=SDLNet_UDP_Send(udpsock, channel, packet);
	if(!numsent) {
		printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		SDLNet_FreePacket(packet);
		return -5;
	}

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(udpsock);
}

int bsn_stream_id () { return contestant_id; }

int bsn_stop_stream () {
	// create a UDPsocket on any available port (client)
	UDPsocket udpsock;
	int channel;
	IPaddress ipaddress;
	UDPpacket *packet;
	int numsent;

	udpsock=SDLNet_UDP_Open(0);
	if(!udpsock) {
		printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return -2;
	}

	SDLNet_ResolveHost(&ipaddress, "localhost", 5555);

	channel = SDLNet_UDP_Bind(udpsock, -1, &ipaddress);
	if(channel==-1) {
		printf("SDLNet_UDP_Bind: %s\n", SDLNet_GetError());
		return -3;
	}

	// create a new UDPpacket to hold 1024 bytes of data
	packet=SDLNet_AllocPacket(512);
	if(!packet) {
		printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
		return -4;
	}

	sprintf((char*)packet->data,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><stop sessionID=\"CID_%06d\"/>\n",contestant_id);
	packet->len = strlen((char*)packet->data) + 1;

	numsent=SDLNet_UDP_Send(udpsock, channel, packet);
	if(!numsent) {
		printf("SDLNet_UDP_Send: %s\n", SDLNet_GetError());
		return -5;
	}

	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(udpsock);
}
