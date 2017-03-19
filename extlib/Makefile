INSTALLDIR=$(shell pwd)/install
all: libevent lua bson mongo

libevent:
	(cd libevent-2.0.22-stable && ./configure --prefix=$(INSTALLDIR)/libevent && make install)

lua:
	(cd lua-5.3.3 && make linux && make install INSTALL_TOP=$(INSTALLDIR)/lua)

bson:
	(cd lua-bson && make linux && cp bson.so $(INSTALLDIR)/)

mongo:
	(cd lua-mongo && make linux && cp mongo.so mongo.lua $(INSTALLDIR)/)

clean:
	(cd libevent-2.0.22-stable && make clean)
	(cd lua-5.3.3 && make clean)
	(cd lua-bson && make clean)
	(cd lua-mongo && make clean)
