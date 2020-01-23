#include <windows.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <iostream>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "freeglut.lib")
#ifdef _WIN64   //如果编译64位程序
#pragma comment(lib, "glew64.lib")
#else 
#pragma comment(lib, "glew32.lib")
#endif
const int viewWidth = 600;
const int viewHeight = 600;

void render() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
   

    //这里进行渲染的工作

    glutSwapBuffers();
    //glutPostRedisplay();
}

void keyEvent(unsigned char key, int, int) {
    printf("%c", key);   //参数key直接给出了输入的字符
}
void specialKeyEvent(int keyCode, int, int) {
    switch (keyCode) {     //通过keyCode的值判断是哪个特殊键
    case GLUT_KEY_F1:    //F1
        exit(0);         //实现按下F1退出
        break;
    default:
        break;
    }
}

void initGL() {
    GLenum err = glewInit();  //初始化
    if (err != GLEW_OK) {
        MessageBox(0, TEXT("初始化GLEW失败"), TEXT("错误"), 0);
        exit(1);
    }
    if (!GLEW_VERSION_4_0) {  //检查OpenGL4.0支持
        MessageBox(0, TEXT("本计算机未支持OpenGL4.0"), TEXT("错误"), 0);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(viewWidth, viewHeight);
    glutCreateWindow("OpenGL Demo");
    initGL();
    glViewport(0, 0, viewWidth, viewHeight);
    glutDisplayFunc(render);
    glutKeyboardFunc(keyEvent);
    glutSpecialFunc(specialKeyEvent);
    glutMainLoop();
    return 0;
}