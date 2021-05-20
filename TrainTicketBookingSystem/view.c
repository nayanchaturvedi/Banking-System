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
#define TRAIN "./db/train"
#define BOOKING "./db/booking"
#define USER_CUSTOMER "./db/accounts/customer"
#define USER_ADMIN "./db/accounts/admin"
#define USER_AGENT "./db/accounts/agent"
#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

struct account{
	int id;
	char name[10];
	char pass[PASS_LENGTH];
};

struct train{
	int tid;
	char train_name[20];
	int train_no;
	int av_seats;
	int last_seatno_used;
};

struct bookings{
	int bid;
	int type;
	int acc_no;
	int tr_id;
	char trainname[20];
	int seat_start;
	int seat_end;
	int cancelled;
};

char *ACC[3] = {"./db/accounts/customer", "./db/accounts/agent", "./db/accounts/admin"};
int view_all();

int main()
{
    printf("Here\n");
    while(view_all()!=4);
}

int view_all(){
    int opt;
    system("clear");
	printf("+++++++ View ++++++++\n");
	printf("1. Bookings\n");
	printf("2. Trains\n");
	printf("3. Users\n");
	printf("4. Exit\n");
	printf("+++++Enter Your Choice++++++\n");
	scanf("%d", &opt);
    int fd,fp;
    printf("--------------------------------------------------\n");
    printf("--------------------------------------------------\n");
    switch (opt)
    {
    case 1:
        fd = open(BOOKING,O_RDONLY);
        fp = lseek(fd,0,SEEK_SET);
        struct bookings book;
        printf("BookingID\tUserID\tType\t1st Ticket\tLast Ticket\tTrainId\tTrainName\tisCancelled\n");
        while(read(fd,&book,sizeof(book)))
            printf("%d\t\t%d\t%d\t%d\t\t%d\t\t%d\t%s\t\t%d\n",book.bid,book.acc_no,book.type,book.seat_start,book.seat_end,book.tr_id,book.trainname,book.cancelled);
        close(fd);
        break;
    
    case 2:
        fd = open(TRAIN,O_RDONLY);
        fp = lseek(fd,0,SEEK_SET);
        struct train tr;
        printf("ID\tT_NO\tAV_SEAT\tTRAIN NAME\tLastSeatNoUsed\n");
        while(read(fd,&tr,sizeof(tr)))
            printf("%d\t%d\t%d\t%s\t\t%d\n",tr.tid,tr.train_no,tr.av_seats,tr.train_name,tr.last_seatno_used);
        close(fd);
        break;
    case 3:
        printf("1. Customer\n");
        printf("2. Agents\n");
        printf("3. Admins\n");
        printf("+++++Enter Your Choice++++++\n");
        int opt2;
        struct account acc;
        scanf("%d", &opt2);
        printf("ID\t\tName\n");
        switch (opt2)
        {
        case 1:
            fd = open(USER_CUSTOMER,O_RDONLY);
            fp = lseek(fd,0,SEEK_SET);
            while(read(fd,&acc,sizeof(acc)))
                printf("%d\t\t%s\n",acc.id,acc.name);
            close(fd);
            break;
        case 2:
            fd = open(USER_AGENT,O_RDONLY);
            fp = lseek(fd,0,SEEK_SET);
            while(read(fd,&acc,sizeof(acc)))
                printf("%d\t\t%s\n",acc.id,acc.name);
            close(fd);
            break;
        case 3:
            fd = open(USER_ADMIN,O_RDONLY);
            fp = lseek(fd,0,SEEK_SET);
            while(read(fd,&acc,sizeof(acc)))
                printf("%d\t\t%s\n",acc.id,acc.name);
            close(fd);
            break;
        default:
            printf("Invalid Choice\n");
            break;
        }        
        break;
    default:
        return 4;
        break;
    }
    getchar();
    getchar();
    return opt;

}