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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tzjson.h"

#define TZJ_CONST_TRUE "true"
#define TZJ_CONST_FALSE "false"
#define TZJ_CONST_NULL "null"

enum tzj_result {
    TZJ_EOF = -1,
    TZJ_OK = 0,
    TZJ_ERROR,
    TZJ_FOUND
};

struct tzj_context {
    const char *src;
    const char *pos;
    int depth;

    char *path;
    int path_segment;
    int path_depth;
    enum {
        TZJ_ARRAY,
        TZJ_OBJECT,
	TZJ_VALIDATE,
        TZJ_DONE
    } path_segment_type;
};

static enum tzj_result tzj_next(struct tzj_context *tzj, int skip_blanks);
static enum tzj_result tzj_parse_object(struct tzj_context *tzj);
static enum tzj_result tzj_parse_pair(struct tzj_context *tzj);
static enum tzj_result tzj_parse_array(struct tzj_context *tzj);
static enum tzj_result tzj_parse_value(struct tzj_context *tzj);
static enum tzj_result tzj_parse_string(struct tzj_context *tzj, bool iskey);
static enum tzj_result tzj_parse_number(struct tzj_context *tzj);
static enum tzj_result tzj_parse_constant(struct tzj_context *tzj, const char *name);
static enum tzj_result tzj_parse(struct tzj_context *tzj, const char *json, char *path);
static void tzj_next_path_segment(struct tzj_context *tzj);


static enum tzj_result tzj_next(struct tzj_context *tzj, int skip_blanks)
{
    tzj->pos++;

    if (skip_blanks) {
        while (*tzj->pos == ' ' || *tzj->pos == '\r' || *tzj->pos == '\n') {
            tzj->pos++;
        }
    }

    return *tzj->pos == '\0' ? TZJ_EOF : TZJ_OK;
}

static enum tzj_result tzj_parse_pair(struct tzj_context *tzj)
{
    enum tzj_result ret;

    if (tzj_parse_string(tzj, true) != TZJ_ERROR
        && tzj_next(tzj, true) != TZJ_EOF
        && *tzj->pos == ':'
        && tzj_next(tzj, true) != TZJ_EOF
        && (ret = tzj_parse_value(tzj)) != TZJ_ERROR) {
    } else {
        ret = TZJ_ERROR;
    }

    return ret;
}

static enum tzj_result tzj_parse_object(struct tzj_context *tzj)
{
    enum tzj_result ret;

    tzj->depth++;

    ret = tzj_next(tzj, true);

    switch (*tzj->pos) {
    case '"':
        ret = tzj_parse_pair(tzj);
        tzj_next(tzj, true);
    case '}':
        break;
    default:
        fprintf(stderr, "ERROR: unexpected character: %s\n", tzj->pos);
        ret = TZJ_ERROR;
    }

    while (ret != TZJ_FOUND && ret != TZJ_ERROR && *tzj->pos != '}') {
        if (*tzj->pos == ','
            && tzj_next(tzj, true) != TZJ_EOF
            && *tzj->pos == '"'
            && (ret = tzj_parse_pair(tzj)) != TZJ_ERROR) {

            /* do not advance if path has been matched */
            if (ret == TZJ_OK) {
                tzj_next(tzj, true);
            }

        } else {
            fprintf(stderr, "ERROR: failed to parse pair\n");
            ret = TZJ_ERROR;
        }
    }

    if (ret == TZJ_OK && *tzj->pos != '}') {
        fprintf(stderr, "ERROR: closing bracket } is missing.\n");
        ret = TZJ_ERROR;
    }

    tzj->depth--;

    return ret;
}

static enum tzj_result tzj_parse_array(struct tzj_context *tzj)
{
    int index = 0;
    enum tzj_result ret = TZJ_OK;

    tzj->depth++;

    tzj_next(tzj, true);

    if (*tzj->pos != ']') {
        if (tzj->path_segment_type == TZJ_ARRAY
            && tzj->path_segment == index++) {

            tzj_next_path_segment(tzj);
            ret = tzj_parse_value(tzj);
            if (ret != TZJ_FOUND) {
                ret = TZJ_ERROR;
            }

        } else {
            ret = tzj_parse_value(tzj);
        }

        tzj_next(tzj, true);
    }

    while (ret == TZJ_OK && *tzj->pos != ']') {
        if (*tzj->pos == ',' && tzj_next(tzj, true) != TZJ_EOF) {

            if (tzj->path_segment_type == TZJ_ARRAY
                && tzj->path_segment == index++) {

                tzj_next_path_segment(tzj);
                ret = tzj_parse_value(tzj);
                if (ret != TZJ_FOUND) {
                    ret = TZJ_ERROR;
                }

            } else {
                ret = tzj_parse_value(tzj);
            }

            /* do not advance if path has been matched */
            if (ret == TZJ_OK) {
                tzj_next(tzj, true);
            }

        } else {
            fprintf(stderr, "ERROR: failed to parse array value.\n");
            ret = TZJ_ERROR;
        }
    }

    if (ret == TZJ_OK && *tzj->pos != ']') {
        fprintf(stderr, "ERROR: closing bracket ] is missing.\n");
        ret = TZJ_ERROR;
    }

    tzj->depth--;

    return ret;
}

static enum tzj_result tzj_parse_value(struct tzj_context *tzj)
{
    if (tzj->path_segment_type == TZJ_DONE)
        return TZJ_FOUND;

    if (*tzj->pos >= '0' && *tzj->pos <= '9' || *tzj->pos == '-') {
        return tzj_parse_number(tzj);
    } else {
        switch (*tzj->pos) {
        case -1:
            return TZJ_ERROR;
        case '{':
            return tzj_parse_object(tzj);
        case '[':
            return tzj_parse_array(tzj);
        case '"':
            return tzj_parse_string(tzj, false);
        case 't':
            return tzj_parse_constant(tzj, TZJ_CONST_TRUE);
        case 'f':
            return tzj_parse_constant(tzj, TZJ_CONST_FALSE);
        case 'n':
            return tzj_parse_constant(tzj, TZJ_CONST_NULL);
        default:
            return TZJ_ERROR;
        }
    }
}

static enum tzj_result tzj_parse_string(struct tzj_context *tzj, bool iskey)
{
    const char *p = tzj->pos + 1;     /* without leading " */

    while (tzj_next(tzj, false) != TZJ_EOF && *tzj->pos != '"')
	if (*tzj->pos == '\\') {
            tzj_next(tzj, false);
        }

    if (*tzj->pos != '"') {
        fprintf(stderr, "ERROR: closing \" missing");
        return TZJ_ERROR;
    }

    if (iskey
        && tzj->depth == tzj->path_depth
        && tzj->path_segment_type == TZJ_OBJECT
        && strncmp(tzj->path, p, tzj->path_segment) == 0) {

        tzj_next_path_segment(tzj);
        return TZJ_FOUND;
    }

    return TZJ_OK;
}

static enum tzj_result tzj_parse_number(struct tzj_context *tzj)
{
    strtod(tzj->pos, (char**)&tzj->pos);
    tzj->pos--;
    return TZJ_OK;
}

static enum tzj_result tzj_parse_constant(struct tzj_context *tzj, const char *name)
{
    if (strncmp(tzj->pos, name, strlen(name)) == 0) {
        tzj->pos += strlen(name) - 1;
        return TZJ_OK;
    }

    return TZJ_ERROR;
}

static void tzj_next_path_segment(struct tzj_context *tzj)
{
    char *p;

    tzj->path_depth++;

    if (tzj->path_segment_type == TZJ_OBJECT) {
        /* if previous type was oject => advance buffer */
        tzj->path += tzj->path_segment;
    }

    if (*tzj->path == '\0') {
        tzj->path_segment_type = TZJ_DONE;
        return;
    }

    if (*tzj->path++ == '[') {
        tzj->path_segment_type = TZJ_ARRAY;
        tzj->path_segment = strtol(tzj->path, &tzj->path, 10);

        /* skip closing bracket */
        tzj->path++;
    } else {
        tzj->path_segment_type = TZJ_OBJECT;

        p = tzj->path;
        while (*p != '\0' && *p != '.' && *p != '[') {
            p++;
        }

        tzj->path_segment = p - tzj->path;
    }
}

static enum tzj_result tzj_parse(struct tzj_context *tzj, const char *json, char *path)
{
    enum tzj_result ret;

    tzj->src = json;
    tzj->pos = json;
    tzj->depth = 0;

    tzj->path = path;
    tzj->path_depth = 0;
    tzj->path_segment = 0;

    tzj_next_path_segment(tzj);

    switch (tzj->path_segment_type) {
    case TZJ_OBJECT:
        ret = tzj_parse_object(tzj);
        break;
    case TZJ_ARRAY:
        ret = tzj_parse_array(tzj);
        break;
    default:
        ret = TZJ_ERROR;
    }

    return ret;
}

bool tzj_json(const char *json, char *path, const char **str, int *len)
{
    struct tzj_context tzj;

    if (tzj_parse(&tzj, json, path) == TZJ_FOUND) {
        *str = tzj.pos;
	if (len != NULL) {
	    tzj.path_segment_type = TZJ_VALIDATE;
	    tzj_parse_value(&tzj);
	    *len = (tzj.pos - *str) + 1;
	}
        return true;
    }

    return false;
}

bool tzj_str(const char *json, char *path, const char **str, int *len)
{
    struct tzj_context tzj;

    if (tzj_parse(&tzj, json, path) == TZJ_FOUND) {
        *str = tzj.pos + 1;
        tzj_parse_string(&tzj, false);
        *len = tzj.pos - *str;
        return true;
    }

    return false;
}

bool tzj_int(const char *json, char *path, int *value)
{
    struct tzj_context tzj;

    if (tzj_parse(&tzj, json, path) == TZJ_FOUND) {
        *value = atoi(tzj.pos);
        return true;
    }

    return false;
}

bool tzj_double(const char *json, char *path, double *value)
{
    struct tzj_context tzj;

    if (tzj_parse(&tzj, json, path) == TZJ_FOUND) {
        *value = strtod(tzj.pos, NULL);
        return true;
    }

    return false;
}

bool tzj_bool(const char *json, char *path, bool *value)
{
    struct tzj_context tzj;

    if (tzj_parse(&tzj, json, path) == TZJ_FOUND) {
        tzj.pos--;

	if (strncmp(tzj.pos, TZJ_CONST_TRUE, strlen(TZJ_CONST_TRUE)) != 0
	    && strncmp(tzj.pos, TZJ_CONST_FALSE, strlen(TZJ_CONST_FALSE)) != 0)
	    return false;

        *value = strncmp(tzj.pos, TZJ_CONST_TRUE, strlen(TZJ_CONST_TRUE)) == 0;
        return true;
    }

    return false;
}
