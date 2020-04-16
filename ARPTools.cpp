#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include<net/ethernet.h>
#include<net/if_arp.h>
#include<iostream>
#include<unistd.h>


u_char MAC_MINE[]={0x08,0x00,0x27,0x84,0x42,0x7A};
const char IP_MINE[]="192.168.0.96";
unsigned char MAC_TARGET[]={0xff,0xff,0xff,0xff,0xff,0xff};
const char IP_TARGET[]="192.168.0.60";

class arp_header
{
    public:
            unsigned char mac_targer[ETH_ALEN];//目的地址全1为广播地址
            unsigned char mac_source[ETH_ALEN];//源地址.

            unsigned short etherType;//帧类型 ARP请求答应为0x0806
            unsigned short hw_type;//硬件类型.1表示以太网地址
            unsigned short proto_type;//协议类型.0x0800即表示IP地址
            unsigned char mac_addr_len;//硬件地址长度(mac长度)单位字节
            unsigned char ip_addr_len;//协议地址长度(IP长度)单位字节
            unsigned short operation_code;//操作码:1请求,2答应,3.RARP请示,4.RARP答应
            unsigned char mac_sender[ETH_ALEN];//发送端以太网地址
            unsigned char ip_sender[4];//发送端IP
            u_char mac_receiver[ETH_ALEN];//目的以太网址址
            u_char ip_receiver[4];//目的IP地址
            u_char padding[4];

};

class arp_packet
{
    private: 
            arp_header ap;
    public:
            static void die(const char *pre);
            static void print_arp_packet(arp_header ap);

            void init_arp_server();
            void arp_server_run(arp_header &ah);
            void init_arp_publish(u_char mac_mine[], const char IP_MINE[], unsigned char MAC_TARGET[], const char IP_TARGET[]);
            void arp_publish();
    private:
            struct sockaddr_ll my_etheraddr,sl;
            int sfd;
            struct in_addr inaddr_sender,inaddr_receiver;
}arp;


void arp_packet::die(const char*pre)
{
	perror(pre);
	exit(1);
}

int main()
{
    arp_packet tmp;
    arp.init_arp_publish(MAC_MINE,IP_MINE,NULL,IP_TARGET);
    arp.arp_publish();
    return 0;
} 
void arp_packet::print_arp_packet(arp_header ap)
{
	printf("\n\n-----------------arp package begin--------------------------\n");
 
	printf("mac_target = ");
	for(int i = 0; i < ETH_ALEN; i++)
	{
		printf(i > 0 ? ":0x%.2x" : "0x%.2x", ap.mac_targer[i]);
	}
 
	printf("\nmac_source = ");
	for(int i = 0; i < ETH_ALEN; i++)
	{
		printf(i > 0 ? ":0x%.2x" : "0x%.2x", ap.mac_source[i]);
	}
 
	printf("\nethertype = 0x%x", ntohs(ap.etherType));
	printf("\nhw_type = 0x%x", ntohs(ap.hw_type));
	printf("\nproto_type = 0x%x", ntohs(ap.proto_type));
	printf("\nmac_addr_len = 0x%x", ap.mac_addr_len);
	printf("\nip_addr_len = 0x%x", ap.ip_addr_len);
	printf("\noperation_code = 0x%x", ntohs(ap.operation_code));
 
	printf("\nmac_sender = ");
	for(int i = 0; i < ETH_ALEN; i++)
	{
		printf(i > 0 ? ":0x%.2x" : "0x%.2x", ap.mac_sender[i]);
	}
 
	printf("\nip_sender = %s", inet_ntoa(*(struct in_addr*)(ap.ip_sender)));
 
	printf("\nmac_receiver = ");
	for(int i = 0; i < ETH_ALEN; i++)
	{
		printf(i > 0 ? ":0x%.2x" : "0x%.2x", ap.mac_receiver[i]);
	}
 
	printf("\nip_receiver = %s", inet_ntoa(*(struct in_addr*)(ap.ip_receiver)));
 
	printf("\n-----------------arp package end----------------------------\n");
}
 
 
 
void arp_packet::arp_publish()
{
    int times=10;
	while(times>0)
	{
		int len = sendto(sfd, &ap, sizeof(ap), 0, (struct sockaddr*)&sl, sizeof(sl));
		print_arp_packet(ap);
		if(-1 == len)
		{
			die("sendto");
		}
		sleep(1);
        times--;
	}
}



void arp_packet::init_arp_publish(u_char mac_mine[], const char IP_MINE[], unsigned char MAC_TARGET[], const char IP_TARGET[])
{
    memset(this,0,sizeof(*this));
    sfd=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));

    if(sfd==-1)
    {
        perror("error:build socket\n");
        return;
        
    }
    if(MAC_TARGET==NULL)
    {
        memset(ap.mac_targer,0xff,sizeof(ap.mac_targer));
    }
    else
    {
        memmove(ap.mac_targer,MAC_TARGET,sizeof(MAC_TARGET));

    }
    ap.etherType=htons(0x0806);
    ap.hw_type=htons(0x1);
    ap.proto_type=htons(0x0800);
    ap.mac_addr_len=ETH_ALEN;
    ap.ip_addr_len=4;
    ap.operation_code=htons(0x1);
    memmove(ap.mac_sender,mac_mine,sizeof(mac_mine));
    inet_aton(IP_MINE,&inaddr_sender);
    mempcpy(&ap.ip_sender,&inaddr_sender,sizeof(inaddr_sender));
    memmove(&ap.mac_receiver,ap.mac_targer,sizeof(ap.mac_targer));
    inet_aton(IP_TARGET, &inaddr_receiver);
    memcpy(&ap.ip_receiver,&inaddr_receiver,sizeof(inaddr_receiver));
    memset(&sl,0,sizeof(sl));
    sl.sll_family=AF_PACKET;
    sl.sll_ifindex=IFF_BROADCAST;

    

}