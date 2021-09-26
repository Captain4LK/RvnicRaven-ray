CC = gcc -O3 -flto=auto -s -std=c99 -pedantic -Wall -Wextra
CFLAGS+=-lm -lSDL2 -lSLK -lSDL2_mixer
HEADERS = ../../src/RvR_config.h ../../src/RvR_error.h

game: main.o state.o game.o player.o ai.o sprite.o sound.o RvR_config.o RvR_error.o RvR_math.o RvR_rand.o RvR_hash.o RvR_compress.o RvR_malloc.o RvR_pal.o RvR_pak.o RvR_texture.o RvR_raycast.o RvR_raycast_draw.o RvR_map.o
	$(CC) -o RvnicRaven $^ $(CFLAGS)
	chmod +x RvnicRaven
	mv RvnicRaven ../

main.o: ../../src/main.c ../../src/state.h ../../src/player.h ../../src/sound.h ../../src/ai.h ../../src/sprite.h ../../src/RvR_malloc.h ../../src/RvR_math.h ../../src/RvR_pak.h ../../src/RvR_pal.h ../../src/RvR_texture.h ../../src/RvR_map.h $(HEADERS)
	$(CC) -c $<

state.o: ../../src/state.c ../../src/state.h ../../src/game.h ../../src/ai.h ../../src/sprite.h ../../src/sound.h ../../src/player.h ../../src/RvR_math.h ../../src/RvR_texture.h $(HEADERS)
	$(CC) -c $<

game.o: ../../src/game.c ../../src/game.h ../../src/player.h ../../src/ai.h ../../src/sprite.h ../../src/RvR_texture.h ../../src/RvR_raycast_draw.h ../../src/RvR_malloc.h ../../src/RvR_math.h ../../src/RvR_map.h $(HEADERS)
	$(CC) -c $<

player.o: ../../src/player.c ../../src/player.h ../../src/ai.h ../../src/sprite.h ../../src/RvR_math.h ../../src/RvR_raycast.h $(HEADERS)
	$(CC) -c $<

ai.o: ../../src/ai.c ../../src/ai.h ../../src/game.h ../../src/RvR_math.h ../../src/sprite.h ../../src/sound.h ../../src/RvR_malloc.h $(HEADERS)
	$(CC) -c $<

sprite.o: ../../src/sprite.c ../../src/sprite.h ../../src/ai.h ../../src/RvR_math.h ../../src/RvR_texture.h $(HEADERS)
	$(CC) -c $<

sound.o: ../../src/sound.c ../../src/sound.h $(HEADERS)
	$(CC) -c $<

RvR_config.o: ../../src/RvR_config.c ../../src/RvR_malloc.h ../../src/RvR_hash.h $(HEADERS)
	$(CC) -c $<

RvR_error.o: ../../src/RvR_error.c $(HEADERS)
	$(CC) -c $<

RvR_math.o: ../../src/RvR_math.c ../../src/RvR_math.h $(HEADERS)
	$(CC) -c $<

RvR_rand.o: ../../src/RvR_rand.c ../../src/RvR_rand.h $(HEADERS)
	$(CC) -c $<

RvR_hash.o: ../../src/RvR_hash.c ../../src/RvR_hash.h $(HEADERS)
	$(CC) -c $<

RvR_compress.o: ../../src/RvR_compress.c ../../src/RvR_compress.h ../../src/RvR_malloc.h $(HEADERS)
	$(CC) -c $<

RvR_malloc.o: ../../src/RvR_malloc.c ../../src/RvR_malloc.h $(HEADERS)
	$(CC) -c $<

RvR_pal.o: ../../src/RvR_pal.c ../../src/RvR_pal.h ../../src/RvR_pak.h ../../src/RvR_malloc.h ../../src/RvR_math.h $(HEADERS)
	$(CC) -c $<

RvR_pak.o: ../../src/RvR_pak.c ../../src/RvR_pak.h ../../src/RvR_malloc.h ../../src/RvR_hash.h $(HEADERS)
	$(CC) -c $<

RvR_texture.o: ../../src/RvR_texture.c ../../src/RvR_texture.h ../../src/RvR_malloc.h ../../src/RvR_pak.h ../../src/RvR_compress.h $(HEADERS)
	$(CC) -c $<

RvR_raycast.o: ../../src/RvR_raycast.c ../../src/RvR_raycast.h ../../src/RvR_math.h ../../src/RvR_map.h ../../src/RvR_raycast_draw.h $(HEADERS)
	$(CC) -c $<

RvR_raycast_draw.o: ../../src/RvR_raycast_draw.c ../../src/RvR_raycast_draw.h ../../src/RvR_raycast.h ../../src/RvR_math.h ../../src/RvR_pal.h ../../src/RvR_texture.h ../../src/RvR_map.h ../../src/RvR_malloc.h $(HEADERS)
	$(CC) -c $<

RvR_map.o: ../../src/RvR_map.c ../../src/RvR_map.h ../../src/RvR_math.h ../../src/RvR_texture.h ../../src/RvR_pak.h ../../src/RvR_compress.h ../../src/RvR_malloc.h $(HEADERS)
	$(CC) -c $<

clean:
	rm *.o ../RvnicRaven
