#include <stdlib.h>
#include <assert.h>
#include "utils.h"
#include "queue.h"
#include "timer.h"
#include "global.h"
#include "logger.h"
#include "book.h"
#include "library.h"
#include "librarian.h"
#include "thread.h"
#include "process.h"

// used in request->id
#define REQ_TERMINATION        -1
#define REQ_COLLECT_BOOKS      -2
#define REQ_ENROLL_STUDENT     -3
#define REQ_DISENROLL_STUDENT  -4


#define ACCESS_LIBRARY 0
#define SIT 1
#define REQBOOKS 2
#define ACCESS 1
#define MESSAGE 0

#include <unistd.h>
#include <iostream>
using namespace std;
#define DEBUG; cout << __FILE__ << ":" << __func__<< " line:" << __LINE__ << endl


typedef struct _Request_
{
   struct _Book_** books;
   int id;
   int requisited;
} Request;

static Request* newTerminationRequest();
static Request* newRequisiteBooksRequest(struct _Book_** books, int id);
static Request* newCollectBooksRequest();
static Request* newEnrollStudentRequest();
static Request* newDisenrollStudentRequest();

enum State
{
   NONE = 0,
   NORMAL,
   BREAKFAST,
   LUNCH,
   DINNER,
   SLEEPING,
   BOOK_REQ,
   BOOK_COLLECT,
   HAVING_FUN,
   DONE
};

static const char* stateText[10] =
{
   "  ",
   FACE_NEUTRAL,
   FOOD_BREAD,
   FOOD_STEAMING_BOWL,
   FOOD_POULTRY_LEG,
   FACE_SLEEPING,
   BOOK_CLOSED,
   BOOKS,
   COMPUTER_OLD_PERSONAL,
   FACE_SMILING_WITH_SUNGLASSES
};

/* TODO: put your code here */
static int semid_librarian;
static int shmid_librarian;

const long keySemLibrarian = 0x1111L;
const long keyShmLibrarian = 0x1112L;
const long keySemLogger = 0x1113L;
const long keyShmLogger = 0x1114L;
const long keySemLibrary = 0x1115L;
//const long keyShmLibrary = 0x1116L;

static const char* descText = "Librarian:";

static int idCount = 0;
static int currIdSize = 0;
static int currIdMaxSize = 128;
static int* currId = (int*)memAlloc(sizeof(int)*currIdMaxSize);

static void currIdIn(int id);
static int containsId(int id);
static void removeId(int id);


static int enrolledStudents;
static State state;
static int logId;

static int alive;
static Queue* reqQueue = newQueue(NULL);
static Queue* pendingRequests = newQueue(NULL);

static int aliveLibrarian();
static void life();
static void sleep();
static void eat(int meal); // 0: breakfast; 1: lunch; 2: dinner
static void handleRequests();
static void collectBooks();
static void handleRequest(Request* req);
static void fun();
static void done();
static int newId();
static char* toStringLibrarian();
void* outQueueShm(void* arg);


void initLibrarian(int line, int column)
{
   assert (line >= 0);
   assert (column >= 0);

   enrolledStudents = 0;
   state = NONE;
   alive = 1;
   static const char* translations[] = {
      "Librarian:", "",
      FACE_NEUTRAL, "NORMAL",
      FOOD_BREAD, "BREAKFAST",
      FOOD_STEAMING_BOWL, "LUNCH",
      FOOD_POULTRY_LEG, "DINNER",
      FACE_SLEEPING, "SLEEPING",
      BOOK_CLOSED, "BOOK_REQ",
      BOOKS, "BOOK_COLLECT",
      FACE_THINKING, "STUDYING",
      COMPUTER_OLD_PERSONAL, "HAVING_FUN",
      FACE_SMILING_WITH_SUNGLASSES, "DONE",
      NULL
   };
   logId = registerLogger((char*)descText, line ,column , 4, lengthLibrarian(), (char**)translations);
   semid_librarian = psemget(keySemLibrarian, 2, IPC_CREAT | IPC_EXCL | 0660);
   union semun{
      int val; /* used for SETVAL only */
      struct semid_ds *buf; /* used for IPC_STAT and IPC_SET */
      ushort *array; 
   };
   union semun arg;
   arg.array = (ushort*) malloc(2*sizeof(ushort));
   arg.array[0] = 1;//access semaphore
   arg.array[1] = 0;//message semaphore
   semctl(semid_librarian, 0, SETALL, arg);
   // printf("SIZE:::::::::::::::::::%d\n", sizeof(Event) + 200);
   shmid_librarian = shmget(keyShmLibrarian, 32, IPC_CREAT | IPC_EXCL | 0660);
   sendLog(logId, toStringLibrarian());
}

void destroyLibrarian()
{
   destroyQueue(reqQueue);
   semctl(semid_librarian, 0, IPC_RMID);
   shmctl(shmid_librarian, IPC_RMID, NULL);
}

int logIdLibrarian()
{
   return logId;
}

void* mainLibrarian(void* arg)
{
   pthread_t messageThreadId;
   thread_create(&messageThreadId,NULL,outQueueShm,NULL);

   life();

   thread_join(messageThreadId,NULL);
   return NULL;
}

static void life()
{
   while(aliveLibrarian())
   {
      sleep();
      eat(0);
      handleRequests();
      collectBooks();
      eat(1);
      handleRequests();
      collectBooks();
      eat(2);
      fun();
		// printf("LIBRARIAN!!!\n");
   }

   done();
}

static int aliveLibrarian()
{
   /** TODO:
    * 1: librarian should be alive until a request for termination and an empty reqQueue
    **/
   if(not(alive) and emptyQueue(reqQueue))
   {
      return 0;
   }
   else{
      return 1;
   }
}

static void sleep()
{
   /** TODO:
    * 1: sleep (state: SLEEPING). Don't forget to spend time randomly in
    *    interval [global->MIN_SLEEPING_TIME_UNITS, global->MAX_SLEEPING_TIME_UNITS]
    **/
   state = SLEEPING;
   spend(randomInt(global->MIN_SLEEPING_TIME_UNITS,global->MAX_SLEEPING_TIME_UNITS));
	sendLog(logId, toStringLibrarian());
   // printf("\n");
   // printf("sleep - librarian");
}

//--------------------------- Allocated memory Queue Wrapper----------------------
//see sendLog
void inQueueShm(Request* req)
{
   assert (req != NULL);
   

   int semid_librarian = semget(keySemLibrarian, 0, 0);


   psem_down(semid_librarian, 0);
   char* memory = (char* )pshmat(shmid_librarian, NULL, 0);
   Request* r = (Request*)memory;

   r->books=NULL;
   r->id=req->id;
   r->requisited=req->requisited;

   psem_up(semid_librarian, 1);

}


//see processMessageChannel
void* outQueueShm(void* arg)
{
   while(aliveLibrarian())
   {
      psem_down(semid_librarian, 1);

      Request* r = (Request* )pshmat(shmid_librarian, NULL, 0);
      int i = r->id;
      int j = r->requisited;
      //char* str = ((char*)r) + 8;

      Request* copyRequest = (Request*)memAlloc(sizeof(Request));

      copyRequest->books = NULL;
      copyRequest->id = i;
      copyRequest->requisited = j;
      inQueue(reqQueue, copyRequest);
      //pshmdt(r);
      psem_up(semid_librarian, 0);

   }
   thread_exit(NULL);

}
//-----------------------------------------------------------------------------

static void eat(int meal) // 0: breakfast; 1: lunch; 2: dinner
{
   assert (meal >= 0 && meal <= 2);

   /** TODO:
    * 1: eat (state: BREAKFAST or LUNCH or DINNER). Don't forget to spend time randomly in
    *    interval [global->MIN_EATING_TIME_UNITS, global->MAX_EATING_TIME_UNITS]
    **/

   switch(meal)
   {
      case 0:
         state = BREAKFAST;
         break;
      case 1:
         state = LUNCH;
         break;
      case 2:
         state = DINNER;
         break;
   }
   spend(randomInt(global->MIN_EATING_TIME_UNITS,global->MAX_EATING_TIME_UNITS));
	sendLog(logId, toStringLibrarian());

	// printf("\n");
   // printf("eat - librarian\n");
}

static void handleRequests()
{
   /** TODO:
    * 1: chose a random number (n) in interval [global->MIN_REQUESTS_PER_PERIOD, global->MAX_REQUESTS_PER_PERIOD]
    * 2. accept n requests (except if termination is requested)
    * 3. requests are placed (elsewhere) in queue reqQueue
    * 4. use function handleRequest to handle a single request.
    * 5. Don't forget to spend time randomly in interval [global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS]
    **/
   if(alive){
		state = NORMAL;
      int n = randomInt(global->MIN_REQUESTS_PER_PERIOD, global->MAX_REQUESTS_PER_PERIOD);
      int i = 0;
      //psem_down(semid_librarian, ACCESS);
      while(i<n)
      {
         if(emptyQueue(reqQueue))
            break;
         Request* req = (Request*) outQueue(reqQueue);
         
         if(req->id == REQ_TERMINATION)
         {
            handleRequest(req);
            //free(req);
            break;
         }
         
         handleRequest(req);

         spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS));

         sendLog(logId, toStringLibrarian());

         i++;
      }
      //psem_up(semid_librarian, ACCESS);
   
   }
	sendLog(logId, toStringLibrarian());
}

static void collectBooks()
{
	state = BOOK_COLLECT;
   /** TODO:
    * 1: traverse all seats searching for books in empty seats, collect those books and put them in library bookshelf
    * 2. check of pending requests can be attended (in order)
    * 3. Don't forget to spend time randomly in interval [global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS]
    **/
    DEBUG;
   int semid_lib = semget(keySemLibrary, 0, 0);
   psem_down(semid_lib,ACCESS_LIBRARY);
   for(int seatNum = 0; seatNum < numSeats(); seatNum++)
   {
      if(not(seatOccupied(seatNum)) and booksInSeat(seatNum))
      {
         DEBUG;
         collectBooksLibrary(seatNum); 
         DEBUG;     
      }
   }
   psem_up(semid_lib,ACCESS_LIBRARY);
   spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS));
	DEBUG;
   sendLog(logId, toStringLibrarian());
}

static void handleRequest(Request* req)
{
   assert (req != NULL);

   /** TODO:
    * 1: handle each request
    *
    * 2. check of pending requests can be attended (in order)
    * 3. Don't forget to spend time randomly in interval [global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS]
    **/

   switch(req->id)
   {
      case REQ_TERMINATION:
         alive = 0;
         if(not(emptyQueue(reqQueue)))
         {
            for(int i=0;i<sizeQueue(reqQueue);i++)
               outQueue(reqQueue);
         }
         break;
      case REQ_COLLECT_BOOKS:
         collectBooks();
         break;
      case REQ_ENROLL_STUDENT:
         enrolledStudents++;
         break;
      case REQ_DISENROLL_STUDENT:
         enrolledStudents--;
         break;
      default: // book requisition
         //requisiteBooksFromLibrary(req->books);
         break;
   }
   spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS));
   sendLog(logId, toStringLibrarian());
}

static void fun()
{
   /** TODO:
    * 1: have fun (state: HAVING_FUN). Don't forget to spend time randomly in
    *    interval [global->MIN_FUN_TIME_UNITS, global->MAX_FUN_TIME_UNITS]
    **/
   state = HAVING_FUN;
   spend(randomInt(global->MIN_FUN_TIME_UNITS,global->MAX_FUN_TIME_UNITS));
	sendLog(logId, toStringLibrarian());

	// printf("\n");
   // printf("fun - librarian");
}

static void done()
{
   /** TODO:
    * 1:  life of librarian is over (state: DONE).
    **/
   state = DONE;
	sendLog(logId, toStringLibrarian());
   //destroyLibrarian();
	 // printf("\n");
   // printf("done - librarian");
}

int lengthLibrarian()
{
   return 4+12*2+11;
}

void reqEnrollStudent()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   Request* newStudent = (Request*)newEnrollStudentRequest();
   inQueueShm(newStudent);
   sendLog(logId, toStringLibrarian());
}

void reqDisenrollStudent()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   Request* freeStudent = (Request*)newDisenrollStudentRequest();
   inQueueShm(freeStudent);
   sendLog(logId, toStringLibrarian());
}

void reqTermination()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   Request* termination = (Request*)newTerminationRequest();
   inQueueShm(termination);
   sendLog(logId, toStringLibrarian());
}

void reqCollectBooks()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   Request* collect = (Request*)newCollectBooksRequest();
   inQueueShm(collect);
   sendLog(logId, toStringLibrarian());

}

int reqBookRequisition(struct _Book_** books)
{
   assert (books != NULL && *books != NULL);

   /** TODO:
    * 1: queue reqQueue should be updated and +
    a notification send to librarian active entity
    * 2. operation fails (return 0) if for some reason the librarian is no longer alive
    * 3. waits until operation is done  (except in situation 2.)
    **/
   if(not(aliveLibrarian()))
   {
      return 0;
   }
}

static int newId()
{
   idCount++;
   if (idCount < 0)
      idCount = 1;

   assert (idCount >= 1);

   return idCount;
}

static char* toStringLibrarian()
{
   char* res = (char*)"";

   res = concat_string_in_stack(res, descText);
   res = concat_string_in_stack(res, "\n");

   res = concat_string_in_stack(res, BOX_TOP_LEFT);
   res = concat_string_in_stack(res, utf8HorizontalLine(2));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   res = concat_string_in_stack(res, utf8HorizontalLine(11));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   res = concat_string_in_stack(res, utf8HorizontalLine(11));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   res = concat_string_in_stack(res, utf8HorizontalLine(10));
   res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, stateText[state]);
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, "Students:");
   res = concat_string_in_stack(res, intToString(NULL, enrolledStudents, 2));
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, "Requests:");
   res = concat_string_in_stack(res, intToString(NULL, sizeQueue(reqQueue), 2));
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, "Delayed:");
   res = concat_string_in_stack(res, intToString(NULL, sizeQueue(pendingRequests), 2));
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
   res = concat_string_in_stack(res, utf8HorizontalLine(2));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   res = concat_string_in_stack(res, utf8HorizontalLine(11));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   res = concat_string_in_stack(res, utf8HorizontalLine(11));
   res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   res = concat_string_in_stack(res, utf8HorizontalLine(10));
   res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   res = concat_string_in_stack(res, "\n");

   return stringClone(res);
}

static void currIdIn(int id)
{
   if (currIdSize == currIdMaxSize)
   {
      int* tmp = (int*)memAlloc(sizeof(int)*2*currIdMaxSize);
      for(int i = 0; i < currIdMaxSize; i++)
         tmp[i] = currId[i];
      free(currId);
      currId = tmp;
      currIdMaxSize = 2*currIdMaxSize;
   }
   currId[currIdSize] = id;
   currIdSize++;
}

static int containsId(int id)
{
   int res = 0;
   for(int i = 0; !res && i < currIdSize; i++)
      res = (currId[i] == id);
   return res;
}

static void removeId(int id)
{
   int i;
   for(i = 0; i < currIdSize && currId[i] != id; i++)
      ;
   assert (currId[i] == id);
   currIdSize--;
   for(; i < currIdSize; i++)
      currId[i] = currId[i+1];

   assert (!containsId(id));
}

static Request* newTerminationRequest()
{
   Request* res = (Request*)memAlloc(sizeof(Request));
   res->books = NULL;
   res->id = REQ_TERMINATION;
   res->requisited = 0;
   return res;
}

static Request* newRequisiteBooksRequest(struct _Book_** books, int id)
{
   Request* res = (Request*)memAlloc(sizeof(Request));
   res->books = books;
   res->id = id;
   res->requisited = 0;
   return res;
}

static Request* newCollectBooksRequest()
{
   Request* res = (Request*)memAlloc(sizeof(Request));
   res->books = NULL;
   res->id = REQ_COLLECT_BOOKS;
   res->requisited = 0;
   return res;
}

static Request* newEnrollStudentRequest()
{
   Request* res = (Request*)memAlloc(sizeof(Request));
   res->books = NULL;
   res->id = REQ_ENROLL_STUDENT;
   res->requisited = 0;
   return res;
}

static Request* newDisenrollStudentRequest()
{
   Request* res = (Request*)memAlloc(sizeof(Request));
   res->books = NULL;
   res->id = REQ_DISENROLL_STUDENT;
   res->requisited = 0;
   return res;
}

