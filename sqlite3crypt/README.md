
# 实现sqlite加密接口

使用说明，将sqlite3crypt.c放在sqlite3.c同一目录，
项目文件中使用sqlite3crypt.c来代替sqlite3.c进行编译
项目中定义SQLITE_HAS_CODEC宏
然后编译出来的库就可以使用sqlite加密相关接口了：
sqlite3_key sqlite3_key_v2 sqlite3_rekey sqlite3_rekey_v2

