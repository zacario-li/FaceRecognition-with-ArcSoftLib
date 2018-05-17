#include "common.h"
#include "faces.h"

int main(int argc, char* argv[]) {
    int ret = 0;
    face_idx_score result;
    //1st create a class instance
    ArcsoftFace face_instance;
    //2nd start engine
    face_instance.Start();
    //3rd create a folder, put header imags in, then build the face features
    ret = face_instance.BuildFeatureList("./img");
    /*
    4th use 'test.jpg' for example to get the face results
    {idx:index in the <this->fileNamesVec>;
    score:0.0 ~ 1.0, the bigger score we get the more similar it is}
    */
    if(!ret)
    {
        Mat tt = imread("test.jpg");
        result = face_instance.GetFaceIDScore(tt);
    }
    //5th release the engine
    face_instance.Stop();
    return 0;
}
