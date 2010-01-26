/*  RSound - A PCM audio client/server
 *  Copyright (C) 2009 - Hans-Kristian Arntzen
 * 
 *  RSound is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RSound is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RSound.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OSS_H
#define OSS_H

#include "audio.h"
#include <sys/soundcard.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

typedef struct
{
   char *buffer;
   int audio_fd;
   uint32_t fragsize;
} oss_t;

#define OSS_DEVICE "/dev/dsp"

void* oss_thread( void* socket );
extern int verbose;
extern char device[];
#endif
