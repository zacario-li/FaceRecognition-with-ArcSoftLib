#ifndef FACES_H
#define FACES_H

#include "common.h"

class ArcsoftFace 
{
    public:
        ArcsoftFace();
        ~ArcsoftFace();
        void    Start();
        void    Stop();
        int     BuildFeatureList(string path);
        face_idx_score     GetFaceIDScore(Mat& img);
        vector<string>     GetFileNameVec(){return fileNamesVec;};
        vector<string>     list_target_files(string path);

    private:
        MByte*                      pWorkMem_FD = nullptr;
        MByte*                      pWorkMem_FR = nullptr;
        MHandle                     hEngine_FD = nullptr;
        MHandle                     hEngine_FR = nullptr;
        vector<face_xy>             faceInfoVec;
        vector<face_xy>&            rFaceInfoVec = faceInfoVec;
        vector<string>              fileNamesVec;
        vector<Mat>                 srcMatImgVec;
        vector<Mat>&                rsrcMatImgVec = srcMatImgVec;
        vector<ASVLOFFSCREEN>       dstAsvImgVec;
        vector<ASVLOFFSCREEN>&      rdstAsvImgVec = dstAsvImgVec;
        vector<AFR_FSDK_FACEMODEL>  faceModelsVec;
        vector<AFR_FSDK_FACEMODEL>& rfaceModelsVec = faceModelsVec;
        vector<LPAFD_FSDK_FACERES>  facesResultVec;
        vector<LPAFD_FSDK_FACERES>& rfacesResultVec = facesResultVec;

        //funcs
        bool            _load_faces_imgs(string path);
        bool            _convert_mat_to_asvl(vector<Mat>& srcVec,
                                        vector<ASVLOFFSCREEN>& targetVec);
        int             _get_face_info(MHandle h,
                                        ASVLOFFSCREEN* pImg,
                                        LPAFD_FSDK_FACERES* pResult);
        int             _get_faces_infos(MHandle h,
                                        vector<ASVLOFFSCREEN>& imgs,
                                        vector<face_xy>& results);
        int             _get_face_feature(MHandle h,
                                        ASVLOFFSCREEN* pImg,
                                        AFR_FSDK_FACEINPUT* pXy,
                                        AFR_FSDK_FACEMODEL* pFm);
        int             _get_faces_features(MHandle h,
                                        vector<ASVLOFFSCREEN>& imgVec,
                                        vector<face_xy>& results,
                                        vector<AFR_FSDK_FACEMODEL>& faceMVec);
        int             _get_the_biggest_face_idx(LPAFD_FSDK_FACERES& pResult);
};

class DlibFace //todo
{};
#endif