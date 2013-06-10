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

#include <assert.h>
#include <stdio.h>

#include "tzjson.h"

static void tzj_rpc_example(void)
{
    const char *request = "{\"jsonrpc\": \"2.0\", "
        "\"method\": \"subtract\", "
        "\"params\": [42, 23], \"id\": \"unknown type\"}";
    char response[128];
    const char *version;
    const char *id;
    int len;
    int a, b;

    printf("RPC Request: %s\n", request);

    if (tzj_str(request, ".jsonrpc", &version, &len)
        && strncmp(version, "2.0", len) == 0
        && tzj_json(request, ".id", &id, &len)
        && tzj_int(request, ".params[0]", &a)
        && tzj_int(request, ".params[1]", &b)) {

        tzj_sprintf(response, "{'jsonrpc': '2.0', 'result': %d, 'id': %j}",
                    a - b, id, len);

    } else {
        fprintf(stderr, "ERROR: failed to parse request\n");
        tzj_sprintf(response, "{'jsonrpc': '2.0', "
                    "'error': {'code': -32700, 'message': 'Parse error'}, "
                    "'id': null}");
    }

    printf("RPC Response: %s\n", response);
}

int main(int argc, char *argv[])
{
    char buf[64];
    const char *sub;
    const char *strarg;
    bool boolarg;
    int intarg;
    int len;

    char *json = "{\"jsonrpc\": \"2.0\","
                  "\"number\": -32.123,"
                  "\"method\": \"subtract\", "
                  "\"params\": [42, {\"key\": true}], "
                  "\"id\": 1}";


    assert(tzj_json(json, ".params", &sub, NULL));

    printf("testing tzj_int(...)\n");
    intarg = 0;
    assert(tzj_int(sub, "[0]", &intarg));
    assert(intarg == 42);

    intarg = 0;
    assert(tzj_int(json, ".id", &intarg));
    assert(intarg == 1);


    printf("testing tzj_bool(...)\n");
    assert(tzj_json(sub, "[1]", &sub, NULL));
    assert(tzj_bool(sub, ".key", &boolarg));
    assert(boolarg);

    assert(tzj_bool(sub, ".params[1].key", &boolarg) == false);
    assert(tzj_bool(json, ".params[1].key", &boolarg));
    assert(boolarg);

    printf("testing tzj_str(...)\n");

    assert(tzj_str(json, ".jsonrpc", &strarg, &len));
    assert(strncmp(strarg, "2.0", len) == 0);

    assert(tzj_str(json, ".method", &strarg, &len));
    assert(strncmp(strarg, "subtract", len) == 0);

    printf("\nall tests passed\n");

    printf("\nRunning JSON RPC example:\n");
    tzj_rpc_example();

    return 0;
}
