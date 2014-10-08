The Service Framework For Game
==============================
TSF4G是一个服务器软件开发工具包

特性
----
- 使用C语言编写， 不链接任何第三方库， 专注于Linux平台下的服务器开发。
- 使用[TDR](http://github.com/randyliu/TDR)对数据进行了严谨的描述。
	

功能
----
	TUtils 定义常用的数据结构。
	TLog 日志工具, 支持写log到文件和共享内存。
	TLogD 日志进程， 支持把共享内存中的Log存入mysql数据库。
	TBus 共享内存通信模块。
	TBusMgr 共享内存管理工具。
	TConnd 用来把tcp连接发上来的流数据， 切割为消息包， 放入共享内存中给后台程序处理。
	Utils 提供实用工具， 如start-stop-daemon可以用来启动一个后台进程。
	
安装
----
- 安装 [TDR](http://github.com/randyliu/TDR) 编译器
- 安装 [re2c](http://www.re2c.org/) 编译器
- 编译

		make
		make install
