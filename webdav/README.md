使用Tomcat搭建webdav文件服务器
==============================

1. 安装tomcat
2. 将Catalina目录拷贝到Tomcat目录的conf目录下
3. 修改conf/Catalina/localhost/webdav.xml文件，将路径改为需要共享的目录路径
4. 将WEB-INF拷贝到需要共享的目录
5. 修改web.xml中的readonly参数，false表示可以上传下载，true表示只能下载
6. 启动tomcat
7. 在浏览器访问，或使用支持webdav的软件(如WinSCP)访问






