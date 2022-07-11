#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
// #include "curr_time.h"

static int X;
static int flag;
static char T[60];
static int start = 1;
static float offset;


FILE *fp;
FILE *fp_X;
FILE *fp_offset;

pthread_mutex_t mtx_sample = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t  condition_sample = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mtx_input = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t  condition_input = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mtx_logging = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t  condition_logging = PTHREAD_COND_INITIALIZER;

static int sample_flag = 0;
static int input_flag = 0;
static int logging_flag = 0;

struct timespec ts = {0,0};
struct timespec ts_previous = {0,0};


void *Sample_function(void * SAMPLE){

   // check if X frequency changed (Input)
   pthread_mutex_lock(&mtx_input);
   while(input_flag ==0){
      pthread_cond_wait(&condition_input, &mtx_input);
   }
   pthread_mutex_unlock(&mtx_input);


   // Read current date and time of the system

   // timespec_get(&ts, TIME_UTC);
   clock_gettime(CLOCK_REALTIME, &ts);
   char buff[100];
   strftime(buff, sizeof buff, "%F %T", gmtime(&ts.tv_sec));
   printf("Time: %s.%09ld  --offset: ", buff, ts.tv_nsec);

   // Save date and time value to T variable
   snprintf(T, sizeof T, "%s.%09ld offset: %.9f \n", buff, ts.tv_nsec, offset);

   // return flag for Input
   input_flag = 0;

   // Create signal for LOGGING thread 
   pthread_mutex_lock(&mtx_sample);
   sample_flag = 1;
   pthread_cond_signal(&condition_sample);
   pthread_mutex_unlock(&mtx_sample);
   
   // sleep for X nanoseconds
   nanosleep((const struct timespec[]){{0, X}}, NULL);
}

void *Logging_function(void *LOGGING){
   pthread_mutex_lock(&mtx_sample);
   while(sample_flag ==0){
      pthread_cond_wait(&condition_sample, &mtx_sample);
   }
   pthread_mutex_unlock(&mtx_sample);

   // Calculate offset
   offset = ((double)ts.tv_sec + 1.0e-9*ts.tv_nsec) - ((double)ts_previous.tv_sec + 1.0e-9*ts_previous.tv_nsec);
   printf("%.9f seconds\n", offset);

   fp_offset = fopen("offset_data.txt","a");
   if (fp_offset){
      fprintf(fp_offset, "%.9f\n", offset);
   }
   else{
      printf("Failed to open the file \n");
   }
   fclose(fp_offset);

   ts_previous.tv_sec = ts.tv_sec;
   ts_previous.tv_nsec = ts.tv_nsec; 

   // Save date and time to "time_and_interval.txt"
   fp = fopen("time_and_interval.txt","a");
   if (fp){
      fputs(T,fp);
   }
   else{
      printf("Failed to open the file \n");
   }
   fclose(fp);
   
   // pthread_mutex_lock(&mtx_sample2);
   // logging_flag = 1;
   sample_flag = 0;
   // pthread_cond_signal(&cond2);
   // pthread_mutex_unlock(&mtx_sample2);

}

void *Input_function(void *INPUT){

   // Read input from freq.txt 
   fp_X = fopen("freq.txt","r");
   fscanf (fp_X, "%d", &X);

   // Notify Sample thread about X frequency
   pthread_mutex_lock(&mtx_input);
   input_flag = 1;
   pthread_cond_signal(&condition_input);
   pthread_mutex_unlock(&mtx_input);

   // printf("%d \n",X);
   fclose(fp_X);
}

int main(int argc, char** argv) { 
   pthread_t SAMPLE, LOGGING, INPUT;
   int X;
   int  iret1, iret2, iret3;

   // fp = fopen("time_and_interval.txt","w");
   printf("X frequency: %d", X);

   // Create independent threads each of which will execute function   //
   while (1){
   iret1 = pthread_create( &INPUT, NULL, Input_function, (void*) &INPUT);
   iret2 = pthread_create( &SAMPLE, NULL, Sample_function, (void *)&SAMPLE);  
   iret3 = pthread_create( &LOGGING, NULL, Logging_function, (void*) &LOGGING);

   // Wait till threads are complete before main continues. Unless we  //
   // wait we run the risk of executing an exit which will terminate   //
   // the process and all threads before the threads have completed.   //

   pthread_join( INPUT, NULL);
   pthread_join( SAMPLE, NULL);
   pthread_join( LOGGING, NULL);
   }  

   exit(0);
}

