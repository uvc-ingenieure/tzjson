******************************
TZ JSON - Simple C JSON Parser
******************************

TZ JSON is a simple JSON parser written in C without any dependencies on other
libraries.
JSON data can be extracted using query strings like common in scripting
languages.

========
Features
========

* Simple and easy to use API
* No dynamic memory allocation, operates directly on input string
* No dependencies
* MIT License

=====
Usage
=====

Desired JSON elements are described by path strings.
The API consists of several extraction functions that operate directly on the
JSON input string but without modifying it:

.. code-block:: c

    enum tzj_type tzj_json(const char *json, char *path, const char **res, int *len);
    bool tzj_str(const char *json, char *path, const char **res, int *len);
    bool tzj_int(const char *json, char *path, int *value);
    bool tzj_double(const char *json, char *path, double *value);
    bool tzj_bool(const char *json, char *path, bool *value);

All functions return ``true`` on successful parsing and return their result by reference.

Assume the following JSON snippet referenced by ``const char *json``

.. code-block:: json

    {
        "jsonrpc": "2.0",
        "method": "foo",
        "params": [
            true,
            {"key": 32}
        ],
        "id": 1
    }



Extracting an Integer is as simple as that:

.. code-block:: c

    tzj_int(json, ".params[1].key", &intarg);

The parsing overhead can be reduced with JSON sub strings which
``tzj_json(...)`` can extract:

.. code-block:: c

    char *sub;
    tzj_json(json, ".params", &sub, NULL);
    tzj_int(sub, "[1].key", &intarg);


The string related functions ``tzj_json(...)`` and ``tzj_str(...)`` return
references into the input string and thus aren't null terminated. Instead,
a optionally length parameter can be passed to those functions.
For further handlinf the object type ``enum tzj_type`` is returned.

.. code-block:: c

    char *str;
    int len;
    tzj_str(json, ".method", &str, &len);

To iterate over arrays, ``tzj_array_next(...)`` advances a pointer to the next
array item:

.. code-block:: c

    enum tzj_type type;
    const char *elem;

    for (type = tzj_json(json, ".params[0]", &elem, NULL);
         type != TZJ_ERROR;
         type = tzj_array_next(&elem)) {

	 handle_element(elem);
    }


To ease the creation of JSON strings,  a ``sprintf(...)`` like function is
provided. In the format string all single quotes are replaced by double quotes
to avoid heavy escaping.
The following formats are recognized by ``tzj_sprintf(...)``:

:%s:
    Strings, containing double quotes will be escaped
:%d:
    Decimal as usual
:%j:
    JSON parts maybe previously extracted with ``tzj_json(...)``
    This format expects the ``char*`` to the JSON and an ``int`` providing the
    length of the JSON

.. code-block:: c

    char buf[64];
    const char *sub;
    int len;

    tzj_json(json, ".params", &sub, &len);
    tzj_sprintf(buf, "{'key': %d, 'sub': %j}", 85, sub, len);



================
JSON RPC Example
================

To show all parts working together, here is an real life example.
Parsing and responding to a JSON RPC request:

.. code-block:: c

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

.. footer:: Copyright (c) UVC Ingenieure http://uvc-ingenieure.de/
