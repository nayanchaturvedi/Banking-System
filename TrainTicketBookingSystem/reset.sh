rm -rf db
mkdir db
mkdir db/accounts
touch db/train
touch db/booking
touch db/accounts/customer
touch db/accounts/agent
touch db/accounts/admin
gcc client.c -o client
gcc server.c -o server
gcc view.c -o view
