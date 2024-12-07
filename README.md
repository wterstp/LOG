# LOG
使用示例



```C++
#include "MyLogger.h"
 
MyLogger globalLogger;
 
int main() {
    //日志开关,默认为关,需要设为true才能使用
    MyLogger::getInstance().setLoggingEnabled(true);
    LOG_INFO("Hello, world");
    LOG_ERROR( "This is an error message");
    LOG_WARN( "This is a warning message");
    LOG_CRITICAL("This is a critical message");
    //打印变量
    int a = 5;
    
    LOG_INFO("point: {}",a);  
    // 继续你的程序逻辑
    return 0;
}
```

