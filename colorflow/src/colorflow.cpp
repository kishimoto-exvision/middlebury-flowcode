#include <stdio.h>
#include <math.h>
#include <string>
#include "FlowImage.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    try
    {
        FlowImage flow_image;
        flow_image.IsVerbose = true;
        int argn = 1;
        if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'c') {
            argn++;
            float trueRange = (argn < argc) ? atof(argv[argn++]) : 10;
            int imageSize = (argn < argc) ? atoi(argv[argn++]) : 151;
            char outputPngImageFilename[256] = { 0 };

            {
                cv::Mat test_image;
                sprintf(outputPngImageFilename, "colortest_truerange=%f_size=%d.png", trueRange, imageSize);
                flow_image.GetReferenceImage(test_image, trueRange, imageSize);
                cv::imwrite(outputPngImageFilename, test_image);
            }
        }
        else
        {
            if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'q') {
                argn++;
            }
            if (argn + 1 <= argc && argc <= argn + 3) {
                char *flowname = argv[argn++];
                std::string flowfilePath(flowname);
                string from("\\"), to("/");
                for (std::string::size_type pos = 0; (pos = flowfilePath.find(from, pos)) != std::string::npos; pos += to.length())
                    flowfilePath.replace(pos, from.length(), to);
                auto&& path_i = flowfilePath.find_last_of("/");
                auto&& ext_i = flowfilePath.find_last_of(".");
                string filenameWithoutExtension = flowfilePath.substr(path_i + 1, ext_i - path_i - 1);
                string outputPngFilename = (argn < argc) ? string(argv[argn]) : (filenameWithoutExtension + ".png");
                argn++;
                float maxmotion = (argn < argc) ? atof(argv[argn]) : -1;
                argn++;

                {
                    flow_image.ReadFromFloFile(flowname);
                    cv::Mat outim;
                    flow_image.GetColorFlowImage(outim, maxmotion);
                    cv::imwrite(outputPngFilename.c_str(), outim);
                }
            }
            else
            {
                const char *usage = "\n"
                    "  usage: colorflow [-quiet] in.flo [out.png] [maxmotion]\n"
                    "     or: colorflow -colortest\n";
                throw CError(usage);
            }
        }
    }
    catch (CError &err) {
        fprintf(stderr, err.message);
        fprintf(stderr, "\n");
        return -1;
    }

    return 0;
}
