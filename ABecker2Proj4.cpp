/******************************************************************************
//
// @file ABecker2Proj4.cpp
// @author Aidan Becker
// @islandID ABecker2
// @professor Dulal Kar
// @class Operating Systems
// @version 1.0
// Project #4: Process Synchronization Using Pthreads: 
//			   The Producer / Consumer Problem With Prime Number Detector 
//
******************************************************************************/


/******************************************************************************
//
//	Includes and Defines
//
******************************************************************************/
#include <string>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <vector>

#ifndef _BUFFER_H_DEFINED_
#define _BUFFER_H_DEFINED_
typedef int buffer_item;
#define BUFFER_SIZE 5
bool buffer_insert_item( buffer_item item );
bool buffer_remove_item( buffer_item *item );
#endif //_BUFFER_H_DEFINED_

using namespace std;

/******************************************************************************
//
//	Global variables
//
******************************************************************************/
//Where the buffer_insert_item() function will place an item
int buffer_in;
//Where the buffer_remove_item() function will get an item
int buffer_out;
//The buffer itself
buffer_item buff[BUFFER_SIZE];
//seed for random numbers
unsigned int seed;
//flag to tell threads when to stop running
bool sim = true;
//flag to output runtime data
bool verbose_output;
//number of times the buffer was full/empty
long int timesEmpty;
long int timesFull;
//amount of time threads will wait if buffer was empty/full (milliseconds)
int thread_wait_time;

//Holds the number of items produced/consumed by a thread
int *produced;
int *consumed;

pthread_attr_t attr;
//mutual exclusion lock
pthread_mutex_t mutex;
//semaphores
sem_t full;
sem_t empty;



/******************************************************************************
//	isPrime function
//	Returns true if an integer is prime
//	@param int num
//	@return bool
******************************************************************************/
bool isPrime(int num)
{
	bool flag = true;
	for(int i = 2; i <= num/2; i++)
	{
		if(num% i == 0)
		{
			flag = false;
			break;
		}
	}
	return flag;
}

/******************************************************************************
//	initialize function
//	Initializes buffer, timesEmpty and timesFull, along with pthread_attribute,
//	mutex lock, and semaphores.
//	@param none
//	@return none
******************************************************************************/
void initialize()
{
	buffer_in = 0;
	buffer_out = 0;
	timesEmpty = 0;
	timesFull = 0;
	
	pthread_attr_init(&attr);
	pthread_mutex_init(&mutex, NULL);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, BUFFER_SIZE);
}

/******************************************************************************
//	buffer_insert_item function
//	Inserts an item into the buffer at location buffer_in, if the space is free.
//	Returns true if the operation was successful, false otherwise
//	Buffer logic referenced from http://www.cs.fsu.edu/~baker/realtime/restricted/notes/prodcons.html
//	@param buffer_item(integer) item
//	@return bool
******************************************************************************/
bool buffer_insert_item( buffer_item item)
{
	if( (buffer_in + 1) % BUFFER_SIZE == buffer_out )
	{
		timesFull++;
		return false; 
	} 
	buff[buffer_in] = item;
	buffer_in = (buffer_in + 1) % BUFFER_SIZE;
	
	return true;
}

/******************************************************************************
//	buffer_remove_item function
//	Removes an item from the buffer at location buffer_in, if the space is free.
//	Returns true if the operation was successful, false otherwise
//	Buffer logic referenced from http://www.cs.fsu.edu/~baker/realtime/restricted/notes/prodcons.html
//	@param *buffer_item(integer) item
//	@return bool
******************************************************************************/
bool buffer_remove_item( buffer_item *item)
{
	if( buffer_in == buffer_out )
	{ 
		timesEmpty++;
		return false; 
	}
	
	*item = buff[buffer_out];
	buffer_out = (buffer_out + 1) % BUFFER_SIZE;
	return true;
}

/******************************************************************************
//	buffers_occupied function
//	Returns the number of items in use in the buffer
//	@param none
//	@return int
******************************************************************************/
int buffers_occupied()
{
	if(buffer_in >= buffer_out)
	{
		return buffer_in - buffer_out;
	}
	return buffer_out - buffer_in;
}

/******************************************************************************
//	print_buffer_stats function
//	Prints stats about items produced/consumed
//	@param none
//	@return none
******************************************************************************/
void print_buffer_stats()
{
	cout << "(buffers occupied: " << buffers_occupied() << ")" << endl;
	cout << "buffers:   ";
	for(int i = 0; i < BUFFER_SIZE; i++)
	{
		cout << buff[i] << "   ";
	}
	cout << endl << "          ---- ---- ---- ---- ----" << endl << "           ";
	for(int i = 0; i < BUFFER_SIZE; i++)
	{
		if(buffer_in == buffer_out)
		{
			if(i+1 % BUFFER_SIZE == buffer_out)
			{
				cout << "RW   ";
			}
			else if(i+1 % BUFFER_SIZE == buffer_in)
			{
				cout << "WR   ";
			}
		}
		else if((i+1 % BUFFER_SIZE) == buffer_in)
		{
			cout << "W    ";
		}
		else if((i+1 % BUFFER_SIZE) == buffer_out)
		{
			cout << "R    ";
		}
		else
		{
			cout << "     ";
		}
	}
	cout << endl << endl;
}


/******************************************************************************
//	producer function
//	Alternates between sleeping for a random period of time no longer than 
//	thread_wait_time milliseconds and generating a random number into the buffer.
//	@param void *param (unused)
//	@return none
******************************************************************************/
void *producer(void *param)
{
	//initial timespec declaration for nanosleep
	struct timespec sleepTime = {0};
	sleepTime.tv_sec = 0;
	
	//specific identity of this consumer thread
	int threadNo = param;
	
	//the number of times the buffer was full and the thread attempted to insert an item
	int prodCount = 0;
	
	//loop until main tells us to stop
	while(sim)
	{
		//sleep for a random period of time no longer than thread_wait_time milliseconds
		sleepTime.tv_nsec = (rand_r(&seed) % thread_wait_time) *1000L;
		nanosleep(&sleepTime, (struct timespec *)NULL);
		
		//produce a random integer
		buffer_item next_produced = rand_r(&seed) % 100;
		
		//wait on empty
		sem_wait(&empty);
		//get mutex lock
		pthread_mutex_lock(&mutex);
		
		//enter critical section
		if( buffer_insert_item(next_produced) )
		{
			if(verbose_output)
			{
				cout << "Producer " << pthread_self() << " writes " << next_produced << endl;
				print_buffer_stats();
			}
			produced[threadNo]++;
		}
		//unlock mutex
		pthread_mutex_unlock(&mutex);
		//signal full
		sem_post(&full);
		//leave the critical section
	}

	sem_post(&full);
	sem_post(&empty);
}

/******************************************************************************
//	producer function
//	Alternates between sleeping for a random period of time no longer than 
//	thread_wait_time milliseconds and pulling a random number from the buffer.
//	determines if the buffer item was prime or not.
//	@param void *param (unused)
//	@return none
******************************************************************************/
void *consumer(void *param)
{
	//variable to hold buffer item
	buffer_item next_consumed;
	
	//specific identity of this consumer thread
	int threadNo = param;
	
	//sleep for a random period of time no longer than thread_wait_time milliseconds
	struct timespec sleepTime = {0};
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = (rand_r(&seed) % thread_wait_time) *1000L;
	
	//the number of times the buffer was empty and the thread attempted to remove an item
	int consCount = 0;
	
	//loop until main tells us to stop
	while(sim)
	{
		//sleep for a random period of time no longer than thread_wait_time milliseconds
		nanosleep(&sleepTime, (struct timespec *)NULL);
		
		//wait on full
		sem_wait(&full);
		//get mutex lock
		pthread_mutex_lock(&mutex);
		
		//enter critical section
		if( buffer_remove_item(&next_consumed) )
		{
			if(verbose_output)
			{
				cout << "Consumer " << pthread_self() << " reads " << next_consumed;
				if(isPrime(next_consumed))
				{
					cout << "   * * * PRIME * * *" << endl;
				}
				else
				{
					cout << endl;
				}
				print_buffer_stats();
			}
			consumed[threadNo]++;		
		}

		//unlock mutex
		pthread_mutex_unlock(&mutex);
		//signal empty
		sem_post(&empty);	
		//leave the critical section
	}
	
	sem_post(&empty);
	sem_post(&full);
}

int main( int argc, char *argv[] )
{
	//Get command line arguments
	
	if( argc != 6 )
	{
		fprintf(stderr, "Incorrect number of arguments. Requires 4 integer arguments.\n");
		return -1;
	}
	//Argv[0] will be ./proj4
	int sim_time = atoi(argv[1]);
	thread_wait_time = atoi(argv[2]);
	int producer_thread_count = atoi(argv[3]);
	int consumer_thread_count = atoi(argv[4]);
	verbose_output = false;
	
	//total number of items produced
	int totalProduced = 0;
	int totalConsumed = 0;
	
	if( !strcmp(argv[5], "yes") )
	{
		verbose_output = true;
	}
	
	//initialize semaphores, mutext, and pthread attribute variable
	initialize();
	
	pthread_t *producer_tid = new pthread_t[producer_thread_count];
	pthread_t *consumer_tid = new pthread_t[consumer_thread_count];
	produced = new int[producer_thread_count];
	consumed = new int[consumer_thread_count];	
	
	cout << "Starting Threads..." << endl;
	
	//Create producer thread(s)
	bool exit = false;
	int counter = 0;
	for(int i = 0; i < producer_thread_count; i++)
	{
		if(pthread_create(&producer_tid[i], &attr, producer, i) != 0)
		{
			fprintf(stderr, "couldn't created the requested number of threads, exiting");
			return -1;
		}
	}
	exit = false;
	counter = 0;
	//Create consumer thread(s)
	for(int i = 0; i < consumer_thread_count; i++)
	{
		//need to guarantee a thread was created
		if(pthread_create(&consumer_tid[i], &attr, consumer, i) != 0)
		{
			fprintf(stderr, "couldn't created the requested number of threads, exiting");
			return -1;
		}
	}
	
	
	//Sleep until threads are done
	struct timespec sleepTime = {0};
	sleepTime.tv_sec = sim_time;
	nanosleep(&sleepTime, (struct timespec *)NULL);
	
	//after sleeping the correct amount of time, tell threads to stop
	sim = false;

	//Join Threads
	for(int i = 0; i < producer_thread_count; i++)
	{
		cout << "joining producer" << endl;
		pthread_join(producer_tid[i], NULL);
	}
	for(int i = 0; i < consumer_thread_count; i++)
	{
		cout << "joining consumer" << endl;
		pthread_join(consumer_tid[i], NULL);
	}
	
	sleepTime.tv_sec = 1;
	nanosleep(&sleepTime, (struct timespec *)NULL);

	//output runtime stats
	//get number of items produced/consumed
	for(int i = 0; i < producer_thread_count; i++)
	{
		totalProduced += produced[i];
	}
	for(int i = 0; i < consumer_thread_count; i++)
	{
		totalConsumed += consumed[i];
	}

	//Display Statistics
	cout << "PRODUCER / CONSUMER SIMULATION COMPLETE" << endl 
		 << "=======================================" << endl
		 << "Simulation Time:                       " << sim_time << endl
		 << "Maximum Thread Sleep Time:             " << thread_wait_time << endl
		 << "Number of Producer Threads:            " << producer_thread_count << endl
		 << "Number of Consumer Threads:	       " << consumer_thread_count << endl
		 << "Size of Buffer:                        " << BUFFER_SIZE << endl
		 << "Total Number of Items Produced:        " << totalProduced << endl;
	for(int i = 0; i < producer_thread_count; i++)
	{
		cout
		 << "  Thread " << i << ":        	" << produced[i] << endl;
	}
	cout << "Total Number of Items Consumed:		" << totalConsumed << endl;
	for(int i = 0; i < consumer_thread_count; i++)
	{
		cout
		 << "  Thread " << i << ":         	" << consumed[i] << endl;
	}
	cout << "Number Of Items Remaining in Buffer:   " << totalProduced - totalConsumed << endl
		 << "Number of Times Buffer Was Full:       " << timesFull << endl
		 << "Number Of Times Buffer Was Empty:      " << timesEmpty << endl;
	
	return(0);
}


