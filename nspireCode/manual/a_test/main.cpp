#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "perceptron.h"
#include "weights_layer1.h"
#include "biases_layer1.h"
#include "a_image.h"

using namespace std;

// Parse weights from embedded string
vector<float> load_weights_from_data() {
    vector<float> weights;
    istringstream iss(reinterpret_cast<const char*>(weights_layer1_txt));
    float w;
    while (iss >> w) {
        weights.push_back(w);
    }
    return weights;
}

// Parse bias from embedded string
float load_bias_from_data() {
    istringstream iss(reinterpret_cast<const char*>(biases_layer1_txt));
    float bias;
    if (iss >> bias) {
        return bias;
    } else {
        cerr << "Error parsing bias data" << endl;
        return 0.0f;
    }
}

// Read binary image from array
vector<float> load_raw_image_from_data() {
    size_t float_count = a_image_bin_len / sizeof(float);
    const float* float_data = reinterpret_cast<const float*>(a_image_bin);
    return vector<float>(float_data, float_data + float_count);
}

int main() {
    vector<float> weights = load_weights_from_data();
    float bias = load_bias_from_data();
    vector<float> sample_input = load_raw_image_from_data();

    if (sample_input.empty()) {
        cerr << "Failed to load image data." << endl;
        return -1;
    }

    if (sample_input.size() != weights.size()) {
        cerr << "Input size and weights size mismatch!" << endl;
        return -1;
    }

    Perceptron perceptron(weights, bias);
    int prediction = perceptron.Predict(sample_input);

    char returnVal = (prediction == 0) ? 'a' : 'b';
    cout << "Prediction: " << returnVal << endl;

    return 0;
}
