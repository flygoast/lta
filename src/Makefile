
POCO_PATH = /usr/local/poco-1.4.6.4
INCLUDE_PATH = $(POCO_PATH)/include
LIBS_PATH = $(POCO_PATH)/lib
CXXFLAGS = -I$(INCLUDE_PATH) -I/usr/local/include -std=c++11 -g
LDFLAGS += -L$(LIBS_PATH) -L/usr/local/lib -lpthread -ldl

#============================================
# The order of the libraries MATERS!!!
#============================================
LDFLAGS += -lPocoUtil -lPocoCrypto -lPocoNet -lPocoNetSSL -lPocoDataSQLite -lPocoData -lPocoZip -lPocoXML -lPocoFoundation  -llua  

lta: jsoncpp.o main.o Application.o MainLoop.o Monitor.o Facts.o Lua.o GenericTask.o Utils.o DatabaseManager.o Store.o Core.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm *.o lta

