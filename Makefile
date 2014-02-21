flvmaker:
	clang++ -oflvmaker -DF4F_MAKER f4fpaser.cpp -lcurl
f4murl:
	clang++ -of4murl -DF4F_DOWNLOAD f4fpaser.cpp -lcurl
cleanfile:
	rm -f videourl.flv output
cleanexe:
	rm -f flvmaker f4murl
clean:cleanfile cleanexe
all:flvmaker f4murl
	