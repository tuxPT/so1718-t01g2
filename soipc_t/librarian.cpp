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

// used in request->id
#define REQ_TERMINATION        -1
#define REQ_COLLECT_BOOKS      -2
#define REQ_ENROLL_STUDENT     -3
#define REQ_DISENROLL_STUDENT  -4

typedef struct _threadlibr_
{
  pthread_mutex_t* pmtxrq;
  pthread_mutexattr_t* attrrq;
  pthread_mutex_t* pmtxpq;
  pthread_mutexattr_t* attrpq;
}threadlibr;
static _threadlibr_* threadlibr1=NULL;

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
static int term=0;
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
   threadlibr1 = (struct _threadlibr_*)shmAlloc(sizeof(struct _threadlibr_));
   threadlibr1->attrrq = (pthread_mutexattr_t *)shmAlloc(sizeof(pthread_mutexattr_t *));
   threadlibr1->attrpq = (pthread_mutexattr_t *)shmAlloc(sizeof(pthread_mutexattr_t *));
   threadlibr1->pmtxrq = (pthread_mutex_t *)shmAlloc(sizeof(pthread_mutex_t *));
   threadlibr1->pmtxpq = (pthread_mutex_t *)shmAlloc(sizeof(pthread_mutex_t *));
   mutexattr_init(threadlibr1->attrrq);
   mutex_init(threadlibr1->pmtxrq,threadlibr1->attrrq);
   mutex_unlock(threadlibr1->pmtxrq);
   mutexattr_init(threadlibr1->attrpq);
   mutex_init(threadlibr1->pmtxpq,threadlibr1->attrpq);
   mutex_unlock(threadlibr1->pmtxpq);
   logId = registerLogger((char*)descText, line ,column , 4, lengthLibrarian(), (char**)translations);
   sendLog(logId, toStringLibrarian());
}

void destroyLibrarian()
{
  alive=0;
  destroyQueue(pendingRequests);
  destroyQueue(reqQueue);
  mutexattr_destroy(threadlibr1->attrrq);
  mutex_destroy(threadlibr1->pmtxrq);
  mutexattr_destroy(threadlibr1->attrpq);
  mutex_destroy(threadlibr1->pmtxpq);
  free(threadlibr1->pmtxrq);
  free(threadlibr1->pmtxpq);
  free(threadlibr1->attrrq);
  free(threadlibr1->attrpq);
  free(threadlibr1);
  state=DONE;
  char* a;
  a=strdup(descText);
  sendLog(logId,a);
}

int logIdLibrarian()
{
   return logId;
}

void* mainLibrarian(void* arg)
{
   life();
   return NULL;
}

static void life()
{
   while(aliveLibrarian())
   {
      sleep();
      eat(0);
      handleRequests();
      char* a;
      a=strdup(descText);
      sendLog(logId,a);
      collectBooks();
      a=strdup(descText);
      sendLog(logId,a);
      eat(1);
      handleRequests();
      a=strdup(descText);
      sendLog(logId,a);
      collectBooks();
      a=strdup(descText);
      sendLog(logId,a);
      eat(2);
      fun();
   }
   done();
}

static int aliveLibrarian()
{
   /** TODO:
    * 1: librarian should be alive until a request for termination and an empty reqQueue
    **/
   return alive;
}

static void sleep()
{
   /** TODO:
    * 1: sleep (state: SLEEPING). Don't forget to spend time randomly in
    *    interval [global->MIN_SLEEPING_TIME_UNITS, global->MAX_SLEEPING_TIME_UNITS]
    **/
   state = SLEEPING;
   spend(randomInt(global->MIN_SLEEPING_TIME_UNITS,global->MAX_SLEEPING_TIME_UNITS));
   printf("\n");
   printf("sleep - librarian");
}

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
   printf("\n");
   printf("eat - librarian");
}

static void handleRequests()
{
   /** TODO:
    * 1: chose a random number (n) in interval [global->MIN_REQUESTS_PER_PERIOD, global->MAX_REQUESTS_PER_PERIOD]*/
    int n=randomInt(global->MIN_REQUESTS_PER_PERIOD,global->MAX_REQUESTS_PER_PERIOD);
    /* 2. accept n requests (except if termination is requested)*/
    /* 3. requests are placed (elsewhere) in queue reqQueue*/
    /* 4. use function handleRequest to handle a single request.*/
    for(int i=1;i<n;i++)
    {
      handleRequest((_Request_*)&(reqQueue->head->elem));
      mutex_lock(threadlibr1->pmtxrq);
      outQueue(reqQueue);
      mutex_unlock(threadlibr1->pmtxrq);
      if (term==1 & reqQueue==NULL) {
        i=n;
      }
    }
    if (term==1 & reqQueue==NULL) {
      done();
      destroyLibrarian();
    }
    /* 5. Don't forget to spend time randomly in interval [global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS]
    **/
}

static void collectBooks()
{
   /** TODO:
    * 1: traverse all seats searching for books in empty seats, collect those books and put them in library bookshelf
    * 2. check of pending requests can be attended (in order)
    * 3. Don't forget to spend time randomly in interval [global->MIN_HANDLE_REQUEST_TIME_UNITS, global->MAX_HANDLE_REQUEST_TIME_UNITS]
    **/
    spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS,global->MAX_HANDLE_REQUEST_TIME_UNITS));
    while(!threadlib2(1));
    while(!threadlib2(2));
    for (int i=0;  i <numSeats();  i++) {
      if(booksInSeat(i) && !seatOccupied(i))
        collectBooksLibrary(i);
    }
    while(!threadlib2(4));
    while(!threadlib2(3));
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
         spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS,global->MAX_HANDLE_REQUEST_TIME_UNITS));
         term=1;
         break;
      case REQ_COLLECT_BOOKS:
         collectBooks();
         break;
      case REQ_ENROLL_STUDENT:
         spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS,global->MAX_HANDLE_REQUEST_TIME_UNITS));
         break;
      case REQ_DISENROLL_STUDENT:
         spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS,global->MAX_HANDLE_REQUEST_TIME_UNITS));
         break;
      default: // book requisition
         while(!threadlib2(1));
         booksAvailableInLibrary(req->books);
         requisiteBooksFromLibrary(req->books);
         while(!threadlib2(3));
         mutex_lock(threadlibr1->pmtxpq);
         inQueue(pendingRequests,req);
         mutex_unlock(threadlibr1->pmtxpq);
         spend(randomInt(global->MIN_HANDLE_REQUEST_TIME_UNITS,global->MAX_HANDLE_REQUEST_TIME_UNITS));
         break;
   }
}

static void fun()
{
   /** TODO:
    * 1: have fun (state: HAVING_FUN). Don't forget to spend time randomly in
    *    interval [global->MIN_FUN_TIME_UNITS, global->MAX_FUN_TIME_UNITS]
    **/
   state = HAVING_FUN;
   spend(randomInt(global->MIN_FUN_TIME_UNITS,global->MAX_FUN_TIME_UNITS));
   printf("\n");
   printf("fun - librarian");
}

static void done()
{
   /** TODO:
    * 1:  life of librarian is over (state: DONE).
    **/
   state = DONE;
   printf("\n");
   printf("done - librarian");
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
   inQueue(reqQueue,newEnrollStudentRequest());
}

void reqDisenrollStudent()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   inQueue(reqQueue,newDisenrollStudentRequest());
}

void reqTermination()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   inQueue(reqQueue,newTerminationRequest());
}

void reqCollectBooks()
{
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2: does not wait for response.
    **/
   inQueue(reqQueue,newCollectBooksRequest());
}
int reqBookRequisition(struct _Book_** books)
{
   assert (books != NULL && *books != NULL);
   int id1=newId();
   /** TODO:
    * 1: queue reqQueue should be updated and a notification send to librarian active entity
    * 2. operation fails (return 0) if for some reason the librarian is no longer alive
    * 3. waits until operation is done  (except in situation 2.)
    **/
    _Request_* req=(_Request_*)pendingRequests->head->elem;
    if(alive==0)
      return 0;
   mutex_lock(threadlibr1->pmtxrq);
   inQueue(reqQueue,newRequisiteBooksRequest(books,id1));
   mutex_unlock(threadlibr1->pmtxrq);
   while(true){
     while(pendingRequests==NULL);
     if (containsId(id1) && req->id==id1) {
        if(req->requisited==0){
          mutex_lock(threadlibr1->pmtxpq);
          outQueue(pendingRequests);
          mutex_unlock(threadlibr1->pmtxpq);
          mutex_lock(threadlibr1->pmtxrq);
          inQueue(reqQueue,newRequisiteBooksRequest(books,id1));
          mutex_unlock(threadlibr1->pmtxrq);
        }
        else
        {
          mutex_lock(threadlibr1->pmtxpq);
          outQueue(pendingRequests);
          mutex_unlock(threadlibr1->pmtxpq);
          removeId(id1);
          return 1;
        }
     }
   }
   return 0;
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
