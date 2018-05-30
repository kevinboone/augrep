/*============================================================================
  augrep 
  nvp.c
  Copyright (c)2018 Kevin Boone, GPL v3.0
============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "log.h"
#include "nvp.h"

/*============================================================================
  nvp_create
============================================================================*/
NVP *nvp_create (const char *name, const char *value)
  {
  NVP *self = malloc (sizeof (NVP));
  self->name = strdup (name);
  self->value = strdup (value);
  return self;
  }

/*============================================================================
  nvp_free 
============================================================================*/
void nvp_free (void *_self)
  {
  NVP *self = _self;
  if (self)
    {
    if (self->name) free (self->name);
    if (self->value) free (self->value);
    free (self);
    }
  }


