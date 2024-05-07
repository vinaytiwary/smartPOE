#ifndef ETHERNET_COMP_H
#define ETHERNET_COMP_H
#include <Sources/_config.h>

#ifdef ETHERNET_EN

#define Ethernet UIPEthernet
#define EthernetClient UIPClient
#define EthernetServer UIPServer
#define EthernetUDP UIPUDP

#endif
#endif
