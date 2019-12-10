#include<arpa/inet.h>
#include<errno.h>
#include<fcntl.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<ctype.h>
#define BUFSIZE 4096
int PlayerTable[1000][11];
int gametable[10];

int CheckWiner(int II){
	if((PlayerTable[II][1]&&PlayerTable[II][4]&&PlayerTable[II][7])||(PlayerTable[II][2]&&PlayerTable[II][5]&&PlayerTable[II][8])||(PlayerTable[II][3]&&PlayerTable[II][6]&&PlayerTable[II][9])||(PlayerTable[II][1]&&PlayerTable[II][2]&&PlayerTable[II][3])||(PlayerTable[II][4]&&PlayerTable[II][5]&&PlayerTable[II][6])||(PlayerTable[II][7]&&PlayerTable[II][8]&&PlayerTable[II][9])||(PlayerTable[II][1]&&PlayerTable[II][5]&&PlayerTable[II][9])||(PlayerTable[II][3]&&PlayerTable[II][5]&&PlayerTable[II][7])){
		return 1;
	}
return 0;
}

void startgame(int II,int enemy,int number){
	int i;

	char buffer[100];
	memset(buffer,0,sizeof(buffer));
	memset(gametable,0,sizeof(gametable));
	
	if(number){
		if(PlayerTable[II][number]||PlayerTable[enemy][number]){
			write(II,"The place has be selected.  Please select a number again.\n",58);
			PlayerTable[II][10]=1;
			PlayerTable[enemy][10]=0;
			return;
		}
		PlayerTable[II][number]=1;
	}

	write(II,"Game Table\n",11);
	write(enemy,"Game Table\n",11);
	int count=0;
	for(i=1;i<=9;i++)
		if(PlayerTable[II][i]||PlayerTable[enemy][i]){
			gametable[i]=1;
			count++;
		}
	write(II,"1 2 3\n4 5 6\n7 8 9\nThe place can be selected:\n",45);
	write(enemy,"1 2 3\n4 5 6\n7 8 9\nThe place can be selected\n",45);
	
	for(i=1;i<=9;i++){
		if(!gametable[i]){
			sprintf(buffer,"%d ",i);
			write(II,buffer,strlen(buffer));
			write(enemy,buffer,strlen(buffer));
		}
	}
	write(II,"\nyour place:",12);
	write(enemy,"\nenemy's place:",15);
	for(i=1;i<=9;i++){
                if(PlayerTable[II][i]){
                        sprintf(buffer,"%d ",i);
                        write(II,buffer,strlen(buffer));
			write(enemy,buffer,strlen(buffer));
                }
        }
	write(II,"\nenemy's place:",15);
	write(enemy,"\nyour place:",12);
        for(i=1;i<=9;i++){
                if(PlayerTable[enemy][i]){
                        sprintf(buffer,"%d ",i);
                        write(II,buffer,strlen(buffer));
			write(enemy,buffer,strlen(buffer));
                }
        }

	if(CheckWiner(II)){
		write(II,"\nYOU WIN !!!!\nGame end.\n",24);
		write(enemy,"YOU LOSE ~~~~\nGame end.\n",25);
		memset(PlayerTable[II],0,sizeof(PlayerTable[II]));
		memset(PlayerTable[enemy],0,sizeof(PlayerTable[enemy]));
		return;
	}

	if(count==9){
		write(II,"\nNo Winer in the game.\nThe game is end.\n",40);
		write(enemy,"\nNo Winer in the game.\nThe game is end.\n",40);
		memset(PlayerTable[II],0,sizeof(PlayerTable[II]));
                memset(PlayerTable[enemy],0,sizeof(PlayerTable[enemy]));
		return;
	}	

	if(PlayerTable[II][10]){
		write(enemy,"\n",1);
		write(II,"\nYour turn.    Select a number(1~9):\n",37);
	}
	if(PlayerTable[enemy][10]){
                write(II,"\n",1);
                write(enemy,"\nYour turn.    Select a number(1~9):\n",37);
        }

	
}

int main(int argc,char *argv[]){
	
	char buffer[BUFSIZE]={};
	int sockfd=0,forClientSockfd=0;
	int reuseaddr=1;
	int pid=1;
	int i,j; 
/*	
	if(fork!=0)
		return 0;
*/
	signal(SIGCLD,SIG_IGN);

	sockfd=socket(AF_INET,SOCK_STREAM,0);

	if(sockfd==-1){
		printf("Fail to creat asocket.\n");
	}
	
	
	
	// SET SOCKET
	struct sockaddr_in ServerInfo,ClientInfo;
	int addrlen=sizeof(ClientInfo);
	bzero(&ServerInfo,sizeof(ServerInfo));

	ServerInfo.sin_family=PF_INET;
	ServerInfo.sin_addr.s_addr=INADDR_ANY;
	ServerInfo.sin_port=htons(4450);
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr));
	if(bind(sockfd,(struct sockaddr*)&ServerInfo,sizeof(ServerInfo))<0){printf("bind error!!!\n");}
	listen(sockfd,64);
	printf("server begin!!!\n");
	
	fd_set master,read_fds,login_fds;
	int fdmax;
	int nbytes,number;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	FD_ZERO(&login_fds);
	FD_SET(sockfd,&master);
	fdmax=sockfd;
	
	memset(PlayerTable,0,sizeof(PlayerTable));	
	while(1){
		read_fds=master;

		if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1){
			perror("select error!!!\n");
			exit(4);
		}	

		for(i=0;i<=fdmax;i++){
			if(FD_ISSET(i,&read_fds)){
				if(i==sockfd){	//handle new connections
					forClientSockfd=accept(sockfd,(struct sockaddr*)&ClientInfo,&addrlen);
					if(forClientSockfd==-1){
						perror("accept error");
					}else{
						FD_SET(forClientSockfd,&master);
						if(forClientSockfd>fdmax){
							fdmax=forClientSockfd;
						}
					}
					
				}else{
					if((nbytes=recv(i,buffer,sizeof(buffer),0))<=0){
						if(nbytes==0){
							printf("selectserver:socket %d hung up\n",i);
						}
						else{perror("recv error!!!\n");}
					close(i);
					FD_CLR(i,&master);
					}else{  //handle Client information
				//	printf("%s\n",buffer);		
						if(PlayerTable[i][0]&&PlayerTable[PlayerTable[i][0]][0]){
							if(!PlayerTable[i][10]){
								write(i,"Is not your turn.\n",18);
							}
							else{
								for(j=0,number=0;j<strlen(buffer);j++)
									if(isdigit(buffer[j])){
										number=number*10+buffer[j]-'0';
									}
								if(number>9||number<1){
									write(i,"The number must between 1~9.\n",29);
							}	
								else{
									PlayerTable[i][10]=0;
									PlayerTable[PlayerTable[i][0]][10]=1;
									startgame(i,PlayerTable[i][0],number);

								}
							}	
						}
						else if(PlayerTable[i][0]){
							if(!strncmp(buffer,"n",1)){
								write(PlayerTable[i][0],"The invitation is refused.\n",27);
								PlayerTable[i][0]=0;
							}
							else if(!strncmp(buffer,"y",1)){
								PlayerTable[PlayerTable[i][0]][0]=i;
								write(i,"The game start.\n",16);
								write(PlayerTable[i][0],"The game start.\n",16);
								PlayerTable[i][10]=1;
								PlayerTable[PlayerTable[i][0]][10]=0;
								startgame(i,PlayerTable[i][0],0);
							}
						}
						else if(!strncmp(buffer,"select",6)){
							if(!FD_ISSET(i,&login_fds)){
								write(i,"You have not log in.\n",21);	
							}
							else{
								number=0;
								for(j=6;j<strlen(buffer);j++){
									if(isdigit(buffer[j])){
										number=number*10+buffer[j]-'0';
									}
								}	
								//printf("number: %d\n",number);
								if(!FD_ISSET(number,&login_fds)){
									write(i,"The id is not login.\n",21);
								}
								else if(number==i){
									write(i,"The id is yours.\n",17);
								} 
								else if(PlayerTable[number][0]||PlayerTable[PlayerTable[number][0]][0])	
									write(i,"The id is in the game.",22);
								else{
									sprintf(buffer,"id:%d want play game with you.(y/n)\n",i);
									write(number,buffer,strlen(buffer));
									write(i,"Send invitation success.\n",25);
									PlayerTable[number][0]=i;
								}	
							}
						}

						else if(!strncmp(buffer,"login",5)){
							FD_SET(i,&login_fds);
							write(i,"login success~~~\n",17);
							sprintf(buffer,"your id: %d\n",i);
							write(i,buffer,strlen(buffer));	
						}							
						else if(!strncmp(buffer,"logout",6)){
							FD_CLR(i,&login_fds);
							write(i,"logout success~~~\n",18);
						}	
						else if(!strncmp(buffer,"table",5)){
							write(i,"~~~~~~~~~~~~~~~~~~~~\n",21);
							write(i,"login table\n",12);
							for(j=0;j<=fdmax;j++)
								if(FD_ISSET(j,&login_fds)){
									sprintf(buffer,"id: %d\n",j);
									write(i,buffer,strlen(buffer));
								}	
							 write(i,"~~~~~~~~~~~~~~~~~~~~\n",21);
						}
						else{
							write(i,"The command is wrong.\n",22);
						}
						
					}
					memset(buffer,0,sizeof(buffer));
				}
			}
		}
	

	}

return 0;
}
