#include <stdio.h> 
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#define BUF_SIZ 266 
//#define BUF_SIZ 9092 
#define MCASTP 3020
extern char *command_str(char *c);
int main(int argc, char **argv){
struct sockaddr_in src, temp,mcast;
struct in_addr mcastaddr;
int so,sc,i,sock;
unsigned int ttl,mlen;
//const char *str1="fdakfdaj";
char message[1025];
FILE *fp;
struct ip_mreq imr;
char str2s[BUF_SIZ];
//for (i=0; i<argc; i++) printf("%s", argv[2]);
if(argc!=1 && argc < 3 ){
printf("%s -x (command) or -f file(write file) -m mcastAddr (Write mode)\n",argv[0]);
printf("%s -m mcastAddr (using -235.235.232.213)(Receive mode)\n",argv[0]);
return 0;
}
char *inetadr="235.235.232.213";
for(i=1; i<argc; i++) 
if(!strcmp(argv[i],"-m")){ inetadr= argv[i+1]; break;}
mcastaddr.s_addr=inet_addr(inetadr);
mcast.sin_family=AF_INET;
mcast.sin_addr.s_addr=inet_addr(inetadr);
mcast.sin_port=htons(MCASTP);
//struct hostent *mcastad=gethostbyname(argv[argc-1]);
//memcpy((char *) &mcastaddr.s_addr, mcastad->h_addr_list[0],mcastad->h_length); 
//char *co=strstr(argv[1],"-c");
//printf(co);

  if(!IN_MULTICAST(ntohl(mcastaddr.s_addr))) {
   printf("%s : given address '%s' is not multicast\n",argv[0],
	   inet_ntoa(mcastaddr));
    exit(1);
  }
if(argc>2 && strcmp(argv[1],"-m")){
so=socket(AF_INET, SOCK_DGRAM,0);
src.sin_family=AF_INET;
src.sin_addr.s_addr=htonl(INADDR_ANY);
src.sin_port=htons(0);
bind(so, (struct sockaddr *) &src, sizeof(src));
setsockopt(so,IPPROTO_IP,IP_MULTICAST_TTL, &ttl,sizeof(ttl));
if(!strcmp(argv[1],"-x")){
char *command=argv[1];
for(i=2;i<argc && strcmp(argv[i],"-m"); i++)
command= strcat(strcat(command,argv[i])," "); 
int slen=strlen(command)+1;
 sc= sendto(so,command, slen, 0, (struct sockaddr *) &mcast, sizeof(mcast));
if(sc==-1) printf("Unable to send, do group exist\n");
//printf("%s%d\n",command, sc);
 }
if(!strcmp(argv[1],"-F")|| !strcmp(argv[1],"-f")){
fp = fopen(argv[2],"r");
char *filename= argv[1]; 
filename= strcat(filename, argv[2]);
sc=sendto(so,filename,strlen(filename)+1, 0, (struct sockaddr *) &mcast, sizeof(mcast)); 
if(sc==-1) printf("Unable to send, do group exist\n");
fseek(fp , 0 , SEEK_END); long size; 
size = ftell(fp); rewind(fp); 
char *buffer = (char*) malloc(sizeof(char)*size); 
char buffer2[size];
int j=0;
int n,numr; 
numr=fread(buffer,1,size,fp);
strcpy(buffer2,buffer);
    n = 0;
n=sendto(so,buffer2+n,numr-n, 0, (struct sockaddr *) &mcast, sizeof(mcast)); 

//while((n=sendto(so,buffer2,numr-n, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0);  //for video maybe
 sc=sendto(so,"EOF",strlen("EOF")+1, 0, (struct sockaddr *) &mcast, sizeof(mcast));
if(sc==-1) printf("Unable to send, do group exist\n");
fclose(fp);
 }
}
else {
temp.sin_family=AF_INET;
temp.sin_addr.s_addr=htonl(INADDR_ANY);
temp.sin_port=htons(MCASTP);
if((sock=socket(AF_INET, SOCK_DGRAM,0))<0) exit(0);
bind(sock, (struct sockaddr *) &temp, sizeof(temp));
imr.imr_multiaddr.s_addr=mcastaddr.s_addr;
imr.imr_interface.s_addr= htonl(INADDR_ANY);
i=setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,  &imr, sizeof(struct ip_mreq));
if(i < 0) {printf("Cannot join Multicast Group\n"); exit(0);}
FILE *fn=NULL;
 while(1){
mlen=sizeof(src);
i=recvfrom(sock, message, 1000, 0, (struct sockaddr *) &src , &mlen);
//else i=recvfrom(sock, buf, bufsize, 0, (struct sockaddr *) &src , &mlen);
//char *cmds=(char *) malloc (100*sizeof(char *)); 
//cmds=strcpy(cmds, message);

if(i==-1) continue;
//printf("%s\n",command_str(cmds));
if(!fn && strstr(message,"-x")){
printf("%s\n", &message[2]);
system(command_str(message));
}
else if(!fn && strstr(message,"-F")) {
 fn= fopen(command_str(message),"w");continue;}
else if(strcmp(message,"EOF") && fn) fputs(message,fn);
else if(fn) fclose(fn);
else fputs(message,stdout);

}//if(getchar()==EOF) 
//return setsockopt(so,IPPRTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(struct ip_mreq));

}
return 0;
}
char *command_str(char *c){
int i=0;
for(i=2;i<strlen(c);i++)
c[i-2]=c[i];
c[i-2]='\0';
return c;
}

/*
static struct option long_opt[] = {
    {"cmd", 1, 0, 'c'},
    {"file", 1, 0, 'f'},
    {"maddr", 1, 0, 'm'},
    {NULL, 0, NULL, 0}
};
static char *short_opt = "c:f:m:";
char *myargs(int argc, char **argv){
    while (1) {
	c = getopt_long(argc, argv, short_opt, long_opt, NULL);
	if (c == -1)
	    break;
         
         else return optarg;
}
*/
