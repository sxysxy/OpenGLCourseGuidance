# 计算机图形学&OpenGL系列教程（三） OpenGL中的基本概念

## 概述

&emsp;&emsp;本次教程将讲述OpenGL中的一些基本的概念，内容涵盖glut(freeglut), glew(严格上讲他们不属于是OpenGL的内容，但是还是需要讲)，图元，图形，模型，坐标系，向量，矩阵，渲染管线以及一些其它的概念，最后创建一个可以编译运行的OpenGL应用程序。

## glut

在第一次教程已经对glut做了简介，这里结合代码使用glut创建一个支持OpenGL渲染的窗口，并加入对键盘按键的处理函数，首先是头文件和库文件的引入：

```C++
#include <windows.h>   //这个头文件应当在opengl相关的头文件之前
#include <gl/glew.h>   //这个头文件应当在其它opengl相关的头文件之前
#include <gl/glut.h>
#include <iostream>    //输入输出
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "freeglut.lib")
#ifdef _WIN64   //如果编译64位程序
#pragma comment(lib, "glew64.lib")
#else 
#pragma comment(lib, "glew32.lib")
#endif
```

&emsp;&emsp;这里解释一下代码，Windows.h是Windows应用程序开发所需的头文件，其中定义了很多与Windows开发有关的常量，会被OpenGL用到(如果你在非Windows平台上开发则不需要Windows.h)，因此应当在所有有关OpenGL的头文件之前包含；虽然下一小节才讲glew，但是glew.h要求在其它OpenGL头文件之前被包含，因此这里仍先写出。然后是glut.h，这是本节要讲的重点。接下来的#pragma comment(lib, ...)则是对库文件使用的声明，告诉链接器去链接后面所给出的库文件中的二进制代码(函数实现)，(有关头文件库文件等实际工程方面的内容本教程不多做介绍，如感觉理解困难请参阅《程序员的自我修养——编译、链接与库》一书)，如果不对此作出声明，那么链接器将会给出无法解析外部符号的错误导致无法生成最终的可执行文件。_WIN64是一个当你要编译生成64位应用程序的时候会被定义的宏，使用这个宏判断生成的目标平台也是一个常用技巧。

&emsp;&emsp;接下来写出main函数：
```C++
const int viewWidth = 600;
const int viewHeight = 600;

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(viewWidth, viewHeight);
    glutCreateWindow("OpenGL Demo");
    glViewport(0, 0, viewWidth, viewHeight);
    glutDisplayFunc(render);
    glutKeyboardFunc(keyEvent);
    glutSpecialFunc(specialKeyEvent);
    glutMainLoop();
}
```





