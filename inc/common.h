#ifndef COMMON_H
#define COMMON_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <fstream>


#include "arcsoft_fsdk_face_detection.h"
#include "arcsoft_fsdk_face_recognition.h"
#include "merror.h"
#include "CTimer.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#define APPID       "77q3jsEg2wWibxjArQnpcmu915R1mn86FMWLruLLjqEx"
//face detection
#define SDKKEY_FD   "8sPbpWZLXjLRAd1dqtDNbvi4sostBmXbRtNoLu3oFDpa"
//face recognition
#define SDKKEY_FR   "8sPbpWZLXjLRAd1dqtDNbviZXQvYjLGKZes7abLSPoTf"

using namespace cv;
using namespace std;

#define INPUT_IMAGE_FORMAT  ASVL_PAF_I420

#define WORKBUF_SIZE        (40*1024*1024)
#define MAX_FACE_NUM        (50)

#define TYPE_MAT 0
#define TYPE_ASVL 1

typedef struct
{
    int idx = 0;
    float score = 0.0f;
}face_idx_score;
typedef struct
{
    int top = 0;
    int left = 0;
    int bottom = 0;
    int right = 0;
}face_xy;
/***************************************************************************************
 * function define
***************************************************************************************/
bool load_faces_imgs(vector<Mat>& imgVec,int imgCount, String path);
bool convert_mat_to_asvl(vector<Mat>& srcVec, vector<ASVLOFFSCREEN>& targetVec);
int arcsoft_engine_start(vector<MHandle>& mHandles, vector<MByte*>& mWorkMems);
int arcsoft_engine_stop(vector<MHandle>& mHandles, vector<MByte*>& mWorkMems);
vector<string> list_Target_File(string fileFolderPath, string fileExt);
bool format_Img_Files(string path, vector<string> filenames);
int arcsoft_get_face_info(MHandle h,ASVLOFFSCREEN* pImg,LPAFD_FSDK_FACERES* pResult);
int get_the_biggest_face_info(LPAFD_FSDK_FACERES& pResult);
int get_face_feature(vector<MHandle>& mHandles,ASVLOFFSCREEN* pImg,AFR_FSDK_FACEINPUT* pXy,AFR_FSDK_FACEMODEL* pFm);

//internal funcs
bool _format_Img_Files(string path, vector<string> filenames);

#endif