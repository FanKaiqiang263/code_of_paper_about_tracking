/*
    - c++ wrapper for the piotr toolbox
    Created by Tomas Vojir, 2014
*/


#ifndef FHOG_HEADER_7813784354687
#define FHOG_HEADER_7813784354687

#include <vector>
#include <opencv2/opencv.hpp>

#include "gradientMex.h"


class FHoG
{
public:
    //description: extract hist. of gradients(use_hog == 0), hog(use_hog == 1) or fhog(use_hog == 2)
    //input: float one channel image as input, hog type    //输入是单通道的图像
    //return: computed descriptor
    std::vector<cv::Mat> extract(const cv::Mat & img, int use_hog = 2, int bin_size = 4, int n_orients = 9, int soft_bin = -1, float clip = 0.2)
    {
        // d image dimension -> gray image d = 1
        // h, w -> height, width of image
        // full -> ??
        // I -> input image, M, O -> mag, orientation OUTPUT
        int h = img.rows, w = img.cols,d = 1;
        bool full = true;
        if (h < 2 || w < 2) {
            std::cerr << "I must be at least 2x2." << std::endl;
            return std::vector<cv::Mat>();
        }
        
        float * I = new float[h*w];
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                I[x*h + y] = img.at<float>(y, x);
            }
        }      //得到每个点的像素值存入数组

        float *M = new float[h*w], *O = new float[h*w];
        gradMag(I, M, O, h, w, d, full);
		//这个算出每个点幅值的大小（M）和方向(O)

        int n_chns = (use_hog == 0) ? n_orients : (use_hog==1 ? n_orients*4 : n_orients*3+5);
		//通道数,看是否用0是9维，1是hog36维，2是fhog一共32维（最后一维是全零）
        int hb = h/bin_size, wb = w/bin_size;   //bin_size是每个cell的大小应该是

        float *H = new float[hb*wb*n_chns];  //看需要多少存储空间来存这个
        memset(H, 0, hb*wb*n_chns*sizeof(float)); //分配空间

        if (use_hog == 0) 
		{
            full = false;   //by default
            gradHist( M, O, H, h, w, bin_size, n_orients, soft_bin, full );   //这个函数是算梯度统计中方图
        } else if (use_hog == 1) {
            full = false;   //by default
            hog( M, O, H, h, w, bin_size, n_orients, soft_bin, full, clip );
        } else {
            fhog( M, O, H, h, w, bin_size, n_orients, soft_bin, clip );
        }

        //convert, assuming row-by-row-by-channel storage
        std::vector<cv::Mat> res;
        int n_res_channels = (use_hog == 2) ? n_chns-1 : n_chns;    //last channel all zeros for fhog
        res.reserve(n_res_channels);    //需要这么多通道，这个是分配空间用
        for (int i = 0; i < n_res_channels; ++i) {
            //output rows-by-rows
//            cv::Mat desc(hb, wb, CV_32F, (H+hb*wb*i));

            //output cols-by-cols
            cv::Mat desc(hb, wb, CV_32F);
            for (int x = 0; x < wb; ++x) {
                for (int y = 0; y < hb; ++y) {
                    desc.at<float>(y,x) = H[i*hb*wb + x*hb + y];
                }
            }
            res.push_back(desc.clone());
        }

        //clean
        delete [] I;
        delete [] M;
        delete [] O;
        delete [] H;

        return res;
    }
};

#endif 
//FHOG_HEADER_7813784354687