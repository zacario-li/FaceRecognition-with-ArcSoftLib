#include "common.h"
#include "faces.h"
#include "CTimer.h"
#include "csvwriter.h"

int main(int argc, char* argv[]) {
    int ret = 0;
    face_idx_score result;
    //1st create a class instance
    ArcsoftFace face_instance;
    //2nd start engine
    face_instance.Start();
    //3rd create a folder, put header imags in, then build the face features
    ret = face_instance.BuildFeatureList("./img");

    //4th load testimg folder imgs to get result
    vector<string> testlist = face_instance.list_target_files("./testimg");
    vector<face_idx_score> testresult;
    for(int i=0; i<testlist.size();i++)
    {
        Mat tt = imread("./testimg/"+testlist.at(i));
        face_idx_score tis = face_instance.GetFaceIDScore(tt);
        testresult.push_back(tis);
    }
    //5 write the result to csv file
    CSVWriter csv;
    csv.enableAutoNewRow(3);
    csv << "被测图像名称" << "识别结果"<<"得分";
    for(int i =0; i<testresult.size(); i++)
    {
        string fname = testlist.at(i);
        int idx = testresult.at(i).idx;
        string resultfname = face_instance.GetFileNameVec().at(idx);
        float sco = testresult.at(i).score;
        csv << fname << resultfname << sco;
    }
    csv.writeToFile("result.csv",true);
    //6th release the engine
    face_instance.Stop();
    return 0;
}
