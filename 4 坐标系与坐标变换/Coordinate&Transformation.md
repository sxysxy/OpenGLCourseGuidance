<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/katex@0.10.1/dist/katex.min.css">

# 计算机图形学&OpenGL系列教程（四） 坐标系与坐标变换

&emsp;&emsp;本次教程将围绕OpenGL中的坐标系和坐标变换，讲解有关的数学方面的内容，并完成所需的数学函数。本文的代码和数学原理都是课程期末考试的重点和难点，同学们一定要弄懂<del>请同学们找出大一上学期的线性代数课本以备不时之需</del>。

## 向量与矩阵

&emsp;&emsp;这里不再赘述向量与矩阵的基本概念与基本运算法则，以及线性空间的性质等内容，只讲OpenGL中的特殊性。OpenGL中常用四维坐标<img src="./formula/xyzw.gif" height="16px">表示点和向量，若第四个分量w的值为1，则表示三维空间中的一个点，如果第四个分量值为0，则表示一个向量，w分量取值这样设计的好处在之后会体现出来。w的取值其实不限定于0和1，它在一次渲染过程中经过运算可能会变成其它的值并具有其它的用途，但牢记0和1这两个取值对我们OpenGL编程来说初步是足够了。OpenGL的向量是**列向量**，方便起见，在书写时就略去了转置符号T，即教程中书写向量(x, y, z, 1)它指代的是：

<img src="./formula/xyz1ColumnVector.gif">

&emsp;&emsp;体现在代码上，我们这样定义点，向量：
```C++
using Point3 = float[3];    //相当于typedef float Point3[3]; 使用using关键字定义类型别名通常更加直观
using Vector3 = float[3];
using Point4 = float[4];
using Vector4 = float[4];
```

&emsp;&emsp;因为使用四维向量，所以进行相关运算(主要是基变换)的矩阵为四行四列的方阵。如果不特殊说明，本教程中术语**矩阵**也都特指四行四列的方阵。

&emsp;&emsp;OpenGL的矩阵是所谓的列优先矩阵，体现在编程中其实是，如果我们使用下面的变量m
```
Vector4 m[4]; 
```
去储存矩阵的元素，那么矩阵第i行第j列的元素储存在m\[j]\[i]，而非我们之前习惯的m\[i]\[j]。这虽然是一个小问题但是如果不注意很容易造成实际编程中出现错误。

&emsp;&emsp;于是我们定义出矩阵类Matrix4和相关的基本的运算：
```C++
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
};
```

## 坐标与坐标变换

&emsp;&emsp;计算机图形学要解决的一个基本问题便是如何将三维空间物体呈现在计算机的显示器上。同时方法还要足够方便，灵活，能够充分利用计算机强大的数字计算能力。将三维物体呈现在二维介质上的问题，从很久之前就被研究了，数百年前(也许是上千年前?)画家们就已经有了成熟的技术使得一副图像看起来有较强立体感。想象你站在一条笔直向前的街道上，向前方望去，你会发现，越向远方看去，本是平行的街道两边，竟然
会挨得越来越近，最终消失在同一点。因此画家们定义出了“消失点(Vanish point)”：

<img src="./PerspectiveWithVanishPoint.png">

那么上图中本来是矩形的红色砖块，就会变形称为梯形，而且水平方向本应同等长度的平行线，却有着“近大远小”的特点。依照在朝向远处的消失点的一对平行线延长将会相交于消失点，以及“近大远小”规律，就可以以此在2维平面上做出有立体感的矩形，立方体等。上面提到的只是所谓的“一点透视”，更加复杂一些的本教程不再讲述，<del>有兴趣同学假期宅着太闲的话可以去学画画。</del>除此之外，光照，图形表面的细节也有助于提高其立体感，这在之后的教程中会提到。

&emsp;&emsp;计算机图形学为了在计算机上完成这个任务，借助线性代数中基变换和坐标变换的方式，完成三维图形的点坐标到屏幕上二维坐标的变换。相关线性代数内容可以参阅<a href="https://wenku.baidu.com/view/fe772ad2195f312b3169a575.html">百度文库</a><del>或者拿出大一上学期线性代数课本好吧我知道你扔了</del>

### 局部空间





