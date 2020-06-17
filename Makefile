# Luca Fossati (Luca.Fossati@esa.int), European Space Agency

# Software distributed under the "European Space Agency Public License � v2.0".

# All Distribution of the Software and/or Modifications, as Source Code or Object Code,
# must be, as a whole, under the terms of the European Space Agency Public License � v2.0.
# If You Distribute the Software and/or Modifications as Object Code, You must:
# (a)	provide in addition a copy of the Source Code of the Software and/or
# Modifications to each recipient; or
# (b)	make the Source Code of the Software and/or Modifications freely accessible by reasonable
# means for anyone who possesses the Object Code or received the Software and/or Modifications
# from You, and inform recipients how to obtain a copy of the Source Code.

# The Software is provided to You on an �as is� basis and without warranties of any
# kind, including without limitation merchantability, fitness for a particular purpose,
# absence of defects or errors, accuracy or non-infringement of intellectual property
# rights.
# Except as expressly set forth in the "European Space Agency Public License � v2.0",
# neither Licensor nor any Contributor shall be liable, including, without limitation, for direct, indirect,
# incidental, or consequential damages (including without limitation loss of profit),
# however caused and on any theory of liability, arising in any way out of the use or
# Distribution of the Software or the exercise of any rights under this License, even
# if You have been advised of the possibility of such damages.
#

LINK_FLAGS=-lm
#CCFLAGS=-O2 -finline-functions -pipe -march=native -fomit-frame-pointer -DNDEBUG
#CCFLAGS=-g3 -Wall
CCFLAGS=-g -Wall -Iinc
CC=g++

#all: compressor converter decompressor
all: compression

#compressor_main.o: ../compressor_main.c ../entropy_encoder.h ../predictor.h Makefile
#	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o compressor_main.o ../compressor_main.c

#converter.o: ../converter.c ../entropy_encoder.h ../predictor.h Makefile
#	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o converter.o ../converter.c

obj/entropy_encoder.o: src/entropy_encoder.c inc/entropy_encoder.h inc/predictor.h inc/utils.h Makefile
	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o obj/entropy_encoder.o src/entropy_encoder.c

obj/predictor.o: src/predictor.c inc/predictor.h inc/utils.h Makefile
	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o obj/predictor.o src/predictor.c

obj/predictor_nolocal.o: src/predictor.c inc/predictor.h inc/utils.h Makefile
	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o obj/predictor_nolocal.o src/predictor.c

obj/utils.o: src/utils.c inc/utils.h Makefile
	$(CC) $(CCFLAGS) -c -o obj/utils.o src/utils.c

#decompressor_main.o: ../decompressor_main.c ../utils.h ../unpredict.h ../decoder.h Makefile
#	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o decompressor_main.o ../decompressor_main.c

obj/unpredict.o: src/unpredict.c inc/unpredict.h inc/utils.h inc/predictor.h Makefile
	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -c -o obj/unpredict.o src/unpredict.c

obj/decoder.o: src/decoder.c inc/decoder.h inc/utils.h Makefile
	$(CC) $(CCFLAGS) -c -o obj/decoder.o src/decoder.c

obj/main.o: main.cpp inc/predictor.h inc/entropy_encoder.h Makefile
	$(CC) $(CCFLAGS) -c -o obj/main.o main.cpp

#decompressor: decompressor_main.o utils.o unpredict.o decoder.o predictor_nolocal.o Makefile
#	$(CC) $(CCFLAGS) -o decompressor decompressor_main.o unpredict.o decoder.o predictor_nolocal.o utils.o $(LINK_FLAGS)

#compressor: compressor_main.o entropy_encoder.o predictor.o utils.o Makefile
#	$(CC) $(CCFLAGS) -o compressor compressor_main.o entropy_encoder.o predictor.o utils.o $(LINK_FLAGS)

#converter: converter.o entropy_encoder.o predictor.o utils.o Makefile
#	$(CC) $(CCFLAGS) -o converter converter.o entropy_encoder.o predictor.o utils.o $(LINK_FLAGS)

compression: obj/main.o obj/entropy_encoder.o obj/predictor.o obj/utils.o Makefile
	$(CC) $(CCFLAGS) -o compression obj/main.o obj/entropy_encoder.o obj/predictor.o obj/utils.o $(LINK_FLAGS)

clean:
#	rm -rf compressor converter decompressor *.o *~ ../*~
	rm -rf obj/*
