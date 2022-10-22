How to run:
1.Tracker:
g++ tracker.cpp -pthread -o tracker
./tracker tracker_info.txt

2.Client:
g++ client.cpp sha1.cpp -pthread -o client
./client 127.0.0.1:5555 tracker_info.txt
