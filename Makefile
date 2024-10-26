build:
	gcc x11.c -lX11 -o main
clean:
	rm main
install:build
	sudo mv main /usr/bin/activatelinux
