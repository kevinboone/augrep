/*============================================================================
  augrep 
  filetypehandler.h 
  Copyright (c)2018 Kevin Boone, GPL v3.0
============================================================================*/
#pragma once

#include "list.h"

typedef void (*FileTypeHandler) (const char *filename, List *list);

