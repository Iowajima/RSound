/*  RSound - A PCM audio client/server
 *  Copyright (C) 2010-2011 - Hans-Kristian Arntzen
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

#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "endian.h"

#ifdef _WIN32
#include <io.h>
#include <rsound.h>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include "librsound/rsound.h"
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "config.h"
#endif

#undef CONST_CAST
#ifdef _WIN32
#undef close
#define close(x) closesocket(x)
#define CONST_CAST (const char*)
#else
#define CONST_CAST
#endif

#define READ_SIZE 1024
#define HEADER_SIZE 44

static int raw_mode = 0;
static uint32_t raw_rate = 44100;
static uint16_t channel = 2;
static int format = 0;
static int high_latency = 0;

static char port[128] = "";
static char host[1024] = "";
static char ident[256] = "rsdplay";

static int set_rsd_params(rsound_t *rd);
static void parse_input(int argc, char **argv);

static ssize_t read_all(FILE* fd, void *buf, size_t size);
static ssize_t write_all(int fd, const void *buf, size_t size);

static FILE* infile = NULL;

int main(int argc, char **argv)
{
   int rc;
   rsound_t *rd;
   char *buffer;

   parse_input(argc, argv);
   if ( rsd_init(&rd) < 0 )
   {
      fprintf(stderr, "Failed to initialize\n");
      exit(1);
   }

#ifdef _WIN32
   // Because Windows sucks.
   if ( infile == NULL )
   {
      setmode(0, O_BINARY);
   }
#endif

   if ( strlen(host) > 0 )
      rsd_set_param(rd, RSD_HOST, (void*)host);
   if ( strlen(port) > 0 )
      rsd_set_param(rd, RSD_PORT, (void*)port);

   if ( set_rsd_params(rd) < 0 )
   {
      fprintf(stderr, "Couldn't read data.\n");
      rsd_free(rd);
      return 1;
   }

   int rsd_fd = rsd_exec(rd);
   if ( rsd_fd < 0 )
   {
      fprintf(stderr, "Failed to establish connection to server\n");
      rsd_free(rd);
      return 1;
   }

   if ( high_latency )
   {
      int bufsiz = 1 << 20;
      setsockopt(rsd_fd, SOL_SOCKET, SO_SNDBUF, CONST_CAST &bufsiz, sizeof(int));
      int flag = 0;
      setsockopt(rsd_fd, IPPROTO_TCP, TCP_NODELAY, CONST_CAST &flag, sizeof(int));
   }

   buffer = malloc ( READ_SIZE );
   if ( buffer == NULL )
   {
      fprintf(stderr, "Failed to allocate memory for buffer\n");
      return 1;
   }

   for(;;)
   {
      rc = read_all(infile, buffer, READ_SIZE);
      if ( rc <= 0 )
         break;

      rc = write_all(rsd_fd, buffer, rc);
      if ( rc <= 0 )
         break;
   }

   free(buffer);
   if ( infile )
      fclose(infile);

   close(rsd_fd);

   return 0;
}

static ssize_t read_all(FILE* infile, void *buf, size_t size)
{
   size_t rc;

   rc = fread(buf, 1, size, (infile) ? infile : stdin);

   if ( rc != size )
      return 0;

   return (ssize_t)size;
}

static ssize_t write_all(int fd, const void *buf, size_t size)
{
   size_t has_written = 0;
   ssize_t rc;

   while ( has_written < size )
   {
      rc = send(fd, (const char*)buf + has_written, size - has_written, 0);

      if ( rc <= 0 )
         return rc;

      has_written += rc;
   }

   return (ssize_t)has_written;
}

static int set_rsd_params(rsound_t *rd)
{
   int rate, channels, bits;
   uint16_t temp_channels, temp_bits, temp_fmt;
   uint32_t temp_rate;

   int rc;
   char buf[HEADER_SIZE] = {0};

#define RATE 24
#define CHANNEL 22
#define BITS_PER_SAMPLE 34
#define FORMAT 20

   if ( !raw_mode )
   {  
      rc = read_all( infile, buf, HEADER_SIZE );
      if ( rc <= 0 )
         return -1;

      // We read raw little endian data from the wave input file. This needs to be converted to
      // host byte order when we pass it to rsd_set_param() later.
      temp_channels = *((uint16_t*)(buf+CHANNEL));
      temp_rate = *((uint32_t*)(buf+RATE));
      temp_bits = *((uint16_t*)(buf+BITS_PER_SAMPLE));
      temp_fmt = *((uint16_t*)(buf+FORMAT));
      if ( !is_little_endian() )
      {
         swap_endian_16(&temp_channels);
         swap_endian_16(&temp_bits);
         swap_endian_32(&temp_rate);
         swap_endian_16(&temp_fmt);
      }

      rate = (int)temp_rate;
      channels = (int)temp_channels;
      bits = (int)temp_bits;

      // Assuming too much, but hey. Not sure how to find big-endian or little-endian in wave files.
      if ( bits == 16 )
         format = RSD_S16_LE;
      else if ( bits == 8 && temp_fmt == 6 )
         format = RSD_ALAW;
      else if ( bits == 8 && temp_fmt == 7 )
         format = RSD_MULAW;
      else if ( bits == 8 )
         format = RSD_U8;
      else
      {
         fprintf(stderr, "Only 8 or 16 bit WAVE files supported.\n");
         rsd_free(rd);
         exit(1);
      }

   }
   else
   {
      rate = (int)raw_rate;
      channels = (int)channel;
   }

   rsd_set_param(rd, RSD_SAMPLERATE, &rate);
   rsd_set_param(rd, RSD_CHANNELS, &channels);
   rsd_set_param(rd, RSD_FORMAT, &format);
   rsd_set_param(rd, RSD_IDENTITY, ident);
   return 0;
}

static void print_help()
{
   printf("rsdplay (librsound) version %s - Copyright (C) 2010 Hans-Kristian Arntzen\n", RSD_VERSION);
   printf("=========================================================================\n");
   printf("Usage: rsdplay [ <hostname> | -p/--port | -h/--help | --raw | -r/--rate | -c/--channels | -B/--bits | -f/--file | -s/--server ]\n");

   printf("\nrsdplay reads PCM data only through stdin (default) or a file, and sends this data directly to an rsound server.\n"); 
   printf("Unless specified with --raw, rsdplay expects a valid WAV header to be present in the input stream.\n\n");
   printf(" Examples:\n"); 
   printf("\trsdplay foo.net < bar.wav\n");
   printf("\tcat bar.wav | rsdplay foo.net -p 4322 --raw -r 48000 -c 2\n\n");

   printf("-p/--port: Defines which port to connect to.\n");
   printf("\tExample: -p 18453. Defaults to port 12345.\n");
   printf("--raw: Enables raw PCM input. When using --raw, rsdplay will generate a fake WAV header\n");
   printf("-r/--rate: Defines input samplerate (raw PCM)\n");
   printf("\tExample: -r 48000. Defaults to 44100\n");
   printf("-c/--channel: Specifies number of sound channels (raw PCM)\n");
   printf("\tExample: -c 1. Defaults to stereo (2)\n");
   printf("-B: Specifies sample format in raw PCM stream\n");
   printf("\tSupported formats are: S16LE, S16BE, U16LE, U16BE, S8, U8, ALAW, MULAW.\n" 
         "\tYou can pass 8 and 16 also, which is equal to U8 and S16LE respectively.\n");
   printf("-h/--help: Prints this help\n");
   printf("-f/--file: Uses file rather than stdin\n");
   printf("-s/--server: More explicit way of assigning hostname\n");
   printf("-i/--identity: Defines the identity associated with this client. Defaults to \"rsdplay\"\n");
   printf("-H/--high: Uses a high-latency connection with big buffers. Ideal for transmission over internet.\n");
}

static void parse_input(int argc, char **argv)
{
   int c, option_index = 0;

   struct option opts[] = {
      { "raw", 0, NULL, 'R' },
      { "port", 1, NULL, 'p' },
      { "help", 0, NULL, 'h'},
      { "rate", 1, NULL, 'r'},
      { "channels", 1, NULL, 'c'},
      { "file", 1, NULL, 'f'},
      { "server", 1, NULL, 's'},
      { "identity", 1, NULL, 'i'},
      { "high", 0, NULL, 'H' },
      { NULL, 0, NULL, 0 }
   };

   char optstring[] = "r:p:hc:f:B:s:i:H";
   while ( 1 )
   {
      c = getopt_long ( argc, argv, optstring, opts, &option_index );

      if ( c == -1 )
         break;

      switch ( c )
      {
         case 'r':
            raw_rate = atoi(optarg);
            break;

         case 'i':
            strncpy(ident, optarg, sizeof(ident));
            ident[sizeof(ident)-1] = '\0';
            break;

         case 'f':
            infile = fopen(optarg, "rb");
            if ( infile == NULL )
            {
               fprintf(stderr, "Could not open file ...\n");
               exit(1);
            }
            break;

         case 'R':
            raw_mode = 1;
            break;

         case 'p':
            strncpy(port, optarg, 127);
            port[127] = 0;
            break;

         case 'c':
            channel = atoi(optarg);
            break;

         case '?':
            print_help();
            exit(1);

         case 'h':
            print_help();
            exit(0);

         case 's':
            strncpy(host, optarg, 1023);
            host[1023] = '\0';
            break;

         case 'B':
            if ( strcmp("S16LE", optarg) == 0 || strcmp("16", optarg) == 0 )
               format = RSD_S16_LE;
            else if ( strcmp("S16BE", optarg) == 0 )
               format = RSD_S16_BE;
            else if ( strcmp("U16LE", optarg) == 0 )
               format = RSD_U16_LE;
            else if ( strcmp("U16BE", optarg) == 0 )
               format = RSD_U16_BE;
            else if ( strcmp("S8", optarg) == 0 )
               format = RSD_S8;
            else if ( strcmp("U8", optarg) == 0 || strcmp("8", optarg) == 0 )
               format = RSD_U8;
            else if ( strcmp("ALAW", optarg) == 0 )
               format = RSD_ALAW;
            else if ( strcmp("MULAW", optarg) == 0 )
               format = RSD_MULAW;
            else if ( strcmp("S32LE", optarg) == 0 )
               format = RSD_S32_LE;
            else if ( strcmp("S32BE", optarg) == 0 )
               format = RSD_S32_BE;
            else if ( strcmp("U32LE", optarg) == 0 )
               format = RSD_U32_LE;
            else if ( strcmp("U32BE", optarg) == 0 )
               format = RSD_U32_BE;
            else
            {
               fprintf(stderr, "Invalid bit format.\n");
               print_help();
               exit(1);
            }
            break;

         case 'H':
            high_latency = 1;
            break;

         default:
            fprintf(stderr, "Error in parsing arguments.\n");
            exit(1);
      }

   }

   while ( optind < argc )
   {
      strncpy(host, argv[optind++], 1023);
      host[1023] = 0;
   }
}


