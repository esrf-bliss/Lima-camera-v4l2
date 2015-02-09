tutorial-objs = TutorialDetInfoCtrlObj.o TutorialSyncCtrlObj.o TutorialInterface.o TutorialCamera.o

SRCS = $(tutorial-objs:.o=.cpp) 

CXXFLAGS += -I../include -I../../../hardware/include -I../../../common/include -I../../../third-party/Processlib/core/include -pthread -fPIC -g

all:	Tutorial.o

Tutorial.o: $(tutorial-objs)
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


.PHONY: check-syntax
check-syntax:
	$(CXX) -Wall -Wextra -fsyntax-only $(CXXFLAGS) $(CHK_SOURCES)
