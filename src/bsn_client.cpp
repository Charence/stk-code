#include "bsn_client.hpp"
#include <stdio.h>
#include "SDL_thread.h"
#include "SDL_net.h"
#include <string.h>

static int repeat = 0;
static IPaddress ipaddress;
static TCPsocket tcpsock;
static SDL_Thread *thread;

#define MAXSAMP 3
static int sample[MAXSAMP];
static int center[MAXSAMP];
static int changed = 0;

static int axis[MAXSAMP];
static float scale[MAXSAMP];

#define MAXLEN 1024
static char buffer[MAXLEN];

#define NET_SENSOR_DATA 0
#define NET_SET_CONFIG 1
#define NET_EXIT 3

static int bsn_loop (void *unused) {
  int k0, n, i;
  if(SDLNet_Init()==-1) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
	return -2;
  }
  if (SDLNet_ResolveHost(&ipaddress, "localhost", 9001)==-1) {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return -3;
  }
  tcpsock=SDLNet_TCP_Open(&ipaddress);
  if(!tcpsock) {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return -4;
  }

  repeat = 1;

  while (repeat) {
    int result = SDLNet_TCP_Recv(tcpsock,buffer,8);
    if (result<=0) break;
    k0 = *((int*)buffer);
    n = *((int*)(buffer+4));
    n -= 8;
    if (k0==NET_EXIT) break;
    if (k0==NET_SENSOR_DATA) {
      if (n>=12) {
		result = SDLNet_TCP_Recv(tcpsock,buffer,12);
		n -= 12;
      }
      else {
		result = SDLNet_TCP_Recv(tcpsock,buffer,n);
		n = 0;
      }
      i = 0;
      while (n) {
		if (n>=2) {
		  result = SDLNet_TCP_Recv(tcpsock,buffer,2);
		  n -= 2;
		  if (i<MAXSAMP)
			sample[i++] = *((short int*)buffer);
		}
		else {
		  result = SDLNet_TCP_Recv(tcpsock,buffer,n);
		  n = 0;
		}
      }
      changed = 1;      
    }
    else {
      while (n) {
		if (n>MAXLEN) {
		  result = SDLNet_TCP_Recv(tcpsock,buffer,MAXLEN);
		  n -= MAXLEN;
		}
		else {
		  result = SDLNet_TCP_Recv(tcpsock,buffer,n);
		  n = 0;
		}
      }
    }
  }
  SDLNet_TCP_Close(tcpsock);
  return 0;
}

int bsn_init (char *config) {
	if (config) {
		FILE *f = fopen(config,"r");
		if (f) {
			fscanf(f,"%d%d%d",axis,axis+1,axis+2);
			fscanf(f,"%f%f%f",scale,scale+1,scale+2);
			fscanf(f,"%d%d%d",center,center+1,center+2);
			fclose(f);
		}
	}
	else {
	  axis[0] = 0;
	  axis[1] = 1;
	  axis[2] = 2;
	  scale[0] = 0.005;
	  scale[1] = 0.005;
	  scale[2] = 0.005;
	  center[0] = 3500;
	  center[1] = 2800;
	  center[2] = 3500;
	}

	thread = SDL_CreateThread(bsn_loop,"BSN_Thread", NULL);
	if ( thread == NULL ) {
		fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
		return -1;
	}
	return 0;
}

int bsn_halt () {
  repeat = 0;
  SDL_WaitThread(thread, NULL);
  return 0;
}

int bsn_state (float *xp, float *yp, float *zp) {
  int result = changed;
  *xp = (sample[axis[0]] - center[axis[0]])*scale[0];
  *yp = (sample[axis[1]] - center[axis[1]])*scale[1];
  *zp = (sample[axis[2]] - center[axis[2]])*scale[2];
  changed = 0;
  return result;
}

int bsn_set_center() {
	int i;
	for (i = 0; i<MAXSAMP; i++)
		center[i] = sample[i];
	return 0;
}

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
