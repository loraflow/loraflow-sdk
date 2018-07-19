# 编译

按以下步骤编译SDK的可执行文件：

```
git clone --recurse-submodules https://github.com/loraflow/loraflow-sdk.git
cd loraflow-sdk
make
```

输出目标文件：

`build/sdkbone/sdkbone.elf`

# 配置

配置文件位于`conf`目录，可参照模板生成和修改配置：

```
cd conf
cp local_conf.json.template local_conf.json
vim local_conf.json
```

其中EUI-64格式的`gateway_ID`必须要和设备的MAC地址保持一致，
如果MAC地址为EUI-48格式，还需要在中间插入`FFFE`将其转换为EUI-64格式

# 运行

SDK的运行方式如下：

```
./build/sdkbone/sdkbone.elf --conf=DIRECTORY --ns=SERVER
```

其中DIRECTORY为配置文件加载的目录, SERVER为LoRaWAN NS的服务器地址

用法举例：

```
./build/sdkbone/sdkbone.elf --conf=./conf --ns=loraflow.io
```

调试模式：

```
./build/sdkbone/sdkbone.elf --conf=./conf --ns=loraflow.io --log=debug
```
