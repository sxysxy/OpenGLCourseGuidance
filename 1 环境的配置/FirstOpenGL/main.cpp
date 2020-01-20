#include <Windows.h>   //���ͷ�ļ�Ҫ��opengl��ͷ�ļ�֮ǰ
#include <gl/glew.h>   //���ͷ�ļ�Ҫ������opengl��ͷ�ļ�֮ǰ
#include <gl/glut.h>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

#define makeString(x) #x
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "freeglut.lib")
#ifdef _WIN64   //�������64λ����
#pragma comment(lib, "glew64.lib")
#else 
#pragma comment(lib, "glew32.lib")
#endif
//����Ŀ�Ⱥ͸߶ȣ����أ�
constexpr int viewWidth = 800;
constexpr int viewHeight = 800; 

class GLProgram
{
    union {
        struct {
            GLuint vshader, fshader;
        }shaders;
        GLuint shadersByIndex[sizeof(shaders) / sizeof(GLuint)];
    };
    constexpr static int shadersGLTable[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    
    GLuint program;

    void createShader(const std::string& code, int index) {
        const GLchar* codea[1] = { code.c_str() };
        GLuint& shader = shadersByIndex[index];

        shader = glCreateShader(shadersGLTable[index]);
        if (!shader)
            throw std::runtime_error("Can not create shader(glCreateShader)");
        glShaderSource(shader, 1, codea, nullptr);
        glCompileShader(shader);
        GLint res;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
        if (res == GL_FALSE) {
            GLint len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                std::string info;
                info.resize(len + 1);
                int w;
                glGetShaderInfoLog(shader, len, &w, (GLchar*)(info.c_str()));
                info[w] = 0;
                throw std::runtime_error("Shader compilation error:" + info);
            }
        }
    }

public:
    constexpr static int INDEX_VERTEX_SHADER = 0;
    constexpr static int INDEX_FRAGMENT_SHADER = 1;

    GLProgram() {
        memset(&shaders, 0, sizeof(shaders));
        program = 0;
    }
    ~GLProgram() {
        if (shaders.vshader)
            glDeleteShader(shaders.vshader);
        if (shaders.fshader)
            glDeleteShader(shaders.fshader);
        if (program)
            glDeleteProgram(program);
    }

    void createVShader(const std::string& code) {
        createShader(code, INDEX_VERTEX_SHADER);
    }
    void createFShader(const std::string& code) {
        createShader(code, INDEX_FRAGMENT_SHADER);
    }
    void createProgram() {
        program = glCreateProgram();
        if (!program)
            throw std::runtime_error("Fail to create opengl program(glCreateProgram)");
        glAttachShader(program, shaders.vshader);
        glAttachShader(program, shaders.fshader);
        glLinkProgram(program);
        GLint res;
        glGetProgramiv(program, GL_LINK_STATUS, &res);
        if (res == GL_FALSE) {
            GLint len;
            glGetProgramiv(shaders.fshader, GL_INFO_LOG_LENGTH, &len);
            if (len > 0) {
                std::string info;
                info.resize(len + 1);
                int w;
                glGetProgramInfoLog(shaders.fshader, len, &w, (GLchar*)(info.c_str()));
                info[w] = 0;
                throw std::runtime_error("Fail to link opengl program:" + info);
            }
        }
    }
    void useProgram() const {
        if (!program) {
            throw std::runtime_error("Trying to use an invalid opengl program");
        }
        glUseProgram(program);
    }
    GLuint getProgram() const { return program; }
    operator GLuint() const {  //����ǿ��ת��ΪGLuintʱ���õĺ���
        return program;
    }
    operator int() const { return program; }
};

//��ʼ��OpenGL
void initGL() {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        MessageBox(0, TEXT("��ʼ��GLEWʧ��"), TEXT("����"), 0);
        exit(1);
    }
    if (!GLEW_VERSION_4_0) {  //���OpenGL4.0֧��
        MessageBox(0, TEXT("�������δ֧��OpenGL4.0"), TEXT("����"), 0);
        exit(1);
    }
    glEnable(GL_FRAMEBUFFER_SRGB); //����٤��У��
}

struct Point {
    float x, y;
    Point middle(const Point& p) const {  //����һ������е�
        return Point{ (x + p.x) / 2, (y + p.y) / 2 };
    }
};
struct Triangle {
    Point points[3];
};
struct Color {
    float r, g, b, a;
};


//�����Ƕ�����ɫ����Ƭ����ɫ���Ĵ��룬���ܷǳ�������ֻ��Ϊ�˲�����ɫ�������Ƿ�����
static const char* vertexShaderCode = makeString(
#version 400\n
layout(location = 0) in vec2 pos;
out vec4 vo_pos;
void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}
);

static const char* fragmentShaderCode = makeString(
#version 400\n
uniform vec4 color;
void main() {
    gl_FragColor = color;
}
);

class RenderState {
    unsigned long long tick;   //��ʱ��
    //TODO: ��������Ҫ����Դ����������
    GLProgram program;
    constexpr static int MAX_DEPTH = 9;
    GLuint VAO, VBO;

    //ÿһ֡�߼����ݸ��µĹ���
    void update() {
        tick++;
    }
public:
    RenderState() {
        tick = 0;
    }
  
    //�ڵ�һ�λ���֮ǰ�Ĺ���
    void preScene() {
        printf("PreScene");
        program.createVShader(vertexShaderCode);
        program.createFShader(fragmentShaderCode);
        program.createProgram();
        program.useProgram();

        //���ﴴ��һ��VAO��VBO���Դ���һ�������ε���������
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, true, 2 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        //ʵ���ϻ������Ӧ�õ���glDeleteBuffer��������Դ...Ȼ��glut���ڹر�ʱ��ֱ���˳��ģ�û�л���������Դ...
        //����������
    }
 
    void recursiveDraw(const Triangle& tr, int depth = 0) {
        if (depth == MAX_DEPTH) 
            return;

        //���¶��㻺�������:���������ݻ��ɵ�ǰ�����ε���������
        glBufferData(GL_ARRAY_BUFFER, sizeof(tr), &tr, GL_DYNAMIC_DRAW);
       
        glUniform4f(glGetUniformLocation(program, "color"), 0.88, 0.76, 0.42, 1.0);  //��ɫ
        glDrawArrays(GL_TRIANGLES, 0, 3);  
  
        //���м��Ϳ�
        Point m1 = tr.points[0].middle(tr.points[1]);
        Point m2 = tr.points[0].middle(tr.points[2]);
        Point m3 = tr.points[1].middle(tr.points[2]);
        Triangle m = { m1, m3, m2 };  //�������е㹹�ɵ�������
        glBufferData(GL_ARRAY_BUFFER, sizeof(m), &m, GL_DYNAMIC_DRAW);
        glUniform4f(glGetUniformLocation(program, "color"), 0.0, 0.0, 0.0, 0.0);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //������һ�㣺
        recursiveDraw(Triangle{ tr.points[0], m1, m2 }, depth + 1);
        recursiveDraw(Triangle{ m1, tr.points[1], m3 }, depth + 1);
        recursiveDraw(Triangle{ m2, m3, tr.points[2] }, depth + 1);
    }
    //ÿһ֡���ƵĹ���
    void render() {
        update(); //�ȸ����߼������������Ⱦ
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  //����͸��ɫΪ����ɫ

        recursiveDraw(Triangle{ Point{0.0, 0.882}, Point{-1.0, -0.85}, Point{1.0, -0.85} });

        glutSwapBuffers();
        glutPostRedisplay();
    }

}state;

void render() {
    state.render();
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);   //RGBA��ɫģʽ��˫����
    glutInitWindowSize(viewWidth, viewHeight);    //���ڵĿ�͸�
    glutCreateWindow("OpenGL Demo");
    //�������д���ע��һ������glutCreateWindow�ĺ�����У��������Ϊû�д���opengl�����Ķ�ʧ�ܡ�
    glViewport(0, 0, viewWidth, viewHeight);
    initGL();  
    state.preScene();
    glutDisplayFunc(render);
    
    glutMainLoop();
    return 0;
}