SERIAL_LIB_DIR := /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message
COMMON_DIR := ../common
BUILDIR = build

SRCS := main.cpp serialMsg_conf.cpp messageImplLinux.cpp
SRCS += $(SERIAL_LIB_DIR)/serialMsg.cpp $(SERIAL_LIB_DIR)/libsrc/serialMsg_conf_lib.cpp
OBJS1 := $(SRCS:.cpp=.o)
OBJS := $(notdir $(OBJS1))
OBJSPATH := $(addprefix $(BUILDIR)/, $(OBJS))



CPPFLAGS = -I$(SERIAL_LIB_DIR) -I$(COMMON_DIR) -I. -std=c++14 -ggdb3 -Wall -Wextra -fstrict-aliasing
CPP = g++-7

#$(info $$OBJSPATH is [${OBJSPATH}])

all: serm


$(BUILDIR)/%.o : %.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

build/serialMsg.o : $(SERIAL_LIB_DIR)/serialMsg.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

build/serialMsg_conf_lib.o : $(SERIAL_LIB_DIR)/libsrc/serialMsg_conf_lib.cpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

serm:  $(OBJSPATH)
	$(CPP) $(CPPFLAGS) -o $@ $(OBJSPATH) -lpthread

clean:
	-rm -f serm */*.o *.o *.a *.so *.so.* *~

depend:
	makedepend -- $(CPPFLAGS) -- $(SRCS)


# ARCHIVE

#OBJS = main.o messageImplLinux.o serialMsg_conf.o \
#       serialMsg.o

# DO NOT DELETE

main.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.hpp
main.o: /usr/include/stdint.h serialMsg_conf.hpp
main.o: ../common/frameCommonConf.hpp
main.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.hpp
main.o: messageImplLinux.hpp ../common/messageCommonDef.hpp
serialMsg_conf.o: serialMsg_conf.hpp ../common/frameCommonConf.hpp
serialMsg_conf.o: /usr/include/stdint.h
serialMsg_conf.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.hpp
messageImplLinux.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.hpp
messageImplLinux.o: /usr/include/stdint.h serialMsg_conf.hpp
messageImplLinux.o: ../common/frameCommonConf.hpp
messageImplLinux.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.hpp
messageImplLinux.o: messageImplLinux.hpp ../common/messageCommonDef.hpp
messageImplLinux.o: ../common/messageCommonRegister.cpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.hpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.o: /usr/include/stdint.h
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.o: serialMsg_conf.hpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.o: ../common/frameCommonConf.hpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/serialMsg.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.hpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.o: /home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.hpp
/home/alex/DEV/STM32/CHIBIOS/COMMON/various/serial_message/libsrc/serialMsg_conf_lib.o: /usr/include/stdint.h
