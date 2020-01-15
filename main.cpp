#include <iostream>
#include <pthread.h>
#include <stdio.h>						//////////EMRE BENER/////////
#include <unistd.h>						/////////////23818///////////
#include <string>
#include <stdlib.h>
#include <queue>
#include <cstdlib>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

#define NUM_THREADS 10
#define M 100

char memory[M] = {'X'};
pthread_t server; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores
int thread_message[NUM_THREADS];
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER;
bool run=true;


struct Node
{
	int id;
	int size;
	int index;
	Node *next;
	Node();
	Node(int listID,int listSize,int listIndex,Node *next):id(listID),size(listSize),index(listIndex),next(NULL){}
};
Node* headPtr;

struct myqueue
{
	int id;
	int size;
	myqueue(int a,int b): id(a),size(b){}
};

queue<myqueue>requestQueue;

void free_mem(Node*& headPtr)
{
	if(headPtr->next != NULL)
    {
        Node* prev = headPtr;
        Node* curr = headPtr->next;
        while(curr != NULL)
        {
            if(prev->id == curr->id)
            {
                prev->size += curr->size;
                prev->next = curr->next;
                delete curr;
                break;
            }
            prev = prev->next;
            curr = curr->next;
        }
    }
}

void use_mem()
{
	int sleepTime = (rand()%7 )+1;
	sleep(sleepTime);
}

void my_malloc(int& ID, int& size)
{
    pthread_mutex_lock(&sharedLock); //unlock
	myqueue elem(ID, size);
    requestQueue.push(elem);
    pthread_mutex_unlock(&sharedLock); //unlock
}

void dump_memory(Node*& head, char memory[M])
{
    Node* temp = head;
    cout<<"List:"<<endl;
    while(temp != NULL)
    {
        cout<<"["<<temp->id<<"]"<<"["<<temp->size<<"]"<<"["<<temp->index<<"]";
        if(temp->next != NULL)
        {
            cout<<"---";
        }
        temp = temp->next;
    }
    cout<<endl<<"Memory Dump:"<<endl;
    for(int i = 0; i<M; i++)
    {
        cout<<memory[i];
    }
    cout<<endl<<"*********************************"<<endl;
}
void* server_function(void *)
{
	while(run)
	{
		int ID;
		bool check=true;
		while(check)
		{
			pthread_mutex_lock(&sharedLock); //unlock
			if(!requestQueue.empty())
			{
				check = false;
			}
			pthread_mutex_unlock(&sharedLock); //unlock
		}
		
			  pthread_mutex_lock(&sharedLock); //unlock
			 
			  myqueue queueReq = requestQueue.front();
			  requestQueue.pop();
			  ID = queueReq.id;

			  if(headPtr->next == NULL)
			  {
				  Node* temp = new Node(ID, queueReq.size,0,NULL);
				  temp->next = headPtr;
				  headPtr = temp;
				  headPtr->next->size=(headPtr->next->size)-queueReq.size;
				  headPtr->next->index+=queueReq.size;

				  for(unsigned int i=0;i<queueReq.size;i++)
					  memory[i] = '0'+headPtr->id;

				  thread_message[queueReq.id] = 1;
				  /*free_mem(headPtr);
				  free_mem(headPtr);*/
			      dump_memory(headPtr, memory);
			  }
			  
			  else if(headPtr->id == -1 && headPtr->size>=queueReq.size)
			  {
				  if( headPtr->size == queueReq.size)
				  {
					  Node *ptr = headPtr;
					  ptr->id=queueReq.id;
					  for(unsigned int i=ptr->index;i<+ptr->index+queueReq.size;i++)
						  memory[i] = '0'+ptr->id;
					  thread_message[queueReq.id]=1;
					/*  free_mem(headPtr);
					  free_mem(headPtr);*/
					  dump_memory(headPtr,memory);
				  }
				  else if(headPtr!=NULL)
				  {
					  Node *temp=headPtr;
					  Node* Pushed = new Node(queueReq.id,queueReq.size,0,NULL);
					  temp->index+=Pushed->size;
					  temp->size=temp->size-queueReq.size;
					  Pushed->next = headPtr;
					  headPtr = Pushed;
					  for(int i = 0; i<headPtr->size; i++)
							memory[i] = '0' + headPtr->id;
					  thread_message[queueReq.id]=1;
					 /* free_mem(headPtr);
					  free_mem(headPtr);*/
					  dump_memory(headPtr,memory);
				  } 
			  }
			  
			  else
			  {
					Node* kosix = headPtr;
					while(kosix != NULL)
					{
						if(kosix->next !=NULL)
						{
							if((kosix->next->id == -1) && (kosix->size >= queueReq.size))
								break;
						}
						kosix = kosix->next;
					}
					if(kosix != NULL)
					{
						cout << "a" << endl;
						if(kosix->size == queueReq.size )
						{
							kosix->id = queueReq.id;
							for(int i = kosix->index; i<kosix->index+queueReq.size; i++)
								memory[i] = '0' + queueReq.id;
							
							thread_message[queueReq.id] =1;
						/*	free_mem(headPtr);
							free_mem(headPtr);*/
							dump_memory(headPtr,memory);
						}
						else
						{
							if(kosix->next != NULL)
							{
								int new_index = kosix->next->index;
								Node* temp = new Node(queueReq.id, queueReq.size, new_index, NULL);
								kosix->next->size = kosix->next->size-temp->size;
								kosix->next->index += temp->size;
								temp->next = kosix->next;
								kosix->next = temp;
								for(int i = new_index; i<new_index+temp->size; i++)
									memory[i] = '0' + temp->id;
							
								thread_message[queueReq.id] =1;
							/*	free_mem(headPtr);
								free_mem(headPtr);*/
								dump_memory(headPtr,memory);
							}
						}	
					}
				}
				sem_post(&(semlist[ID]));
				pthread_mutex_unlock(&sharedLock); //unlock
		}
			  
	}



void* thread_function(void* arg)
{
    while(run)
    {
        int* ID_ptr = (int*)arg;
        int size = (rand()%(M/3)+1);
        my_malloc(*ID_ptr, size);
        sem_wait(&(semlist[*ID_ptr]));
		pthread_mutex_lock(&sharedLock);
		
		if(thread_message[*ID_ptr]==1)
		{
			pthread_mutex_unlock(&sharedLock);
		
			use_mem();
			pthread_mutex_lock(&sharedLock);
		
			Node* temp = headPtr;
            
			while(temp != NULL && temp->id !=(*ID_ptr)) 
                temp = temp->next;
            if(temp != NULL)
			{
				temp->id = -1;
				for(int i = temp->index; i<temp->index+temp->size; i++)
					memory[i] = 'X';
				
				free_mem(headPtr);
				free_mem(headPtr);
				dump_memory(headPtr, memory);
				thread_message[*ID_ptr] = 0;
			}
		}		pthread_mutex_unlock(&sharedLock); //unlock
    }
}

void release_function()
{
	Node* temp=headPtr;

	while(temp != NULL)
	{
		headPtr=headPtr->next;
		delete temp;
		temp=headPtr;
	}

	for(unsigned int i=0;i<M;i++)
		memory[i]='X';

	run=false;

}


void init()	 
{
	pthread_mutex_lock(&sharedLock);	//lock
	for(int i = 0; i < NUM_THREADS; i++) //initialize semaphores11
	{sem_init(&semlist[i],0,0);}
   	pthread_create(&server,NULL,server_function,NULL); //start server 
	pthread_mutex_unlock(&sharedLock); //unlock
}



int main()
{
	headPtr = new Node(-1,M,0,NULL);

	init();

    for(int i = 0; i<NUM_THREADS; i++)
		thread_message[i] = 0;
    
    for(int i = 0; i<M; i++)
        memory[i] = 'X';
    
	pthread_t threads[NUM_THREADS];
    
    int thread_args[NUM_THREADS];

    for(int i = 0; i<NUM_THREADS; i++)
        thread_args[i]=i;
    
	for(int i = 0; i<NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, thread_function, (void*) &thread_args[i]);
    
	 sleep(10);

	release_function();
    for(int i = 0; i<NUM_THREADS; i++)
        pthread_join(threads[i], 0);

	cout << "terminating knkm" << endl;
    return 0;

}
