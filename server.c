#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#define PORT 8090
#define PASS_LENGTH 20

#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

struct account{
	char name[10];	
	int id;
	int amount;
	char pass[PASS_LENGTH];
};

struct admin{
	char name[10];
	int id;
	char pass[20];
};

struct jaccount{
	char name[10];	
	int id;
	char name1[10];
	int amount;
	char pass[PASS_LENGTH];
};

char *ACC[3] = {"./individual", "./joint", "./admin"};

void service(int sock);
int login(int sock);
int signup(int sock);
int normal(int sock, int id, int type,int fd);
int joint(int sock, int id, int type,int fd);
int admin(int sock);

int main(){
	printf("Initializing connection...\n");
	/*
	AF_INET : ipv4 protocol family
	SOCK_STREAM : for reliable, sequencial connection oriented connection
	0 : only available protocol in given adress family
	*/
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) {
		//if socket creation fails
		printf("socket creation failed\n");
		ERR_EXIT("socket()");
	}
	int optval = 1;
	int optlen = sizeof(optval);
	/*
	to close socket automatically while terminating process
	SOL_SOCKET : to manipulate option at API level o/w specify level
	*/
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, optlen)==-1){
		printf("set socket options failed\n");
		ERR_EXIT("setsockopt()");
	}
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT);
	/*
	sockaddr_in is from family AF_INET (ip(7)) and varies as per the adress family
	*/
	printf("Binding socket...\n");
	if(bind(sockfd, (struct sockaddr *)&sa, sizeof(sa))==-1){
		printf("binding port failed\n");
		ERR_EXIT("bind()");
	}
	//2nd arg : size of backlog
	if(listen(sockfd, 100)==-1){
		printf("listen failed\n");
		ERR_EXIT("listen()");
	}
	printf("Listening...\n");
	while(1){
		int connectedfd;
		if((connectedfd = accept(sockfd, (struct sockaddr *)NULL, NULL))==-1){
			printf("connection error\n");
			ERR_EXIT("accept()");
		}
		// pthread_t cli;
		if(fork()==0){
			service(connectedfd);
			exit(1);
		}
	}
	close(sockfd);
	printf("Connection closed!\n");
	return 0;
}

void service(int sock){
	int func_id;
	printf("Client [%d] connected\n", sock);
	while(1){
		printf("Reading option\n");
		read(sock, &func_id, sizeof(int));
		printf("Read %d\n",func_id);
		if(func_id==1) {login(sock);}
		else if(func_id==2) {signup(sock);}
		// if(func_id==3) break;
		else { printf("Other choice!\n"); break;}
	}
	close(sock);
	printf("Client [%d] disconnected\n", sock);
}

int login(int sock)
{
	int type, acc_no, fd, valid=1, invalid=0, login_success=0;
	char password[PASS_LENGTH];
	
	read(sock, &type, sizeof(type));
	if(type == 4) {return 0;}
	read(sock, &acc_no, sizeof(acc_no));
	read(sock, &password, sizeof(password));

	if((fd = open(ACC[type-1], O_RDWR))==-1)printf("File Error\n");


	if(type == 1){
		lseek(fd, (acc_no - 1)*sizeof(struct account), SEEK_CUR);
		struct flock lock;

		lock.l_start = (acc_no-1)*sizeof(struct account);
		lock.l_len = sizeof(struct account);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();	
		struct account temp;
		lock.l_type = F_WRLCK;


		fcntl(fd,F_SETLKW, &lock);
		read(fd, &temp, sizeof(struct account));

		if(temp.id == acc_no){
			if(!strcmp(temp.pass, password)){
				write(sock, &valid, sizeof(valid));
				while(-1!=normal(sock, temp.id, type,fd));
				login_success = 1;
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
		close(fd);
		if(login_success)
		return 3;
	}
	else if(type == 2){
		lseek(fd, (acc_no - 1)*sizeof(struct jaccount), SEEK_CUR);
		struct jaccount temp;
		read(fd, &temp, sizeof(struct jaccount));

		if(temp.id == acc_no){
			if(!strcmp(temp.pass, password)){
				write(sock, &valid, sizeof(valid));
				while(-1!=joint(sock, temp.id, type,fd));
				return 3;
			}
		}
		close(fd);
		if(login_success)
		return 3;
	}
	else if(type == 3){
		
		lseek(fd, (acc_no - 1)*sizeof(struct account), SEEK_CUR);
		struct admin temp;
		read(fd, &temp, sizeof(struct account));
		if(temp.id == acc_no){
			if(!strcmp(temp.pass, password)){
				write(sock, &valid, sizeof(valid));
				while(-1!=admin(sock));
				login_success = 1;
			}
		}
		close(fd);
		if(login_success)
		return 3;
	}
	write(sock, &invalid, sizeof(invalid));
	return 3;
}

int signup(int sock)
{
	int type;
        read(sock, &type, sizeof(type));	//1:customer 2:Agent 3:Admin 4:Back

	if(type == 4){ return 0;}	//when back is chose
	if(type==1)
	{
	int fd, acc_no=0;
	char password[PASS_LENGTH], name[10];
	struct account temp;

	read(sock, &name, sizeof(name));
	read(sock, &password, sizeof(password));

	if((fd = open(ACC[type-1], O_RDWR))==-1){
		printf("File Error\n");
		ERR_EXIT("open()");
	}


	int fp = lseek(fd, 0, SEEK_END);

	if(fp==0){
		temp.id = 1;
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct account), SEEK_CUR);
		read(fd, &temp, sizeof(temp));
		temp.id++;
	}
	strcpy(temp.name, name);
	strcpy(temp.pass, password);
	temp.amount=0;
	write(fd, &temp, sizeof(temp));
	write(sock, &temp.id, sizeof(temp.id));

	close(fd);
	return 3;
	}

	if(type==2)
	{
	int fd, acc_no=0;
	char password[PASS_LENGTH], name[10],name1[10];
	struct jaccount temp;

	read(sock, &name, sizeof(name));
	read(sock, &name1, sizeof(name1));
	read(sock, &password, sizeof(password));

	if((fd = open(ACC[type-1], O_RDWR))==-1){
		printf("File Error\n");
		ERR_EXIT("open()");
	}


	int fp = lseek(fd, 0, SEEK_END);

	if(fp==0){
		temp.id = 1;
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct jaccount), SEEK_CUR);
		read(fd, &temp, sizeof(temp));
		temp.id++;
	}
	strcpy(temp.name, name);
	strcpy(temp.name1, name1);
	strcpy(temp.pass, password);
	temp.amount=0;
	write(fd, &temp, sizeof(temp));
	write(sock, &temp.id, sizeof(temp.id));

	close(fd);
	return 3;
	}
	if(type==3)
	{
	int fd, acc_no=0;
	char password[PASS_LENGTH], name[10];
	struct admin temp;

	read(sock, &name, sizeof(name));
	read(sock, &password, sizeof(password));

	if((fd = open(ACC[type-1], O_RDWR))==-1){
		printf("File Error\n");
		ERR_EXIT("open()");
	}


	int fp = lseek(fd, 0, SEEK_END);

	if(fp==0){
		temp.id = 1;
	}
	else{
		fp = lseek(fd, -1 * sizeof(struct admin), SEEK_CUR);
		read(fd, &temp, sizeof(temp));
		temp.id++;
	}
	strcpy(temp.name, name);
	strcpy(temp.pass, password);
	write(fd, &temp, sizeof(temp));
	write(sock, &temp.id, sizeof(temp.id));

	close(fd);
	return 3;
	}
}

int normal(int sock, int id, int type,int fd)
{
	int op_id;
	read(sock, &op_id, sizeof(op_id));
	if(op_id==1)//withdrwal
	{
		struct account user;		
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		read(fd,&user,sizeof(user));
		int debit,flag;
		read(sock,&debit,sizeof(debit));
		if(debit>user.amount)
		{
			flag=0;
			write(sock,&flag,sizeof(flag));
		}
		else
		{ 
			flag=1;
			write(sock,&flag,sizeof(flag));
			user.amount=user.amount-debit;
			lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
			write(fd,&user,sizeof(user));
		}
		return 1;
	}
	if(op_id==2)//deposit
	{
		struct account user;		
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		read(fd,&user,sizeof(user));
		int credit;
		read(sock,&credit,sizeof(credit));
		//printf("%d       %d\n",user.amount,credit);
		user.amount=user.amount+credit;
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		write(fd,&user,sizeof(user));

		return 2;
	}
	if(op_id==3)//account detalis
	{
		struct account user;		
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		read(fd,&user,sizeof(user));
		
		write(sock,&user.id,sizeof(user.id));
		write(sock,&user.name,sizeof(user.name));
		write(sock,&user.pass,sizeof(user.pass));
		write(sock,&user.amount,sizeof(user.amount));
		return 3;
	}
	if(op_id==4)//account balance
	{
		struct account user;		
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		read(fd,&user,sizeof(user));
		//printf("%d\n",user.amount);
		write(sock,&user.amount,sizeof(user.amount));
		return 4;		
	}
	if(op_id==5)
	{
		struct account user;		
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		read(fd,&user,sizeof(user));
		char pass[20];
		read(sock,&pass,sizeof(pass));
		strcpy(user.pass,pass);
		lseek(fd, (id - 1)*sizeof(struct account), SEEK_SET);
		write(fd,&user,sizeof(user));
		
		return 5;		
	}
	if(op_id == 6) {
		write(sock, &op_id, sizeof(op_id));
		return -1;
	}
	return 0;
}


int joint(int sock, int id, int type,int fd)
{
	int op_id;
	int acc_no=id;
	read(sock, &op_id, sizeof(op_id));
	if(op_id==1)//withdrwal
	{
		struct jaccount user;		
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		struct flock lock;

		lock.l_start = (acc_no-1)*sizeof(struct jaccount);
		lock.l_len = sizeof(struct jaccount);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();	
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW, &lock);

		read(fd,&user,sizeof(user));
		int debit,flag;
		read(sock,&debit,sizeof(debit));
		if(debit>user.amount)
		{
			flag=0;
			write(sock,&flag,sizeof(flag));
		}
		else
		{ 
			flag=1;
			write(sock,&flag,sizeof(flag));
			user.amount=user.amount-debit;
			lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
			write(fd,&user,sizeof(user));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW, &lock);
		return 1;
	}
	if(op_id==2)//deposit
	{
		struct jaccount user;		
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		struct flock lock;

		lock.l_start = (acc_no-1)*sizeof(struct jaccount);
		lock.l_len = sizeof(struct jaccount);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();	
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW, &lock);

		read(fd,&user,sizeof(user));
		int credit;
		read(sock,&credit,sizeof(credit));
		//printf("%d       %d\n",user.amount,credit);
		user.amount=user.amount+credit;
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		write(fd,&user,sizeof(user));

		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW, &lock);

		return 2;
	}
	if(op_id==3)//account detalis
	{
		struct jaccount user;		
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		read(fd,&user,sizeof(user));
		
		write(sock,&user.id,sizeof(user.id));
		write(sock,&user.name,sizeof(user.name));
		write(sock,&user.name1,sizeof(user.name1));
		write(sock,&user.pass,sizeof(user.pass));
		write(sock,&user.amount,sizeof(user.amount));
		return 3;
	}
	if(op_id==4)//account balance
	{
		struct jaccount user;		
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		read(fd,&user,sizeof(user));
		//printf("%d\n",user.amount);
		write(sock,&user.amount,sizeof(user.amount));
		return 4;		
	}
	if(op_id==5)
	{
		struct jaccount user;		
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		struct flock lock;

		lock.l_start = (acc_no-1)*sizeof(struct jaccount);
		lock.l_len = sizeof(struct jaccount);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();	
		lock.l_type = F_WRLCK;
		fcntl(fd,F_SETLKW, &lock);

		read(fd,&user,sizeof(user));
		char pass[20];
		read(sock,&pass,sizeof(pass));
		strcpy(user.pass,pass);
		lseek(fd, (id - 1)*sizeof(struct jaccount), SEEK_SET);
		write(fd,&user,sizeof(user));
		
		lock.l_type = F_UNLCK;
		fcntl(fd,F_SETLKW, &lock);

		return 5;		
	}
	if(op_id == 6) {
		write(sock, &op_id, sizeof(op_id));
		return -1;
	}
	return 0;
}

int admin(int sock)
{
	int op_id;
	read(sock, &op_id, sizeof(op_id));

	if(op_id==1){
		int fd1,fd2,flag=1;
		fd1 = open(ACC[0], O_RDONLY);
		lseek(fd1,0,SEEK_SET);
		struct account temp;
		while(read(fd1,&temp,sizeof(temp)))
		{

			write(sock,&flag,sizeof(flag));
			write(sock,&temp,sizeof(temp));
			//write(sock,&temp.id,sizeof(temp.id));
			//write(sock,&temp.name,sizeof(temp.name));
			//write(sock,&temp.pass,sizeof(temp.pass));
			//write(sock,&temp.amount,sizeof(temp.amount));
			//printf("%d\t%s\t%s\t%d\n",temp.id,temp.name,temp.pass,temp.amount);

		}
		close(fd1);
		flag=0;
		write(sock,&flag,sizeof(flag));
		flag=1;
		fd2 = open(ACC[1], O_RDONLY);
		lseek(fd2,0,SEEK_SET);
		struct jaccount user;
		while(read(fd2,&user,sizeof(user)))
		{
			write(sock,&flag,sizeof(flag));
			write(sock,&user,sizeof(user));
			//write(sock,&user.id,sizeof(user.id));
			//write(sock,&user.name,sizeof(user.name));
			//write(sock,&user.name1,sizeof(user.name1));
			//write(sock,&user.pass,sizeof(user.pass));
			//write(sock,&user.amount,sizeof(user.amount));
			//printf("%d\t%s\t%s\t%s\t%d\n",user.id,user.name,user.name1,user.pass,user.amount);
		}
		close(fd2);
		flag=0;
		write(sock,&flag,sizeof(flag));
		return 1;
	}
	if(op_id == 2) {
		write(sock, &op_id, sizeof(op_id));
		return -1;
	}
	return 0;
}
