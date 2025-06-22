#include <vector>
#include <iostream>
#include <fstream>
#include "perceptron.h"
using namespace std;Perceptron::Perceptron(vector<float> iWeights,float iBias){weights=iWeights;bias=iBias;}int Perceptron::Predict(vector<float>x){if(x.size()!=weights.size()){return -1;}float linear_output=0.0;for(unsigned int i=0;i<x.size();++i){linear_output+=x[i]*weights[i];}linear_output+=bias;int prediction=(linear_output>0)?1:0;return prediction;}