/*============================================================================
  augrep 
  audiohandler.c
  Copyright (c)2018 Kevin Boone, GPL v3.0
============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include "defs.h"
#include "log.h"
#include "filetypehandler.h"
#include "audiohandler.h"
#include "tag_reader.h"
#include "list.h"
#include "nvp.h"

/*============================================================================
  audio_handler
  Given a filename, fill a list with name-value pair structs containing
  the metadata. There is no error return from this function -- an empty
  list represents either a valid file with no tags, or a file that could
  not be read
============================================================================*/
void audio_handler (const char *filename, List *list)
  {
  IN

  TagData *tag_data = NULL;
  TagResult r = tag_get_tags (filename, &tag_data);
  if (r == TAG_OK)
    {
    Tag *t = tag_data->tag;
    while (t)
      {
      char *common_name = strdup (t->frameId);
      NVP *nvp = nvp_create (common_name, (char *)t->data);
      free (common_name);
      list_append (list, nvp);
      t = t->next;
      }
    }
  else
    log_debug ("tag_get_tags returned error %d", r);

  tag_free_tag_data (tag_data);

  OUT
  }



