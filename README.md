TServer
=======
- TCommon 公共的定义， 以及对bscp和sip协议的处理函数。
- TLog 日志工具。
- TBus 共享内存通信模块。
- TBusMgr 共享内存管理工具。
- TConnd 用来把tcp连接发上来的流数据， 切割为消息包， 放入共享内存中给后台程序处理。
- utils 提供实用工具， 如start-stop-daemon可以用来启动一个后台进程。

项目依赖
========

- 基本数据结构：[TLibC](http://github.com/TDorm/TLibC)
- 数据交换格式：[TData](http://github.com/TDorm/TData)

安装
====
	make
	make install
