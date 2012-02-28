all:
	$(CC) -ggdb -O2 -I../libwebsockets/lib/ -I../liblo-0.26 -c glproxy.c -o glproxy.o
	#$(CC) glproxy.o ../libwebsockets/lib/.libs/libwebsockets.a -lz -lpthread -o glproxy
	$(CC) -shared glproxy.o ../libwebsockets/lib/.libs/libwebsockets.a ../liblo-0.26/src/.libs/liblo.a -lz -lpthread -compatibility_version 1.0 -current_version 1.0.0 -o glproxy.so
	mv glproxy.so OpenGL
