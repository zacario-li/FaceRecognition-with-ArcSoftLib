#include "common.h"

bool load_faces_imgs(vector<Mat>& imgVec,
                    int imgCount, 
                    String pathPrefix)
{
    vector<string> names = list_Target_File("./img",".jpg");
    //_format_Img_Files("./img/",names);

    bool ret = true;
    for (int i =0; i < names.size(); i++)
    {
        Mat tt;
        String path = "./img/"+names.at(i);
        tt = imread(path);
        if(tt.empty())
        {
            cout<<"img empty, path is :" << path << endl;
            ret = false;
            goto LOAD_FACES_IMGS_FINAL;
        }
        imgVec.push_back(tt);
    }
    LOAD_FACES_IMGS_FINAL:
    return ret;
}

bool convert_mat_to_asvl(vector<Mat>& srcVec, 
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
            goto CVT_MAT_2_ASVL;
        }
        cvtColor(srcVec.at(i),tmp,CV_BGR2YUV_I420);
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

int arcsoft_engine_start(vector<MHandle>& mHandles, 
                    vector<MByte*>& mWorkMems)
{
    int ret = 0;
    mWorkMems.at(0) = (MByte *)malloc(WORKBUF_SIZE);
    //MByte *pWorkMem_FD = (MByte *)malloc(WORKBUF_SIZE);
    if(mWorkMems.at(0) == nullptr){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        exit(0);
    }

    ret = AFD_FSDK_InitialFaceEngine(APPID, SDKKEY_FD, mWorkMems.at(0), WORKBUF_SIZE, 
                                         &mHandles.at(0), AFD_FSDK_OPF_0_HIGHER_EXT, 16, MAX_FACE_NUM);
    if (ret != 0) {
        fprintf(stderr, "fail to AFD_FSDK_InitialFaceEngine(): 0x%x\r\n", ret);
        exit(0);
    }

    //face recog
    mWorkMems.at(1) = (MByte *)malloc(WORKBUF_SIZE);
    //MByte *pWorkMem_FR = (MByte *)malloc(WORKBUF_SIZE);
    if(mWorkMems.at(1) == nullptr){
        fprintf(stderr, "fail to malloc workbuf\r\n");
        exit(0);
    }

    ret = AFR_FSDK_InitialEngine(APPID,SDKKEY_FR,mWorkMems.at(1),WORKBUF_SIZE,
                                        &mHandles.at(1));
    if (ret != 0) {
        fprintf(stderr, "fail to AFR_FSDK_InitialEngine(): 0x%x\r\n", ret);
        exit(0);
    }

    return ret;
}

int arcsoft_engine_stop(vector<MHandle>& mHandles,
                    vector<MByte*>& mWorkMems)
{
    int ret = 0;

//free engine
    if (mHandles.at(0) != nullptr)
    {
        ret = AFD_FSDK_UninitialFaceEngine(mHandles.at(0));
    }
    if (mHandles.at(1) != nullptr)
    {
        ret = AFR_FSDK_UninitialEngine(mHandles.at(1));
    }

//free working memory
    for(int i = 0; i < mWorkMems.size(); i++)
    {
        if(mWorkMems.at(i) != nullptr)
        {
            free(mWorkMems.at(i));
            mWorkMems.at(i) = nullptr;
        }
    }

    return ret;
}

int arcsoft_get_face_info(MHandle h,
                    ASVLOFFSCREEN* pImg,
                    LPAFD_FSDK_FACERES* pResult)
{
    int ret = 0;
    ret = AFD_FSDK_StillImageFaceDetection(h,pImg,pResult);
    return ret;
}

int get_the_biggest_face_info(LPAFD_FSDK_FACERES& pResult)
{
    int ret = 0, max = 0, index = 0;
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


int get_face_feature(vector<MHandle>& mHandles,
                    ASVLOFFSCREEN* pImg,
                    AFR_FSDK_FACEINPUT* pXy,
                    AFR_FSDK_FACEMODEL* pFm)
{
    int ret = AFR_FSDK_ExtractFRFeature(mHandles.at(1),pImg,pXy,pFm);
    return ret;
}

//list dir files
bool _format_Img_Files(string path, vector<string> filenames)
{
    for(int i=0; i < filenames.size(); i++)
    {
        string fullpath = path + filenames.at(i);
        Mat tt = imread(fullpath);
        if(! tt.empty())
        {
            imwrite(fullpath,tt);
        }
    }
    return true;
}
vector<string> list_Target_File(string fileFolderPath, string fileExt)
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
//end