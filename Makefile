
.PHONY: bighaul nanohaul sdkhaul bigbone nanobone sdkbone

default: sdkbone

all: bighaul nanohaul sdkhaul bigbone nanobone sdkbone clean

bighaul:
	make -f make/bighaul.mak

nanohaul:
	make -f make/nanohaul.mak

sdkhaul:
	make -f make/sdkhaul.mak

bigbone:
	make -f make/bigbone.mak

nanobone:
	make -f make/nanobone.mak

sdkbone:
	make -f make/sdkbone.mak

clean:
	@rm -rf build
