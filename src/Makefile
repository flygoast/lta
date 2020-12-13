LUA_PATH = /usr/local/lua-5.1.5
LUA_INCLUDE_PATH = $(LUA_PATH)/include
LUA_LIBS_PATH = $(LUA_PATH)/lib

POCO_PATH = /usr/local/poco-1.10.1
POCO_INCLUDE_PATH = $(POCO_PATH)/include
POCO_LIBS_PATH = $(POCO_PATH)/lib

#CXXFLAGS = -I$(LUA_INCLUDE_PATH) -I$(POCO_INCLUDE_PATH) -std=c++11 -g
CXXFLAGS = -I$(LUA_INCLUDE_PATH) -I$(POCO_INCLUDE_PATH) -g
LDFLAGS += -L$(LUA_LIBS_PATH) -L$(POCO_LIBS_PATH) -lpthread -ldl

#============================================
# The order of the libraries MATTERS!!!
#============================================
LDFLAGS += -lPocoUtil -lPocoJSON -lPocoCrypto -lPocoNet -lPocoNetSSL -lPocoDataSQLite -lPocoData -lPocoZip -lPocoXML -lPocoFoundation  -llua  

lta: jsoncpp.o main.o Application.o MainLoop.o Monitor.o Facts.o Lua.o GenericTask.o Utils.o DatabaseManager.o Store.o Core.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f *.o lta

