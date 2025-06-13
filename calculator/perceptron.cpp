#include <vector>
#include "perceptron.h"

using namespace std;

Perceptron::Perceptron(vector<float> iWeights, float iBias) {
  weights = iWeights;
  bias = iBias;
}

int Perceptron::Predict(vector<float> x) {
  float linear_output = 0.0;
  for (unsigned int i = 0; i < x.size(); ++i) {
    linear_output += x[i] * weights[i];
  }

  linear_output += bias;

  if(linear_output > 0){
    return 1;
  }

  return 0;
}
