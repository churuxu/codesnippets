@echo off

javac Hello.java


javah Hello


set JNI_INC_ARG=-I%JAVA_HOME%\include -I%JAVA_HOME%\include\win32
rem set JNI_INC_ARG=-I.


g++ %JNI_INC_ARG% -shared -o hello.dll Hello.cpp

