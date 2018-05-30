/*============================================================================
  augrep 
  nvp.h 
  Copyright (c)2018 Kevin Boone, GPL v3.0
============================================================================*/
#pragma once

typedef struct _NVP
  {
  char *name;
  char *value;
  } NVP;

NVP   *nvp_create (const char *name, const char *value);
void   nvp_free (void *self); 


