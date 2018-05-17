#include "faces.h"

ArcsoftFace::ArcsoftFace()
{
    pWorkMem_FD = (MByte *)malloc(WORKBUF_SIZE);
    pWorkMem_FR = (MByte *)malloc(WORKBUF_SIZE);
}

ArcsoftFace::~ArcsoftFace()
{
    if(nullptr != pWorkMem_FD) free(pWorkMem_FD);
    if(nullptr != pWorkMem_FR) free(pWorkMem_FR);
    for(int i = 0; i < faceModelsVec.size(); i++)
    {
        if(faceModelsVec.at(i).pbFeature == nullptr)
        {
            free(faceModelsVec.at(i).pbFeature);
            faceModelsVec.at(i).pbFeature = nullptr;
        }
    }
}

void ArcsoftFace::Start()
{
    int ret = AFD_FSDK_InitialFaceEngine(APPID, 
                                        SDKKEY_FD, 
                                        pWorkMem_FD, 
                                        WORKBUF_SIZE, 
                                        &hEngine_FD, 
                                        AFD_FSDK_OPF_0_HIGHER_EXT, 
                                        16, 
                                        MAX_FACE_NUM);
    cout<<"init fd ret:"<<ret<<endl;

    ret = AFR_FSDK_InitialEngine(APPID,
                                SDKKEY_FR,
                                pWorkMem_FR,
                                WORKBUF_SIZE,
                                &hEngine_FR);
    cout<<"init fr ret:"<<ret<<endl;
}

void ArcsoftFace::Stop()
{
    int ret = AFD_FSDK_UninitialFaceEngine(hEngine_FD);
    cout<<"uninit fd ret:"<<ret<<endl;
    ret = AFR_FSDK_UninitialEngine(hEngine_FR);
    cout<<"uninit fr ret:"<<ret<<endl;
}

int ArcsoftFace::BuildFeatureList(string path)
{
    int ret = 0;
    //1st load image from dir
    _load_faces_imgs(path);
    //2nd convert mat to asv
    bool result = _convert_mat_to_asvl(rsrcMatImgVec,rdstAsvImgVec);
    if(false == result)
    {
        return -1;
    }
    //3rd retrive face xy
    ret = _get_faces_infos(hEngine_FD,rdstAsvImgVec,rfacesResultVec);
    //4th extract features
    ret = _get_faces_features(hEngine_FR,rdstAsvImgVec,rfacesResultVec,rfaceModelsVec);

    return ret;
}

face_idx_score ArcsoftFace::GetFaceIDScore(Mat& img)
{
    MFloat score = 0.0f, max = 0.0f;
    int index = 0;
    face_idx_score result_idx_score;
    //convert mat to asv
    vector<Mat> targetMat;
    vector<Mat>& rTargetMat = targetMat;
    vector<ASVLOFFSCREEN> targetAsv;
    vector<ASVLOFFSCREEN>& rTargetAsv = targetAsv;
    targetMat.push_back(img);
    LPAFD_FSDK_FACERES tempFaceResult;
    LPAFD_FSDK_FACERES& rFaceResult = tempFaceResult;

    AFR_FSDK_FACEINPUT faceXy;
    AFR_FSDK_FACEMODEL tempLocalFaceModels = { 0 }, targetFaceModels = {0};


    _convert_mat_to_asvl(rTargetMat,rTargetAsv);
    _get_face_info(hEngine_FD,&(rTargetAsv.at(0)),&tempFaceResult);
    int idx = _get_the_biggest_face_idx(rFaceResult);

    faceXy.lOrient = AFR_FSDK_FOC_0;
    faceXy.rcFace.left = tempFaceResult->rcFace[idx].left;
    faceXy.rcFace.top = tempFaceResult->rcFace[idx].top;
    faceXy.rcFace.right = tempFaceResult->rcFace[idx].right;
    faceXy.rcFace.bottom = tempFaceResult->rcFace[idx].bottom;

    _get_face_feature(hEngine_FR,&(rTargetAsv.at(0)),&faceXy,&tempLocalFaceModels);
    targetFaceModels.lFeatureSize = tempLocalFaceModels.lFeatureSize;
    targetFaceModels.pbFeature = (MByte*)malloc(targetFaceModels.lFeatureSize);
    memcpy(targetFaceModels.pbFeature, tempLocalFaceModels.pbFeature, targetFaceModels.lFeatureSize);

    //
    for(int i=0;i<faceModelsVec.size();i++)
    {
        int ret = AFR_FSDK_FacePairMatching(hEngine_FR,&faceModelsVec.at(i),&targetFaceModels,&score);
        if(score >= max)
        {
            max = score;
            index = i;
        }
    }
    result_idx_score.idx = index;
    result_idx_score.score = max;
    free(targetFaceModels.pbFeature);
    cout<<"face score is: "<<max<<endl;
    return result_idx_score;
}

bool ArcsoftFace::_load_faces_imgs(string path)
{
    vector<string> names = _list_target_files(path);
    fileNamesVec = names;
    for(int i=0; i<names.size();i++)
    {
        Mat tempMat;
        String fullpath = path+"/"+names.at(i);
        tempMat = imread(fullpath);
        if(!tempMat.empty())
        {
            srcMatImgVec.push_back(tempMat);
        }else
        {
            cout<<"this file "<<fullpath<<" can not loaded"<<endl;
        }
    }
}
vector<string> ArcsoftFace::_list_target_files(string fileFolderPath)
{
    struct dirent *ptr;
    DIR *dir;
    string PATH = fileFolderPath;
    dir = opendir(PATH.c_str());
    vector<string> files;
    
    while( (ptr = readdir(dir)) != NULL )
    {
        if( ptr->d_name[0] == '.')
        {
            continue;
        }
        files.push_back(ptr->d_name);
    }

    for(int i = 0; i < files.size(); i++)
    {
        cout << files[i] << endl;
    }

    closedir(dir);
    return files;
}
bool ArcsoftFace::_convert_mat_to_asvl(vector<Mat>& srcVec, 
                    vector<ASVLOFFSCREEN>& targetVec)
{
    bool ret = true;
    for (int i = 0; i < srcVec.size(); i++)
    {
        Mat tmp;
        ASVLOFFSCREEN inputImg = {0};
        inputImg.u32PixelArrayFormat = INPUT_IMAGE_FORMAT;
        inputImg.i32Width = srcVec.at(i).cols;
        inputImg.i32Height = srcVec.at(i).rows;
        inputImg.ppu8Plane[0] = (MUInt8*)malloc(srcVec.at(i).cols*srcVec.at(i).rows*srcVec.at(i).channels()*sizeof(uchar));
        if(!inputImg.ppu8Plane[0])
        {
            cout<<"convert_mat_to_asvl malloc fail"<<endl;
            ret = false;
        }
        try{
            cvtColor(srcVec.at(i),tmp,CV_BGR2YUV_I420);
        }catch(...){
            cout<<"convert error, double check the file:"<<fileNamesVec.at(i)<<endl;
            ret = false;
            goto CVT_MAT_2_ASVL;
        }
        memcpy(inputImg.ppu8Plane[0],tmp.data,tmp.cols*tmp.rows*tmp.channels()*sizeof(uchar));

        if (ASVL_PAF_I420 == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width;
            inputImg.pi32Pitch[1] = inputImg.i32Width/2;
            inputImg.pi32Pitch[2] = inputImg.i32Width/2;
            inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
            inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height/2;
        } else if (ASVL_PAF_NV12 == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width;
            inputImg.pi32Pitch[1] = inputImg.i32Width;
            inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
        } else if (ASVL_PAF_NV21 == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width;
            inputImg.pi32Pitch[1] = inputImg.i32Width;
            inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
        } else if (ASVL_PAF_YUYV == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width*2;
        } else if (ASVL_PAF_I422H == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width;
            inputImg.pi32Pitch[1] = inputImg.i32Width / 2;
            inputImg.pi32Pitch[2] = inputImg.i32Width / 2;
            inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + inputImg.pi32Pitch[0] * inputImg.i32Height;
            inputImg.ppu8Plane[2] = inputImg.ppu8Plane[1] + inputImg.pi32Pitch[1] * inputImg.i32Height;
        } else if (ASVL_PAF_LPI422H == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width;
            inputImg.pi32Pitch[1] = inputImg.i32Width;
            inputImg.ppu8Plane[1] = inputImg.ppu8Plane[0] + (inputImg.pi32Pitch[0] * inputImg.i32Height);
        } else if (ASVL_PAF_RGB24_B8G8R8 == inputImg.u32PixelArrayFormat) {
            inputImg.pi32Pitch[0] = inputImg.i32Width*3;
        } else {
            cout<<"convert error"<<endl;
            ret = false;
            goto CVT_MAT_2_ASVL;
        }
        targetVec.push_back(inputImg);
    }
    CVT_MAT_2_ASVL:
        return ret;
}
int ArcsoftFace::_get_face_info(MHandle h,ASVLOFFSCREEN* pImg,LPAFD_FSDK_FACERES* pResult)
{
    int ret = AFD_FSDK_StillImageFaceDetection(h,pImg,pResult);
    return ret;
}
int ArcsoftFace::_get_faces_infos(MHandle h,
                                vector<ASVLOFFSCREEN>& imgs,
                                vector<LPAFD_FSDK_FACERES>& results)
{
    int ret = 0;
    for(int i=0; i<imgs.size(); i++)
    {
        LPAFD_FSDK_FACERES faceResult;
        ret = _get_face_info(h,&(imgs.at(i)),&faceResult);
        results.push_back(faceResult);
    }
    return ret;
}
int ArcsoftFace::_get_face_feature(MHandle h,
                                ASVLOFFSCREEN* pImg,
                                AFR_FSDK_FACEINPUT* pXy,
                                AFR_FSDK_FACEMODEL* pFm)
{
    int ret = AFR_FSDK_ExtractFRFeature(h,pImg,pXy,pFm);
    return ret;
}
int ArcsoftFace::_get_faces_features(MHandle h,
                                vector<ASVLOFFSCREEN>& imgVec,
                                vector<LPAFD_FSDK_FACERES>& results,
                                vector<AFR_FSDK_FACEMODEL>& faceMVec)
{
    int ret = 0;
    for(int i=0; i < imgVec.size(); i++)
    {
        int idx = _get_the_biggest_face_idx(results.at(i));
        AFR_FSDK_FACEINPUT faceXy;
        AFR_FSDK_FACEMODEL LocalFaceModels = { 0 }, targetFaceModels = {0};

        faceXy.lOrient = AFR_FSDK_FOC_0;
        faceXy.rcFace.left = results.at(i)->rcFace[idx].left;
        faceXy.rcFace.top = results.at(i)->rcFace[idx].top;
        faceXy.rcFace.right = results.at(i)->rcFace[idx].right;
        faceXy.rcFace.bottom = results.at(i)->rcFace[idx].bottom;

        ret = _get_face_feature(h,&(imgVec.at(i)),&faceXy,&LocalFaceModels);
        targetFaceModels.lFeatureSize = LocalFaceModels.lFeatureSize;
        if(0 == ret){
            targetFaceModels.pbFeature = (MByte*)malloc(targetFaceModels.lFeatureSize);
            memcpy(targetFaceModels.pbFeature,LocalFaceModels.pbFeature,targetFaceModels.lFeatureSize);
        }else{
            targetFaceModels.pbFeature = nullptr;
            cout<<"get face feature failed, set feature to NULL"<<endl;
        }
        faceMVec.push_back(targetFaceModels);
    }
    return ret;
}
int ArcsoftFace::_get_the_biggest_face_idx(LPAFD_FSDK_FACERES& pResult)
{
    int max = 0, index = 0;
    for(int i = 0; i < pResult->nFace; i++)
    {
        int tmpV = abs(pResult->rcFace[i].left - pResult->rcFace[i].right);
        if(tmpV >= max)
        {
            max = tmpV;
            index = i;
        }
    }
    return index;
}
