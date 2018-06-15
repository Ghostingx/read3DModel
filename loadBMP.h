#include<GL/gl.h>
#include<GL/glu.h>
#include<iostream>

#define BMP_Header_Length 54
int power_of_two(int n)
{
    if (n <= 0)
        return 0;
    return (n & (n - 1)) == 0;
}

/* 函数load_texture
* 读取一个BMP文件作为纹理
* 如果失败，返回0，如果成功，返回纹理编号
*/
GLuint load_texture(const char* file_name)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;

    GLint last_texture_ID;//上一次绑定的纹理编号
    GLuint texture_ID = 0;//纹理编号

                          // 打开参数中传入的纹理文件，如果失败，返回
    FILE* pFile = fopen(file_name, "rb");
    if (pFile == 0)
        return 0;

    // 读取文件中图象的宽度和高度
    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);//int32 8位4个字节
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    // 计算每行像素所占字节数，并根据此数据计算总像素字节数
    {
        GLint line_bytes = width * 3;
        while (line_bytes % 4 != 0)
            ++line_bytes;
        total_bytes = line_bytes * height;
    }

    // 根据总像素字节数分配内存
    pixels = (GLubyte*)malloc(total_bytes);
    if (pixels == 0)
    {
        fclose(pFile);
        return 0;
    }

    // 读取像素数据
    if (fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    // 在旧版本的OpenGL中
    // 如果图象的宽度和高度不是的整数次方，则需要进行缩放
    // 这里并没有检查OpenGL版本，出于对版本兼容性的考虑，按旧版本处理
    // 另外，无论是旧版本还是新版本，
    // 当图象的宽度和高度超过当前OpenGL实现所支持的最大值时，也要进行缩放
    {
        GLint max;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);//获取支持的最大纹理

                                                 //如果长宽有一个不是整数次方，或有一个大于支持的最大纹理，就重新设置长宽重新算一遍
        if (!power_of_two(width) || !power_of_two(height) || width > max || height > max)
        {
            //设定一些用于中间计算的新的值，这些值会在稍后被替代

            const GLint new_width = 256;
            const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
            GLint new_line_bytes, new_total_bytes;
            GLubyte* new_pixels = 0;

            // 计算每行需要的字节数和总字节数
            new_line_bytes = new_width * 3;
            while (new_line_bytes % 4 != 0)
                ++new_line_bytes;
            new_total_bytes = new_line_bytes * new_height;

            // 重新分配内存
            new_pixels = (GLubyte*)malloc(new_total_bytes);
            if (new_pixels == 0)
            {
                free(pixels);
                fclose(pFile);
                return 0;
            }

            // 进行像素缩放
            gluScaleImage(GL_RGB, width, height, GL_UNSIGNED_BYTE, pixels,
                new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

            // 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
            free(pixels);

            pixels = new_pixels;
            width = new_width;
            height = new_height;
        }
    }

    // 分配一个新的纹理编号，并不是分配编号1，而是分配1个新的编号到textureID....
    glGenTextures(1, &texture_ID);

    if (texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        std::cout<<"gentexture fail!"<<std::endl;
        return 0;
    }

    // 绑定新的纹理，载入纹理并设置纹理参数
    // 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture_ID); //第一个参数，表示你要得到什么状态的值,第二个参数即输出这个值

    glBindTexture(GL_TEXTURE_2D, texture_ID);//状态机，选定当前的纹理状态，以后的操作都是基于这个纹理

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //模型比纹理小了怎么办
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //模型比纹理大了怎么办
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //x坐标越界怎么办，GL_CLAMP表示超过的1.0的都按1.0那点的坐标绘制，GL_REPEAT表示不足重复补充直至达到那个值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //y坐标越界怎么办，重复
    GLfloat fColor[4] = { 0.0f,0.0f,0.0f,0.0f };
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, fColor);

    //载入pixels数组中的图像为当前的纹理状态
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);

    //glBindTexture(GL_TEXTURE_2D, last_texture_ID);

    // 之前为pixels分配的内存可在使用glTexImage2D以后释放
    // 因为此时像素数据已经被OpenGL另行保存了一份（可能被保存到专门的图形硬件中）
    free(pixels);
    //该ID可以唯一标识一纹理
    return texture_ID;
}
