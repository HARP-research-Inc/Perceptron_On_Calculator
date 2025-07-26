#include <os.h>
#include <libndls.h>
#include <vector>
#include <iostream>
#include <sstream>
#include "perceptron.h"
#include "weights_layer1.h"
#include "biases_layer1.h"

using namespace std;

// Screen buffer - we'll draw to this and blit it to screen
static unsigned short screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
// Temporary buffer for display (includes cursor)
static unsigned short display_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

// Simple function to set a pixel in our buffer
void setPixel(unsigned short* buffer, int x, int y, unsigned short color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        buffer[y * SCREEN_WIDTH + x] = color;
    }
}

// Clear screen buffer
void clearScreen(unsigned short* buffer, unsigned short color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        buffer[i] = color;
    }
}

// Copy one buffer to another
void copyBuffer(unsigned short* dest, unsigned short* src) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        dest[i] = src[i];
    }
}

// Simple line drawing using Bresenham's algorithm with thickness
void drawLine(unsigned short* buffer, int x0, int y0, int x1, int y1, unsigned short color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        // Draw a thicker line by drawing multiple pixels around the main point
        for(int dy_offset = -2; dy_offset <= 2; dy_offset++) {
            for(int dx_offset = -2; dx_offset <= 2; dx_offset++) {
                setPixel(buffer, x0 + dx_offset, y0 + dy_offset, color);
            }
        }
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

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
    float bias = 0.0f;
    iss >> bias;
    return bias;
}

// Convert screen buffer to feature vector for perceptron
vector<float> convertScreenToFeatures(unsigned short* buffer) {
    vector<float> features;
    
    // Convert to binary and downsample using area averaging (320x240 -> 28x28 = 784 features)
    const int target_width = 28;
    const int target_height = 28;
    
    // Calculate how many original pixels map to each target pixel
    float x_scale = (float)SCREEN_WIDTH / target_width;
    float y_scale = (float)SCREEN_HEIGHT / target_height;
    
    for (int ty = 0; ty < target_height; ty++) {
        for (int tx = 0; tx < target_width; tx++) {
            // Calculate the area in the original image this target pixel represents
            int start_x = (int)(tx * x_scale);
            int end_x = (int)((tx + 1) * x_scale);
            int start_y = (int)(ty * y_scale);
            int end_y = (int)((ty + 1) * y_scale);
            
            // Count white pixels in this area
            int white_pixels = 0;
            int total_pixels = 0;
            
            for (int y = start_y; y < end_y && y < SCREEN_HEIGHT; y++) {
                for (int x = start_x; x < end_x && x < SCREEN_WIDTH; x++) {
                    unsigned short pixel = buffer[y * SCREEN_WIDTH + x];
                    if (pixel != 0x0000) {  // Not black
                        white_pixels++;
                    }
                    total_pixels++;
                }
            }
            
            // Calculate the ratio of white pixels in this area
            float feature = (total_pixels > 0) ? (float)white_pixels / total_pixels : 0.0f;
            
            // Convert to binary: if more than 25% of the area is white, consider it drawn
            feature = (feature > 0.25f) ? 1.0f : 0.0f;
            
            features.push_back(feature);
        }
    }
    
    return features;
}

// Simple text display function (placeholder - not used in main functionality)
void displayText(const char* text, int /*x*/, int /*y*/, unsigned short /*color*/) {
    // This is a placeholder - you'll need to use the actual text display function
    // available in your TI-Nspire SDK. Common options might be:
    // - draw_string_small() or similar
    // - or implement a simple bitmap font
    
    // For now, we'll just print to console (if available)
    printf("%s\n", text);
}

int main(void) {
    // Colors (16-bit RGB565 format)
    const unsigned short COLOR_WHITE = 0xFFFF;
    const unsigned short COLOR_BLACK = 0x0000;
    const unsigned short COLOR_GREEN = 0x07E0;
    const unsigned short COLOR_RED = 0xF800;
    const unsigned short COLOR_BLUE = 0x001F;
    const unsigned short COLOR_YELLOW = 0xFFE0;
    
    // Initialize perceptron
    vector<float> weights = load_weights_from_data();
    float bias = load_bias_from_data();
    printf("Loaded %d weights and bias: %f\n", (int)weights.size(), bias);
    Perceptron perceptron(weights, bias);
    
    // Clear the screen buffer
    clearScreen(screen_buffer, COLOR_BLACK);
    
    // Drawing variables
    int x = 160, y = 120; // Start in center
    int prevX = x, prevY = y;
    int drawing = 0;
    
    // Prediction display variables
    char last_prediction = '?';
    int show_prediction = 0;
    int prediction_timer = 0;
    
    // Main loop
    while (1) {
        // Store previous position
        prevX = x;
        prevY = y;
        
        // Check for key presses
        if (isKeyPressed(KEY_NSPIRE_ESC)) {
            break;
        }
        
        // Arrow key movement
        if (isKeyPressed(KEY_NSPIRE_UP) && y > 0) {
            y--;
        }
        if (isKeyPressed(KEY_NSPIRE_DOWN) && y < SCREEN_HEIGHT - 1) {
            y++;
        }
        if (isKeyPressed(KEY_NSPIRE_LEFT) && x > 0) {
            x--;
        }
        if (isKeyPressed(KEY_NSPIRE_RIGHT) && x < SCREEN_WIDTH - 1) {
            x++;
        }
        
        // Draw line to permanent buffer when position changed
        if (x != prevX || y != prevY) {
            unsigned short line_color = drawing ? COLOR_WHITE : COLOR_BLACK;
            drawLine(screen_buffer, prevX, prevY, x, y, line_color);
        }
        
        // Space to toggle drawing mode
        if (isKeyPressed(KEY_NSPIRE_SPACE)) {
            drawing = !drawing;
            msleep(200); // Debounce
        }
        
        // Clear screen with 'c'
        if (isKeyPressed(KEY_NSPIRE_C)) {
            clearScreen(screen_buffer, COLOR_BLACK);
            show_prediction = 0;
        }
        
        // Predict with 'p' key
        if (isKeyPressed(KEY_NSPIRE_P)) {
            printf("P key pressed!\n");
            vector<float> features = convertScreenToFeatures(screen_buffer);
            printf("Features extracted: %d\n", (int)features.size());
            printf("Expected weights: %d\n", (int)weights.size());
            
            // Debug: Count non-zero features and show pattern
            int non_zero_count = 0;
            printf("Feature pattern (. = empty, X = drawn):\n");
            for(int y = 0; y < 28; y++) {
                for(int x = 0; x < 28; x++) {
                    int idx = y * 28 + x;
                    if(features[idx] > 0.0f) {
                        printf("X");
                        non_zero_count++;
                    } else {
                        printf(".");
                    }
                }
                printf("\n");
            }
            printf("Non-zero features (drawn pixels): %d\n", non_zero_count);
            
            // Check if we have the right number of features
            if (features.size() == weights.size()) {
                printf("Running prediction...\n");
                
                // Let's manually calculate what the perceptron sees
                float weighted_sum = 0.0f;
                for(int i = 0; i < (int)features.size(); i++) {
                    weighted_sum += features[i] * weights[i];
                }
                weighted_sum += bias;
                printf("Weighted sum: %f\n", weighted_sum);
                printf("Bias: %f\n", bias);
                
                int prediction = perceptron.Predict(features);
                printf("Prediction result: %d\n", prediction);
                // Try the original mapping
                last_prediction = (prediction == 0) ? 'a' : 'b';  // UN-FLIPPED
                show_prediction = 1;
                prediction_timer = 0;
                printf("Should show: %c\n", last_prediction);
            } else {
                printf("Size mismatch! Features: %d, Weights: %d\n", 
                       (int)features.size(), (int)weights.size());
            }
            msleep(200); // Debounce
        }
        
        // Copy screen buffer to display buffer
        copyBuffer(display_buffer, screen_buffer);
        
        // Draw cursor on display buffer only (not permanent)
        unsigned short cursor_color = drawing ? COLOR_GREEN : COLOR_RED;
        
        // Draw a 5x5 cross cursor
        for (int i = -2; i <= 2; i++) {
            setPixel(display_buffer, x + i, y, cursor_color);  // Horizontal line
            setPixel(display_buffer, x, y + i, cursor_color);  // Vertical line
        }
        
        // Draw prediction result
        if (show_prediction) {
            // Draw prediction in top-right corner
            unsigned short pred_color = (last_prediction == 'a') ? COLOR_BLUE : COLOR_YELLOW;
            
            // Draw a simple letter representation (you might want to improve this)
            if (last_prediction == 'a') {
                // Draw a better 'A' pattern (right-side up)
                int base_x = 289;
                int base_y = 8;
                
                // Left diagonal line
                for (int i = 0; i < 12; i++) {
                    setPixel(display_buffer, base_x - i/2, base_y + i, pred_color);
                    setPixel(display_buffer, base_x - i/2 + 1, base_y + i, pred_color);
                }
                
                // Right diagonal line
                for (int i = 0; i < 12; i++) {
                    setPixel(display_buffer, base_x + i/2, base_y + i, pred_color);
                    setPixel(display_buffer, base_x + i/2 + 1, base_y + i, pred_color);
                }
                
                // Horizontal crossbar
                for (int i = -3; i <= 3; i++) {
                    setPixel(display_buffer, base_x + i, base_y + 7, pred_color);
                    setPixel(display_buffer, base_x + i, base_y + 8, pred_color);
                }
            } else {
                // Draw a better 'B' pattern
                int base_x = 285;
                int base_y = 8;
                
                // Left vertical line
                for (int i = 0; i < 14; i++) {
                    setPixel(display_buffer, base_x, base_y + i, pred_color);
                    setPixel(display_buffer, base_x + 1, base_y + i, pred_color);
                }
                
                // Top horizontal line
                for (int i = 0; i < 8; i++) {
                    setPixel(display_buffer, base_x + i, base_y, pred_color);
                    setPixel(display_buffer, base_x + i, base_y + 1, pred_color);
                }
                
                // Middle horizontal line
                for (int i = 0; i < 7; i++) {
                    setPixel(display_buffer, base_x + i, base_y + 6, pred_color);
                    setPixel(display_buffer, base_x + i, base_y + 7, pred_color);
                }
                
                // Bottom horizontal line
                for (int i = 0; i < 8; i++) {
                    setPixel(display_buffer, base_x + i, base_y + 12, pred_color);
                    setPixel(display_buffer, base_x + i, base_y + 13, pred_color);
                }
                
                // Right edges for top bump
                for (int i = 2; i < 6; i++) {
                    setPixel(display_buffer, base_x + 7, base_y + i, pred_color);
                    setPixel(display_buffer, base_x + 8, base_y + i, pred_color);
                }
                
                // Right edges for bottom bump
                for (int i = 8; i < 12; i++) {
                    setPixel(display_buffer, base_x + 7, base_y + i, pred_color);
                    setPixel(display_buffer, base_x + 8, base_y + i, pred_color);
                }
            }
            
            prediction_timer++;
            if (prediction_timer > 100) {  // Hide after ~5 seconds
                show_prediction = 0;
            }
        }
        
        // Draw instructions in bottom-left corner
        // (You'll need to implement proper text rendering for your platform)
        
        // Blit display buffer to screen
        lcd_blit(display_buffer, SCR_320x240_565);
        
        msleep(50);
    }
    
    return 0;
}