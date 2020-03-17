web:
	mkdir sense
	cp index.html sense/
	emcc fish.c -O3 -DNOSCROLL -DWEB -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 --embed-file graphics -o sense/daFish.js
trip:
	mkdir trip
	cp index.html trip/
	emcc trip.c -O3 -DNOSCROLL -DWEB -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 --embed-file tripgraph -o trip/game.js
splat:
	mkdir splat
	cp index.html splat/
	emcc splat.c -O3 -DNOSCROLL -DWEB -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 --embed-file splatgraph -o splat/game.js
