
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
	make -f make/bighaul.mak clean
	make -f make/bigbone.mak clean
	make -f make/nanohaul.mak clean
	make -f make/nanobone.mak clean
	make -f make/sdkhaul.mak clean
	make -f make/sdkbone.mak clean
