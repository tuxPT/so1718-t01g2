#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include "process.h"
#include "utils.h"
#include "logger.h"
#include "thread.h"
#include <csignal>

pid_t launchLogger();
pid_t launchClient(int id);

#define DEFAULT_NUM_CLIENTS 2
#define DEFAULT_NUM_MESSAGES 10
static int num_clients = DEFAULT_NUM_CLIENTS;
static int num_messages = DEFAULT_NUM_MESSAGES;

static void help(char* prog);
static void processArgs(int argc, char* argv[]);

int foo = 0;

void my_handler(int s)
{
	if (foo)
	{
		foo = 1;
		// printf("Caught signal %d\n", s);
		// termLogger();
		exit(1);
	}
}

int main(int argc, char*argv[])
{
	// Handle SIGTERM gracefully
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	processArgs(argc, argv);
   initLogger();

   int* ids = (int*)alloca(sizeof(int)*num_clients);

   for(int i = 0; i < num_clients; i++)
   {
      char msg[1000];
      sprintf(msg, "Client %d", i+1);
      ids[i] = registerLogger(strdup(msg), 1+i, 1+i, 1, 20, NULL);
   }

   pid_t logger = launchLogger();
   pid_t* clients = (pid_t*)alloca(sizeof(pid_t)*num_clients);

   for(int i = 0; i < num_clients; i++)
      clients[i] = launchClient(ids[i]);

   int status;
   for(int i = 0; i < num_clients; i++)
      pwaitpid(clients[i], &status, 0);

   termLogger();
   pwaitpid(logger, &status, 0);

   return 0;
}

pid_t launchLogger()
{
   pid_t res = pfork();

   if (res == 0) // child -> logger
   {
      mainLogger(NULL);
      exit(0);
   }

   return res;
}

pid_t launchClient(int id)
{
   pid_t res = pfork();
   if (res == 0) // child -> client
   {
      srand(time(0)+getpid());
      char msg[1000];
      for(int i = 0; i < num_messages; i++)
      {
         usleep(randomInt(100000, 1000000));
         sprintf(msg, "Message %d", i+1);
         sendLog(id, msg);
      }
      sprintf(msg, "Client %d DONE.", id+1);
      sendLog(id, msg);
      exit(0);
   }
   return res;
}

static void help(char* prog)
{
   assert (prog != NULL);

   printf("\n");
   printf("Usage: %s [OPTION] ...\n", prog);
   printf("\n");
   printf("Options:\n");
   printf("\n");
   printf("  -h,--help                                   show this help\n");
   printf("  -l,--line-mode\n");
   printf("  -w,--window-mode (default)\n");
   printf("  -c,--num-clients <NUM>\n");
   printf("  -m,--num-messages <NUM>\n");
   printf("\n");
}

static void processArgs(int argc, char* argv[])
{
   assert(argc >= 0 && argv != NULL && argv[0] != NULL);

   static struct option long_options[] =
   {
      {"help",                         no_argument,       NULL, 'h'},
      {"--line-mode",                  no_argument,       NULL, 'l'},
      {"--window-mode",                no_argument,       NULL, 'w'},
      {"--num-clients",                required_argument, NULL, 'c'},
      {"--num-messages",               required_argument, NULL, 'm'},
      {0, 0, NULL, 0}
   };
   int op=0;

   while (op != -1)
   {
      int option_index = 0;

      op = getopt_long(argc, argv, "hlwc:m:", long_options, &option_index);
      int st;
      switch (op)
      {
         case -1:
            break;

         case 'h':
            help(argv[0]);
            exit(EXIT_SUCCESS);

         case 'l':
            if (!lineMode())
               switchToLineMode();
            break;

         case 'w':
            if (lineMode())
               switchToWindowMode();
            break;

         case 'c':
            st = sscanf(optarg, "%d", &num_clients);
            if (st != 1 || num_clients < 1)
            {
               fprintf(stderr, "ERROR: invalid number of clients \"%s\"\n", optarg);
               exit(EXIT_FAILURE);
            }
            break;

         case 'm':
            st = sscanf(optarg, "%d", &num_messages);
            if (st != 1 || num_messages < 1)
            {
               fprintf(stderr, "ERROR: invalid number of messages \"%s\"\n", optarg);
               exit(EXIT_FAILURE);
            }
            break;

         default:
            help(argv[0]);
            exit(EXIT_FAILURE);
            break;
      }
   }

   if (optind < argc)
   {
      fprintf(stderr, "ERROR: invalid extra arguments\n");
      exit(EXIT_FAILURE);
   }
}

