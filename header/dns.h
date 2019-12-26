
typedef unsigned short U16;
#define BUF_SIZE 1024
#define SRV_PORT 53
typedef struct _DNS_HDR
{  
  U16 id;
  U16 tag;
  U16 numq;
  U16 numa;
  U16 numa1;
  U16 numa2;
}DNS_HDR;

typedef struct _DNS_QER
{
   U16 type;
   U16 classes;
}DNS_QER;