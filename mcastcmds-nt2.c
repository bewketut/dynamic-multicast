#include <stdio.h> 
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#define BUF_SIZ 899 
#define MCASTBUF_SIZ (BUF_SIZ+1) 
#define MCASTP 4020
extern char *base256(int num,char *str);
extern int tobase10(char *str);
//extern char *command_str(char *c);
//extern unsigned char calcfhash(char *m2, char m1, char *m, unsigned char hashes[], int arraysize);
/*
struct srcmutexfiles {
		unsigned char *fhash;
		FILE *fname;
}; typedef struct srcmutexfiles mcastsrcfile;
*/
int main(int argc, char **argv){
struct sockaddr_in src, temp,mcast;
struct in_addr mcastaddr;
int so,sc,i,sock,n;
unsigned int ttl,mlen;
char message[MCASTBUF_SIZ];
unsigned char c,d;
FILE *fp;
struct ip_mreq imr;
char str[20],*str2; 
//str2=base256(65489,str);
//printf("%d\n,", tobase10(str2));
if(argc!=1 && argc < 3 ){
printf("%s -c[f] command [command file] or -F(f) file(write file -f on stdout) -m mcastAddr (Write mode)\n",argv[0]);
printf("%s -m mcastAddr (using -235.235.232.213)(Receive mode)\n",argv[0]);
return 0;
}
char *inetadr="234.235.232.213";
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
//sock=socket(AF_INET, SOCK_DGRAM,0);
if(argc>2 && strcmp(argv[1],"-m")){
if((sock=socket(AF_INET, SOCK_DGRAM,0))<0) exit(0);
src.sin_family=AF_INET;
src.sin_addr.s_addr=htonl(INADDR_ANY);
src.sin_port=htons(0);
bind(sock, (struct sockaddr *) &src, sizeof(src));
setsockopt(sock,IPPROTO_IP,IP_MULTICAST_TTL, &ttl,sizeof(ttl));
if(!strcmp(argv[1],"-c")){
char *command=argv[1]; 
for(i=2;i<argc && strcmp(argv[i],"-m"); i++) 
command= strcat(strcat(command,argv[i])," "); 
 sc= sendto(sock,command, strlen(command)+1, 0, (struct sockaddr *) &mcast, sizeof(mcast));
if(sc==-1) printf("Unable to send, do group exist\n");
 }
if(!strcmp(argv[1],"-F")|| !strcmp(argv[1],"-f") || 
 !strcmp(argv[1],"-cf")){
fp = fopen(argv[2],"r");
//if(argc>3) {printf("Put a file name\n"); return 0;}
if(!fp) {printf("%s\n","Unable to open file for reading (read permission).");
            exit(1);}
if(!strcmp(argv[1],"-cf")){
   message[0]='-';message[1]='c'; message[2]='f'; 
   while(fgets(message+3,50,fp))
       //sc=sendto(so,message,53, 0, (struct sockaddr *) &mcast, sizeof(mcast));
       while((n=sendto(sock,message,54, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0) if(n!=-1)break;  
}
else{
unsigned char fileround=argv[2][0] + argv[2][strlen(argv[2])-5] - argv[2][strlen(argv[2])-3];
unsigned char filehash3= fileround%90;  
//printf("filehash: %d%s\n",filehash3,argv[2]);
unsigned char filehash= filehash3;
char *filename= (char *) malloc(sizeof(char)*300);
if(!strcmp(argv[1],"-F")) { filename[1]='S'; filename[2]='0'; filename[3]='F';filename[4]='!';}
filename[0]=filehash; filename[5]='\0';
if(strrchr(argv[2],'/')) filename=strcat(filename,strrchr(argv[2],'/')+1);
else
filename=strcat(filename,argv[2]);//initialization 
fseek(fp , 0 , SEEK_END); long size; 
size = ftell(fp); rewind(fp); 
long ntimes= (size/BUF_SIZ);
int rem = size%BUF_SIZ;
char rem1 =rem/256; char rem2=rem%256;

char *buffer = (char *) malloc(sizeof(char *)*(size+MCASTBUF_SIZ)); 
int numr; 
sc=sendto(sock,filename,strlen(filename)+1, 0, (struct sockaddr *) &mcast, sizeof(mcast)); 
if(sc==-1) printf("Unable to send, do group exist\n");

//int ntimes0=ntimes*3.2;
//for(i=0; i<ntimes0; i++)
//while((numr=fread(buffer,sizeof(char),size,fp))!=0);

for(i=0;i< ntimes;i++){
do {numr=fread(buffer,sizeof(char),size,fp);} while(numr!=0);
  c=buffer[i*BUF_SIZ+MCASTBUF_SIZ-1];
buffer[i*BUF_SIZ+MCASTBUF_SIZ-1]=filehash3;
while((n=sendto(sock,buffer+i*BUF_SIZ,MCASTBUF_SIZ, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0) if(n!=-1) break; 
buffer[i*BUF_SIZ+MCASTBUF_SIZ-1]=c;
} 
do {numr=fread(buffer,sizeof(char),size,fp);} while (numr!=0);
fclose(fp);
 char *remn=(char *)malloc(sizeof(char)*6); remn[4]= rem1; remn[5]=rem2;
remn[0]=filehash3; remn[1]='E'; remn[2]='O'; remn[3]='L'; remn[6]='\0'; 
while((sc=sendto(sock,remn,6, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0)
if(sc!=-1) break;
buffer[i*BUF_SIZ+MCASTBUF_SIZ-1]=filehash3;
//buffer[i*BUF_SIZ+rem]=filehash3;
while((n=sendto(sock,buffer+i*BUF_SIZ,MCASTBUF_SIZ, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0) if(n!=-1) break ; 
remn[3]='f'; 
while((n=sendto(sock,remn,6, 0, (struct sockaddr *) &mcast, sizeof(mcast)))!=0) if(n!=-1) break; 
if(sc==-1) printf("Unable to send, do group exist\n");
}
 }
}
else {
if((so=socket(AF_INET, SOCK_DGRAM,0))<0) exit(0);
temp.sin_family=AF_INET;
temp.sin_addr.s_addr=htonl(INADDR_ANY);
temp.sin_port=htons(MCASTP);
//if((sock=socket(AF_INET, SOCK_DGRAM,0))<0) exit(0);
bind(so, (struct sockaddr *) &temp, sizeof(temp));
imr.imr_multiaddr.s_addr=mcastaddr.s_addr;
imr.imr_interface.s_addr= htonl(INADDR_ANY);
i=setsockopt(so, IPPROTO_IP, IP_ADD_MEMBERSHIP,  &imr, sizeof(struct ip_mreq));
if(i < 0) {printf("Cannot join Multicast Group\n"); exit(0);}

char *argstr= argv[0];
if(strrchr(argv[0],'/'))argstr=strrchr(argv[0],'/')+1;
if(strrchr(argv[0],'.'))argstr=strrchr(argv[0],'/');
printf("%s%s%s%s%s\n","Prepared to receive commands and file transfers!\n Now do ",argstr," -F filename or ",argstr," -c commandname\n on another terminal or computer on the network.\n waiting...");
FILE *fn[90]; 
unsigned char fhash=0,hash0,fhashes[90];
unsigned char index=0,previndex, files2write=0,k=0;
unsigned char  diff=0;
 char fsub,ssub,m2;
int nextlen[90]; for(i=0;i<90;i++)nextlen[i]=BUF_SIZ;
 while(1){
mlen=sizeof(temp);
while((i=recvfrom(so, message, MCASTBUF_SIZ, 0, (struct sockaddr *) &temp , &mlen))!=0)
if(i!=-1) break;

if(!strncmp(message+1,"S0F!",4)){ 
index=((unsigned char)message[0])%90;
if(fn[index]==NULL)
fn[index]= fopen(message+5,"w"); files2write++;
printf("opening file %s for writing %d\n",message+5,index);
}
else if(!strncmp(message+1,"EOL",3)){
index=((unsigned char)message[0])%90;
 nextlen[index]=((unsigned char)message[4])*256 + ((unsigned char)message[5]) ;
}
else if(!strncmp(message+1,"EOf",3)){
index=((unsigned char)message[0])%90;
if(files2write && fn[index]!=NULL){fclose(fn[index]);
fn[index]=NULL;
files2write--;
printf("%s %d\n","Finished writing", index);
} 
}
else if(files2write){
index=(unsigned char) message[MCASTBUF_SIZ-1];
message[MCASTBUF_SIZ-1]=0;
if(index>0){
fwrite(message,1,nextlen[index],fn[index]);
if(nextlen[index]!=BUF_SIZ){
printf("%s %d\n","Finishing writing file", index);
nextlen[index]=BUF_SIZ;}
}
else if(!strncmp(message,"-c",2)){
if(!strncmp(message,"-cf",3)){
printf("%s\n", &message[3]);
system(&message[3]);
}
else {
printf("%s\n", &message[2]);
system(&message[2]);
}}
}
else if(!files2write && !strncmp(message,"-c",2)){
if(!strncmp(message,"-cf",3)){
printf("%s\n", &message[3]);
system(&message[3]);
}
else {
printf("%s\n", &message[2]);
system(&message[2]);
}
/*
  char *mystr= strstr(message, "-P");
if(mystr){
if(mystr[2]==' ')
 fprintf(stdout,"%s\n", mystr+3);
else 
 fprintf(stdout, "%s\n", mystr+2);
 }*/
}

else  
 fwrite(message,1,nextlen[index], stdout);

}
//fclose(fn[index]); fn[index]=NULL;
//if(getchar()==EOF) 
//return setsockopt(so,IPPRTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(struct ip_mreq));

}
return 0;
}

char *base256(int num,char *str){
unsigned char rem;
int div;
int len=strlen(str);printf("%d\n",strlen(str)); str[len--]='\0';
for(div= num/256,rem=num%256,str[len--]=rem; div > 256; div=div/256,rem=div%256)
str[len--]=rem;
str[len--]=div;
return str+len;
}
int tobase10(char *str){
int len= strlen(str);
int tot,i,j;
tot=(unsigned char) str[len-1];
printf("%s\n",str);
int base=100;
int mult=256;
for(i=0, j=len-2; j>-1 && i <len-2; i++,j--){
tot+=((unsigned char) str[j])*mult;
mult=mult*base;
base=base*100;
printf("total:%d\n",tot);
}

return tot;
}
/*

char *int2str(int k)
{ int i;
  char str[10];
      for(i=1; i<k; i*=10);

       k%i 
}


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