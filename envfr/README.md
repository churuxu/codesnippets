# 文本替换环境变量工具

用于将文本文件中的 ${XXX} 替换成环境变量实际的值


编译结果需要有两个:  
envfr.exe    用于处理普通文本文件
envfrw.exe   用于处理unicode编码的文本文件



定义UNICODE宏（vs项目设置-常规-使用Unicode字符集）
编译结果文件作为 envfrw.exe

不定义UNICODE宏（vs项目设置-常规-使用多字节字符集）
编译结果文件作为 envfr.exe



