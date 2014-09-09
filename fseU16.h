/* ******************************************************************
   FSEU16 : Finite State Entropy coder for 16-bits input
   header file
   Copyright (C) 2013-2014, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - Public forum : https://groups.google.com/forum/#!forum/lz4c
****************************************************************** */
#pragma once

#if defined (__cplusplus)
extern "C" {
#endif


/******************************************
   FSE U16 functions
******************************************/

/* same as FSE normal functions,
   but data is presented as a table of unsigned short (2 bytes per symbol).
   Useful for alphabet size > 256.
   All symbol values within input table must be < 'nbSymbols'.
   Maximum allowed 'nbSymbols' value is controlled by constant FSE_MAX_NB_SYMBOLS inside fse.c */
int FSE_compressU16  (void* dest,
                      const unsigned short* source, unsigned sourceSize, unsigned nbSymbols, unsigned tableLog);
int FSE_decompressU16(unsigned short* dest, unsigned originalSize,
                      const void* compressed);


/******************************************
   FSE U16 advanced functions
******************************************/
/*
FSE_decompressU16_safe():
    Same as FSE_decompressU16(), but ensures that the decoder never reads beyond compressed + maxCompressedSize.
    note : you don't have to provide the exact compressed size. If you provide more, it's fine too.
    This function is safe against malicious data.
    return : size of compressed data
             or -1 if there is an error
*/
int FSE_decompressU16_safe (unsigned short* dest, unsigned originalSize,
                            const void* compressed, unsigned maxCompressedSize);


#if defined (__cplusplus)
}
#endif