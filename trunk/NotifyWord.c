/* For using NotifyNotification structure and related-functions  */
#include <libnotify/notify.h>

/* For using malloc function */
#include <stdlib.h>

/* For using memset function */
#include <string.h>

/* Please this file for more information */
#include "debugmacro.h"

/* Define a delay time between 2 word notifications in seconds*/
#define DELAY_NOTIFY_WORD 5*60

/* Application name */
#define APP_NAME "NotifyWord"

/* Default notification title, this is used when word is empty  */
#define DEFAULT_NOTIFY_TITLE "NotifyWord"

/* Default notification content,this is used when mean is emty  */
#define DEFAULT_NOTIFY_CONTENT "Let's dance with word."

/* Fisrt start-up app notification title */
#define FIRST_TIME_NOTIFY_TTILE "New word here"

/* Fisrt start-up app notification content */
#define FIRST_TIME_NOTIFY_CONTENT "New mean here"

/* New word file name */
/* This file is in following format:*/
/*
  <new word> : <Mean> 
 */
#define WORD_FILE "word.txt"

/* Log file which store reading offset in new word file above*/
#define LOG_FILE "notifylog.txt"

/* The longest possible length for word */
#define WORD_MAX_LENGTH 200

/* The longest possible length for mean */
#define MEAN_MAX_LENGTH 700

/* The longest possible length for a line*/
#define LINE_MAX_LENGTH (WORD_MAX_LENGTH + MEAN_MAX_LENGTH + 2)

/* Seperator which is between word and mean */
#define WORD_MEAN_SEPERATOR ':'
#define SHIFT_JS_WORD_MEAN_SEPERATOR_STR "："

/* File pointer of word file */
FILE *wordFile = NULL;

/* Reading offset in word file */
long wordOffset = 0;

/* File pointer for mean file */
FILE *logFile = NULL;

  
/* Prototype functions */

/* Prototype of main functions */

long loadWordOffset(FILE *logFile);

/* Read a new line (word and mean), return current reading offset */
int readNewWord(FILE *fi, char *word, char* mean); 

/* Write reading offset to logFile */
void writeLogFile(FILE *fo, unsigned int offset);

/* Close word file */
void closeWordFile(FILE *fi);

/* Close log file */
void closeLogFile(FILE *fo);

/*Prototype for system event handler */

/* Handler when this thread receive SIGINT signal */
void sig_int_handler(int signum); 

/* Handler when this thread receive SIGHUP signal  */
void sig_hup_handler(int signum);

/*Finish prototype */

/* Implement for defined above prototypes */

/**
   @function   : main
   @description: Normal main
   @return     : int
 */
int main (int argc, char *argv[])
{
  char newWord[WORD_MAX_LENGTH]; /* String which contains new word */
  char newMean[MEAN_MAX_LENGTH]; /* String which contains new mean */
	
	/* Register SIGINT signal handler as sig_int_handler() function */
	signal(SIGTERM, sig_int_handler);

	/* Register SIGHUP signal hander as sig_hup_handler() function */
	signal(SIGHUP, sig_hup_handler);
	
	/* NotifyNotification object */
	NotifyNotification *notification = NULL;
	
	/* Open word file */
	if (NULL == (wordFile = fopen(WORD_FILE,"rt")))
	{
	  _DEBUG("%s","Error reading word file\n");
	  return EXIT_FAILURE;
	}
	
	/* If log file already was exist, load reading offset  */
	if (NULL != (logFile = fopen(LOG_FILE,"rt")))
	  {	    
	    wordOffset = loadWordOffset(logFile);
	    _DEBUG("Word offset : %ld\n",wordOffset);
	    closeLogFile(logFile);
	  }
	else /* else no log file */
	  {
	    _DEBUG("%s","First time run NotifyWord\n");
	    _DEBUG("Log file : %s\n",LOG_FILE);
	  }
	
	/* Open log write for rewriting reading offset */
	if ((logFile = fopen(LOG_FILE,"wt")) == NULL)
	  {
	    _DEBUG("%s","Can't rewrite log file\n");
	  }
	
	/* Initialize Notify */
	if (! notify_init (APP_NAME))
	  {
	    _DEBUG ("%s","Notify init Error.\n");
	    return EXIT_FAILURE;
	  }
	
	/* Create new NotifyNotification object  */
	notification = notify_notification_new (
						"New word here",      /* The title of  notification  */
						"The mean is here",   /* The content of notification */
						"stock_dialog-info"); /* Icon id */
	notify_notification_set_timeout(notification,3000); /* Set timeout for notification (optional ) */
	notify_notification_set_urgency (notification,NOTIFY_URGENCY_CRITICAL); /* Set urgency (optional) */
	
	/* Jump to above reading offset position from file beginning */
	fseek(wordFile, wordOffset, SEEK_SET);
	
	unsigned int newOffsetWidth = 0; /* New offset value which is plus to wordOffset for new wordOffset */
	/* Unfinish loop */
	while (1)
	  {
	  memset(newWord, 0x00, sizeof(newWord)); /* Reset new word string */
	  memset(newMean, 0x00, sizeof(newMean)); /* Reset new mean string */
	  
	  newOffsetWidth = readNewWord(wordFile,newWord, newMean); /* Read new word and new mean */
	  wordOffset += newOffsetWidth; /* Plus newOffsetWidth to wordOffset */
	  if (0 == newOffsetWidth) /* If nothing to read (newOffsetWidth = 0)*/
	    { /* Jump to begining of word file */
	      wordOffset = 0;
	      fseek(wordFile, wordOffset, SEEK_SET);
	      continue;
	    }
	  /* Update title and content of above created notification */
	  notify_notification_update (
				      notification,
				      newWord,
				      newMean,
				      "stock_dialog-info"
				      );
	  notify_notification_show(notification, NULL ); /* Show it*/
	  
	  /* Write new wordOffset to log file */
	  writeLogFile(logFile,wordOffset);
	  /* Force to flush all data to file, to avoid unpredictable termination  */
	  fflush(logFile);
	  
	  /* Delay DELAY_NOTIFY_WORD */
	  sleep(DELAY_NOTIFY_WORD);
	}
	notify_uninit();	/* Finish notification */
	
	/* Close file */
	closeWordFile(wordFile);
	closeLogFile(logFile);
	return EXIT_SUCCESS;
}

/**
   @function   : readNewWord
   @param      : FILE* fi File Word file ponter
   @param      : Char* word String which contains new word
   @param      : Char* mean String which contain new mean
   @return     : Size of read line which is used offset width
   @description: Read a couple of word and mean from word file
 */
int readNewWord(FILE* fi, char* word, char* mean)
{
  if (NULL == fi)
    {
      return 0;
    }
  
  static char line[LINE_MAX_LENGTH]; /* Contain a line which will read */
  char *delimterStr = NULL; /* Contain pointer which contains address of mean part*/
  char *retStr = NULL; /* */
  memset(line , 0x00, sizeof(line)); /* Reset line string */
  unsigned int offsetWidth = 0;	/* Offset width */
  if ((retStr = fgets(line, LINE_MAX_LENGTH, fi)) != NULL) /* Read a line */
    {
      offsetWidth = strlen(line); 
      _DEBUG("Line : %s\n",line);
      /* Seperator as one byte character */
      delimterStr = strchr(line , WORD_MEAN_SEPERATOR); /* Find position which start width eperator character*/
      /* Seperator as multiple bytes character, in this case : Japanese*/
      if (NULL == delimterStr)
	{
	  delimterStr = strstr(line, SHIFT_JS_WORD_MEAN_SEPERATOR_STR);
	  if (delimterStr != NULL)
	    {
	      delimterStr += strlen(SHIFT_JS_WORD_MEAN_SEPERATOR_STR);
	    }
	}
      /* After*/
      if (delimterStr != NULL)
	{
	  *delimterStr = '\0'; /* Consider first part of line is word */
	  strcpy(word ,line ); /* Save to new word*/
	  strcpy(mean ,delimterStr + 1); /* Save to new mean */
	}
      else
	{
	  strcpy(word,DEFAULT_NOTIFY_TITLE);
	  strcpy(mean,line);
	}
    }
  return offsetWidth;
}

/**
   @function   : writeLogFile
   @param      : FILE* fo Log file pointer
   @param      : unsigned int offset Value of reading offset in word file
   @description: Update reading offset value
 */
void writeLogFile(FILE *fo, unsigned int offset)
{
  if (NULL == fo)
    {
      return;
    }
  fprintf(fo,"                         ");
  fflush(fo);
  fseek(logFile, 0, SEEK_SET);
  fprintf(fo,"offset:%d",offset);
}

/**
   @function   : closeWordFile
   @param      : FILE* fi Word file pointer
   @description: Closing word file pointer if it is openning.
 */
void closeWordFile(FILE *fi)
{
  if (fi != NULL)
    {
      fclose(fi);	
    }
  fi = NULL;
}

/**
   @function   : closeLogFile
   @param      : FILE* fo Log file pointer
   @description: Close word file pointer if it is openning
 */
void closeLogFile(FILE *fo)
{
  if (fo != NULL)
    {
      fclose(fo);
    }
  fo = NULL;	
}

/**
   @function   : loadWordOffset
   @param      : FILE* logFile Log file pointer
   @description: Loading reading offset
 */
long loadWordOffset(FILE *logFile)
{
  if (NULL == logFile)
    {
      return 0;
    } 
  char offsetLine[20];
  memset(offsetLine, 0x00, sizeof(offsetLine));
  fgets(offsetLine, sizeof(offsetLine) - 1, logFile);
  int i = 0;
  for (i = 0; i < sizeof(offsetLine); i++)
    {
      if (':' == offsetLine[i])
	{
	  break;
	}
    }
  return atol(offsetLine + i + 1);
}

/**
   @function   : sig_int_handler
   @param      : int sig_num Signal number
   @descripton : This is template for signal processing
 */
void sig_int_handler(int signum)
{
  _DEBUG("Receive signal int handler : %d",signum);
  closeLogFile(logFile);
  closeWordFile(wordFile);
  exit(EXIT_SUCCESS);
}

/**
   @function   : sig_hup_handler
   @param      : int sig_num Signal number
   @descripton : This is template for signal processing
 */

void sig_hup_handler(int signum)
{
  _DEBUG("Receive signal hup handler : %d",signum);
  closeLogFile(logFile);
  closeWordFile(wordFile);
  exit(EXIT_SUCCESS);
}
