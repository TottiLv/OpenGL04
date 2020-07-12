//
//  main.cpp
//  OpenGL04
//
//  Created by lvjianxiong on 2020/7/12.
//  Copyright © 2020 lvjianxiong. All rights reserved.
//

#include <stdio.h>
#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLGeometryTransform.h"
#include "GLFrustum.h"

#include <math.h>
#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

////设置角色帧，作为相机
GLFrame             viewFrame;
//使用GLFrustum类来设置透视投影
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLMatrixStack       modelViewMatix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transformPipeline;
GLShaderManager     shaderManager;


GLFrame                cameraFrame;
//标记：背面剔除、深度测试
int iCull = 0;
int iDepth = 0;

void SetUpRC(){
    //设置背景色
    glClearColor(0.7, 0.7, 0.7, 1.0);
    //初始化着色器
    shaderManager.InitializeStockShaders();
    //将相机向后移动10个单元：肉眼到物体之间的距离
    viewFrame.MoveForward(10);
    //创建一个甜甜圈
    //void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
    //参数1：GLTriangleBatch 容器帮助类
    //参数2：外边缘半径
    //参数3：内边缘半径
    //参数4、5：主半径和从半径的细分单元数量(一般主半径是从半径的2倍)
    gltMakeTorus(torusBatch, 1.0, 0.3, 52, 26);
    
    //点的大小(方便填充时，肉眼观察)
    glPointSize(4.0);
}

void ChangeSize(int w,int h){
    
    //防止h变为0
    if (h==0){
        h = 1;
    }
    //设置视口
    glViewport(0, 0, w, h);
    
    //setPerspective函数的参数是一个从顶点方向看去的视场角度（用角度值表示）
    //设置透视模式，初始化其透视矩阵
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    //把透视矩阵加载到透视矩阵对阵中
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    transformPipeline.SetMatrixStacks(modelViewMatix, projectionMatrix);
}

//渲染场景
void RenderScene(void){
    //清空颜色、深度缓存区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (iCull){
        //开启正面替除
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }else {
        glDisable(GL_CULL_FACE);
    }
    
    
    if (iDepth){
        glEnable(GL_DEPTH_TEST);
    }else {
        glDisable(GL_DEPTH_TEST);
    }
    
    //把摄像机矩阵压入模型矩阵中
    modelViewMatix.PushMatrix(viewFrame);
    
    //3.设置绘图颜色
    GLfloat vRed[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    
    //4.
     //使用平面着色器
     //参数1：平面着色器
     //参数2：模型视图投影矩阵
     //参数3：颜色
//     shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vRed);
    //使用默认光源着色器
    //通过光源、阴影效果跟提现立体效果
    //参数1：GLT_SHADER_DEFAULT_LIGHT 默认光源着色器
    //参数2：模型视图矩阵
    //参数3：投影矩阵
    //参数4：基本颜色值
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vRed);
    //5.绘制
    torusBatch.Draw();

    //6.出栈 绘制完成恢复
    modelViewMatix.PopMatrix();
    
    //7.交换缓存区
    glutSwapBuffers();
    
}

//键位设置，通过不同的键位对其进行设置
//控制Camera的移动，从而改变视口
void SpecialKeys(int key, int x, int y){
    //key 用来判断方向
    //viewFrame.RotateWorld 根据方向调整观察者位置
    if(key == GLUT_KEY_UP)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_LEFT)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 0.0f, 1.0f, 0.0f);
    
    //3.重新刷新
    glutPostRedisplay();
}

//注册右键出现菜单
void ProcessMenu(int value)
{
    switch (value) {
        case 1:
            iDepth = !iDepth;
            break;
        case 2:
            iCull = !iCull;
            
        default:
            break;
    }
    
    glutPostRedisplay();
}


int main(int argc,char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Geometry Program");
        
    //
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    //添加右击菜单栏
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("Toggle depth test",1);
    glutAddMenuEntry("Toggle cull backface",2);
//    glutAddMenuEntry("Set Fill Mode", 3);
//    glutAddMenuEntry("Set Line Mode", 4);
//    glutAddMenuEntry("Set Point Mode", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    GLenum err = glewInit();
    if (GLEW_OK != err ){
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetUpRC();
    
    glutMainLoop();
    
    return 0;
}
