
# 用于动态加载dll/so的包装类

用法示例：
```
class User32DLL {
public:
    DYLIB_LOAD(User32DLL, "user32");
    DYLIB_IMPORT(MessageBoxA);
    DYLIB_IMPORT(MessageBoxW);
};


User32DLL user32;

user32.MessageBoxA(NULL, "Hello World", "test", MB_OK);
```

