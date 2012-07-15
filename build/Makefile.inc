ifneq ("$(MACHINE)", "")
	OBJDIR := $(BUILD_TYPE)-$(PLATFORM)-$(MACHINE)
else
	OBJDIR := $(BUILD_TYPE)-$(PLATFORM)
endif

# ------------------------------------------------------------------

CORE_LIB_SOURCES := \
	MojApp.cpp \
	MojBuffer.cpp \
	MojDataSerialization.cpp \
	MojDecimal.cpp \
 	MojEpollReactor.cpp \
	MojErr.cpp \
	MojFile.cpp \
	MojGmainReactor.cpp \
	MojHashBase.cpp \
	MojJson.cpp \
	MojList.cpp \
	MojLog.cpp \
	MojLogEngine.cpp \
	MojMessageDispatcher.cpp \
	MojObject.cpp \
	MojObjectBuilder.cpp \
	MojObjectFilter.cpp \
	MojObjectSerialization.cpp \
	MojOs.cpp \
	MojPmLogAppender.cpp \
	MojRbTreeBase.cpp \
	MojRefCount.cpp \
	MojSchema.cpp \
	MojService.cpp \
	MojServiceApp.cpp \
	MojServiceMessage.cpp \
	MojServiceRequest.cpp \
	MojSignal.cpp \
	MojSock.cpp \
	MojString.cpp \
	MojTestRunner.cpp \
	MojThread.cpp \
	MojTokenSet.cpp \
	MojUtil.cpp

DB_LIB_SOURCES := \
	MojDb.cpp \
	MojDbAdmin.cpp \
	MojDbClient.cpp \
	MojDbCursor.cpp \
	MojDbExtractor.cpp \
	MojDbIdGenerator.cpp \
	MojDbIndex.cpp \
	MojDbIsamQuery.cpp \
	MojDbKey.cpp \
	MojDbKind.cpp \
	MojDbKindEngine.cpp \
	MojDbKindState.cpp \
	MojDbObjectHeader.cpp \
	MojDbObjectItem.cpp \
	MojDbPermissionEngine.cpp \
	MojDbPutHandler.cpp \
	MojDbQuery.cpp \
	MojDbQueryFilter.cpp \
	MojDbQueryPlan.cpp \
	MojDbQuotaEngine.cpp \
	MojDbReq.cpp \
	MojDbRevisionSet.cpp \
	MojDbSearchCursor.cpp \
	MojDbServiceClient.cpp \
	MojDbServiceDefs.cpp \
	MojDbServiceHandler.cpp \
	MojDbServiceHandlerBase.cpp \
	MojDbServiceSchemas.cpp \
	MojDbStorageEngine.cpp \
	MojDbTextCollator.cpp \
	MojDbTextTokenizer.cpp \
	MojDbTextUtils.cpp \
	MojDbUtils.cpp \
	MojDbWatcher.cpp \
	MojDbServiceHandlerInternal.cpp \

LUNA_LIB_SOURCES := \
	MojLunaErr.cpp \
	MojLunaMessage.cpp \
	MojLunaRequest.cpp \
	MojLunaService.cpp

DB_LUNA_SOURCES := \
	MojDbLunaServiceApp.cpp \

CORE_TEST_SOURCES := \
	MojCoreTestRunner.cpp \
	MojAtomicIntTest.cpp \
	MojAutoPtrTest.cpp \
	MojBufferTest.cpp \
	MojDataSerializationTest.cpp \
	MojDecimalTest.cpp \
	MojErrTest.cpp \
	MojHashMapTest.cpp \
	MojJsonTest.cpp \
	MojListTest.cpp \
	MojLogTest.cpp \
	MojMapTest.cpp \
	MojMessageDispatcherTest.cpp \
	MojObjectFilterTest.cpp \
	MojObjectSerializationTest.cpp \
	MojObjectTest.cpp \
	MojReactorTest.cpp \
	MojRefCountTest.cpp \
	MojSchemaTest.cpp \
	MojSetTest.cpp \
	MojSignalTest.cpp \
	MojStringTest.cpp \
	MojThreadTest.cpp \
	MojTimeTest.cpp \
	MojUtilTest.cpp \
	MojVectorTest.cpp \
	
DB_TEST_SOURCES := \
	MojDbTestRunner.cpp \
	MojDbBulkTest.cpp \
	MojDbConcurrencyTest.cpp \
	MojDbCrudTest.cpp \
	MojDbDumpLoadTest.cpp \
	MojDbIndexTest.cpp \
	MojDbKindTest.cpp \
	MojDbLocaleTest.cpp \
	MojDbPermissionTest.cpp \
	MojDbPurgeTest.cpp \
	MojDbQueryTest.cpp \
	MojDbQueryFilterTest.cpp \
	MojDbQuotaTest.cpp \
	MojDbRevTest.cpp \
	MojDbRevisionSetTest.cpp \
	MojDbSearchTest.cpp \
	MojDbTextCollatorTest.cpp \
	MojDbTextTokenizerTest.cpp \
	MojDbTextUtils.cpp \
	MojDbWatchTest.cpp \
	MojDbTestStorageEngine.cpp \
		
DB_CLIENT_TEST_SOURCES :=  \
	MojDbClientTest.cpp	
	
DB_PERF_TEST_SOURCES := \
	MojDbPerfTestRunner.cpp \
	MojDbPerfTest.cpp \
	MojDbPerfCreateTest.cpp \
	MojDbPerfDeleteTest.cpp \
	MojDbPerfReadTest.cpp \
	MojDbPerfUpdateTest.cpp
	
DB_STRESS_TEST_SOURCES :=  \
	MojDbStressTest.cpp	

LUNA_SERVICE_TEST_SOURCES := \
	MojLunaServiceTest.cpp

# ------------------------------------------------------------------
MOJ_USE_BDB := 1
MOJ_USE_LDB := 0

ifeq ("$(MOJ_USE_BDB)", "1")
	 DB_LUNA_SOURCES += \
		MojDbBerkeleyEngine.cpp \
		MojDbBerkeleyFactory.cpp \
		MojDbBerkeleyQuery.cpp
		
	DB_TEST_SOURCES  += \
		MojDbBerkeleyEngine.cpp \
		MojDbBerkeleyFactory.cpp \
		MojDbBerkeleyQuery.cpp

	DB_PERF_TEST_SOURCES  += \
		MojDbBerkeleyEngine.cpp \
		MojDbBerkeleyFactory.cpp \
		MojDbBerkeleyQuery.cpp

	DB_ENGINE_LIBS += -ldb-4.8
	LOCAL_DEFINES += -DMOJ_USE_BDB
endif

ifeq ("$(MOJ_USE_LDB)", "1")
	 DB_LUNA_SOURCES += \
		MojDbLevelEngine.cpp \
		MojDbLevelFactory.cpp \
		MojDbLevelQuery.cpp
		
	DB_TEST_SOURCES  += \
		MojDbLevelEngine.cpp \
		MojDbLevelFactory.cpp \
		MojDbLevelQuery.cpp

	DB_PERF_TEST_SOURCES  += \
		MojDbLevelEngine.cpp \
		MojDbLevelFactory.cpp \
		MojDbLevelQuery.cpp

	DB_ENGINE_LIBS += -lleveldb
	LOCAL_DEFINES += -DMOJ_USE_LDB
endif


# ------------------------------------------------------------------

SOURCES := \
	$(CORE_LIB_SOURCES) \
	$(DB_LIB_SOURCES) \
	$(LUNA_LIB_SOURCES) \
	$(DB_LUNA_SOURCES) \
	$(CORE_TEST_SOURCES) \
	$(DB_TEST_SOURCES) \
	$(DB_CLIENT_TEST_SOURCES) \
	$(DB_PERF_TEST_SOURCES) \
	$(DB_STRESS_TEST_SOURCES) \
	$(LUNA_SERVICE_TEST_SOURCES)
	
SOURCE_DIRS := src/core src/db src/luna src/db-luna test/core test/db test/dbclient test/luna-service

ifeq ("$(BUILD_TYPE)", "debug")
	LOCAL_DEFINES += -DDEBUG -DMOJ_DEBUG
	LOCAL_FLAGS_OPT := -O0 -g
else
	LOCAL_DEFINES += -DNDEBUG
	LOCAL_FLAGS_OPT := -fexpensive-optimizations -frename-registers -O2 -g
endif

LOCAL_INCLUDES := \
	-I./src \
	-I./src/service \
	-I./src/test \
	-I./inc \
	$(INCLUDES)

LOCAL_LIBS := \
	$(LIBS) \
	-lpthread
	
LOCAL_DEFINES += \
	-DMOJ_INTERNAL \
	-D_REENTRANT
	
ifneq ("$(VERSION_STRING)", "")
	LOCAL_DEFINES += -DMOJ_VERSION_STRING="\"$(VERSION_STRING)\""
endif

LOCAL_CFLAGS := $(CFLAGS) -fPIC -Wall -Wconversion -Werror $(LOCAL_FLAGS_OPT) $(LOCAL_DEFINES) $(ARCH_CFLAGS)
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS) -fno-exceptions 
LOCAL_LDFLAGS := -L$(OBJDIR) $(LOCAL_FLAGS_OPT) $(LDFLAGS) $(ARCH_LDFLAGS) 

# ------------------------------------------------------------------

ICU_LIBS := -licuuc -licutu

CORE_LIB_OBJS := $(CORE_LIB_SOURCES:%.cpp=$(OBJDIR)/%.o)
CORE_LIB_LIBS := -lglib-2.0 -lgthread-2.0 -lPmLogLib
CORE_LIB_NAME := libmojocore
CORE_LIB_SHARED_TARGET := $(OBJDIR)/$(CORE_LIB_NAME).so

DB_LIB_OBJS := $(DB_LIB_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_LIB_LIBS := $(ICU_LIBS) 
DB_LIB_NAME := libmojodb
DB_LIB_SHARED_TARGET := $(OBJDIR)/$(DB_LIB_NAME).so

LUNA_LIB_OBJS := $(LUNA_LIB_SOURCES:%.cpp=$(OBJDIR)/%.o)
LUNA_LIB_LIBS := -lmojocore -llunaservice
LUNA_LIB_NAME := libmojoluna
LUNA_LIB_SHARED_TARGET := $(OBJDIR)/$(LUNA_LIB_NAME).so

DB_LUNA_OBJS := $(DB_LUNA_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_LUNA_LIBS := -lmojodb -lmojoluna -lmojocore -llunaservice $(ICU_LIBS) $(DB_ENGINE_LIBS) $(LOCAL_LIBS)
DB_LUNA_TARGET := $(OBJDIR)/mojodb-luna

CORE_TEST_OBJS := $(CORE_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
CORE_TEST_LIBS := -lmojocore -lglib-2.0 $(LOCAL_LIBS)
CORE_TEST_TARGET := $(OBJDIR)/mojocore-test

DB_TEST_OBJS := $(DB_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_TEST_LIBS := -lmojodb -lmojocore $(ICU_LIBS) $(DB_ENGINE_LIBS) $(LOCAL_LIBS)
DB_TEST_TARGET := $(OBJDIR)/mojodb-test

DB_CLIENT_TEST_OBJS := $(DB_CLIENT_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_CLIENT_TEST_LIBS :=  -lmojoluna -lmojodb -lmojocore -llunaservice  $(LOCAL_LIBS)
DB_CLIENT_TEST_TARGET := $(OBJDIR)/mojodbclient-test

DB_PERF_TEST_OBJS := $(DB_PERF_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_PERF_TEST_LIBS :=  -lmojodb -lmojocore  $(ICU_LIBS) $(DB_ENGINE_LIBS) $(LOCAL_LIBS)
DB_PERF_TEST_TARGET := $(OBJDIR)/mojodbperf-test

DB_STRESS_TEST_OBJS := $(DB_STRESS_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
DB_STRESS_TEST_LIBS :=  -lmojoluna -lmojodb -lmojocore -llunaservice $(DB_ENGINE_LIBS) $(LOCAL_LIBS)
DB_STRESS_TEST_TARGET := $(OBJDIR)/mojodbstress-test

LUNA_SERVICE_TEST_OBJS := $(LUNA_SERVICE_TEST_SOURCES:%.cpp=$(OBJDIR)/%.o)
LUNA_SERVICE_TEST_LIBS := -lmojoluna -lmojocore -llunaservice $(LOCAL_LIBS)
LUNA_SERVICE_TEST_TARGET := $(OBJDIR)/mojolunaservice-test

SHARED_LIB_TARGETS := $(CORE_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET) ${LUNA_LIB_SHARED_TARGET} $(SYNC_LUNA_LIB_SHARED_TARGET)

# ------------------------------------------------------------------

ALL_TARGETS := libmojocore libmojodb libmojoluna mojodb-luna mojocore-test mojodb-test mojodbclient-test mojodbperf-test mojodbstress-test mojolunaservice-test
.PHONY: clean all setup install $(ALL_TARGETS)

all: $(ALL_TARGETS)

libmojocore: setup $(CORE_LIB_SHARED_TARGET)

libmojodb: setup $(DB_LIB_SHARED_TARGET)

libmojoluna: setup $(LUNA_LIB_SHARED_TARGET)

mojodb-luna: $(DB_LUNA_TARGET)

mojocore-test: setup $(CORE_TEST_TARGET)

mojodb-test: setup $(DB_TEST_TARGET)

mojodbclient-test: setup $(DB_CLIENT_TEST_TARGET)

mojodbperf-test: setup $(DB_PERF_TEST_TARGET)

mojodbstress-test: setup $(DB_STRESS_TEST_TARGET)

mojolunaservice-test: setup $(LUNA_SERVICE_TEST_TARGET)

setup: $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

vpath %.cpp $(SOURCE_DIRS)

$(CORE_LIB_SHARED_TARGET): $(CORE_LIB_OBJS)
	$(CXX) -o $(CORE_LIB_SHARED_TARGET) -shared -Wl,-soname,$(CORE_LIB_NAME).so $(LOCAL_LDFLAGS) $(CORE_LIB_OBJS) $(CORE_LIB_LIBS)

$(DB_LIB_SHARED_TARGET): $(DB_LIB_OBJS)
	$(CXX) -o $(DB_LIB_SHARED_TARGET) -shared -Wl,-soname,$(DB_LIB_NAME).so $(LOCAL_LDFLAGS) $(DB_LIB_OBJS) $(DB_LIB_LIBS)

$(LUNA_LIB_SHARED_TARGET): $(LUNA_LIB_OBJS)
	$(CXX) -o $(LUNA_LIB_SHARED_TARGET) -shared -Wl,-soname,$(LUNA_LIB_NAME).so $(LOCAL_LDFLAGS) $(LUNA_LIB_OBJS) $(LUNA_LIB_LIBS)

$(DB_LUNA_TARGET): $(DB_LUNA_OBJS) $(CORE_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET) $(LUNA_LIB_SHARED_TARGET)
	$(CXX) -o $(DB_LUNA_TARGET) $(DB_LUNA_OBJS) $(LOCAL_LDFLAGS) $(DB_LUNA_LIBS) 

$(CORE_TEST_TARGET): $(CORE_TEST_OBJS) $(CORE_LIB_SHARED_TARGET)
	$(CXX) -o $(CORE_TEST_TARGET) $(CORE_TEST_OBJS) $(LOCAL_LDFLAGS) $(CORE_TEST_LIBS)

$(DB_TEST_TARGET): $(DB_TEST_OBJS) $(CORE_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET)
	$(CXX) -o $(DB_TEST_TARGET) $(DB_TEST_OBJS) $(LOCAL_LDFLAGS) $(DB_TEST_LIBS)  

$(DB_CLIENT_TEST_TARGET): $(DB_CLIENT_TEST_OBJS) $(CORE_LIB_SHARED_TARGET) $(LUNA_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET)
	$(CXX) -o $(DB_CLIENT_TEST_TARGET) $(DB_CLIENT_TEST_OBJS) $(LOCAL_LDFLAGS) $(DB_CLIENT_TEST_LIBS)  

$(DB_PERF_TEST_TARGET): $(DB_PERF_TEST_OBJS) $(CORE_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET)
	$(CXX) -o $(DB_PERF_TEST_TARGET) $(DB_PERF_TEST_OBJS) $(LOCAL_LDFLAGS) $(DB_PERF_TEST_LIBS)  

$(DB_STRESS_TEST_TARGET): $(DB_STRESS_TEST_OBJS) $(CORE_LIB_SHARED_TARGET) $(DB_LIB_SHARED_TARGET) $(LUNA_LIB_SHARED_TARGET)
	$(CXX) -o $(DB_STRESS_TEST_TARGET) $(DB_STRESS_TEST_OBJS) $(LOCAL_LDFLAGS) $(DB_STRESS_TEST_LIBS)  

$(LUNA_SERVICE_TEST_TARGET): $(LUNA_SERVICE_TEST_OBJS) $(CORE_LIB_SHARED_TARGET) $(LUNA_LIB_SHARED_TARGET)
	$(CXX) -o $(LUNA_SERVICE_TEST_TARGET) $(LUNA_SERVICE_TEST_OBJS) $(LOCAL_LDFLAGS) $(LUNA_SERVICE_TEST_LIBS) 

$(OBJDIR)/%.o: %.cpp
	$(CXX) -MMD $(LOCAL_INCLUDES) $(LOCAL_CPPFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -MMD $(LOCAL_INCLUDES) $(LOCAL_CFLAGS) -c $< -o $@

-include $(SOURCES:%.cpp=$(OBJDIR)/%.d) 

clean:
	rm -rf $(OBJDIR)
	
install: all
	mkdir -p $(LUNA_STAGING)/lib
	mkdir -p $(LUNA_STAGING)/include/mojodb/core/internal
	mkdir -p $(LUNA_STAGING)/include/mojodb/db
	mkdir -p $(LUNA_STAGING)/include/mojodb/luna
	mkdir -p $(LUNA_STAGING)/share/dbus-1/system-services
	install -m 0755 $(SHARED_LIB_TARGETS) $(LUNA_STAGING)/lib
	install -m 0644 inc/core/*.h $(LUNA_STAGING)/include/mojodb/core
	install -m 0644 inc/core/internal/*.h $(LUNA_STAGING)/include/mojodb/core/internal
	install -m 0644 inc/db/*.h $(LUNA_STAGING)/include/mojodb/db
	install -m 0644 inc/luna/*.h $(LUNA_STAGING)/include/mojodb/luna
	install -m 0644 src/db-luna/com.palm.db.service $(LUNA_STAGING)/share/dbus-1/system-services
	install -m 0644 src/db-luna/com.palm.tempdb.service $(LUNA_STAGING)/share/dbus-1/system-services
	
