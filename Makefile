src = ./src/*.cpp ./inc/*.h
CC = g++
INC = -I./inc/

LDFLAGS = -lpthread -lrt -lsqlite3 -L./lib/linux_x64 -larcsoft_fsdk_face_detection -larcsoft_fsdk_face_recognition
LDFLAGS += -Wl,-rpath=./lib/linux_x64
CPPFLAG = -std=c++11 -g

arc_test : $(src)
	$(CC) -o $@ $(src) $(INC) $(CPPFLAG) $(LDFLAGS) $(shell pkg-config --cflags --libs opencv)
	echo "compile done"

.PHONY:clean test
clean:
	rm arc_test
test:
	echo "hi,test"
