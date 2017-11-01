#include <iostream>
#include <string>
#include "FlowImage.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace std;

int main(int argc, char *argv[])
{
    if (false)
    {
        char* argv_debug[] = { "", "-c" };
        argc = sizeof(argv_debug) / sizeof(char*);
        argv = argv_debug;
    }
    else if (false)
    {
        char* argv_debug[] = { "", "-quiet", "../../examples/lmb-freiburg_flownet2_result.flo" };
        argc = sizeof(argv_debug) / sizeof(char*);
        argv = argv_debug;
    }

    try
    {
        FlowImage flow_image;
        flow_image.IsVerbose = true;

        int argn = 1;
        if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'c') {
            argn++;
            float flow_l2_distance_max = (argn < argc) ? atof(argv[argn++]) : 10;
            int image_size = (argn < argc) ? atoi(argv[argn++]) : 151;
            char reference_image_path[256] = { 0 };
            sprintf(reference_image_path, "colortest_range=%f_size=%d.png", flow_l2_distance_max, image_size);
            cout << flow_l2_distance_max << endl;
            cout << image_size << endl;
            cout << reference_image_path << endl;
            {
                cv::Mat reference_image;
                flow_image.GetReferenceImage(reference_image, flow_l2_distance_max, image_size);
                cv::imwrite(reference_image_path, reference_image);
            }
        }
        else
        {
            if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'q')
            {
                flow_image.IsVerbose = false;
                argn++;
            }
            if (argn + 1 <= argc && argc <= argn + 3)
            {
                std::string flo_file_path(argv[argn++]);
                string from("\\"), to("/");
                for (std::string::size_type pos = 0; (pos = flo_file_path.find(from, pos)) != std::string::npos; pos += to.length())
                    flo_file_path.replace(pos, from.length(), to);
                auto&& path_i = flo_file_path.find_last_of("/");
                auto&& ext_i = flo_file_path.find_last_of(".");
                string color_flow_image_png_file_name_without_ext = flo_file_path.substr(path_i + 1, ext_i - path_i - 1);
                string color_flow_image_png_file_path = (argn < argc) ? string(argv[argn]) : (color_flow_image_png_file_name_without_ext + ".png");
                argn++;
                float flow_l2_distance_max = (argn < argc) ? atof(argv[argn]) : -1;
                argn++;

                {
                    flow_image.ReadFromFloFile(flo_file_path.c_str());
                    if (flow_l2_distance_max < 0)
                    {
                        flow_l2_distance_max = flow_image.GetFlowL2DistanceMaximum();
                    }
                    cv::Mat color_flow_image;
                    flow_image.GetColorFlowImage(color_flow_image, flow_l2_distance_max);
                    cv::imwrite(color_flow_image_png_file_path.c_str(), color_flow_image);
                }
            }
            else
            {
                const char *usage = "\n"
                    "  usage: colorflow [-quiet] in.flo [out.png] [flow_l2_distance_max]\n"
                    "     or: colorflow -colortest [flow_l2_distance_max] [image_size] \n";
                throw CError(usage);
            }
        }
    }
    catch (CError &err)
    {
        cerr << err.message << endl;
        return -1;
    }

    return 0;
}
