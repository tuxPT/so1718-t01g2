#include <stdlib.h>
#include <assert.h>
#include "sim-alloc.h"
#include "utils.h"
#include "global.h"
#include "timer.h"
#include "logger.h"
#include "book.h"
#include "library.h"
#include "librarian.h"
#include "student.h"
#include "process.h"
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
using namespace std;
#define DEBUG cout << "pid="<<getpid()<<" "<< __FILE__ << " : " << __func__<< " line:" << __LINE__ << endl

#define ACCESS_LIBRARY 0
#define SIT 1
#define REQBOOKS 2

enum State
{
   NONE = 0,
   NORMAL,
   BREAKFAST,
   LUNCH,
   DINNER,
   SLEEPING,
   REQ_BOOKS,
   REQ_SEAT,
   STUDYING,
   HAVING_FUN,
   DONE
};

static const char* stateText[11] =
{
   "  ",
   FACE_NEUTRAL,
   FOOD_BREAD,
   FOOD_PIZZA,
   FOOD_POT,
   FACE_SLEEPING,
   BOOKS,
   SEAT,
   FACE_THINKING,
   DANCER" ",
   FACE_SMILING_WITH_SUNGLASSES
};

typedef struct _Student_
{
   int alloc;
   char* name;
   State state;
   struct _CourseUnit_** courses;
   int numCourses;
   int* concludedCourses;
   int actualCourse;
   int completionPercentage;
   struct _Book_** bookList;
   struct _Book_** studyBookList;
   int* studyTime;
   int logId;
} Student;

static void life(Student* student);
static void enrollUniversity(Student* student);
static void unEnrollUniversity(Student* student);
static void enrollCourse(Student* student);
static void sleep(Student* student);
static void eat(Student* student, int meal); // 0: breakfast; 1: lunch; 2: dinner
static void study(Student* student);
static void fun(Student* student);
static void done(Student* student);
static int courseConcluded(Student* student);
static char* toStringStudent(struct _Student_* student);
static int lengthStudent();
static int chooseBooksToStudy(Student* student);
static int bookSearch(Student* student, struct _Book_* book);

struct _Student_* newStudent(struct _Student_* student, char* name, struct _CourseUnit_** courses, int line, int column)
{
   assert (name != NULL && strlen(name) > 0 && strlen(name) <= MAX_NAME_SIZE);
   assert (courses != NULL);
   assert (line >= 0);
   assert (column >= 0);

   if (student == NULL)
   {
      student = (Student*)memAlloc(sizeof(Student));
      student->alloc = 1;
   }
   else
      student->alloc = 0;

   student->name = name;
   student->state = NONE;
   student->courses = courses;
   for(student->numCourses = 0; courses[student->numCourses] != NULL; student->numCourses++)
      ;
   student->concludedCourses = (int*)memAlloc(sizeof(int)*student->numCourses);
   student->actualCourse = -1;
   student->completionPercentage = 0;
   student->bookList = NULL;
   student->studyBookList = NULL;
   student->studyTime = NULL;
   static const char* translations[] = {
      FACE_NEUTRAL, " NORMAL",
      FOOD_BREAD, " BREAKFAST",
      FOOD_PIZZA, " LUNCH",
      FOOD_POT, " DINNER",
      FACE_SLEEPING, " SLEEPING",
      BOOKS, " REQ_BOOKS",
      SEAT, " REQ_SEAT",
      FACE_THINKING, " STUDYING",
      DANCER" ", " HAVING_FUN",
      FACE_SMILING_WITH_SUNGLASSES, " DONE",
      NULL
   };
   student->logId = registerLogger((char*)"Student:", line ,column , 3, lengthStudent(), (char**)translations);
   printf("%d HERE !!!!\n",student->logId);
   sendLog(student->logId, toStringStudent(student));

   return student;
}

struct _Student_* destroyStudent(struct _Student_* student)
{
   assert (student != NULL);

   free(student->concludedCourses);
   if (student->alloc)
   {
     // free(student);
      student = NULL;
   }
   return student;
}

void* mainStudent(void* args)
{
   Student* student = (Student*)args;
   life(student);
   destroyStudent(student);
   return NULL;
}

int logIdStudent(struct _Student_* student)
{
   return student->logId;
}

static void life(Student* student)
{
   enrollUniversity(student);
   ////DEBUG;
   for(int c = 0; c < student->numCourses; c++)
   {
      //DEBUG;
      enrollCourse(student);
      do
      {
         sleep(student);
         ////DEBUG;
         //printf("LIFE !!!!!!!!!!!!!! %s -- %d\n ", student->name,student->state);
         eat(student, 0);
         ////DEBUG;
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
         study(student);
         ////DEBUG;
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
         eat(student, 1);
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
         study(student);
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
         eat(student, 2);
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
         fun(student);
         //printf("LIFE !!!!!!!!!!!!!!%s -- %d\n ", student->name,student->state);
      }
      while(!courseConcluded(student));
   }
   unEnrollUniversity(student);
   done(student);
}

static void enrollUniversity(Student* student)
{  
   /* TODO: student should notify librarian */
   reqEnrollStudent();
   // printf("\n");
   // printf("enrollUniversity - %s \n", student->name);
   //wait(&librarian);
	sendLog(student->logId, toStringStudent(student));
}

static void unEnrollUniversity(Student* student)
{
   /* TODO: student should notify librarian */
   reqDisenrollStudent();
   // printf("goodbye        ------>>>%s !!!!!!!!!!!!\n",student->name);
   // printf("\n");
   // printf("unEnrollUniversity - %s \n", student->name);
   //signal(&librarian);
	 sendLog(student->logId, toStringStudent(student));
}
   
static void enrollCourse(Student* student)
{
   // 1: set the student state;
   student->state = NONE;
      
   // 2: choose a random non-concluded course;
   int newCourse;
   do{
      newCourse = randomInt(0,student->numCourses-1);
   }while(student->concludedCourses[newCourse]!=0);
   student->actualCourse = newCourse;
   
   // 3: get booklist from course;
   student->bookList = bookListCourseUnit(student->courses[student->actualCourse]); 
   
   // 4: initialize student relevant state;
   student->state = NORMAL;
   student->studyTime = (int*)calloc(bookListLength(student->bookList),sizeof(int));
   student->completionPercentage=0;
	 sendLog(student->logId, toStringStudent(student));
	 // printf("\n");
   // printf("enrollCourse - %s \n", student->name);

}

static void sleep(Student* student)
{
   /** TODO:
    * 1: sleep (state: SLEEPING). Don't forget to spend time randomly in
    *    interval [global->MIN_SLEEPING_TIME_UNITS, global->MAX_SLEEPING_TIME_UNITS]
    **/
   student->state = SLEEPING;
   spend(randomInt(global->MIN_SLEEPING_TIME_UNITS,global->MAX_SLEEPING_TIME_UNITS));
	 sendLog(student->logId, toStringStudent(student));

	 // printf("\n");
   // printf("sleep - %s \n", student->name);

}

static void eat(Student* student, int meal) // 0: breakfast; 1: lunch; 2: dinner
{
   assert (meal >= 0 && meal <= 2);

   /** TODO:
    * 1: eat (state: BREAKFAST or LUNCH or DINNER). Don't forget to spend time randomly in
    *    interval [global->MIN_EATING_TIME_UNITS, global->MAX_EATING_TIME_UNITS]
    **/
   switch(meal)
   {
      case 0:
         student->state = BREAKFAST;
         break;
      case 1:
         student->state = LUNCH;
         break;
      case 2:
         student->state = DINNER;
         break;
   }
   spend(randomInt(global->MIN_EATING_TIME_UNITS,global->MAX_EATING_TIME_UNITS));
	 sendLog(student->logId, toStringStudent(student));

	// printf("\n");
   // printf("eat - %s \n", student->name);
   
}

static void study(Student* student)
{
   assert (student->completionPercentage == 100 || student->studyTime != NULL);
   ////DEBUG;
   if (student->completionPercentage < 100)
   {
      //////DEBUG;
		int n = chooseBooksToStudy(student); // (no need to understand the algorithm)
      int pos;
      ////DEBUG;

      // 1: request librarian(now strait from library) to requisite chosen books (state: REQ_BOOKS), wait until available
      student->state = REQ_BOOKS;
		sendLog(student->logId, toStringStudent(student));
      ////DEBUG;

		//waits for the books to be available
   
  
      requisiteBooksFromLibrary(student->studyBookList);


      ////DEBUG;
      student->state = REQ_SEAT;
      sendLog(student->logId, toStringStudent(student));
      
      pos = sit(student->studyBookList);
  
     

      // 4: study (state: STUDYING). Don't forget to spend time randomly in
      //    interval [global->MIN_STUDY_TIME_UNITS, global->MAX_STUDY_TIME_UNITS]
      student->state = STUDYING;
		int timeSpent = randomInt(global->MIN_STUDY_TIME_UNITS,global->MAX_STUDY_TIME_UNITS);
      spend(timeSpent);

	
      int progress = timeSpent/n;
		// 5: use time spent to update completion of course (studyTime field).
      //    Distribute time *equally* on all books studied (regardless of being completed)
      for(int i = 0; i < bookListLength(student->studyBookList); i++)
      {
         int idx = bookSearch(student,student->studyBookList[i]);
         student->studyTime[idx] += progress;
      }



		// 6: update field completionPercentage (by calling completionPercentageCourseUnit)
      student->completionPercentage = completionPercentageCourseUnit(student->courses[student->actualCourse],student->bookList,student->studyTime);
      sendLog(student->logId, toStringStudent(student));

		// 7: check if completed and act accordingly
      if(courseConcluded(student)){
         student->concludedCourses[student->actualCourse]= 1;
      }

      // 8: rise from the seat (study session finished)
      rise(pos);
      //psem_up(semid_lib,SIT);

		// leave books in table

      student->studyBookList = NULL;
      sendLog(student->logId, toStringStudent(student));

      reqCollectBooks();
      //free(studyBookListToBookListIndex);

   }


	// printf("\n");
   // printf("study - %s \n", student->name);
}

static void fun(Student* student)
{
   /** TODO:
    * 1: have fun (state: HAVING_FUN). Don't forget to spend time randomly in
    *    interval [global->MIN_FUN_TIME_UNITS, global->MAX_FUN_TIME_UNITS]
    **/
   student->state = HAVING_FUN;
   spend(randomInt(global->MIN_FUN_TIME_UNITS,global->MAX_FUN_TIME_UNITS));
	sendLog(student->logId, toStringStudent(student));

	// printf("\n");
   // printf("fun - %s \n", student->name);
}

static void done(Student* student)
{
   /** TODO:
    * 1:  life in university is over (state: DONE).
    **/
   
   //student = destroyStudent(student);???
   student->state = DONE;
	sendLog(student->logId, toStringStudent(student));

	// printf("\n");
   // printf("done - %s \n", student->name);
}

static int courseConcluded(Student* student)
{
   return student->completionPercentage == 100;
}

static char* toStringStudent(struct _Student_* student)
{
   char* res = (char*)"";
   res = concat_string_in_stack(res, BOX_TOP_LEFT);
   res = concat_string_in_stack(res, utf8HorizontalLine(MAX_NAME_SIZE+3));
   res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   res = concat_string_in_stack(res, BOX_TOP_LEFT);
   for(int i = 0; i < student->numCourses; i++)
   {
      int l = strlen(nameCourseUnit(student->courses[i]))+5;
      res = concat_string_in_stack(res, utf8HorizontalLine(l));
      if (i < student->numCourses-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
   }
   res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   if (student->studyBookList != NULL && *student->studyBookList != NULL)
   {
      res = concat_string_in_stack(res, BOX_TOP_LEFT);
      for(int b = 0; student->studyBookList[b] != NULL; b++)
      {
         if (b > 0)
            res = concat_string_in_stack(res, BOX_HORIZONTAL_DOWN);
         int l = strlen(nameBook(student->studyBookList[b]));
         res = concat_string_in_stack(res, utf8HorizontalLine(l));
      }
      res = concat_string_in_stack(res, BOX_TOP_RIGHT);
   }
   res = concat_string_in_stack(res, utf8HorizontalSpace(global->MAX_BOOKS_PER_COURSE*3+1));
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_VERTICAL);
   int l = MAX_NAME_SIZE - strlen(student->name);
   res = concat_string_in_stack(res, utf8HorizontalSpace(l));
   res = concat_string_in_stack(res, student->name);
   res = concat_string_in_stack(res, ":");
   res = concat_string_in_stack(res, stateText[student->state]);
   res = concat_string_in_stack(res, BOX_VERTICAL);
   res = concat_string_in_stack(res, BOX_VERTICAL);
   for(int i = 0; i < student->numCourses; i++)
   {
      res = concat_string_in_stack(res, nameCourseUnit(student->courses[i]));
      res = concat_string_in_stack(res, ":");
      if (student->concludedCourses[i])
         res = concat_string_in_stack(res, "100%");
      else if (i == student->actualCourse)
         res = concat_string_in_stack(res, percentageToString(NULL, student->completionPercentage));
      else
         res = concat_string_in_stack(res, " -- ");
      if (i < student->numCourses-1)
         res = concat_string_in_stack(res, BOX_VERTICAL);
   }
   res = concat_string_in_stack(res, BOX_VERTICAL);
   if (student->studyBookList != NULL && *student->studyBookList != NULL)
   {
      res = concat_string_in_stack(res, BOX_VERTICAL);
      for(int b = 0; student->studyBookList[b] != NULL; b++)
      {
         if (b > 0)
            res = concat_string_in_stack(res, " ");
         res = concat_string_in_stack(res, nameBook(student->studyBookList[b]));
      }
      res = concat_string_in_stack(res, BOX_VERTICAL);
   }
   res = concat_string_in_stack(res, utf8HorizontalSpace(global->MAX_BOOKS_PER_COURSE*3+1));
   res = concat_string_in_stack(res, "\n");
   res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
   res = concat_string_in_stack(res, utf8HorizontalLine(MAX_NAME_SIZE+3));
   res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
   for(int i = 0; i < student->numCourses; i++)
   {
      int l = strlen(nameCourseUnit(student->courses[i]))+5;
      res = concat_string_in_stack(res, utf8HorizontalLine(l));
      if (i < student->numCourses-1)
         res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
   }
   res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   if (student->studyBookList != NULL && *student->studyBookList != NULL)
   {
      res = concat_string_in_stack(res, BOX_BOTTOM_LEFT);
      for(int b = 0; student->studyBookList[b] != NULL; b++)
      {
         if (b > 0)
            res = concat_string_in_stack(res, BOX_HORIZONTAL_UP);
         int l = strlen(nameBook(student->studyBookList[b]));
         res = concat_string_in_stack(res, utf8HorizontalLine(l));
      }
      res = concat_string_in_stack(res, BOX_BOTTOM_RIGHT);
   }
   res = concat_string_in_stack(res, utf8HorizontalSpace(global->MAX_BOOKS_PER_COURSE*3+1));

   return stringClone(res);
}

static int lengthStudent()
{
   return MAX_NAME_SIZE+5+MAX_COURSE_UNITS*8+global->MAX_BOOKS_PER_COURSE*3;
}

static int chooseBooksToStudy(Student* student)
{
  int numBooks = bookListLength(student->bookList);

  int n = randomInt(1, numBooks);
  int b = 0;
  int idxList[numBooks];
  int idxListSize = 0;
  while(b < numBooks)
  {
     while (b < numBooks && student->studyTime[b] >= minStudyBookTimeUnitsCourseUnit(student->courses[student->actualCourse], student->bookList[b]))
        b++;
     if (b < numBooks)
     {
        idxList[idxListSize++] = b;
        b++;
     }
  }
  assert (idxListSize > 0);
  if (n > idxListSize)
     n = idxListSize;

  student->studyBookList = (struct _Book_**)memAlloc(sizeof(struct _Book_*)*(n+1));
  for(int i = 0; i < n; i++)
  {
     int exists;
     do
     {
        int r = randomInt(0, idxListSize-1);
        student->studyBookList[i] = student->bookList[idxList[r]];
        exists = 0;
        for(int j = 0; !exists && j < i; j++)
        {
           exists = (student->studyBookList[i] == student->studyBookList[j]);
        }
     }
     while(exists);
  }
  student->studyBookList[n] = NULL;

  return n;
}

static int bookSearch(Student* student, struct _Book_* book)
{
   int res;
   for(res = 0; student->bookList[res] != NULL && book != student->bookList[res]; res++)
      ;

   return res;
}

