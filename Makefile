all: libtzjson.a

libtzjson.a: tzjson.o
	$(AR) rc $@ $^

%.o: %.c tzjson.h
	$(CC) -c $(CFLAGS) $< -o $@

test: tzjson_test
	./tzjson_test

tzjson_test: tzjson_test.o
	$(CC)  $< -L. -ltzjson -o $@

tzjson_test.o: tzjson_test.c libtzjson.a

doc:
	rst2html.py ./README.rst README.html

clean:
	rm -f *.[oa]
	rm -f tzjson_test
	rm -f ./README.html

.PHONY: clean test doc
