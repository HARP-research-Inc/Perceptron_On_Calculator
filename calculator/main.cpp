#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "perceptron.h"

using namespace std;

// Function to load weights from a text file into vector<float>
vector<float> load_weights(const string& filename) {
    vector<float> weights;
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        float w;
        while (iss >> w) {  // In case multiple floats per line
            weights.push_back(w);
        }
    }
    return weights;
}

// Function to load bias (single float) from a file
float load_bias(const string& filename) {
    ifstream infile(filename);
    float bias = 0.0f;
    if (infile >> bias) {
        return bias;
    } else {
        cerr << "Error loading bias from " << filename << endl;
        return 0.0f;
    }
}

std::vector<float> load_raw_image(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return {};
    }
    std::vector<float> data(28 * 28);
    file.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(float));
    if (!file) {
        std::cerr << "Error reading file: " << filename << std::endl;
        return {};
    }
    return data;
}

int main() {
    vector<float> weights = load_weights("weights_layer1.txt");
    float bias = load_bias("biases_layer1.txt");

    Perceptron perceptron(weights, bias);

    vector<float> sample_input = load_raw_image("as/a_image.bin");

    if (sample_input.empty()) {
        cerr << "Failed to load image data." << endl;
        return -1;
    }

    if (sample_input.size() != weights.size()) {
        cerr << "Input size and weights size mismatch!" << endl;
        return -1;
    }

    int prediction = perceptron.Predict(sample_input);

    cout << "Prediction: " << prediction << endl;

    return 0;
}

