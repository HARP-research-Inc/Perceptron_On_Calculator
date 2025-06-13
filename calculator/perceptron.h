#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include <vector>
using namespace std;

class Perceptron {
public:
  Perceptron(vector<float> iWeights, float iBias);
  int Predict(vector<float> x);  

private:
  vector<float> weights;
  float bias;
};

#endif
