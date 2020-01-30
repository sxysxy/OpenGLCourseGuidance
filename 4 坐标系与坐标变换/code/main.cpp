#include <windows.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include <iostream>
#include <cassert>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "freeglut.lib")
#ifdef _WIN64   //如果编译64位程序
#pragma comment(lib, "glew64.lib")
#else 
#pragma comment(lib, "glew32.lib")
#endif
const int viewWidth = 600;
const int viewHeight = 600;

using Point3 = float[3];
using Vector3 = float[3];
using Point4 = float[4];
using Vector4 = float[4];
class Matrix4 {
    float data[4][4];
public:
    Matrix4() {
        memset(data, 0, sizeof(data));
        makeUnit();
    }
    Matrix4(const Matrix4& m) {  //拷贝构造函数
        *this = m;
    }
    Matrix4(const std::initializer_list<Vector4> &vs) {
        assert(vs.end() - vs.begin() == 4);
        for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) {  //因为OpenGL矩阵是列优先矩阵，这里将输入的数据转置储存
            data[i][j] = (*(vs.begin() + j))[i];
        }
    }
    //索引。注意OpenGL矩阵的第i行第j列元素存放在data[j][i]
    float& operator()(int i, int j) { return data[j][i]; }
    const float& operator()(int i, int j) const { return data[j][i]; }
    using float4 = Vector4;
    //下面两个成员函数在对象被强制转换为float4时调用
    operator float4* () const { return const_cast<float4*>(data); }
    operator float4* () { return data; }
    //下面两个成员函数在对象被强制转换为void*时调用
    operator void* () const { return const_cast<float4*>(data); }
    operator void* () { return data; }
    Matrix4& operator=(const Matrix4& m) {
        memcpy(data, (float4*)m, sizeof(data));
        return *this;
    }
    Matrix4& multiple(const Matrix4& m) {
        float4 tmp[4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                tmp[j][i] = 0.0;  //注意i,j反过来
                for (int k = 0; k < 4; k++) {
                    tmp[j][i] += (*this)(i, k) * m(k, j);
                }
            }
        }
        memcpy(data, tmp, sizeof(data));
        return *this;
    }
    //单位阵
    Matrix4& makeUnit() {
        data[0][0] = data[1][1] = data[2][2] = data[3][3] = 1.0;
        return *this;
    }
    //伸缩变换
    Matrix4& scale(float sx, float sy, float sz) {
        Matrix4 m;
        m(0, 0) = sx;
        m(1, 1) = sy;
        m(2, 2) = sz;
        multiple(m);
        return *this;
    }
    //向量点乘
    template<int N, class vectorType = float[N]>
    static float dot(const vectorType &v1, const vectorType &v2) {
        float res = 0.0;
        for (int i = 0; i < N; i++)
            res += v1[i] * v2[i];
        return res;
    }
    //向量-
    template<int N, class vectorType = float[N]>
    static void sub(const vectorType& v1, const vectorType& v2, vectorType& res) {
        for (int i = 0; i < N; i++)
            res[i] = v1[i] - v2[i];
    }
    //三维向量叉积
    static void cross(const Vector3& v1, const Vector3& v2, Vector3 &res) {
        res[0] = v1[1] * v2[2] - v1[2] * v2[1];
        res[1] = v1[2] * v2[0] - v1[0] * v2[2];
        res[2] = v1[0] * v2[1] - v1[1] * v2[0];
    }
    //向量长度
    template<int N, class vectorType = float[N]>
    static inline float length(const vectorType &vec) {
        float sum = 0.0;
        for (int i = 0; i < N; i++)
            sum += vec[i] * vec[i];
        return sqrtf(sum);
    }
    //向量归一化(取单位向量)
    template<int N, class vectorType = float[N]>
    static void normalize(vectorType& vec) {
        float len = length<N>(vec);
        if (len < 1e-8)  //consider as a zero vector 
            return;
        for (int i = 0; i < N; i++)
            vec[i] /= len;
    } 
    Matrix4& rotate(const Vector3& axis, float angle) {
        float c = cosf(angle), s = sinf(angle);
        Vector3 N;
        memcpy(N, axis, sizeof(N));
        normalize<3>(N);
        float xy = N[0] * N[1];
        float xz = N[0] * N[2];
        float yz = N[1] * N[2];
        Matrix4 trans({
            {c+(1-c)*N[0]*N[0], (1-c)*xy+s*N[2], (1-c)*xz-s*N[1], 0},
            {(1-c)*xy-s*N[2], c+(1-c)*N[1]*N[1], (1-c)*yz+s*N[0], 0},
            {(1-c)*xz+s*N[1], (1-c)*yz-s*N[0], c+(1-c)*N[2]*N[2], 0},
            {              0,               0,                 0, 1}
        });
        multiple(trans);
        return *this;
    }
    
    //平移变换
    Matrix4& translate(float tx, float ty, float tz) {
        Matrix4 m;
        m(0, 3) = tx;
        m(1, 3) = ty;
        m(2, 3) = tz;
        multiple(m);
        return *this;
    }
    
    Matrix4& view(const Point3& eyePosition, const Point3& targetPosition, const Vector3& upDirection) {
        Vector3 F, S, U;
        sub<3>(targetPosition, eyePosition, F);
        normalize<3>(F);
        cross(F, upDirection, S);
        normalize<3>(S);
        cross(S, F, U);
        Matrix4 trans({
            {S[0], S[1], S[2],   -dot<3>(S, eyePosition)},
            {U[0], U[1], U[2],   -dot<3>(U, eyePosition)},
            {-F[0], -F[1], -F[2], dot<3>(F, eyePosition)},
            {0, 0, 0, 1}
        });
        multiple(trans);
        return *this;
    }

    //投影变换：
    //正交投影
    Matrix4& ortho(const Point3& minPoints, const Point3& maxPoints) {
        Matrix4 trans({
            {2 / (maxPoints[0] - minPoints[0]), 0, 0, -(maxPoints[0] + minPoints[0]) / (maxPoints[0] - minPoints[0])},
            {0, 2 / (maxPoints[1] - minPoints[1]), 0, -(maxPoints[1] + minPoints[1]) / (maxPoints[1] - minPoints[1])},
            {0, 0, 2 / (maxPoints[2] - minPoints[2]), -(maxPoints[2] + minPoints[2]) / (maxPoints[2] - minPoints[2])},
            {0, 0, 0, 1}
        });
        multiple(trans);
        return *this;
    }
    //透视投影
    Matrix4& perspective(float fovyAngle, float aspectRatio, float zNear, float zFar) {
        assert(aspectRatio > 1e-8);
        float cot2 = 1.0f / tanf(fovyAngle / 2.0f);
        Matrix4 trans({
            {cot2 / aspectRatio, 0, 0, 0},
            {0, cot2, 0, 0},
            {0, 0, -(zFar + zNear) / (zFar - zNear), -(2 * zFar * zNear) / (zFar - zNear)},
            {0, 0, -1, 0}
        });
        multiple(trans);
        return *this;
    }
};

class GLProgram
{
    union {
        struct {
            GLuint vshader, fshader;
        }shaders;
        GLuint shadersByIndex[sizeof(shaders) / sizeof(GLuint)];
    };
    constexpr static int shadersGLTable[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
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
    operator GLuint() const {  //对象被强制转换为GLuint时调用的函数
        return program;
    }
    operator int() const { return program; }
};

#define makeString(x) #x

const static char* vertexShaderCode = makeString(
#version 400\n
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
out vec4 vo_pos;
out vec3 vo_color;
uniform mat4 WVP;
void main() {
    gl_Position = WVP * vec4(pos, 1.0);
    vo_color = color;
}
);
const static char* fragmentShaderCode = makeString(
#version 400\n
in vec3 vo_color;
void main() {
    gl_FragColor = vec4(vo_color, 1.0);
}
);

GLProgram program;
unsigned long long tick = 0;
GLuint VAO, VBO;

void preRender() {
    program.createVShader(vertexShaderCode);
    program.createFShader(fragmentShaderCode);
    program.createProgram();
    program.useProgram();

    //三棱锥
    using Vertex = float[6];
    Vertex vertexes[] = {
        {0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-0.6f, -0.6f, 0.5f, 0.0f, 1.0f, 0.0f},
        {0.6f, -0.6f, 0.5f, 0.0f, 0.0f, 1.0f},

        {-0.6f, -0.6f, 0.5f, 0.0f, 1.0f, 0.0f},
        {0.6f, -0.6f, 0.5f, 0.0f, 0.0f, 1.0f},
        {0.0f, -0.6f, -0.5f, 1.0f, 0.0f, 0.0f},

        {0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f},
        {-0.6f, -0.6f, 0.5f, 0.0f, 1.0f, 0.0f},
        {0.0f, -0.6f, -0.5f, 1.0f, 0.0f, 0.0f},

        {0.0f, 0.8f, 0.0f, 1.0f, 0.0f, 1.0f},
        {0.6f, -0.6f, 0.5f, 0.0f, 0.0f, 1.0f},
        {0.0f, -0.6f, -0.5f, 1.0f, 0.0f, 0.0f},
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);
}

void render() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    tick++;
    //这里进行渲染的工作
    const float angleRad = static_cast<float>(tick % 360)* acosf(-1) / 180.0f;
    //const float angleRad = 0.4f;
    
    //WVP变换
    Matrix4 WVP;
    //SRTVP
    WVP.perspective(45.0f * acosf(-1) / 180.0f, 1.0f, 0.1f, 100.0f).
        view({ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }).
        translate(0.0f, 0.1f, -1.0f).rotate({ 0.0f, 1.0f, -0.4f }, angleRad).scale(1.5f, 1.5f, 1.5f);
    glUniformMatrix4fv(glGetUniformLocation(program, "WVP"), 1, GL_FALSE, static_cast<GLfloat*>((void*)WVP));

    glDrawArrays(GL_TRIANGLES, 0, 4 * 3);
    
    glutSwapBuffers();
    Sleep(10);
    glutPostRedisplay();
}

void keyEvent(unsigned char key, int, int) {
    printf("%c", key);   //参数key直接给出了输入的字符
}
void specialKeyEvent(int keyCode, int, int) {
    switch (keyCode) {     //通过keyCode的值判断是哪个特殊键
    case GLUT_KEY_F1:    //F1
        exit(0);         //实现按下F1退出
        break;
    case GLUT_KEY_UP:

        break;
    case GLUT_KEY_DOWN:

        break;
    case GLUT_KEY_LEFT:
        
        break;
    case GLUT_KEY_RIGHT:

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
    preRender();
    glutMainLoop();
    return 0;
}