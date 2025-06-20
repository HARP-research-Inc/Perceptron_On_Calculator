#include <vector>
#include <iostream>
#include <fstream>
#include "perceptron.h"

using namespace std;

Perceptron::Perceptron(vector<float> iWeights, float iBias) {
    weights = iWeights;
    bias = iBias;
}

int Perceptron::Predict(vector<float> x) {
    // Check if input size matches weights size
    if (x.size() != weights.size()) {
        cerr << "Error: Input size (" << x.size() 
             << ") doesn't match weights size (" << weights.size() << ")" << endl;
        return -1; // Error indicator
    }
    
    // Calculate linear output (dot product + bias)
    float linear_output = 0.0;
    for (unsigned int i = 0; i < x.size(); ++i) {
        linear_output += x[i] * weights[i];
    }
    linear_output += bias;
    
    // Apply step function (same as Python: x > 0 ? 1 : 0)
    int prediction = (linear_output > 0) ? 1 : 0;
    
    // Remove this line that was flipping your results!
    // int final_prediction = 1 - raw_prediction;
    cout << linear_output << endl;
    return prediction;
}