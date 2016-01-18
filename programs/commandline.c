/*
  commandline.c - simple command line interface for FSE
  Copyright (C) Yann Collet 2013-2015

  GPL v2 License

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

  You can contact the author at :
  - Public forum : https://groups.google.com/forum/#!forum/lz4c
  */
/*
  Note : this is stand-alone program.
  It is not part of FSE compression library, just a user program of the FSE library.
  The license of FSE library is BSD.
  The license of this program is GPLv2.
  */


/***************************************************
*  Compiler instructions
****************************************************/
#define _CRT_SECURE_NO_WARNINGS   /* Remove warning under visual studio */
#define _POSIX_SOURCE 1           /* get fileno() within <stdio.h> for Unix */


/***************************************************
*  Includes
***************************************************/
#include <stdlib.h>   /* exit */
#include <stdio.h>    /* fprintf */
#include <string.h>   /* strcmp, strcat */
#include "bench.h"
#include "fileio.h"   /* FIO_setCompressor */

#include "isaac64\isaac64.h"

//#include "fileiozstd.h"   /* FIO_setCompressor */


/***************************************************
*  OS-specific Includes
***************************************************/
#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>    /* _O_BINARY */
#  include <io.h>       /* _setmode, _isatty */
#  ifdef __MINGW32__
int _fileno(FILE *stream);   /* MINGW somehow forgets to include this windows declaration into <stdio.h> */
#  endif
#  define SET_BINARY_MODE(file) _setmode(_fileno(file), _O_BINARY)
#  define IS_CONSOLE(stdStream) _isatty(_fileno(stdStream))
#else
#  include <unistd.h>   /* isatty */
#  define SET_BINARY_MODE(file)
#  define IS_CONSOLE(stdStream) isatty(fileno(stdStream))
#endif


/***************************************************
*  Constants
***************************************************/
#define COMPRESSOR_NAME "FSE : Finite State Entropy"
#define AUTHOR "Yann Collet"
#define WELCOME_MESSAGE "%s, %i-bits demo by %s (%s)\n", COMPRESSOR_NAME, (int)sizeof(void*)*8, AUTHOR, __DATE__
#define FSE_EXTENSION ".fse"


/***************************************************
*  Macros
***************************************************/
#define DISPLAY(...)         fprintf(stderr, __VA_ARGS__)
#define DISPLAYLEVEL(l, ...) if (displayLevel>=l) { DISPLAY(__VA_ARGS__); }


/***************************************************
*  Local variables
***************************************************/
static char* programName;
static int   displayLevel = 2;   // 0 : no display  // 1: errors  // 2 : + result + interaction + warnings ;  // 3 : + progression;  // 4 : + information
static int   fse_pause = 0;


/***************************************************
*  Functions
***************************************************/
static int usage(void)
{
	DISPLAY("Usage :\n");
	DISPLAY("%s [arg] inputFilename [outputFilename]\n", programName);
	DISPLAY("Arguments %s :\n", getNumber64());
	DISPLAY("(default): fse core loop timing tests\n");
	DISPLAY(" -e : use fse (default)\n");
	DISPLAY(" -h : use huff0\n");
	DISPLAY(" -p#: use password to encode\\decode\n");
	DISPLAY(" -z : use zlib's huffman\n");
	DISPLAY(" -s : use ZSTD\n");
	DISPLAY(" -c : decompression ZSTD\n");
	DISPLAY(" -d : decompression (default for %s extension)\n", FSE_EXTENSION);
	DISPLAY(" -b : benchmark mode\n");
	DISPLAY(" -i#: iteration loops [1-9](default : 4), benchmark mode only\n");
	DISPLAY(" -B#: block size (default : 32768), benchmark mode only\n");
	DISPLAY(" -H : display help and exit\n");
	return 0;
}


static int badusage(void)
{
	DISPLAYLEVEL(1, "Incorrect parameters\n");
	if (displayLevel >= 1) usage();
	exit(1);
}


static void waitEnter(void)
{
	int unused;
	DISPLAY("Press enter to continue...\n");
	unused = getchar();
	(void)unused;
}

int main(int argc, char** argv)
{
	int   i,
		forceCompress = 1, decode = 0, bench = 0; /* default action if no argument */
	int   indexFileNames = 0;
	const char* input_filename = NULL;
	const char* output_filename = NULL;
	const char* passwordValue = NULL;
	char*  tmpFilenameBuffer = NULL;
	size_t tmpFilenameSize = 0;
	char  extension[] = FSE_EXTENSION;
	FIO_compressor_t compressor = FIO_fse;

	const char* dictFileName = NULL;
	int nextEntryIsDictionary = 0;

	int zstdCompression = 0;
	int zstdDecompression = 0;


	/* Welcome message */
	programName = argv[0];
	DISPLAY(WELCOME_MESSAGE);

	if (argc < 1) badusage();

	for (i = 1; i <= argc; i++)
	{
		char* argument = argv[i];

		if (!argument) continue;   /* Protection if argument empty */

		// Decode command (note : aggregated commands are allowed)
		if (argument[0] == '-')
		{
			// '-' means stdin/stdout
			if (argument[1] == 0)
			{
				if (!input_filename) input_filename = stdinmark;
				else output_filename = stdoutmark;
			}

			while (argument[1] != 0)
			{
				argument++;

				switch (argument[0])
				{
					// Display help
				case 'V': DISPLAY(WELCOME_MESSAGE); return 0;   // Version
				case 'H': usage(); return 0;

					// Decoding
				case 'd': decode = 1; bench = 0; break;

					// Benchmark full mode
				case 'b': bench = 1; break;

					// fse selection (default)
				case 'e':
					BMK_SetByteCompressor(1);
					compressor = FIO_fse;
					break;
				case 's':
					DISPLAY("\nZSTD compression\n");
					// need to add compression
					zstdCompression = 1;
					//DISPLAY(zstd);
					break;
					// zstd selection
				case 'c':
					DISPLAY("\nZSTD decompression\n");
					decode = 1;
					zstdDecompression = 1;
					break;
				case 'a':
					DISPLAY("\nZSTD compression\n");
					// need to add compression
					//zstd = 1;
					//DISPLAY(zstd);
					break;
					// zstd selection
				case 'h':
					BMK_SetByteCompressor(2);
					compressor = FIO_huff0;
					break;

					// zlib mode
				case 'z':
					BMK_SetByteCompressor(3);
					compressor = FIO_zlibh;
					break;

					// Test
				case 't': decode = 1; output_filename = nulmark; break;

					// Overwrite
				case 'f': FIO_overwriteMode(); break;

					// Verbose mode
				case 'v': displayLevel = 4; break;

					// Quiet mode
				case 'q': displayLevel--; break;

					// keep source file (default anyway, so useless) (for xz/lzma compatibility)
				case 'k': break;

					// Modify Block Properties
				case 'B':
				{
							unsigned bSize = 0;
							while ((argument[1] >= '0') && (argument[1] <= '9'))
							{
								unsigned digit = argument[1] - '0';
								bSize *= 10;
								bSize += digit;
								argument++;
							}
							if (argument[1] == 'K') bSize <<= 10, argument++;  /* allows using KB notation */
							if (argument[1] == 'M') bSize <<= 20, argument++;
							if (argument[1] == 'B') argument++;
							BMK_SetBlocksize(bSize);
				}
					break;

					// Modify Stream properties
				case 'S': break;   // to be completed later

					// Modify Nb Iterations (benchmark only)
				case 'i':
					if ((argument[1] >= '1') && (argument[1] <= '9'))
					{
						int iters = argument[1] - '0';
						BMK_SetNbIterations(iters);
						argument++;
					}
					break;

					// Pause at the end (hidden option)
				case 'P': fse_pause = 1; break;

				case 'p':
					argument++;
					passwordValue = argument;
					argument = "";
					break;


					// Change FSE table size (hidden option)
				case 'M':
					if ((argument[1] >= '1') && (argument[1] <= '9'))
					{
						int tableLog = argument[1] - '0';
						BMK_SetTableLog(tableLog);
						argument++;
					}
					break;

					/* Unrecognised command */
				default: badusage();
				}
			}
			continue;
		}

		/* first provided filename is input */
		if (!input_filename) { input_filename = argument; indexFileNames = i; continue; }

		/* second provided filename is output */
		if (!output_filename) { output_filename = argument; continue; }

		/* dictionary */
		if (nextEntryIsDictionary)
		{
			nextEntryIsDictionary = 0;
			dictFileName = argument;
			continue;
		}
	}





	/* DISPLAYLEVEL(3, WELCOME_MESSAGE); */

	/* No input filename ==> use stdin */
	if (!input_filename) { input_filename = stdinmark; }

	/* Check if input is defined as console; trigger an error in this case */
	if (!strcmp(input_filename, stdinmark) && IS_CONSOLE(stdin)) badusage();

	/* Check if benchmark is selected */
	if (bench == 1) { BMK_benchFiles(argv + indexFileNames, argc - indexFileNames); goto _end; }
	if (bench == 3) { BMK_benchCore_Files(argv + indexFileNames, argc - indexFileNames); goto _end; }   /* no longer possible */

	/* No output filename ==> try to select one automatically (when possible) */
	while (!output_filename)
	{
		if (!IS_CONSOLE(stdout)) { output_filename = stdoutmark; break; }   // Default to stdout whenever possible (i.e. not a console)
		if ((!decode) && !(forceCompress))   // auto-determine compression or decompression, based on file extension
		{
			size_t l = strlen(input_filename);
			if (!strcmp(input_filename + (l - 4), FSE_EXTENSION)) decode = 1;
		}
		if (!decode)   /* compression to file */
		{
			size_t l = strlen(input_filename);
			if (tmpFilenameSize < l + 6) tmpFilenameSize = l + 6;
			tmpFilenameBuffer = (char*)calloc(1, tmpFilenameSize);
			if (tmpFilenameBuffer == NULL)
			{
				DISPLAY("Not enough memory, exiting ... \n");
				exit(1);
			}
			strcpy(tmpFilenameBuffer, input_filename);
			strcpy(tmpFilenameBuffer + l, FSE_EXTENSION);
			output_filename = tmpFilenameBuffer;
			DISPLAYLEVEL(2, "Compressed filename will be : %s \n", output_filename);
			break;
		}
		/* decompression to file (automatic name will work only if input filename has correct format extension) */
		{
			size_t outl;
			size_t inl = strlen(input_filename);
			if (tmpFilenameSize < inl + 2) tmpFilenameSize = inl + 2;
			tmpFilenameBuffer = (char*)calloc(1, tmpFilenameSize);
			strcpy(tmpFilenameBuffer, input_filename);
			outl = inl;
			if (inl>4)
			while ((outl >= inl - 4) && (input_filename[outl] == extension[outl - inl + 4])) tmpFilenameBuffer[outl--] = 0;
			if (outl != inl - 5) { DISPLAYLEVEL(1, "Cannot determine an output filename\n"); badusage(); }
			output_filename = tmpFilenameBuffer;
			DISPLAYLEVEL(2, "Decoding into filename : %s \n", output_filename);
		}
	}

	/* No warning message in pure pipe mode (stdin + stdout) */
	if (!strcmp(input_filename, stdinmark) && !strcmp(output_filename, stdoutmark) && (displayLevel == 2)) displayLevel = 1;

	/* Check if input or output are defined as console; trigger an error in this case */
	if (!strcmp(input_filename, stdinmark) && IS_CONSOLE(stdin)) badusage();
	if (!strcmp(output_filename, stdoutmark) && IS_CONSOLE(stdout)) badusage();



	if (decode)
	{
		if (zstdDecompression){
			FIO_decompressZstdFilename(output_filename, input_filename, dictFileName);
		}
		else
			FIO_decompressFilename(output_filename, input_filename, passwordValue);

	}
	else
	{
		FIO_setCompressor(compressor);
		DISPLAY("WARTOSC zstdCompression = %d\n", zstdCompression);
		if (zstdCompression == 1){
			FIO_compressZstdFilename(output_filename, input_filename, dictFileName, 1, passwordValue);
		}
		else{
			//zstd = 0;
			FIO_compressFilename(output_filename, input_filename, passwordValue);
		}

	}

_end:
	if (fse_pause) waitEnter();
	free(tmpFilenameBuffer);
	return 0;
}
