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

    bool tzj_json(const char *json, char *path, const char **res, int *len);
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
a optionally length parameter can be passed to those functions:

.. code-block:: c

    char *str;
    int len;
    tzj_str(json, ".method", &str, &len);

.. footer:: Copyright (c) UVC Ingenieure http://uvc-ingenieure.de/
