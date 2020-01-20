# 计算机图形学&OpenGL系列教程（一） OpenGL环境的配置

## 前言

&emsp; &emsp;本教程为北京交通大学2020年春季学期计算机与信息技术学院开设的计算机图形学课程准备，使用<a href="https://wiki.hk.wjbk.site/wiki/OpenGL">OpenGL</a>作为图形渲染引擎，并使用<a href="https://wiki.hk.wjbk.site/wiki/Freeglut">freeglut</a>创建窗口，处理键盘，鼠标输入等。内容涵盖从OpenGL环境的配置一直到创建简单的带有光源的三维场景。计算机图形学这门课程相比于之前所学的专业课如web基础训练，计算思维导论等，入门门槛较高，需要一些高等数学中空间解析几何，线性代数的知识；对于没有图形编程或者图像处理基础的同学来说，需要理解和掌握的新的概念也很多；程序代码量较大，需要一定编码能力；加之同学们从学长学姐那边听到的各种妖魔化这门课程的言论；因此我这里出了一个针对这门课程的教程，本配有相关代码及Visual studio工程；同时也算是作为学院科协信息部部长的工作。同时也因为只是针对课程的教程，也会有很多相关计算机图形学的内容的缺失，有兴趣的同学可以自行学习更多内容。

如转载本教程请保留出处。

## 什么是计算机图形学

&emsp; &emsp;简单地说，计算机图形学的主要研究内容就是研究如何在计算机中表示图形、以及利用计算机进行图形的计算、处理和显示的相关原理与算法。图形通常由点、线、面、体等几何元素和灰度、色彩、线型、线宽等非几何属性组成。从处理技术上来看，图形主要分为两类，一类是基于线条信息表示的，如工程图、等高线地图、曲面的线框图等，另一类是明暗图，也就是通常所说的真实感图形。

&emsp; &emsp;计算机图形学一个主要的目的就是要利用计算机产生令人赏心悦目的真实感图形。为此，必须创建图形所描述的场景的几何表示，再用某种光照模型，计算在假想的光源、纹理、材质属性下的光照明效果。所以计算机图形学与另一门学科计算机辅助几何设计有着密切的关系。事实上，图形学也把可以表示几何场景的曲线曲面造型技术和实体造型技术作为其主要的研究内容。同时，真实感图形计算的结果是以数字图象的方式提供的，计算机图形学也就和图像处理有着密切的关系。

&emsp; &emsp;图形与图像两个概念间的区别越来越模糊，但还是有区别的：图像纯指计算机内以位图形式存在的灰度信息，而图形含有几何属性，或者说更强调场景的几何表示，是由场景的几何模型和景物的物理属性共同组成的。

&emsp; &emsp;计算机图形学的研究内容非常广泛，如图形硬件、图形标准、图形交互技术、光栅图形生成算法、曲线曲面造型、实体造型、真实感图形计算与显示算法、非真实感绘制，以及计算可视化、计算机动画、自然景物仿真、虚拟现实等。

（来自wikipedia，更多信息可见<a href="https://wiki.hk.wjbk.site/wiki/%E8%AE%A1%E7%AE%97%E6%9C%BA%E5%9B%BE%E5%BD%A2%E5%AD%A6">这里</a>）

## 什么是OpenGL

<img src="./OpenGLLogo.png">

&emsp; &emsp;OpenGL（全称：Open Graphics Library，译名：开放图形库或者“开放式图形库”）是用于渲染2D、3D矢量图形的跨语言、跨平台的应用程序编程接口（API）。这个接口由近350个不同的函数调用组成，用来从简单的图形比特绘制复杂的三维景象，常用于CAD、虚拟现实、科学可视化程序和电子游戏开发。

&emsp; &emsp;OpenGL规范由1992年成立的OpenGL架构评审委员会（ARB）维护。ARB由一些对创建一个统一的、普遍可用的API特别感兴趣的公司组成。根据OpenGL官方网站，2002年6月的ARB投票成员包括3Dlabs、Apple Computer、ATI Technologies、Dell Computer、Evans & Sutherland、Hewlett-Packard、IBM、Intel、Matrox、NVIDIA、SGI和Sun Microsystems，Microsoft曾是创立成员之一，但已于2003年3月退出。

（以上来自wikipedia，更多信息可见<a href="https://wiki.hk.wjbk.site/wiki/OpenGL">这里</a>）

&emsp; &emsp;本次教程将使用OpenGL4.0，主要在Windows平台上使用Visual studio，配之以freeglut, glew两个第三方库（没有使用glm数学库，因为相关数学方面的东西属于是考试内容，因此教程中将自行实现所需的数学函数），利用可编程渲染管线实现一个分形三角形的绘制，效果如图：

<img src="./OpenGLDemo1.png">

## 在Visual Studio中使用OpenGL

&emsp; &emsp;据说计算机图形学课程最让人头疼的问题就是配置一个可以编译和运行OpenGL应用程序的环境。<del> 相传有同学从课程开始到结束都没能配出环境，如果找助教帮忙他们会先让你装个VS2013再说 </del>。依照前面说的，本教程使用freeglut和glew两个第三方库。freeglut是一个用来创建支持OpenGL渲染的窗口，以及能够处理键盘鼠标等输入的库，并且使用起来相当简单(相比于直接使用Windows API创建OpenGL设备上下文(Device Context)和渲染上下文(Render Context)，或是其它类似的库例如glfw)。glew是一个让你能够使用现代OpenGL特性的库(由于微软公司有自家的产品Direct3D与OpenGL是竞争对手关系，Windows系统自带的OpenGL API只支持到OpenGL1.1，为了能使用更新的OpenGL需要glew)。

&emsp; &emsp;freeglut和glew的文件我已经打包整合到一起，可以前往<a href="https://pan.baidu.com/s/1cwnPLTeflOh_lo85ILwJUQ">百度网盘</a>下载，提取码: ed1h。下面把它们安装到你的Visual studio里面



## 自测题目&启示

<li > 简述计算机图形学是什么 </li>

<li > 在上一节中安装freeglut和glew的方法，也适用于安装其它任何的库，尽量掌握 </li>