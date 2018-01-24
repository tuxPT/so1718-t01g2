#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "utils.h"
#include "global.h"
#include "queue.h"
#include "logger.h"

#define MAX_REGISTS 100
#define MAX_FILTER 100

typedef struct _Regist_
{
   char* name;
   int line, column, numLines, numColumns;
   char** lineModeTranslations;
} Regist;

typedef struct _Event_
{
   int logId;
   char* text;
} Event;

static Regist* newRegist(char* name, int line, int column, int numLines, int numColumns, char** lineModeTranslations);
static Event* newEvent(int logId, char* text);
static void destroyEvent(Event* e);

static int eventExists();
static void processEvents();
static void printEvent(Event* e);

/* TODO: put your code here */

static Queue* queue = newQueue(NULL);
static Regist** areas = (Regist**)memAlloc(MAX_REGISTS*sizeof(Regist*));
static int areasLength = 0;
static int _alive_ = 1;
static int maxLine = 0;
static int _lineMode_ = 0;
static char** filterOut = (char**)memAlloc(MAX_FILTER*sizeof(char*));
static int filterOutLength = 0;
//Nossos static
static int mq_id;

int registerLogger(char* name, int line, int column, int numLines, int numColumns, char** lineModeTranslations)
{
   /* TODO: change this function to your needs */

   assert (name != NULL);
   assert (line >= 0);
   assert (column >= 0);
   assert (numLines > 0);
   assert (numColumns > 0);
   assert (validLineModeTranslation(lineModeTranslations));

   assert (areasLength < MAX_REGISTS);
   Regist* reg = newRegist(name, line, column, numLines, numColumns, lineModeTranslations);
   int res = areasLength;
   areas[areasLength] = reg;
   areasLength++;
   if (maxLine < line + numLines)
      maxLine = line + numLines;
   return res;
}

int alive()
{
   /* TODO: change this function to your needs */

   int res;
   res = _alive_ && eventExists();
   return res;
}

void initLogger()
{
   /* TODO: change this function to your needs */
   mq_id = mq_open("mq_logger", O_CREAT | O_RDONLY | 0600);
   assert(mq_id != (mqd_t)-1);
}

void termLogger()
{
   /* TODO: change this function to your needs */
   int status = mq_close(mq_id);
   assert(status == 0);
   _alive_ = 0;
}

int validLogId(int logId)
{
   /* TODO: change this function to your needs */

   int res;
   res = logId >= 0 && logId < areasLength;
   return res;
}

int getLineLogger(int logId)
{
   /* TODO: change this function to your needs */

   assert (validLogId(logId));

   int res;
   res = areas[logId]->line;
   return res;
}

int getColumnLogger(int logId)
{
   /* TODO: change this function to your needs */

   assert (validLogId(logId));

   int res;
   res = areas[logId]->column;
   return res;
}

int getNumLinesLogger(int logId)
{
   /* TODO: change this function to your needs */

   assert (validLogId(logId));

   int res;
   res = areas[logId]->numLines;
   return res;
}

int getNumColumnsLogger(int logId)
{
   /* TODO: change this function to your needs */

   assert (validLogId(logId));

   int res;
   res = areas[logId]->numColumns;
   return res;
}

void sendLog(int logId, char* text)
{
   /* TODO: change this function to your needs */

   assert (validLogId(logId));
   assert (text != NULL);

   mq_id = mq_open("logger_logger", O_WRONLY);
   assert(mq_id != (mqd_t)-1);
   char* e = (char*) newEvent(logId, text);
   int status = mq_send(mq_id, e, sizeof(*e), 0);
   assert(status == 0);

   
}

void* mainLogger(void* arg)
{
   if (!_lineMode_)
      clearConsole();
   while(alive())
   {
      /** TODO:
       * 1: wait for a log event (or termination)
       * 2. processEvents (if any)
       **/
      char* buf = (char*) malloc(sizeof(Event));
      int status = mq_receive(mq_id, buf, sizeof(*buf), 0);
      assert(status == 0);
      inQueue(queue, (void*)buf);
      processEvents();
      printf("mainLogger");
      printf("\n");
   }
   return NULL;
}

static int eventExists()
{
   return sizeQueue(queue) > 0;
}

static void processEvents()
{
   assert (eventExists());

   while(sizeQueue(queue) > 0)
   {
      Event* e = (Event*)outQueue(queue);
      //remove Lines 72255
      printf("\n");
      printf(" id = %d", e->logId);
      printf("\n");
      //
      printEvent(e);
      destroyEvent(e);
   }
}

// returns -1 if it is not a prefix, otherwise it returns the length of prefix
static int isPrefix(char* text, char* prefix)
{
   int res = 0;
   while(res >= 0 && prefix[res])
   {
      if (text[res] == prefix[res])
         res++;
      else
         res = -1;
   }
   return res;
}

static void printEvent(Event* e)
{
   assert (e != NULL);

   if (_lineMode_)
   {
      char *p = e->text;
      int spacePrinted = 1;
      int printed;

      char* logLine = (char*)"";
      logLine = concat_string_in_stack(logLine, areas[e->logId]->name);
      logLine = concat_string_in_stack(logLine, " ");

      while(*p != '\0')
      {
         printed = 0;
         // remove filterOut strings:
         int res = -1;
         for(int i = 0; res < 0 && i < filterOutLength; i++)
            res = isPrefix(p, filterOut[i]);
         if (res > 0)
         {
            p += res;
            if (!spacePrinted)
               logLine = concat_string_in_stack(logLine, " ");
            spacePrinted = 1;
            printed = 1;
         }

         char** translations = areas[e->logId]->lineModeTranslations;
         if (translations != NULL)
         {
            int res = 0;
            while (*translations != NULL)
            {
               res = isPrefix(p, *translations);
               if (res > 0)
               {
                  p += res;
                  logLine = concat_string_in_stack(logLine, (*(translations+1)));
                  if (strlen(*(translations+1)) > 0)
                  {
                     spacePrinted = (*(translations+1))[strlen(*(translations+1)) - 1] == ' ';
                  }
                  printed = 1;
               }
               translations+=2;
            }
         }

         if (!printed)
         {
            if (!spacePrinted || *p != ' ')
            {
               char tmp[2];
               tmp[0] = *p;
               tmp[1] = '\0';
               logLine = concat_string_in_stack(logLine, tmp);
               spacePrinted = *p == ' ';
            }
            p++;
         }
      }
      // trim
      while(*logLine != '\n' && *logLine == ' ')
         logLine++;
      for(int i = strlen(logLine)-1; logLine[i] == ' '; i++)
         logLine[i] = '\0';
      printf("%s\n", logLine);
   }
   else
   {
      char* t = e->text;

      int l = 0;
      int done = (*t == '\0');
      while(!done)
      {
         int end;
         for(end = 0; t[end] != '\0' && t[end] != '\n';end++)
            ;
         assert (t[end] == '\0' || t[end] == '\n');

         done = (t[end] == '\0');
         if (!done)
            t[end] = '\0';
         moveCursor(areas[e->logId]->line+l, areas[e->logId]->column);
         printf("%s", t);
         fflush(stdout);
         t += end+1;
         l++;
      }
   }
}

int validLineModeTranslation(char** lineModeTranslations)
{
   int res = 1;
   if (lineModeTranslations != NULL)
   {
      char** translations = lineModeTranslations;
      while (res && *translations != NULL)
      {
         res = *(translations+1) != NULL;
         translations+=2;
      }
   }
   return res;
}

/**
 * In line mode all "logging" is filtered and presented line by line
 */
int lineMode()
{
   return _lineMode_;
}

void switchToLineMode()
{
   assert (!lineMode());

   _lineMode_ = 1;
}

void switchToWindowMode()
{
   assert (lineMode());

   _lineMode_ = 0;
}

void addToFilterOut(char** remove)
{
   assert (remove != NULL);

   while(*remove != NULL)
   {
      assert (filterOutLength < MAX_FILTER);
      filterOut[filterOutLength] = *remove;
      filterOutLength++;
      remove++;
   }
}

static Regist* newRegist(char* name, int line, int column, int numLines, int numColumns, char** lineModeTranslations)
{
   Regist* res = (Regist*)memAlloc(sizeof(Regist));
   res->name = name;
   res->line = line;
   res->column = column;
   res->numLines = numLines;
   res->numColumns = numColumns;
   res->lineModeTranslations = lineModeTranslations;
   return res;
}

static Event* newEvent(int logId, char* text)
{
   Event* res = (Event*)memAlloc(sizeof(Event));
   res->logId = logId;
   res->text = strdup(text);
   return res;
}

static void destroyEvent(Event* e)
{
   free(e->text);
   free(e);
}

void dummyLogger()
{
   // this function is not to be used!!! (it is simply to show something in the given code)

   if (eventExists())
      processEvents();
}
