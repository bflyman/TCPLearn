#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <iostream>
#include <bitset>
#include "header/dns.h"


using namespace std;

const char srv_ip[] = "208.67.222.222";

int main(int argc, char** argv)
{
    int      servfd,clifd,len = 0,i;
    struct   sockaddr_in servaddr, addr;
    int      socklen = sizeof(servaddr);
    char     buf[BUF_SIZE];
    char     *p;
    DNS_HDR  *dnshdr = (DNS_HDR *)buf;
    DNS_QER  *dnsqer = (DNS_QER *)(buf + sizeof(DNS_HDR));
   
    if ((clifd  =  socket(AF_INET, SOCK_DGRAM, 0 ))  <   0 )//打开socket
    {
         printf( " create socket error!\n " );
         return -1;
    }
   
    bzero(&servaddr, sizeof(servaddr)); //设置服务器地址内存块为\0
    servaddr.sin_family = AF_INET;
    inet_aton(srv_ip, &servaddr.sin_addr);//把IP字符串转换为网络字节序的整数值.
    servaddr.sin_port = htons(SRV_PORT);
   
    /*if (connect(clifd, (struct sockaddr *)&servaddr, socklen) < 0)
    {
          printf( " can't connect to %s!\n ", argv[ 1 ]);
          return -1;
    }*/
    memset(buf, 0, BUF_SIZE);
    dnshdr->id = (U16)1;
    
    dnshdr->tag = htons(0x0100);
    dnshdr->numq = htons(1);
    dnshdr->numa = 0;
    cout<<"id:"<<bitset<sizeof(dnshdr->id)*8>(dnshdr->id)<<endl;
    cout<<"tag 网络序:"<<bitset<sizeof(dnshdr->tag)*8>(dnshdr->tag)<<endl;
    cout<<"tag QR:"<<bitset<1>(dnshdr->tag>>7)<<endl;
    cout<<"tag opcode:"<<bitset<4>(dnshdr->tag>>3)<<endl;
    cout<<"tag AA:"<<bitset<1>(dnshdr->tag>>2)<<endl;
    cout<<"tag TC:"<<bitset<1>(dnshdr->tag>>1)<<endl;
    cout<<"tag RD:"<<bitset<1>(dnshdr->tag)<<endl;
    cout<<"tag RA:"<<bitset<1>(dnshdr->tag>>8>>7)<<endl;
    cout<<"tag (zero)):"<<bitset<3>(dnshdr->tag>>8>>4)<<endl;
    cout<<"tag rcode:"<<bitset<4>(dnshdr->tag>>8)<<endl;
    cout<<"tag 计算机:"<<bitset<sizeof(dnshdr->tag)*8>(ntohs(dnshdr->tag))<<endl;
    cout<<"numq:"<<bitset<sizeof(dnshdr->numq)*8>(dnshdr->numq)<<endl;
    printf("id:%x tag:%x numq:%x\n", dnshdr->id,dnshdr->tag,dnshdr->numq);

    strcpy(buf + sizeof(DNS_HDR) + 1, argv[1]);
    p = buf + sizeof(DNS_HDR) + 1;
     i = 0;
    while (p < (buf + sizeof(DNS_HDR) + 1 + strlen(argv[1])))
    {
        if ( *p == '.')
        {
            *(p - i - 1) = i;
            i = 0;
        }
        else
        {
            i++;
        }
        p++;
    }

    *(p - i - 1) = i;
       
    dnsqer = (DNS_QER *)(buf + sizeof(DNS_HDR) + 2 + strlen(argv[1]));
    dnsqer->classes = htons(1);
    dnsqer->type = htons(1);
   
     
    len = sendto(clifd, buf, sizeof(DNS_HDR) + sizeof(DNS_QER) + strlen(argv[1]) + 2, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //len = send(clifd, buf, sizeof(DNS_HDR) + sizeof(DNS_QER) + strlen(argv[1]) + 2, 0);
    socklen_t slen = sizeof(struct sockaddr_in);
    len = recvfrom(clifd, buf, BUF_SIZE, 0, (struct sockaddr *)&servaddr, &slen);
    //len = recv(clifd, buf, BUF_SIZE, 0);
    if (len < 0)
    {
          printf("recv error\n");
          return -1;
    }
    if (dnshdr->numa == 0)
    {
          printf("ack error\n");
          return -1;
    }
    p = buf + len -4;
    printf("%s ==> %u.%u.%u.%u\n", argv[1], (unsigned char)*p, (unsigned char)*(p + 1), (unsigned char)*(p + 2), (unsigned char)*(p + 3));
 
    close(clifd);
    return 0;
}