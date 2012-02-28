all:
	$(CC) -ggdb -I../libwebsockets/lib/ -c glproxy.c -o glproxy.o
	#$(CC) glproxy.o ../libwebsockets/lib/.libs/libwebsockets.a -lz -lpthread -o glproxy
	$(CC) -shared glproxy.o ../libwebsockets/lib/.libs/libwebsockets.a -lz -lpthread -compatibility_version 1.0 -current_version 1.0.0 -o glproxy.so
	mv glproxy.so OpenGL
