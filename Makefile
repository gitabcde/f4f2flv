flvmaker:
	clang++ -oflvmaker -DF4F_MAKER f4fpaser.cpp -lcurl
f4murl:
	clang++ -of4murl -DF4F_DOWNLOAD f4fpaser.cpp -lcurl
clean:
	rm -f videourl.flv output flvmaker f4murl
all:flvmaker f4murl
	