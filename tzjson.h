/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 UVC Ingenieure http://uvc-ingenieure.de/
 * Author: Max Holtzberg <mholtzberg@uvc-ingenieure.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __TZJSON_H_
#define __TZJSON_H_

#include <stdbool.h>

bool tzj_json(const char *json, char *path, const char **res, int *len);
bool tzj_str(const char *json, char *path, const char **res, int *len);
bool tzj_int(const char *json, char *path, int *value);
bool tzj_double(const char *json, char *path, double *value);
bool tzj_bool(const char *json, char *path, bool *value);

int tzj_sprintf(char *str, const char *fmt, ...);

#endif	/* __TZJSON_H_ */
