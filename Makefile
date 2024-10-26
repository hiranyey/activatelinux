build:
	gcc x11.c -lX11 -I/usr/include/freetype2 -lXft -lfontconfig -o main
clean:
	rm main
install:build
	sudo mv main /usr/bin/activatelinux
