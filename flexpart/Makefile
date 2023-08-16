CC      = cc
CFLAGS  = -g
TARGET  = flexpart
OBJECT  = flexpart.o rdgroups.o
LIBS	= -lm
${TARGET}: ${OBJECT}
	${CC} ${CFLAGS} -o ${TARGET} ${OBJECT} ${LIBS}

clean:
	rm *.o ${TARGET}
