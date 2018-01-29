#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"
#include "sim-alloc.h"
#include "global.h"
#include "logger.h"
#include "library.h"
#include "process.h"
#include <sys/types.h>
#include <stdio.h>


typedef struct _Library_
{
   struct _Book_ bookList[MAX_BOOKS]; // global->NUM_BOOKS
   int bookQuantity[MAX_BOOKS];
   int bookAvailable[MAX_BOOKS];
   int seatOccupied[MAX_SEATS];
   int booksInTransit; // not in the bookshelf or in table
   struct _Book_ booksInSeat[MAX_SEATS][MAX_BOOKS];
   int numBooksInSeat[MAX_SEATS]; // new
   int logIdBookShelf;
   int logIdTables;
   int semid_lib;
   /* TODO: change this structure to your needs */

} Library;

#include <unistd.h>
#include <iostream>
using namespace std;
#define DEBUG cout << __FILE__ << ":" << __func__<< " line:" << __LINE__ << endl
#define SHOW cout << __FILE__ << ":" << __func__<< " line:" << __LINE__ << "\t"


#define ACCESS_LIBRARY 0
#define SITSEM 1
#define REQBOOKS 2

static const long keySemLibrarian = 0x1111L;
static const long keyShmLibrarian = 0x1112L;
static const long keySemLogger = 0x1113L;
static const long keyShmLogger = 0x1114L;
static const long keySemLibrary = 0x1115L;

static const char* descText = "Library:";
static Library* library = NULL;
static const long keyShmLibrary = 0x1116L;
static int shmid_library;
static void allocLibraryDataStructure();
static void freeLibraryDataStructure();
static void invariantLibrary();
static int booksExistsInLibrary(struct _Book_** books);
static int booksRequisited(struct _Book_** books);
static void returnBooks(struct _Book_** books);
static char* toStringBookShelfs();
static char* toStringTables();
static int getSeat();
static int getRandomSeat();
static int lengthBookShelf();
static int lengthTable();
static int lengthAllTables();
static int bookSearch(struct _Book_* book);
void gen_random(char *s, const int len);


int totalSizeOfLibraryDataStructure()
{
   return sizeof(Library);
}

static void allocLibraryDataStructure()
{
   /* TODO: change this function to your needs */

   assert (library == NULL);
   shmid_library = shmget(keyShmLibrary, sizeof(Library), IPC_CREAT | IPC_EXCL | 0660);
   library = (Library*)pshmat(shmid_library, NULL, 0);

   assert (library!=NULL);
}

static void freeLibraryDataStructure()
{
   /* TODO: change this function to your needs */

   assert (library != NULL);

   library = NULL;
}

void initLibrary()
{
   /* TODO: change this function to your needs */

   if (library == NULL)
      allocLibraryDataStructure();
   char n[3];
   strcpy(n, "Bx");
   for (int i = 0; i < global->NUM_BOOKS; i++)
   {
      n[1] = (char)((int)'a'+i);
      newBook(&library->bookList[i], n);
      library->bookAvailable[i] = library->bookQuantity[i] = randomInt(global->MIN_BOOK_COPIES, global->MAX_BOOK_COPIES);
   }
   for (int i = 0; i < numSeats(); i++)
   {
      library->seatOccupied[i] = 0;
      library->numBooksInSeat[i] = 0;
   }
   library->booksInTransit = 0;
   static const char* bookShelfLineModeTranslations[] = {
      "Library:", "",
      NULL
   };
   library->logIdBookShelf = registerLogger((char*)"BookShelf:", 0 ,0 , 9, lengthBookShelf(), (char**)bookShelfLineModeTranslations);
   static const char* tablesLineModeTranslations[] = {
      FACE_PENSIVE, "SEAT_TAKEN",
      BLACK_RECTANGLE, "SEAT_EMPTY",
      BOOK_OPEN, "BOOKS_OPEN",
      BOOKS, "BOOKS_LEFT",
      NULL
   };
   library->logIdTables = registerLogger((char*)"Tables:", 2 ,lengthBookShelf()+2 , 6, lengthAllTables(), (char**)tablesLineModeTranslations);
   sendLog(library->logIdBookShelf, toStringBookShelfs());
   sendLog(library->logIdTables, toStringTables());
   invariantLibrary();
   library->semid_lib = psemget(0x1115L, 3, IPC_CREAT  | 0660);
   if (library->semid_lib == -1)
   {
      perror("Fail creating locker semaphore");
      exit(EXIT_FAILURE);
   }

   union semun{
      int val; /* used for SETVAL only */
      struct semid_ds *buf; /* used for IPC_STAT and IPC_SET */
      ushort *array; 
   };
   union semun arg;
   arg.array = (ushort*) malloc(2*sizeof(ushort));
   arg.array[ACCESS_LIBRARY] = 1;//access semaphore
   arg.array[SITSEM] = 1;//sit semaphore
   arg.array[REQBOOKS] = 1;//requisit books semaphore
   semctl(library->semid_lib, 0, SETALL, arg);
}


void destroyLibrary()
{
   /* TODO: change this function to your needs */
   semctl(library->semid_lib, 0, IPC_RMID);
   pshmctl(shmid_library, IPC_RMID, NULL);
   freeLibraryDataStructure();
}

int booksAvailableInLibrary(struct _Book_** books)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (books != NULL);
   assert (booksExistsInLibrary(books));

   int semid_lib = semget(keySemLibrary, 0, 0);
   //psem_down(semid_lib,ACCESS_LIBRARY);
   int res = 1;
   for(int i = 0; res && books[i] != NULL; i++)
      res = library->bookAvailable[bookSearch(books[i])] > 0;
   //DEBUG;
	//invariantLibrary();
	//sendLog(library->logIdBookShelf, toStringBookShelfs());
   //psem_up(semid_lib,ACCESS_LIBRARY);
	return res;
}

void requisiteBooksFromLibrary(struct _Book_** books)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   //psem_wait(semName);
   assert (books != NULL);
   assert(booksExistsInLibrary(books));
   int semid_lib = semget(keySemLibrary, 0, 0);

   int hasbooks=0;
   while(hasbooks==0)
   {
      //SHOW<<"BEFORE down\n";
      psem_down(semid_lib,ACCESS_LIBRARY);
      //SHOW<<"AFTER down\n";
   
      hasbooks= booksAvailableInLibrary(books);

      if(hasbooks){
         for(int i = 0; books[i] != NULL; i++)
         {
            int idx = bookSearch(books[i]);
            library->bookAvailable[idx]--;
            library->booksInTransit++;
         }
      }
      //SHOW<<"BEFORE UP\n";
      psem_up(semid_lib,ACCESS_LIBRARY);
      //SHOW<<"AFTER UP\n";

   }
   sendLog(library->logIdBookShelf, toStringBookShelfs());
      
   //DEBUG;
   invariantLibrary();
}

struct _Book_** randomBookListFromLibrary(struct _Book_** result, int n)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (n >= 1 && n < global->NUM_BOOKS);

   if (result == NULL)
   {
      result = (struct _Book_**)shmAlloc((n+1)*sizeof(struct _Book_*));
      for(int i = 0; i < n; i++)
         result[i] = (struct _Book_*)memAlloc(totalSizeOfBook());
   }
   for(int i = 0; i < n; i++)
   {
      int exists;
      int idx;
      do
      {
         idx = randomInt(0,global->NUM_BOOKS-1);
         exists = 0;
         for(int j = 0; !exists && j < i; j++)
            exists = equalBook(result[j], &library->bookList[idx]);
      }
      while(exists);
      *(result[i]) = library->bookList[idx];
   }
   result[n] = NULL;
   return result;
}

int validSeatPosition(int pos)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   int res;
   res = pos >= 0 && pos < numSeats();
	//invariantLibrary();
	// sendLog(library->logIdTables, toStringTables());
	return res;
}

int seatOccupied(int pos)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (validSeatPosition(pos));

   int res;
   res = library->seatOccupied[pos];
	//invariantLibrary();
	//sendLog(library->logIdTables, toStringTables());
	return res;
}

int seatAvailable()
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
	//Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   int n = getSeat();

	//invariantLibrary();
	//sendLog(library->logIdTables, toStringTables());

	if (n < numSeats())
	{
		return 1;
	}
	return 0;
}

int booksInSeat(int pos)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (validSeatPosition(pos));

   int res;
   res = library->numBooksInSeat[pos] > 0;

	//invariantLibrary();
	// sendLog(library->logIdTables, toStringTables());
	return res;
}

int sit(struct _Book_** books)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (books != NULL);
   assert (booksExistsInLibrary(books));
   //assert (seatAvailable());
   int res=numSeats();
   int semid_lib = semget(keySemLibrary, 0, 0);
   
   while(res>=numSeats()){
      psem_down(semid_lib,SITSEM);

      if(seatAvailable()){
         res = getRandomSeat();
         library->seatOccupied[res] = 1;
         for(int i = 0; books[i] != NULL; i++)
         {
            library->booksInSeat[res][i] = *(books[i]);
            library->numBooksInSeat[res]++;
         }
         library->booksInTransit -= bookListLength(books);
         sendLog(library->logIdTables, toStringTables());

         invariantLibrary();
      }
      psem_up(semid_lib,SITSEM);
	}
   return res;
}

void rise(int pos)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (validSeatPosition(pos));
   assert (seatOccupied(pos));

   int semid_lib = semget(keySemLibrary, 0, 0);
   psem_down(semid_lib,SITSEM);

   library->seatOccupied[pos] = 0;
   sendLog(library->logIdTables, toStringTables());
	//printf("SEM8\n");

	invariantLibrary();
	//printf("SEM9\n");

   psem_up(semid_lib,SITSEM);

	//sendLog(library->logIdTables, toStringTables());
	//printf("SEM 10\n");
}

void collectBooksLibrary(int pos)
{
   /* TODO: change this function to your needs */
   //shmid_library = shmget(keyShmLibrary, 0, 0);
   //Library* library = (Library*)pshmat(shmid_library, NULL, 0);
   assert (validSeatPosition(pos));
   assert (booksInSeat(pos));

   int semid_lib = semget(keySemLibrary, 0, 0);
   //psem_down(semid_lib,ACCESS_LIBRARY);

   struct _Book_**books = (struct _Book_**)alloca(sizeof(struct _Book_*)*(library->numBooksInSeat[pos]+1));
   for(int i = 0; i < library->numBooksInSeat[pos]; i++)
      books[i] = &library->booksInSeat[pos][i];
   books[library->numBooksInSeat[pos]] = NULL;
   returnBooks(books);
   library->numBooksInSeat[pos] = 0;
   sendLog(library->logIdTables, toStringTables());
   //sendLog(library->logIdBookShelf, toStringBookShelfs());
	invariantLibrary();
	//psem_up(semid_lib,ACCESS_LIBRARY);
}

int numLinesLibrary()
{
   return getNumLinesLogger(library->logIdBookShelf);
}

int numSeats()
{
   return global->NUM_TABLES*global->NUM_SEATS_PER_TABLE;
}

static void invariantLibrary()
{
   int initialBooks = 0;
   int dynamicBooks = library->booksInTransit;
   for (int i = 0; i < global->NUM_BOOKS; i++)
   {
      initialBooks += library->bookQuantity[i];
      dynamicBooks += library->bookAvailable[i];
   }
   for (int i = 0; i < numSeats(); i++)
      dynamicBooks += library->numBooksInSeat[i];

   //printf("DYNAMIC BOOKS ------------------------------------------------%d\n", dynamicBooks);
   if (lineMode())
   {
      if (!(library->booksInTransit >= 0))
         printf("[Library - invariant]: (booksInTransit:%d >= 0)\n", library->booksInTransit);
      if (!(initialBooks >= 1))
         printf("[Library - invariant]: (initialBooks:%d >= 1)\n", initialBooks);
      if (!(dynamicBooks >= 0))
         printf("[Library - invariant]: (dynamicBooks:%d >= 0)\n", dynamicBooks);
      if (!(initialBooks == dynamicBooks))
         printf("[Library - invariant]: initialBooks:%d == dynamicBooks:%d\n", initialBooks, dynamicBooks);
   }
   assert (library->booksInTransit >= 0);
   assert (initialBooks >= 1);
   assert (dynamicBooks >= 0);
   assert (initialBooks == dynamicBooks);
}

static int booksExistsInLibrary(struct _Book_** books)
{
   assert (books != NULL);

   int res = 1;
   for(int i = 0; res && books[i] != NULL; i++)
      res = bookSearch(books[i]) < global->NUM_BOOKS;

   return res;
}

static int booksRequisited(struct _Book_** books)
{
   assert (books != NULL);
   assert (booksExistsInLibrary(books));

   int res = 1;
   for(int i = 0; res && books[i] != NULL; i++)
   {
      int idx = bookSearch(books[i]);
      res = library->bookAvailable[idx] < library->bookQuantity[idx];
   }

   return res;
}

static void returnBooks(struct _Book_** books)
{
   assert (books != NULL);
   assert (booksExistsInLibrary(books));
   assert (booksRequisited(books));

   for(int i = 0; books[i] != NULL; i++)
   {
      int idx = bookSearch(books[i]);
      library->bookAvailable[idx]++;
   }
   sendLog(library->logIdBookShelf, toStringBookShelfs());
}

static char* toStringBookShelfs()
{
   char* res = (char*)"";

   res = concat_string_in_stack(res, descText);
   res = concat_string_in_stack(res, "\n");
   int half = global->NUM_BOOKS/2;
   // top
   res = concat_string_in_stack(res, BOX_TOP_LEFT);
   for(int i = 0; i < half; i++)
   {
      res = concat_string_in_stack(res, utf8HorizontalLine(5));
      if (i < half-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   }
   res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_VERTICAL);
   for(int i = 0; i < half; i++)
   {
      res = concat_string_in_stack(res, nameBook(&library->bookList[i]));
      res = concat_string_in_stack(res, ":");
      res = concat_string_in_stack(res, intToString((char*)alloca(3), library->bookAvailable[i], 2));
      res = concat_string_in_stack(res, BOX_VERTICAL);
   }
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
   for(int i = 0; i < half; i++)
   {
      for(int j = 0; j < 5; j++)
         res = concat_string_in_stack(res, " ");
      if (i < half-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   }
   res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   res = concat_string_in_stack(res, "\n\n\n");

   // bottom
   res = concat_string_in_stack(res, BOX_TOP_LEFT);
   for(int i = half; i < global->NUM_BOOKS; i++)
   {
      for(int j = 0; j < 5; j++)
         res = concat_string_in_stack(res, " ");
      if (i < global->NUM_BOOKS-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   }
   res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_VERTICAL);
   for(int i = half; i < global->NUM_BOOKS; i++)
   {
      res = concat_string_in_stack(res, nameBook(&library->bookList[i]));
      res = concat_string_in_stack(res, ":");
      res = concat_string_in_stack(res, intToString((char*)alloca(3), library->bookAvailable[i], 2));
      res = concat_string_in_stack(res, BOX_VERTICAL);
   }
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
   for(int i = half; i < global->NUM_BOOKS; i++)
   {
      res = concat_string_in_stack(res, utf8HorizontalLine(5));
      if (i < global->NUM_BOOKS-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   }
   res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   res = concat_string_in_stack(res, "\n");

   return stringClone(res);
}

static char* toStringTables()
{
   char* res = (char*)"";

   for(int t = 0; t < global->NUM_TABLES; t++)
   {
      if (t > 0)
         res = concat_string_in_stack(res, "  ");
      for(int s = 0; s < global->NUM_SEATS_PER_TABLE/2; s++)
      {
         int idx = t*(global->NUM_SEATS_PER_TABLE/2)+s;
         if (idx < numSeats() && library->seatOccupied[idx])
            res = concat_string_in_stack(res, " "FACE_PENSIVE);
         else
            res = concat_string_in_stack(res, " "BLACK_RECTANGLE);
      }
      res = concat_string_in_stack(res, " ");
   }
   res = concat_string_in_stack(res, "\n");
   for(int t = 0; t < global->NUM_TABLES; t++)
   {
      if (t > 0)
         res = concat_string_in_stack(res, "  ");
      res = concat_string_in_stack(res, BOX_TOP_LEFT);
      res = concat_string_in_stack(res, utf8HorizontalLine(lengthTable()-2));
      res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   }
   res = concat_string_in_stack(res, "\n");
   for(int p = 0; p < 2; p++) // to insert position to place books!
   {
      for(int t = 0; t < global->NUM_TABLES; t++)
      {
         if (t > 0)
            res = concat_string_in_stack(res, "  ");
         res = concat_string_in_stack(res, BOX_VERTICAL);
         //for(int i = 0; i < lengthTable()-2; i++)
         for(int s = 0; s < global->NUM_SEATS_PER_TABLE/2; s++)
         {
            int idx = p*global->NUM_TABLES*global->NUM_SEATS_PER_TABLE/2 + t*(global->NUM_SEATS_PER_TABLE/2)+s;
            if (idx < numSeats() && booksInSeat(idx))
            {
               if (library->seatOccupied[idx])
                  res = concat_string_in_stack(res, BOOK_OPEN);
               else
                  res = concat_string_in_stack(res, BOOKS);
            }
            else
               res = concat_string_in_stack(res, "  ");
            if (s < global->NUM_SEATS_PER_TABLE/2 - 1)
               res = concat_string_in_stack(res, " ");
         }
         res = concat_string_in_stack(res, BOX_VERTICAL);
      }
      res = concat_string_in_stack(res, "\n");
   }
   for(int t = 0; t < global->NUM_TABLES; t++)
   {
      if (t > 0)
         res = concat_string_in_stack(res, "  ");
      res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
      res = concat_string_in_stack(res, utf8HorizontalLine(lengthTable()-2));
      res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   }
   res = concat_string_in_stack(res, "\n");
   for(int t = 0; t < global->NUM_TABLES; t++)
   {
      if (t > 0)
         res = concat_string_in_stack(res, "  ");
      for(int s = 0; s < global->NUM_SEATS_PER_TABLE/2; s++)
      {
         int idx = global->NUM_TABLES*global->NUM_SEATS_PER_TABLE/2 + t*(global->NUM_SEATS_PER_TABLE/2)+s;
         if (idx < numSeats() && library->seatOccupied[idx])
            res = concat_string_in_stack(res, " "FACE_PENSIVE);
         else
            res = concat_string_in_stack(res, " "BLACK_RECTANGLE);
      }
      res = concat_string_in_stack(res, " ");
   }
   res = concat_string_in_stack(res, "\n");

   return stringClone(res);
}

static int getSeat()
{
   int res;
   for(res = 0; res < numSeats() && (library->seatOccupied[res] || booksInSeat(res)); res++)
      ;
   return res;
}

static int getRandomSeat()
{
   int res = -1;
   int count = randomInt(1, numSeats());
   for(int c = 0; c < count; c++)
   {
      res = (res+1) % numSeats();
      while(library->seatOccupied[res] || booksInSeat(res))
         res = (res+1) % numSeats();
   }
   if (library->seatOccupied[res] || booksInSeat(res))
      res = numSeats();
   return res;
}


static int lengthBookShelf()
{
   int res = strlen(descText);
   if (res < ((global->NUM_BOOKS+1)/2*6)+1)
      res = ((global->NUM_BOOKS+1)/2*6)+1;
   return res;
}

static int lengthTable()
{
   return (global->NUM_SEATS_PER_TABLE/2)*3+1;
}

/*
 * one table with 4 seats:
 *  -- -- 
 * |     |
 *  -- -- 
 */
static int lengthAllTables()
{
   return global->NUM_TABLES*lengthTable() + /* table interval: */ (global->NUM_TABLES-1)*2;
   //return 2*((numSeats()+numSeats()%2)/2)+1;
}

static int bookSearch(struct _Book_* book)
{
   int res;
   for(res = 0; res < global->NUM_BOOKS && !equalBook(book, &library->bookList[res]); res++)
      ;

   return res;
}

void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}