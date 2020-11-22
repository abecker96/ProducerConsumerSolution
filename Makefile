###########################################################
#
# Simple Makefile for Operating Systems Project 4
# proj4
#
###########################################################
.SUFFIXES: .h .c .cpp .l .o
	
proj4:
	g++ ABecker2Proj4.cpp -std=c++11 -fpermissive -lpthread -o proj4	

clean:
	/bin/rm -f *.o *~ core proj4