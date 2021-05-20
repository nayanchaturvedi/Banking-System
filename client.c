#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8090
#define PASS_LENGTH 20
#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

int bank(int sock);
int do_normal_action(int sock, int opt);
int menu(int sock, int type);
int do_joint_action(int sock, int opt);
int do_admin_action(int sock, int opt);


int main(){
	char *ip = "127.0.0.1";
	
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd == -1){
		printf("socket creation failed\n");
		exit(0);
	}
	struct sockaddr_in ca;
	ca.sin_family=AF_INET;
	ca.sin_port= htons(PORT);
	ca.sin_addr.s_addr = inet_addr(ip);
	if(connect(client_fd, (struct sockaddr *)&ca, sizeof(ca))==-1){
		printf("connect failed\n");
		exit(0);
	}
	printf("connection established\n");

	while(bank(client_fd)!=3);
	//while(1);
	close(client_fd);

	return 0;
}

int bank(int sock)
{
	int opt;
	system("clear");
	printf("---------------------------------------------------\n");
	printf("-------------------BANK-MANAGMENT------------------\n");
	printf("1 for Sign In\n");
	printf("2 for Sign Up\n");
	printf("3 for Exit\n");
	printf("-----------------Enter Your Choice-----------------\n");
	scanf("%d", &opt);
	
	while(opt > 3 || opt < 1)
	{
		printf("Invalid Choice!\n");
		printf("-----------------Enter Your Choice-----------------\n");
		scanf("%d", &opt);
	}

	write(sock, &opt, sizeof(opt));

	if(opt==1){
		int type, acc_no;
		char password[PASS_LENGTH];
		printf("Enter type of the account:\n");
		printf("1 for Individual\n2 for Joint\n3 for Admin\n4 for Back\n");
		printf("Enter Your Choice: ");
		scanf("%d", &type);

		while(type > 4 || type < 1)
		{
			printf("Invalid Choice!\n");
			printf("Your Choice: ");
			scanf("%d", &type);
		}

		write(sock, &type, sizeof(type));
		if(type == 4)
		{
			//go back to main menu
			//returning !=3 will result in calling bank() again
			//write(sock,&type,sizeof(type));
			return 0;
		}

		printf("Enter Your Account Number: ");
		scanf("%d", &acc_no);
		strcpy(password,getpass("Enter password: "));

		write(sock, &acc_no, sizeof(acc_no));
		write(sock, &password, strlen(password));
		
		int valid_login;
		read(sock, &valid_login, sizeof(valid_login));
		if(valid_login == 1){
			while(menu(sock, type)!=-1);
			system("clear");
			return 1;
		}
		else{
			printf("Login Failed\n");
			while(getchar()!='\n');
			getchar();
			return 1;
		}
	}
	else if(opt==2){
		int type, acc_no;
		char password[PASS_LENGTH], secret_pin[5], name[10],name1[10];
		printf("Enter the type of account :\n");
		printf("1 - Individual\n2 - Joint\n3 - Admin\n4 - Back\n");
		printf("Your Choice: ");
		scanf("%d", &type);

		while(type > 4 || type < 1)
		{
			printf("Invalid Choice!\n");
			printf("Your Choice : ");
			scanf("%d", &type);
		}

		if(type == 4){
			//go back to main menu
			//returning !=3 will result in calling irctc() again
			write(sock, &type, sizeof(type));
			return 0;
		}
		printf("Enter your name : ");scanf("%s", name);
		if(type==2)
		{
		printf("Enter other user's name : ");scanf("%s", name1);
		}
		strcpy(password,getpass("Enter password: "));
		if(type == 3){
			int attempt = 1;
			while(1){
				strcpy(secret_pin, getpass("Enter secret ADMIN PIN : "));attempt++;
				if(strcmp(secret_pin, "admin")!=0 && attempt<=3) printf("Invalid PIN. Please try again.\n");
				else break;
			}
			if(!strcmp(secret_pin, "admin"));
			else exit(0);
		}
		write(sock, &type, sizeof(type));
		write(sock, &name, sizeof(name));
		if(type==2)
		{
		write(sock, &name1, sizeof(name1));
		}
		write(sock, &password, strlen(password));

		read(sock, &acc_no, sizeof(acc_no));
		printf("Your account No. is : %d.\nRemember the account no. for further login\n", acc_no);
		while(getchar()!='\n');
		getchar();
		return 2;
	}
	else if(opt == 3)
	{	return 3;}

}

int menu(int sock, int type){
	int opt = 0;
	if(type == 1){
		system("clear");
		printf("----- OPTIONS -----\n");
		printf("1. Withdrawl\n");
		printf("2. Deposit\n");
		printf("3. View Account Details\n");
		printf("4. View Balance\n");
		printf("5. Change Password\n");
		printf("6. Logout\n");
		printf("Your Choice: ");
		scanf("%d", &opt);
		return do_normal_action(sock, opt);
		return -1;
	}
	else if(type==2)
	{
		system("clear");
		printf("----- OPTIONS -----\n");
		printf("1. Withdrawl\n");
		printf("2. Deposit\n");
		printf("3. View Account Details\n");
		printf("4. View Balance\n");
		printf("5. Change Password\n");
		printf("6. Logout\n");
		printf("Your Choice: ");
		scanf("%d", &opt);
		return do_joint_action(sock, opt);
		return -1;
	}
	else
	{
		system("clear");
		printf("----- OPTIONS -----\n");
		printf("1. Details about all the users\n");
		printf("2. Logout\n");

		printf("Your Choice: ");
		scanf("%d", &opt);
		return do_admin_action(sock, opt);
		return-1;
	}
}

int do_normal_action(int sock, int opt){
	write(sock, &opt, sizeof(opt));
	switch(opt){
		case 1:{
			int amount,flag;
			printf("Enter the amount to be withdrawn: ");
			scanf("%d",&amount);
			write(sock,&amount,sizeof(amount));
			read(sock,&flag,sizeof(flag));
			if(flag==1)
			{
				printf("Transaction Successfull.....\nPress enter to continue\n");
				getchar();
				getchar();
			}
			else if(flag==0)
			{
				printf("Insufficient amount....\nPress enter to continue\n");
				getchar();
				getchar();
			}
			return 1;
		}
		case 2:{
			int amount;
			printf("Enter the amount to be deposited: ");
			scanf("%d",&amount);
			write(sock,&amount,sizeof(amount));
			
			printf("Transaction Successfull.....\nPress enter to continue\n");
			getchar();
			getchar();

			return 2;
		}
		case 3:{
			int id, amount;
			char name[10],pass[20];
			read(sock,&id,sizeof(id));
			read(sock,&name,sizeof(name));
			read(sock,&pass,sizeof(pass));
			read(sock,&amount,sizeof(amount));
			
			printf("Individual Account\nAccount Number: %d\nName: %s\nPassword: %s\nAmount: %d\nPress enter to continue",id,name,pass,amount);
			getchar();
			getchar();
			return 3;
		}
		case 4:{
			int amount;
			read(sock,&amount,sizeof(amount));
			printf("Available balance: %d\nPress enter to continue",amount);
			getchar();
			getchar();
			return 4;
		}
		case 5:{
			char new[20];
			strcpy(new,getpass("Enter new password: "));
			write(sock,&new,sizeof(new));

			printf("Password is Updated.....\nPress enter to continue");
			getchar();
			getchar();
			return 5;
		}
		case 6:{
			read(sock, &opt, sizeof(opt));
			if(opt == 6) printf("Logged out successfully.\n");
			while(getchar()!='\n');
			getchar();
			return -1;
			break;			
		}
                default: return -1;
	}
}

int do_joint_action(int sock, int opt){
	write(sock, &opt, sizeof(opt));
	switch(opt){
		case 1:{
			int amount,flag;
			printf("Enter the amount to be withdrawn: ");
			scanf("%d",&amount);
			write(sock,&amount,sizeof(amount));
			read(sock,&flag,sizeof(flag));
			if(flag==1)
			{
				printf("Transaction Successfull.....\nPress enter to continue\n");
				getchar();
				getchar();
			}
			else if(flag==0)
			{
				printf("Insufficient amount....\nPress enter to continue\n");
				getchar();
				getchar();
			}
			return 1;
		}
		case 2:{
			int amount;
			printf("Enter the amount to be deposited: ");
			scanf("%d",&amount);
			write(sock,&amount,sizeof(amount));
			
			printf("Transaction Successfull.....\nPress enter to continue\n");
			getchar();
			getchar();

			return 2;
		}
		case 3:{
			int id, amount;
			char name[10],pass[20],name1[10];
			read(sock,&id,sizeof(id));
			read(sock,&name,sizeof(name));
			read(sock,&name1,sizeof(name1));
			read(sock,&pass,sizeof(pass));
			read(sock,&amount,sizeof(amount));
			
			printf("Individual Account\nAccount Number: %d\nName1: %s\nName2: %s\nPassword: %s\nAmount: %d\nPress enter to continue",id,name,name1,pass,amount);
			getchar();
			getchar();
			return 3;
		}
		case 4:{
			int amount;
			read(sock,&amount,sizeof(amount));
			printf("Available balance: %d\nPress enter to continue",amount);
			getchar();
			getchar();
			return 4;
		}
		case 5:{
			char new[20];
			strcpy(new,getpass("Enter new password: "));
			write(sock,&new,sizeof(new));

			printf("Password is Updated.....\nPress enter to continue");
			getchar();
			getchar();
			return 5;
		}
		case 6:{
			read(sock, &opt, sizeof(opt));
			if(opt == 6) printf("Logged out successfully.\n");
			while(getchar()!='\n');
			getchar();
			return -1;
			break;			
		}
                default: return -1;
	}
}

int do_admin_action(int sock, int opt)
{
struct acc{
char name[10];
int id;
int amount;
char pass[20];
} temp;
struct jacc{
char name[10];
int id;
char name1[10];
int amount;
char pass[20];
} user;
	write(sock, &opt, sizeof(opt));
		switch(opt){
		case 1:{
			printf("-----------Individual Account Detalis-----------\n");
			printf("________________________________________________\n");
			printf("ACCOUNT NO.\tNAME\tPASSWORD\tAMOUNT\n");
			int flag,amount,id;
			char name[10],pass[10];

			while(1)
			{
				read(sock,&flag,sizeof(flag));
				if(flag==0)
				break;
				read(sock,&temp,sizeof(temp));
				//read(sock,&id,sizeof(id));
				//read(sock,&name,sizeof(name));
				//read(sock,&pass,sizeof(pass));
				//read(sock,&amount,sizeof(amount));
				
			printf("\t%d\t%s\t%s\t\t%d\n",temp.id,temp.name,temp.pass,temp.amount);
			}
			printf("#########################################################\n");
			
			flag=1;
			printf("------------------Joint Account Detalis------------------\n");
			printf("_________________________________________________________\n");
			printf("ACCOUNT NO.\tNAME1\tNAME2\tPASSWORD\tAMOUNT\n");
			char name1[10];
			while(1)
			{
				read(sock,&flag,sizeof(flag));
				if(flag==0)
				break;
				read(sock,&user,sizeof(user));
				//read(sock,&id,sizeof(id));
				//read(sock,&name,sizeof(name));
				//read(sock,&name1,sizeof(name1));
				//read(sock,&pass,sizeof(pass));
				//read(sock,&amount,sizeof(amount));
				
			printf("\t%d\t%s\t%s\t%s\t\t%d\n",user.id,user.name,user.name1,user.pass,user.amount);
			}
			printf("_________________________________________________________\n");
			getchar();
			getchar();
			return 1;
		}
		case 2:{
			read(sock, &opt, sizeof(opt));
			if(opt == 2) printf("Logged out successfully.\n");
			while(getchar()!='\n');
			getchar();
			return -1;
			break;			
		}
		default: return -1;		
	}
}
