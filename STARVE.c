#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

//Functions(2)- North to South(NTS) & South to North(STN)
void* NTS(void*);
void* STN(void*);
//mutexes(mutual exclusion) : shared resources er control access kore
// Mutex variables , example : nts, Used to control access to the N to S crossing section.
pthread_mutex_t mutex, nts, stn, mutex1;

//counter(4) - 1.a villagers crossed, 2. cycle , 3. currently crossing 4. total villagers

// Counters for N to S and S to N crossings
//count the number of times a villager crosses from north to south.
int ntscnt, stncnt;

//Counters for N to S and S to N cycles
//ekta complete cycle count - ekta villager north to south & back to north ashle cycle create hoi
int ntscyc, stncyc;

//Counters for currently crossing N to S & S to N villagers
int onnts, onstn;

// Start time of the program
time_t start;

// Total number of villages, & the number of N to S and S to N crossings
int VILLCNT, STARVCNT;

//display the current state of villagers
void display(int id, char* str)
{
    pthread_mutex_lock(&mutex); 
    //only one thread can access the count variable at a time, 
    //preventing the variable from being corrupted by concurrent access
    for (int i = 0; i < VILLCNT; i++)
    {
        if (id == -1)
        {
            printf("Vill. %d     |", i + 1);
            //prints village name
        }
        else
        {
            if (i == id - 1)
            {
                //print name of the specific village to be highlighted
                printf("%8s    |", str);
            }
            else
            {
                printf("            |");
            }
        }
    }
    printf("%ld sec\n", time(NULL) - start);
    pthread_mutex_unlock(&mutex); 
    //thread unlock hle next ta ashbe, jate corrupted na hoi
}

// N to S crossing function
void* NTS(void* id)
{
    // Villager er current state display
    //(*(int*)id -> integer value that the pointer id points to 
    display(*(int*)id, "Appeared");
    display(*(int*)id, "N to S");
    display(*(int*)id, "Waiting");

    // Acquire permission to cross from N to S
    pthread_mutex_lock(&nts);
    display(*(int*)id, "Got Perm");

    // New cycle start er jonno counter update
  pthread_mutex_lock(&mutex); // general mutex lock - to protect shared variables
   ntscnt++;                   // Increment the N to S crossing counter
   ntscyc++;                   // Increment the N to S crossing cycle counter

    // If this is the first villager in the cycle, try to lock the S to N mutex
    if (ntscnt == 1)
        pthread_mutex_trylock(&stn); 

    // Check if the cycle is complete, unlock the N to S mutex accordingly
    if (ntscyc != STARVCNT)
        pthread_mutex_unlock(&nts);
    else
        ntscyc = 0;

    pthread_mutex_unlock(&mutex);

    // Begin crossing
    display(*(int*)id, "Crossing");

    // Synchronize access to the onnts counter
    pthread_mutex_lock(&mutex1);
    onnts++;

    // Wait until all N to S villagers have started crossing
    while (onnts > 1);
    pthread_mutex_unlock(&mutex1);

    // Simulate crossing time
    sleep((rand() % 5) + 2);

    // Display that the villager has crossed
    display(*(int*)id, "Crossed!");

    // Update counters and release locks
    pthread_mutex_lock(&mutex);
    ntscnt--;
    onnts--;

    // If all N to S villagers have crossed, unlock the S to N mutex
    if (ntscnt == 0)
        pthread_mutex_unlock(&stn); // Comment this line

    pthread_mutex_unlock(&mutex);

    // Sleep to simulate some time passing after crossing
    sleep(2);

    // Try to acquire permission to cross from S to N, release locks if successful
    if (!pthread_mutex_trylock(&stn))
    {
        pthread_mutex_unlock(&nts);
        pthread_mutex_unlock(&stn);
    }

    // Exit the thread
    pthread_exit(NULL);
}

// S to N crossing function
void* STN(void* id)
{
    // Display the current state of the villager
    display(*(int*)id, "Appeared");
    display(*(int*)id, "S to N");
    display(*(int*)id, "Waiting");

    // Acquire permission to cross from S to N
    pthread_mutex_lock(&stn);
    display(*(int*)id, "Got Perm");

    // counters update & check for the start of a new cycle
    pthread_mutex_lock(&mutex);
    stncnt++;
    stncyc++;

    // If first villager cycle e hoi - try to lock the N to S mutex
    if (stncnt == 1)
        pthread_mutex_trylock(&nts); 

    // if ycle complete - unlock the S to N mutex accordingly
    if (stncyc != STARVCNT)
        pthread_mutex_unlock(&stn);
    else
        stncyc = 0;

    pthread_mutex_unlock(&mutex);

    // Begin crossing
    display(*(int*)id, "Crossing");

    // give access to the onstn counter
    pthread_mutex_lock(&mutex1);
    onstn++;

    // shob S to N villagers crossing kora porjnto wait - loop
    while (onstn > 1);
    pthread_mutex_unlock(&mutex1);

    // Simulate crossing time
    sleep((rand() % 5) + 2);

    // Display that the villager has crossed
    display(*(int*)id, "Crossed!");

    //counters update & locks release 
    pthread_mutex_lock(&mutex);
    stncnt--;
    onstn--;

    // shob S to N villagers cross sesh korle - unlock the N to S mutex
    if (stncnt == 0)
        pthread_mutex_unlock(&nts); 

    pthread_mutex_unlock(&mutex);

    // Sleep to simulate some time passing after crossing
    sleep(2);

    // Try to acquire permission to cross from N to S, release locks if successful
    if (!pthread_mutex_trylock(&nts))
    {
        pthread_mutex_unlock(&stn);
        pthread_mutex_unlock(&nts);
    }

    // Exit thread
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    
    srand(time(NULL)); //initialize the random number generator in C
    // program generates a different sequence of random numbers each time it runs

    // Variables to store random counts for N to S and S to N villages
    int n1 = (rand() % 3) + 2;
    int n2 = (rand() % 3) + 2, STARVCNT = 3; //starvation count

    // Check command line arguments for Villagers Count
    if (argc == 1)
    {
        VILLCNT = n1 + n2; //total number of villagers 
    }
    else if (argc == 2)
    {
        STARVCNT = atoi(argv[1]); //atoi - converts a string to an integer.
        VILLCNT = n1 + n2;
    }
    else if (argc == 3)
    {
        n1 = atoi(argv[1]);
        n2 = atoi(argv[2]);
        VILLCNT = n1 + n2;
    }
    else if (argc == 4)
    {
        n1 = atoi(argv[1]);
        n2 = atoi(argv[2]);
        STARVCNT = atoi(argv[3]);
        VILLCNT = n1 + n2;
    }
    else
    {
        printf("Invalid number of arguments\n");
        exit(0);
    }

    // Set the start time
    start = time(NULL);

    // Create an array of threads and initialize counters
    pthread_t vill[VILLCNT];
    ntscnt = 0;
    stncnt = 0;
    ntscyc = 0;
    stncyc = 0;
    onnts = 0;
    onstn = 0;

    // Array to store village IDs
    int id[VILLCNT];
    int t;

    // Initialize mutexes
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&nts, NULL);
    pthread_mutex_init(&stn, NULL);
    pthread_mutex_init(&mutex1, NULL);

    //initial state of villages display
    display(-1, "");

    //iterates through the VILLCNT villages & creates a thread for each village
    for (int i = 0; i < VILLCNT; i++)
    {
        void* (*villgen)(void*); //function pointer declare named villgen
        id[i] = i + 1; //assign village id to id array
        t = rand() % 2; //randomly select thread function - generates random number between 0 and 1
        if (t == 1 && n1 != 0)
        {
            villgen = &NTS; //a north-to-south village thread is created
            n1--;
        }
        else if (t == 0 && n2 != 0)
        {
            villgen = &STN; //a south-to-north village thread is created
            n2--;
        }
        else if (t == 0 && n1 != 0)
        {
            villgen = &NTS; 
            //favors north-to-south villages if there are no south-to-north villages
            n1--;
        }
        else if (t == 1 && n2 != 0)
        {
            villgen = &STN;
            //favors south-to-north villages if there are no north-to-south villages
            n2--;
        }
        sleep(4); //adds a slight delay of 4 seconds before creating the thread
        pthread_create(&vill[i], NULL, villgen, (&id[i]));
        //creates thread for village with function pointer
    }

    // Wait for all threads to finish
    pthread_exit(NULL);
    exit(0);
}
