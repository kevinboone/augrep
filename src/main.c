/*============================================================================
  augrep 
  main.c
  Copyright (c)2018 Kevin Boone, GPL v3.0
============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <regex.h>
#include "defs.h"
#include "log.h"
#include "filetypehandler.h"
#include "audiohandler.h"
#include "nvp.h"

/*============================================================================
  Definitions of common tag names. English only so far ;) 
============================================================================*/
#define TAG_LABEL_TITLE "title"
#define TAG_LABEL_ARTIST "artist"
#define TAG_LABEL_ALBUM_ARTIST "album-artist"
#define TAG_LABEL_GENRE "genre"
#define TAG_LABEL_ALBUM "album"
#define TAG_LABEL_COMPOSER "composer"
#define TAG_LABEL_DATE "date"
#define TAG_LABEL_TRACK "track"
#define TAG_LABEL_COMMENT "comment"


/*============================================================================
  forward defs 
============================================================================*/
void do_path (const char *key, const regex_t *preq, 
   const char *path, BOOL recursive, BOOL hidden, BOOL links, BOOL file_only,
   BOOL raw_labels, BOOL terminal);

/*============================================================================
  file type handlers 
  The use of a table of handlers is to allow for possible future
  expansion
============================================================================*/
FileTypeHandler handlers[] = { audio_handler, NULL };


/*============================================================================
  check_metadata 
  Check the metadata in *list against the specified key and regex.
  Dump the results to stdout
============================================================================*/
BOOL check_metadata (const char *key, const char *filename, 
     const regex_t *preq, List *list, BOOL file_only, BOOL terminal)
  {
  IN
  BOOL ret = FALSE;

  int len = list_length (list);
  
  BOOL shown_file = FALSE;
  for (int i = 0; i < len; i++)
    {
    const NVP *nvp = list_get (list, i);
    const char *name = nvp->name;
    const char *value = nvp->value;

    if (key == NULL || strcasecmp (key, name) == 0)
      {
      regmatch_t match[1];
      if (regexec (preq, value, 1, match, 0) == 0)
        {
        char *expanded_value = malloc (strlen (value) + 100);

        int start = match[0].rm_so;
        int end = match[0].rm_eo;

        strcpy (expanded_value, value); 
        if (terminal)
          {
          char *start_code = "\033[31m";
          char *end_code = "\033[0m";
   
          strcpy (expanded_value + start, start_code);
          strcpy (expanded_value + start + strlen (start_code), value + start);
          strcpy (expanded_value + end + strlen (start_code), end_code);
          strcpy (expanded_value + end + strlen (start_code) 
            + strlen (end_code), value + end);
           }

        if (!shown_file)
          {
          if (terminal && !file_only) printf ("\033[1m");
          printf ("%s\n", filename);
          if (terminal && !file_only) printf ("\033[0m");
          shown_file = TRUE;
          }
        if (!file_only)
          printf ("%s: %s\n", name, expanded_value);
        free (expanded_value);
        }
      }
    }
  if (shown_file && !file_only)
    printf ("\n");

  OUT
  return ret;
  }

/*============================================================================
  _map_name
  Return the human-readable, common name corresponding to various internal
  tag IDs. This operation is only meaningful for the subset of metadata that
  is used in all supported file types -- title, artist, etc
============================================================================*/
char *map_name (const char *frameId)
  {
  if (strcasecmp (frameId, "TIT2") == 0) return strdup (TAG_LABEL_TITLE);
  if (strcasecmp (frameId, "TT2") == 0) return strdup (TAG_LABEL_TITLE);
  if (strcasecmp (frameId, "TITLE") == 0) return strdup (TAG_LABEL_TITLE);
  if (strcasecmp (frameId, "nam") == 0) return strdup (TAG_LABEL_TITLE);

  if (strcasecmp (frameId, "TPE1") == 0) return strdup (TAG_LABEL_ARTIST);
  if (strcasecmp (frameId, "TP1") == 0) return strdup (TAG_LABEL_ARTIST);
  if (strcasecmp (frameId, "ARTIST") == 0) return strdup (TAG_LABEL_ARTIST);
  if (strcasecmp (frameId, "PERFORMER") == 0) return strdup (TAG_LABEL_ARTIST);
  if (strcasecmp (frameId, "ART") == 0) return strdup (TAG_LABEL_ARTIST);

  if (strcasecmp (frameId, "TPE2") == 0) return strdup (TAG_LABEL_ALBUM_ARTIST);
  if (strcasecmp (frameId, "TP2") == 0) return strdup (TAG_LABEL_ALBUM_ARTIST);
  if (strcasecmp (frameId, "ALBUMALBUM_ARTIST") == 0) 
       return strdup (TAG_LABEL_TITLE);
  if (strcasecmp (frameId, "aART") == 0) return strdup (TAG_LABEL_ALBUM_ARTIST);

  if (strcasecmp (frameId, "TCON") == 0) return strdup (TAG_LABEL_GENRE);
  if (strcasecmp (frameId, "TCO") == 0) return strdup (TAG_LABEL_GENRE);
  if (strcasecmp (frameId, "GENRE") == 0) return strdup (TAG_LABEL_GENRE);
  if (strcasecmp (frameId, "gen") == 0) return strdup (TAG_LABEL_GENRE);
  if (strcasecmp (frameId, "gnre") == 0) return strdup (TAG_LABEL_GENRE);

  if (strcasecmp (frameId, "TALB") == 0) return strdup (TAG_LABEL_ALBUM);
  if (strcasecmp (frameId, "TAL") == 0) return strdup (TAG_LABEL_ALBUM);
  if (strcasecmp (frameId, "ALBUM") == 0) return strdup (TAG_LABEL_ALBUM);
  if (strcasecmp (frameId, "alb") == 0) return strdup (TAG_LABEL_ALBUM);

  if (strcasecmp (frameId, "TCOM") == 0) return strdup (TAG_LABEL_COMPOSER);
  if (strcasecmp (frameId, "TCM") == 0) return strdup (TAG_LABEL_COMPOSER);
  if (strcasecmp (frameId, "COMPOSER") == 0) return strdup (TAG_LABEL_COMPOSER);
  if (strcasecmp (frameId, "wrt") == 0) return strdup (TAG_LABEL_COMPOSER);

  if (strcasecmp (frameId, "TYER") == 0) return strdup (TAG_LABEL_DATE);
  if (strcasecmp (frameId, "TYE") == 0) return strdup (TAG_LABEL_DATE);
  if (strcasecmp (frameId, "DATE") == 0) return strdup (TAG_LABEL_DATE);
  if (strcasecmp (frameId, "day") == 0) return strdup (TAG_LABEL_DATE);

  if (strcasecmp (frameId, "TRCK") == 0) return strdup (TAG_LABEL_TRACK);
  if (strcasecmp (frameId, "TRK") == 0) return strdup (TAG_LABEL_TRACK);
  if (strcasecmp (frameId, "TRACKNUMBER") == 0) return strdup (TAG_LABEL_TRACK);
  if (strcasecmp (frameId, "trkn") == 0) return strdup (TAG_LABEL_TRACK);

  if (strcasecmp (frameId, "COMM") == 0) return strdup (TAG_LABEL_COMMENT);
  if (strcasecmp (frameId, "COM") == 0) return strdup (TAG_LABEL_COMMENT);
  if (strcasecmp (frameId, "DESCRIPTION") == 0) return strdup (TAG_LABEL_COMMENT);
  if (strcasecmp (frameId, "COMMENT") == 0) return strdup (TAG_LABEL_COMMENT);
  if (strcasecmp (frameId, "cmt") == 0) return strdup (TAG_LABEL_COMMENT);

  return strdup (frameId);
  }


/*============================================================================
  substitute_common_names
  Scan a list of metadata entries, and replace entries that are common
  to all file types with a human-readable name 
============================================================================*/
void substitute_common_names (List *list)
  {
  IN
  int len = list_length (list);
  for (int i = 0; i < len; i++)
    {
    NVP *nvp = list_get (list, i);
    char *name = nvp->name;
    char *new_name = map_name (name);
    free (name);
    nvp->name = new_name;
    }
  OUT
  }


/*============================================================================
  do_file 
============================================================================*/
void do_file (const char *path, const char *key, const regex_t *preq, 
    BOOL file_only, BOOL raw_labels, BOOL terminal)
  {
  IN
  log_debug ("Checking file %s", path);
  BOOL found = FALSE;
  int i = 0;
  FileTypeHandler handler = handlers [i];
  while (!found && handler != NULL)
    {
    handler = handlers [i];
    if (handler)
      {
      List *list = list_create (nvp_free); 
      log_debug ("Trying file handler %d", i);
      handler (path, list);
      int len = list_length (list);
      if (len > 0)
        {
        if (!raw_labels)
          substitute_common_names (list);
        log_debug ("handler found %d tags", len);
        found = check_metadata (key, path, preq, list, file_only, terminal);
        }
      list_destroy (list);
      }
    i++;
    }
  OUT
  }


/*============================================================================
  expand_directory 
============================================================================*/
void expand_directory (const char *key, const regex_t *preq, 
     const char *path, BOOL hidden, BOOL links, BOOL file_only, 
     BOOL raw_labels, BOOL terminal)
  {
  IN 
  DIR *dir = opendir (path);
  if (dir)
    {
    struct dirent *d = readdir (dir);
    while (d) 
      {
      BOOL skip = FALSE;
      const char *name = d->d_name;   
      if (strcmp (name, ".") == 0) skip = TRUE;
      if (strcmp (name, "..") == 0) skip = TRUE;
      if (name[0] == '.' && !hidden) skip = TRUE; 
      if (!skip)
        {
        char *newpath = malloc (strlen(path) + 5 + strlen(name));
        strcpy (newpath, path);
        if (newpath[strlen(newpath) - 1] != '/')
          strcat (newpath, "/");
        strcat (newpath, name);

        do_path (key, preq, newpath, TRUE, hidden, links, 
         file_only, raw_labels, terminal);

        free (newpath);
        }

      d = readdir (dir);
      } 
    closedir (dir);
    }
  else
   log_warning ("Can't expand directory %s: %s", path, strerror (errno));  
  OUT
  }


/*============================================================================
  do_path 
  Handle a path that may be a file or a directory (or something else)
============================================================================*/
void do_path (const char *key, const regex_t *preq, 
     const char *path, BOOL recursive, BOOL hidden, BOOL links, BOOL file_only,
     BOOL raw_labels, BOOL terminal)
  {
  IN
  log_debug ("Considering path %s", path);

  struct stat sb;
  if (links)
    {
    if (stat (path, &sb))
      {
      log_error ("Can't stat %s: %s", path, strerror (errno));
      OUT
      return;
      }  
    }
  else
    {
    if (lstat (path, &sb))
      {
      log_error ("Can't stat %s: %s", path, strerror (errno));
      OUT
      return;
      }  
    }
  
  switch (sb.st_mode & S_IFMT) 
    {
    case S_IFBLK:  
    case S_IFCHR:  
       log_warning ("Skipping device %s", path);
       break;

    case S_IFIFO: 
       log_warning ("Skipping FIFO %s", path);
       break;

    case S_IFLNK: 
       log_warning ("Skipping symlink %s", path);
       // This will only happen if we did not specify -R 
       break;
 
    case S_IFSOCK: 
       log_warning ("Skipping socket %s", path);
       break;

    case S_IFDIR:  
       if (recursive)
         expand_directory (key, preq, path, hidden, links, 
         file_only, raw_labels, terminal);
       else
         log_debug ("Not expanding directory %s", path);
       break;
 
    case S_IFREG:  
       do_file (path, key, preq, file_only, raw_labels, terminal); 
       break;

    default: 
       log_warning ("Skipping unknown entity %s", path);
       break;
    }

  OUT
  }


/*============================================================================
  main
============================================================================*/
int main (int argc, char **argv)
  {
  BOOL show_version = FALSE;
  BOOL show_help = FALSE;
  BOOL recursive = FALSE;
  BOOL links = FALSE;
  BOOL all = FALSE;
  BOOL case_insens = FALSE;
  BOOL file_only = FALSE;
  BOOL raw_labels = FALSE;
  BOOL terminal = FALSE;

 if (isatty (STDOUT_FILENO))
    {
    terminal = TRUE;
    }

  static struct option long_options[] =
    {
     {"all", required_argument, NULL, 'a'},
     {"version", no_argument, NULL, 'v'},
     {"recursive", no_argument, NULL, 'r'},
     {"dereference-recursive", no_argument, NULL, 'R'},
     {"file-only", no_argument, NULL, 'f'},
     {"help", no_argument, NULL, 'h'},
     {"ignore-case", required_argument, NULL, 'i'},
     {"no-remap", no_argument, NULL, 'n'},
     {"log", required_argument, NULL, 'l'},
     {0, 0, 0, 0}
    };


  log_set_level (WARNING);
  
  int opt;
  while (1)
    {
    int option_index = 0;
    opt = getopt_long (argc, argv, "nfirRavhl:",
      long_options, &option_index);

    if (opt == -1) break;

    switch (opt)
      {
      case 0:
        if (strcmp (long_options[option_index].name, "version") == 0)
          show_version = TRUE;
        else if (strcmp (long_options[option_index].name, "help") == 0)
          show_help = TRUE; 
        else if (strcmp (long_options[option_index].name, "all") == 0)
          all = TRUE; 
        else if (strcmp (long_options[option_index].name, "recursive") == 0)
          recursive = TRUE; 
        else if (strcmp (long_options[option_index].name, "ignore-case") == 0)
          case_insens = TRUE; 
        else if (strcmp (long_options[option_index].name, "file-only") == 0)
          file_only = TRUE; 
        else if (strcmp (long_options[option_index].name, "no-remap") == 0)
          raw_labels = TRUE; 
        else if (strcmp (long_options[option_index].name, "log") == 0)
          log_set_level (atoi (optarg));
        else
          exit (-1);
      case 'a':
        all = TRUE; break;
      case 'f':
        file_only = TRUE; break;
      case 'i':
        case_insens = TRUE; break;
      case 'l':
        log_set_level (atoi(optarg)); break;
      case 'h':
        show_help = TRUE; break;
      case 'r':
        recursive = TRUE; break;
      case 'R':
        links = TRUE; 
        recursive = TRUE; 
        break;
      case 'n':
        raw_labels = TRUE; break;
        break;
      case 'v':
        show_version = TRUE; break;
        break;
      default:
        exit (-1);
      }
    }

  if (show_version)
    {
    printf (APPNAME " version " VERSION "\n");
    printf ("Copyright (c)2018 Kevin Boone\n");
    printf ("Distributed under the terms of the GNU Public Licence, v3.0\n");
    exit (0);
    }

  if (show_help)
    {
    printf ("Usage: %s [options] {pattern} {paths...}\n", argv[0]);
    printf ("  -a,--all              include hidden files\n");
    printf ("  -f,--file-only        report only filename\n");
    printf ("  -h,--help             show this message\n");
    printf ("  -i,--ignore-case      ignore text case\n");
    printf ("  -l,--log=N            set log level, 0-4\n");
    printf ("  -n,--no-remap         don't re-map tag labels\n");
    printf ("  -r,--recursive        expand directories\n");
    printf 
      ("  -R,--dereference-recursive expand directories and follow links\n");
    printf ("  -v,--version               show version\n");
    exit (0);
    }

  if (optind >= (argc - 1))
    {
    fprintf (stderr, "%s: not enough arguments\n", argv[0]); 
    fprintf (stderr, "'%s --help' for usage\n", argv[0]); 
    exit (-1);
    }

  char *pattern_spec = strdup (argv[optind]);
  char *eqpos = strchr (pattern_spec, '=');
  char *key = NULL;
  char *pattern;

  if (eqpos)
    {
    *eqpos = 0;
    pattern = strdup (eqpos + 1);
    key = strdup (pattern_spec);
    }
  else
    {
    pattern = strdup (pattern_spec);
    key = NULL;
    }

  
  regex_t preq;
  int rflags = 0;
  if (case_insens) rflags |= REG_ICASE;
  if (regcomp (&preq, pattern, rflags) == 0) 
    {
    for (int i = optind + 1; i < argc; i++)
      {
      const char *path = argv[i];
      do_path (key, &preq, path, recursive, all, links, file_only, raw_labels,
        terminal);
      }
    regfree (&preq);
    }
  else
    log_error ("Invalid regular expression: %s", pattern);

  if (key) free (key);
  free (pattern);
  free (pattern_spec);
  return 0;
  }

