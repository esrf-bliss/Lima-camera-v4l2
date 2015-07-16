v4l2-objs = V4L2DetInfoCtrlObj.o V4L2SyncCtrlObj.o V4L2Interface.o V4L2VideoCtrlObj.o

SRCS = $(v4l2-objs:.o=.cpp) 

CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include -I../../../third-party/Processlib/core/include -pthread -fPIC -g

all:	V4L2.o

V4L2.o: $(v4l2-objs)
	$(LD) -o $@ -r $+

clean:
	rm -f *.o *.P

.PHONY: check-syntax
check-syntax:
	$(CXX) -Wall -Wextra -pedantic -fsyntax-only $(CXXFLAGS) $(CHK_SOURCES)


%.o : %.cpp
	$(COMPILE.cpp) -MD $(CXXFLAGS) -o $@ $<
	@cp $*.d $*.P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	rm -f $*.d

-include $(SRCS:.cpp=.P)
