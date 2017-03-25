INSTALLDIR=$(shell pwd)/install
#all: libevent lua bson mongo
all: libevent lua mongoc

libevent:
	(cd libevent-2.0.22-stable && ./configure --prefix=$(INSTALLDIR)/libevent && make install)

lua:
	#(cd lua-5.3.3 && make linux && make install INSTALL_TOP=$(INSTALLDIR)/lua)
	(cd lua-5.1.5 && make linux && make install INSTALL_TOP=$(INSTALLDIR)/lua)

bson:
	(cd lua-bson && make linux && cp bson.so $(INSTALLDIR)/)

mongo:
	(cd lua-mongo && make linux && cp mongo.so mongo.lua $(INSTALLDIR)/)

mongoc:
	(cd mongo-c-driver-1.6.1 && ./configure --disable-automatic-init-and-cleanup --prefix=$(INSTALLDIR)/mongoc && make install)
	#(cd mongo-c-driver-1.6.1 && ./configure --enable-static --enable-shared --prefix=$(INSTALLDIR)/mongoc && make install)

clean:
	(cd libevent-2.0.22-stable && make clean)
	#(cd lua-5.3.3 && make clean)
	(cd lua-5.1.5 && make clean)
	(cd lua-bson && make clean)
	(cd lua-mongo && make clean)
	(cd mongo-c-driver-1.6.1 && make clean)
