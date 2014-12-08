esp8266-smartlink
=======
使用方法：在需要使用 `smartlink` 的地方调用

```c
smartlink_init(callback_func, callback_func_args);
```

此时芯片会清除 wifi 设置并且将 wifi 设置为 station 模式，监听周围的 `probe_request` 包，此时如果有这样的包并且ssid格式为 `ssid<!-SL-!>password`，则会获取这个ssid和密码自动连接。

KNOWN BUGS
=======
`ssid<!-SL-!>password` 不能超过 32 字节，后续版本会改进

TODOLIST
=======
自定义一个传输协议，可以传输更多的数据
