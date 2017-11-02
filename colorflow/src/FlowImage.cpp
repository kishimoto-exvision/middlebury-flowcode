// flow_io.cpp
//
// read and write our simple .flo flow file format

// ".flo" file format used for optical flow evaluation
//
// Stores 2-band float image for horizontal (u) and vertical (v) flow components.
// Floats are stored in little-endian order.
// A flow value is considered "unknown" if either |u| or |v| is greater than 1e9.
//
//  bytes  contents
//
//  0-3     tag: "PIEH" in ASCII, which in little endian happens to be the float 202021.25
//          (just a sanity check that floats are represented correctly)
//  4-7     width as an integer
//  8-11    height as an integer
//  12-end  data (width*height*2*4 bytes total)
//          the float values for u and v, interleaved, in row order, i.e.,
//          u[row0,col0], v[row0,col0], u[row0,col1], v[row0,col1], ...
//


// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25  // check for this when READING the file
#define TAG_STRING "PIEH"    // use this when WRITING the file

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "FlowImage.h"

using namespace std;

void FlowImage::ReadFromFloFile(const char* file_path)
{
    if (strcmp(file_path, "") == 0)
        throw CError("ReadFlowFile: empty file_path");

    const char *dot = strrchr(file_path, '.');
    if (strcmp(dot, ".flo") != 0)
        throw CError("ReadFlowFile (%s): extension .flo expected", file_path);

    FILE* stream = fopen(file_path, "rb");
    if (stream == 0)
        throw CError("ReadFlowFile: could not open %s", file_path);

    int width, height;
    float tag;

    if ((int)fread(&tag, sizeof(float), 1, stream) != 1 ||
        (int)fread(&width, sizeof(int), 1, stream) != 1 ||
        (int)fread(&height, sizeof(int), 1, stream) != 1)
        throw CError("ReadFlowFile: problem reading file %s", file_path);

    if (tag != TAG_FLOAT) // simple test for correct endian-ness
        throw CError("ReadFlowFile(%s): wrong tag (possibly due to big-endian machine?)", file_path);

    // another sanity check to see that integers were read correctly (99999 should do the trick...)
    if (width < 1 || width > 99999)
        throw CError("ReadFlowFile(%s): illegal width %d", file_path, width);

    if (height < 1 || height > 99999)
        throw CError("ReadFlowFile(%s): illegal height %d", file_path, height);

    int nBands = 2;
    FlowMatF32C2 = cv::Mat(height, width, CV_32FC2);

    //printf("reading %d x %d x 2 = %d floats\n", width, height, width*height*2);
    int n = nBands * width * height;
    if ((int)fread(FlowMatF32C2.data, sizeof(float), n, stream) != n)
        throw CError("ReadFlowFile(%s): file is too short", file_path);

    if (fgetc(stream) != EOF)
        throw CError("ReadFlowFile(%s): file is too long", file_path);

    fclose(stream);
}

// write a 2-band image into flow file 
void FlowImage::WriteToFloFile(const char* file_path)
{
    if (strcmp(file_path, "") == 0)
        throw CError("WriteFlowFile: empty file_path");

    const char *dot = strrchr(file_path, '.');
    if (dot == NULL)
        throw CError("WriteFlowFile: extension required in file_path '%s'", file_path);

    if (strcmp(dot, ".flo") != 0)
        throw CError("WriteFlowFile: file_path '%s' should have extension '.flo'", file_path);

    int width = FlowMatF32C2.rows;
    int height = FlowMatF32C2.cols;
    int nBands = FlowMatF32C2.channels();

    if (nBands != 2)
        throw CError("WriteFlowFile(%s): image must have 2 bands", file_path);

    FILE *stream = fopen(file_path, "wb");
    if (stream == 0)
        throw CError("WriteFlowFile: could not open %s", file_path);

    // write the header
    fprintf(stream, TAG_STRING);
    if ((int)fwrite(&width, sizeof(int), 1, stream) != 1 ||
        (int)fwrite(&height, sizeof(int), 1, stream) != 1)
        throw CError("WriteFlowFile(%s): problem writing header", file_path);

    // write the rows
    int n = nBands * width * height;
    if ((int)fwrite(FlowMatF32C2.data, sizeof(float), n, stream) != n)
        throw CError("WriteFlowFile(%s): problem writing data", file_path);

    fclose(stream);
}

inline void FlowImage::SetColorWheelColor(int r, int g, int b, int k)
{
    ColorWheel[k][0] = r;
    ColorWheel[k][1] = g;
    ColorWheel[k][2] = b;
}

void FlowImage::MakeColorWheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    ColorsCount = RY + YG + GC + CB + BM + MR;
    //printf("ColorsCount = %d\n", ColorsCount);
    if (ColorsCount > ColorsCountMax)
        exit(1);
    int i;
    int k = 0;
    for (i = 0; i < RY; i++) SetColorWheelColor(255, 255 * i / RY, 0, k++);
    for (i = 0; i < YG; i++) SetColorWheelColor(255 - 255 * i / YG, 255, 0, k++);
    for (i = 0; i < GC; i++) SetColorWheelColor(0, 255, 255 * i / GC, k++);
    for (i = 0; i < CB; i++) SetColorWheelColor(0, 255 - 255 * i / CB, 255, k++);
    for (i = 0; i < BM; i++) SetColorWheelColor(255 * i / BM, 0, 255, k++);
    for (i = 0; i < MR; i++) SetColorWheelColor(255, 0, 255 - 255 * i / MR, k++);
}

inline void FlowImage::GetColor(float fx, float fy, unsigned char *pix)
{
    if (ColorsCount == 0) { MakeColorWheel(); }

    float rad = sqrt(fx * fx + fy * fy);
    float a = atan2(-fy, -fx) / 3.14159265358979323846;
    float fk = (a + 1.0) / 2.0 * (ColorsCount - 1);
    int k0 = (int)fk;
    int k1 = (k0 + 1) % ColorsCount;
    float f = fk - k0;
    //f = 0; // uncomment to see original color wheel
    for (int b = 0; b < 3; b++)
    {
        float col0 = ColorWheel[k0][b] / 255.0;
        float col1 = ColorWheel[k1][b] / 255.0;
        float col = (1 - f) * col0 + f * col1;
        if (rad <= 1)
            col = 1 - rad * (1 - col); // increase saturation with radius
        else
            col *= .75; // out of range
        pix[2 - b] = (int)(255.0 * col);
    }
}

void FlowImage::GetReferenceImage(cv::Mat& reference_image, float flow_l2_distance_max, int image_one_side_length)
{
    // make picture a bit bigger to show out-of-range coding
    float range = 1.04 * flow_l2_distance_max;
    try
    {
        if (reference_image.rows != image_one_side_length || reference_image.cols != image_one_side_length)
        {
            reference_image = cv::Mat(image_one_side_length, image_one_side_length, CV_8UC3);
        }
        int s2 = image_one_side_length / 2;
        for (int y = 0; y < image_one_side_length; y++)
        {
            int row_offset = y * image_one_side_length;
            uchar* rgb_row_ptr = reference_image.data + 3 * row_offset;
            for (int x = 0; x < image_one_side_length; x++)
            {
                float fx = (float)x / (float)s2 * range - range;
                float fy = (float)y / (float)s2 * range - range;
                // make black coordinate axes
                if (x == s2 || y == s2)
                    continue;
                GetColor(fx / flow_l2_distance_max, fy / flow_l2_distance_max, rgb_row_ptr + 3 * x);
            }
        }
        int ir = (int)flow_l2_distance_max;
        int ticksize = image_one_side_length < 120 ? 1 : 2;
        for (int k = -ir; k <= ir; k++)
        {
            int ik = (int)(k / range * s2) + s2;
            for (int t = -ticksize; t <= ticksize; t++)
            {
                auto&& p1 = reference_image.at<cv::Vec3b>(s2 + t, ik);
                p1[0] = p1[1] = p1[2] = 0;
                auto&& p2 = reference_image.at<cv::Vec3b>(ik, s2 + t);
                p2[0] = p2[1] = p2[2] = 0;
            }
        }
    }
    catch (exception &err)
    {
        fprintf(stderr, err.what());
        fprintf(stderr, "\n");
    }
}

float FlowImage::GetFlowL2DistanceMaximum()
{
    int width = FlowMatF32C2.cols;
    int height = FlowMatF32C2.rows;

    float maxx = -1e10, maxy = -1e10;
    float minx = 1e10, miny = 1e10;
    float ret = -1;
    for (int y = 0; y < height; y++)
    {
        int row_offset = y * width;
        float* flow_row_ptr = ((float*)FlowMatF32C2.data) + 2 * row_offset;
        for (int x = 0; x < width; x++)
        {
            float fx = flow_row_ptr[2 * x + 0];
            float fy = flow_row_ptr[2 * x + 1];
            if (IsUnknownFlow(fx, fy)) { continue; }
            maxx = std::max(maxx, fx);
            maxy = std::max(maxy, fy);
            minx = std::min(minx, fx);
            miny = std::min(miny, fy);
            float rad = sqrt(fx * fx + fy * fy);
            ret = std::max(ret, rad);
        }
    }

    if (IsVerbose)
    {
        printf("max motion: %.4f  motion range: u = %.3f .. %.3f;  v = %.3f .. %.3f\n", ret, minx, maxx, miny, maxy);
    }

    // if flow == 0 everywhere
    if (ret == 0)
    {
        printf("ret == 0.  set ret to 1.\n");
        ret = 1;
    }
    return ret;
}

void FlowImage::GetColorFlowImage(cv::Mat& color_flow_image, float flow_l2_distance_max)
{
    if (flow_l2_distance_max <= 0) { throw CError("flow_l2_distance_max must be larger than 0"); }
    int width = FlowMatF32C2.cols;
    int height = FlowMatF32C2.rows;
    if (width <= 0 || height <= 0) { throw CError("FlowMatF32C2 is invalid"); }
    if (color_flow_image.rows != FlowMatF32C2.rows || color_flow_image.cols != FlowMatF32C2.cols)
    {
        color_flow_image = cv::Mat(height, width, CV_8UC3);
    }

    for (int y = 0; y < height; y++)
    {
        int row_offset = y * width;
        float* flow_row_ptr = ((float*)FlowMatF32C2.data) + 2 * row_offset;
        uchar* rgb_row_ptr = color_flow_image.data + 3 * row_offset;
        for (int x = 0; x < width; x++)
        {
            float fx = flow_row_ptr[2 * x + 0];
            float fy = flow_row_ptr[2 * x + 1];
            uchar* pix = rgb_row_ptr + 3 * x;
            if (IsUnknownFlow(fx, fy)) { pix[0] = pix[1] = pix[2] = 0; }
            else
            {
                GetColor(fx / flow_l2_distance_max, fy / flow_l2_distance_max, pix);
            }
        }
    }
}
