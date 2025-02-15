/*
 * Copyright (c) 2011 NLNet Labs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * String utilities.
 *
 */

#ifndef SHARED_STR_H
#define SHARED_STR_H

#include "config.h"
#include "status.h"
#include <time.h>


/**
 * Concatenate characters without custom allocators.
 * 
 * Will always allocate at least 1 byte (when catting empty strings) so
 * result should always be freed by the caller.
 * 
 * \param[in] argc, number of strings in argv.
 * \param[in] argv, storage of strings.
 * \param[in] delim, delimiter used to join the strings.
 * \return string, may be empty string.
 */
char *ods_strcat_delim(int argc, char* argv[], char delim);

/**
 * Remove leading and trailing whitespace.
 * \param[in] str string to trim
 * \param[in] keep_newline whether to keep a single trailing newline or not
 * \return the same reference to the string
 *
 */
char* ods_str_trim(char* str, int keep_newline);

/**
 * Version of ctime_r that does not feature a trailing '\n' character
 *
 * \param[in] buf buffer to write the formatted date/time to
 * \param[in] nbuf size of buf, including any terminating zero characters.
 * \param[in] t time_t value to print to buf
 * \return	buf to indicate success or NULL to indicate failure.
 */
char *ods_ctime_r(char *buf, size_t nbuf, time_t t);

#endif /* SHARED_STR_H */
