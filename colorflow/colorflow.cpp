#include <stdio.h>
#include <math.h>
#include "imageLib.h"
#include "flowIO.h"
#include "colorcode.h"

#include <string>

int verbose = 1;

void ExportColorTestImage(float truerange, char* outname, int size)
{
    float range = 1.04 * truerange; // make picture a bit bigger to show out-of-range coding
    CShape sh(size, size, 3);
    CByteImage out(sh);

    int s2 = size / 2;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float fx = (float)x / (float)s2 * range - range;
            float fy = (float)y / (float)s2 * range - range;
            if (x == s2 || y == s2) // make black coordinate axes
                continue;
            uchar *pix = &out.Pixel(x, y, 0);
            //fx = rintf(fx);
            //fy = rintf(fy);
            computeColor(fx / truerange, fy / truerange, pix);
        }
    }
    int ir = (int)truerange;
    int ticksize = size < 120 ? 1 : 2;
    for (int k = -ir; k <= ir; k++) {
        int ik = (int)(k / range * s2) + s2;
        for (int t = -ticksize; t <= ticksize; t++) {
            uchar *pix;
            pix = &out.Pixel(ik, s2 + t, 0); pix[0] = pix[1] = pix[2] = 0;
            pix = &out.Pixel(s2 + t, ik, 0); pix[0] = pix[1] = pix[2] = 0;
        }
    }

    WriteImageVerb(out, outname, verbose);
}

void MotionToColor(CFloatImage motim, CByteImage &colim, float maxmotion)
{
    CShape sh = motim.Shape();
    int width = sh.width, height = sh.height;
    colim.ReAllocate(CShape(width, height, 3));
    int x, y;
    // determine motion range:
    float maxx = -999, maxy = -999;
    float minx = 999, miny = 999;
    float maxrad = -1;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            float fx = motim.Pixel(x, y, 0);
            float fy = motim.Pixel(x, y, 1);
            if (unknown_flow(fx, fy))
                continue;
            maxx = __max(maxx, fx);
            maxy = __max(maxy, fy);
            minx = __min(minx, fx);
            miny = __min(miny, fy);
            float rad = sqrt(fx * fx + fy * fy);
            maxrad = __max(maxrad, rad);
        }
    }
    printf("max motion: %.4f  motion range: u = %.3f .. %.3f;  v = %.3f .. %.3f\n",
        maxrad, minx, maxx, miny, maxy);


    if (maxmotion > 0) // i.e., specified on commandline
        maxrad = maxmotion;

    if (maxrad == 0) // if flow == 0 everywhere
        maxrad = 1;

    if (verbose)
        fprintf(stderr, "normalizing by %g\n", maxrad);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            float fx = motim.Pixel(x, y, 0);
            float fy = motim.Pixel(x, y, 1);
            uchar *pix = &colim.Pixel(x, y, 0);
            if (unknown_flow(fx, fy)) {
                pix[0] = pix[1] = pix[2] = 0;
            }
            else {
                computeColor(fx / maxrad, fy / maxrad, pix);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    try
    {
        int argn = 1;
        if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'c') {
            argn++;
            float trueRange = (argn < argc) ? atof(argv[argn++]) : 10;
            int imageSize = (argn < argc) ? atoi(argv[argn++]) : 151;
            char outputPngImageFilename[256] = { 0 };
            sprintf(outputPngImageFilename, "colortest_truerange=%f_size=%d.png", trueRange, imageSize);
            ExportColorTestImage(trueRange, outputPngImageFilename, imageSize);
        }
        else
        {
            if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'q') {
                verbose = 0;
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
                    CFloatImage im, fband;
                    ReadFlowFile(im, flowname);
                    CByteImage band, outim;
                    CShape sh = im.Shape();
                    sh.nBands = 3;
                    outim.ReAllocate(sh);
                    outim.ClearPixels();
                    MotionToColor(im, outim, maxmotion);
                    
                    char outname[256] = { 0 };
                    strcpy(outname, outputPngFilename.c_str());
                    // causes runtime error in debug build on Windows
                    WriteImageVerb(outim, outname, verbose);
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
