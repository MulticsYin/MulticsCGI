# CGI_server-ProcessPool
						
	该项目是使用进程池实现一个并发的CGI服务器.		
	processpool.h是一个半同步/半异步并发模式的进程池，为了避免在父、子进程之间传递文件描述符描述符我们将接受新连接的操作放到子进程中（该模式下，一个客户端连接上的所有任务始终是由一个子进程来处理的）。		
	CGI_poll.cpp是该项目的源代码。		
	CGI_dup.cpp为演示CGI工作原理，使用dup函数实现一个基本的CGI服务器，先关闭标准输出文件符STDOUT_FILENO（值为1),然后复制socket文件描述符connfd。因为dup总是返回系统中最小的可用文件描述符，所以返回值实际上是1,就是之前关闭的标准输出文件描述符的值。因此，服务器输出到标准输出的内容就会直接发送到与客户连接对应的socket上，因此printf调用的输出将被客户端获得（而不是显示在服务器程序的终端上）。这就是CGI服务器的基本原理。		
	passfd.cpp描述在进程间传递文件描述符。		
	参考：CGI简介：https://en.wikipedia.org/wiki/CGI
		      http://baike.baidu.com/item/CGI/607810
