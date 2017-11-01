#ifndef _FLOW_IMAGE_H_
#define _FLOW_IMAGE_H_

// Original code is Middlebury's flowcode

#include <opencv2/core.hpp>
#include <string>
#include <cmath>
#include <exception>

struct CError : public std::exception
{
public:
    CError(const char* msg) { strcpy(message, msg); }
    CError(const char* fmt, int d) { sprintf(message, fmt, d); }
    CError(const char* fmt, float f) { sprintf(message, fmt, f); }
    CError(const char* fmt, const char *s) { sprintf(message, fmt, s); }
    CError(const char* fmt, const char *s, int d) { sprintf(message, fmt, s, d); }
    char message[1024];         // longest allowable message
};

class FlowImage
{
public:
    // the "official" threshold - if the absolute value of either 
    // flow component is greater, it's considered unknown
    static float UnknownFlowThresh() { return 1e9; }

    // value to use to represent unknown flow
    static float UnknownFlow() { return 1e10; }

    static const int ColorsCountMax = 60;

    // return whether flow vector is unknown
    static bool IsUnknownFlow(float u, float v)
    {
        return (fabs(u) > UnknownFlow())
            || (fabs(v) > UnknownFlow())
            || isnan(u) || isnan(v);
    }
    static bool IsUnknownFlow(float *f) { return IsUnknownFlow(f[0], f[1]); }

    cv::Mat FlowMatF32C2;
    int width() { return FlowMatF32C2.cols; }
    int height() { return FlowMatF32C2.rows; }

    FlowImage()
    {
        IsVerbose = false;
        ColorsCount = 0;
    }

    void ReadFromFloFile(const char* file_path);
    void WriteToFloFile(const char* file_path);

    bool IsVerbose;
    int ColorWheel[ColorsCountMax][3];
    int ColorsCount;

    void SetColorWheelColor(int r, int g, int b, int k);
    void MakeColorWheel();
    void GetColor(float fx, float fy, unsigned char *pix);

    void GetReferenceImage(cv::Mat& reference_image, float flow_l2_distance_max, int image_one_side_length);
    float GetFlowL2DistanceMaximum();
    void GetColorFlowImage(cv::Mat& color_flow_image, float flow_l2_distance_max);
};

#endif
