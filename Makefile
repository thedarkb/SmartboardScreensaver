web:
	mkdir sense
	cp index.html sense/
	emcc main.c -O3 -DNOSCROLL -DWEB -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --embed-file graphics -o sense/daFish.js
