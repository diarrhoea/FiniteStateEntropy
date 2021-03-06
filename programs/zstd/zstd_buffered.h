/*
Buffered version of Zstd compression library
Copyright (C) 2015, Yann Collet.

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
- zstd source repository : https://github.com/Cyan4973/zstd
- ztsd public forum : https://groups.google.com/forum/#!forum/lz4c
*/
#ifndef ZSTD_BUFFERED_H
#define ZSTD_BUFFERED_H

/* The objects defined into this file should be considered experimental.
* They are not labelled stable, as their prototype may change in the future.
* You can use them for tests, provide feedback, or if you can endure risk of future changes.
*/

#if defined (__cplusplus)
extern "C" {
#endif

	/* *************************************
	*  Includes
	***************************************/
#include <stddef.h>   /* size_t */


	/* ***************************************************************
	*  Tuning parameters
	*****************************************************************/
	/*!
	*  ZSTD_DLL_EXPORT :
	*  Enable exporting of functions when building a Windows DLL
	*/
#if defined(_WIN32) && defined(ZSTD_DLL_EXPORT) && (ZSTD_DLL_EXPORT==1)
#  define ZSTDLIB_API __declspec(dllexport)
#else
#  define ZSTDLIB_API
#endif


	/* *************************************
	*  Streaming functions
	***************************************/
	typedef struct ZBUFF_CCtx_s ZBUFF_CCtx;
	ZSTDLIB_API ZBUFF_CCtx* ZBUFF_createCCtx(void);
	ZSTDLIB_API size_t      ZBUFF_freeCCtx(ZBUFF_CCtx* cctx);

	ZSTDLIB_API size_t ZBUFF_compressInit(ZBUFF_CCtx* cctx, int compressionLevel);
	ZSTDLIB_API size_t ZBUFF_compressWithDictionary(ZBUFF_CCtx* cctx, const void* src, size_t srcSize);
	ZSTDLIB_API size_t ZBUFF_compressContinue(ZBUFF_CCtx* cctx, void* dst, size_t* maxDstSizePtr, const void* src, size_t* srcSizePtr, unsigned scrambler);
	ZSTDLIB_API size_t ZBUFF_compressFlush(ZBUFF_CCtx* cctx, void* dst, size_t* maxDstSizePtr);
	ZSTDLIB_API size_t ZBUFF_compressEnd(ZBUFF_CCtx* cctx, void* dst, size_t* maxDstSizePtr);

	/** ************************************************
	*  Streaming compression
	*
	*  A ZBUFF_CCtx object is required to track streaming operation.
	*  Use ZBUFF_createCCtx() and ZBUFF_freeCCtx() to create/release resources.
	*  Use ZBUFF_compressInit() to start a new compression operation.
	*  ZBUFF_CCtx objects can be reused multiple times.
	*
	*  Optionally, a reference to a static dictionary can be created with ZBUFF_compressWithDictionary()
	*  Note that the dictionary content must remain accessible during the compression process.
	*
	*  Use ZBUFF_compressContinue() repetitively to consume input stream.
	*  *srcSizePtr and *maxDstSizePtr can be any size.
	*  The function will report how many bytes were read or written within *srcSizePtr and *maxDstSizePtr.
	*  Note that it may not consume the entire input, in which case it's up to the caller to present again remaining data.
	*  The content of dst will be overwritten (up to *maxDstSizePtr) at each function call, so save its content if it matters or move dst .
	*  @return : a hint to preferred nb of bytes to use as input for next function call (it's only a hint, to improve latency)
	*            or an error code, which can be tested using ZBUFF_isError().
	*
	*  ZBUFF_compressFlush() can be used to instruct ZBUFF to compress and output whatever remains within its buffer.
	*  Note that it will not output more than *maxDstSizePtr.
	*  Therefore, some content might still be left into its internal buffer if dst buffer is too small.
	*  @return : nb of bytes still present into internal buffer (0 if it's empty)
	*            or an error code, which can be tested using ZBUFF_isError().
	*
	*  ZBUFF_compressEnd() instructs to finish a frame.
	*  It will perform a flush and write frame epilogue.
	*  Note that the epilogue is necessary for decoders to consider a frame completed.
	*  Similar to ZBUFF_compressFlush(), it may not be able to output the entire internal buffer content if *maxDstSizePtr is too small.
	*  In which case, call again ZBUFF_compressFlush() to complete the flush.
	*  @return : nb of bytes still present into internal buffer (0 if it's empty)
	*            or an error code, which can be tested using ZBUFF_isError().
	*
	*  Hint : recommended buffer sizes (not compulsory) : ZBUFF_recommendedCInSize / ZBUFF_recommendedCOutSize
	*  input : ZBUFF_recommendedCInSize==128 KB block size is the internal unit, it improves latency to use this value.
	*  output : ZBUFF_recommendedCOutSize==ZSTD_compressBound(128 KB) + 3 + 3 : ensures it's always possible to write/flush/end a full block. Skip some buffering.
	*  By using both, you ensure that input will be entirely consumed, and output will always contain the result.
	* **************************************************/


	typedef struct ZBUFF_DCtx_s ZBUFF_DCtx;
	ZSTDLIB_API ZBUFF_DCtx* ZBUFF_createDCtx(void);
	ZSTDLIB_API size_t      ZBUFF_freeDCtx(ZBUFF_DCtx* dctx);

	ZSTDLIB_API size_t ZBUFF_decompressInit(ZBUFF_DCtx* dctx);
	ZSTDLIB_API size_t ZBUFF_decompressWithDictionary(ZBUFF_DCtx* dctx, const void* src, size_t srcSize);

	ZSTDLIB_API size_t ZBUFF_decompressContinue(ZBUFF_DCtx* dctx, void* dst, size_t* maxDstSizePtr, const void* src, size_t* srcSizePtr, unsigned scrambler);

	/** ************************************************
	*  Streaming decompression
	*
	*  A ZBUFF_DCtx object is required to track streaming operation.
	*  Use ZBUFF_createDCtx() and ZBUFF_freeDCtx() to create/release resources.
	*  Use ZBUFF_decompressInit() to start a new decompression operation.
	*  ZBUFF_DCtx objects can be reused multiple times.
	*
	*  Optionally, a reference to a static dictionary can be set, using ZBUFF_decompressWithDictionary()
	*  It must be the same content as the one set during compression phase.
	*  Dictionary content must remain accessible during the decompression process.
	*
	*  Use ZBUFF_decompressContinue() repetitively to consume your input.
	*  *srcSizePtr and *maxDstSizePtr can be any size.
	*  The function will report how many bytes were read or written by modifying *srcSizePtr and *maxDstSizePtr.
	*  Note that it may not consume the entire input, in which case it's up to the caller to present remaining input again.
	*  The content of dst will be overwritten (up to *maxDstSizePtr) at each function call, so save its content if it matters or change dst.
	*  @return : a hint to preferred nb of bytes to use as input for next function call (it's only a hint, to improve latency)
	*            or 0 when a frame is completely decoded
	*            or an error code, which can be tested using ZBUFF_isError().
	*
	*  Hint : recommended buffer sizes (not compulsory) : ZBUFF_recommendedDInSize / ZBUFF_recommendedDOutSize
	*  output : ZBUFF_recommendedDOutSize==128 KB block size is the internal unit, it ensures it's always possible to write a full block when it's decoded.
	*  input : ZBUFF_recommendedDInSize==128Kb+3; just follow indications from ZBUFF_decompressContinue() to minimize latency. It should always be <= 128 KB + 3 .
	* **************************************************/


	/* *************************************
	*  Tool functions
	***************************************/
	ZSTDLIB_API unsigned ZBUFF_isError(size_t errorCode);
	ZSTDLIB_API const char* ZBUFF_getErrorName(size_t errorCode);

	/** The below functions provide recommended buffer sizes for Compression or Decompression operations.
	*   These sizes are not compulsory, they just tend to offer better latency */
	ZSTDLIB_API size_t ZBUFF_recommendedCInSize(void);
	ZSTDLIB_API size_t ZBUFF_recommendedCOutSize(void);
	ZSTDLIB_API size_t ZBUFF_recommendedDInSize(void);
	ZSTDLIB_API size_t ZBUFF_recommendedDOutSize(void);


#if defined (__cplusplus)
}
#endif

#endif  /* ZSTD_BUFFERED_H */
