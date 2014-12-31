esp8266-smartlink 
=======

Instruction in English is followed by Chinese.

### 名词定义
*Smartlink* 用于在物理网设备没有外部输入端时，使用手机对其进行配置的一种通用的说法。  

### Definition of Smartlink
*Smartlink* is a generic phrase when you configure your IoT devices by mobile phone, PC etc. instead of physical external devices.

### 使用方法
在需要使用 `smartlink` 的地方调用

```c
smartlink_init(callback_func, callback_func_args, callback_failed_func, max_retry);
```

**手机端**: 设置完成后会调用 `(*callback_func)(callback_func_args)`，连接失败则会调用 `(*callback_failed_func)(callback_func_args)`
使用手机手动添加一个Wi-Fi网络，ssid为 ssid<!-SL-!>password,确定的几乎一瞬间 8266 就会收到并且开始配置。

**SDK 版本**: 0.9.4 by Espressif

### Usage
Call `smartlink_init(callback_func, callback_func_args, callback_failed_func, max_retry);` when you want to use Smarklink to configure.

`callback_func`: callback function when connect target successfully.  
`callback_func_args`: arguments of success callback function.  
`callback_failed_func`: callback function when fail to connect target.  
`max_retry`: maximiun number of retry.

**Mobile**: Add a ssid named `ssid<!-SL-!>password` manually, 8266 will receive this request almost immediately and finish configuration.

**SDK Version**: 0.9.4 by Espressif

### 原理
`Probe Request` 包的 `tagged parameter` 字段是不加密的，这个字段中的 `ssid` 可以每次最大传送 32 字节的数据。

### Principle
`tagged parameter` in `Probe Request` packet is not encrypted, we can reuse this area to send data to our IoT devices.

### 协议
The MIT License (MIT)

Copyright (c) 2014 latyas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


### License
See above.

### Reference
[Cisco Support Forums](https://supportforums.cisco.com/document/52391/80211-frames-starter-guide-learn-wireless-sniffer-traces)
