#include <os.h>
#include <libndls.h>
#include <vector>
#include <iostream>
#include <sstream>
#include "perceptron.h"
#include "weights_layer1.h"
#include "biases_layer1.h"

using namespace std;

static unsigned short screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static unsigned short display_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void setPixel(unsigned short* buffer, int x, int y, unsigned short color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        buffer[y * SCREEN_WIDTH + x] = color;
    }
}

void clearScreen(unsigned short* buffer, unsigned short color) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        buffer[i] = color;
    }
}

void copyBuffer(unsigned short* dest, unsigned short* src) {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        dest[i] = src[i];
    }
}

void drawLine(unsigned short* buffer, int x0, int y0, int x1, int y1, unsigned short color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        for (int dy_offset = -2; dy_offset <= 2; dy_offset++) {
            for (int dx_offset = -2; dx_offset <= 2; dx_offset++) {
                setPixel(buffer, x0 + dx_offset, y0 + dy_offset, color);
            }
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

vector<float> load_weights_from_data() {
    vector<float> weights;
    istringstream iss(reinterpret_cast<const char*>(weights_layer1_txt));
    float w;
    while (iss >> w) weights.push_back(w);
    return weights;
}

float load_bias_from_data() {
    istringstream iss(reinterpret_cast<const char*>(biases_layer1_txt));
    float bias = 0.0f;
    iss >> bias;
    return bias;
}

vector<float> convertScreenToFeatures(unsigned short* buffer) {
    vector<float> features;
    int min_x = SCREEN_WIDTH, max_x = -1;
    int min_y = SCREEN_HEIGHT, max_y = -1;

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (buffer[y * SCREEN_WIDTH + x] != 0x0000) {
                min_x = min(x, min_x);
                max_x = max(x, max_x);
                min_y = min(y, min_y);
                max_y = max(y, max_y);
            }
        }
    }

    if (max_x == -1) {
        for (int i = 0; i < 784; i++) features.push_back(0.0f);
        return features;
    }

    int content_width = max_x - min_x + 1;
    int content_height = max_y - min_y + 1;
    int content_size = max(content_width, content_height);
    int padding = content_size / 5;
    content_size += 2 * padding;
    int center_x = (min_x + max_x) / 2;
    int center_y = (min_y + max_y) / 2;
    int content_min_x = center_x - content_size / 2;
    int content_min_y = center_y - content_size / 2;

    const int target_width = 28;
    const int target_height = 28;

    for (int ty = 0; ty < target_height; ty++) {
        for (int tx = 0; tx < target_width; tx++) {
            int start_x = content_min_x + (tx * content_size) / target_width;
            int end_x = content_min_x + ((tx + 1) * content_size) / target_width;
            int start_y = content_min_y + (ty * content_size) / target_height;
            int end_y = content_min_y + ((ty + 1) * content_size) / target_height;

            int white_pixels = 0, total_pixels = 0;
            for (int y = start_y; y < end_y; y++) {
                for (int x = start_x; x < end_x; x++) {
                    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
                        unsigned short pixel = buffer[y * SCREEN_WIDTH + x];
                        if (pixel != 0x0000) white_pixels++;
                    }
                    total_pixels++;
                }
            }

            float feature = (total_pixels > 0) ? (float)white_pixels / total_pixels : 0.0f;
            feature = (feature > 0.25f) ? 1.0f : 0.0f;
            features.push_back(feature);
        }
    }

    return features;
}

int main(void) {
    const unsigned short COLOR_WHITE = 0xFFFF;
    const unsigned short COLOR_BLACK = 0x0000;
    const unsigned short COLOR_GREEN = 0x07E0;
    const unsigned short COLOR_RED = 0xF800;
    const unsigned short COLOR_BLUE = 0x001F;
    const unsigned short COLOR_YELLOW = 0xFFE0;

    vector<float> weights = load_weights_from_data();
    float bias = load_bias_from_data();
    Perceptron perceptron(weights, bias);
    clearScreen(screen_buffer, COLOR_BLACK);

    int x = 160, y = 120;
    int prevX = x, prevY = y;
    int drawing = 0;
    char last_prediction = '?';
    int show_prediction = 0, prediction_timer = 0;

    // Trackpad tracking variables
    static int last_tp_x = -1, last_tp_y = -1;
    static bool first_contact = true;

    while (1) {
        prevX = x;
        prevY = y;

        if (isKeyPressed(KEY_NSPIRE_ESC)) break;

        // Trackpad movement
        touchpad_report_t tp;
        touchpad_scan(&tp);

        if (tp.contact) {
            if (first_contact || last_tp_x == -1) {
                // First contact - just record position, don't move cursor
                last_tp_x = tp.x;
                last_tp_y = tp.y;
                first_contact = false;
            } else {
                // Calculate relative movement from last trackpad position
                int dx = tp.x - last_tp_x;
                int dy = tp.y - last_tp_y;
                
                // Apply movement to cursor with sensitivity scaling and inverted Y
                x += dx / 9;  // Slow down horizontal movement
                y -= dy / 9;  // Invert and slow down vertical movement
                
                // Keep cursor within screen bounds
                if (x < 0) x = 0;
                if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
                if (y < 0) y = 0;
                if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
                
                // Draw line if we're in drawing mode
                if (drawing) {
                    drawLine(screen_buffer, prevX, prevY, x, y, COLOR_WHITE);
                }
                
                // Update last trackpad position
                last_tp_x = tp.x;
                last_tp_y = tp.y;
            }
        } else {
            // No contact - reset for next touch
            first_contact = true;
            last_tp_x = -1;
            last_tp_y = -1;
        }

        if (isKeyPressed(KEY_NSPIRE_SPACE)) {
            drawing = !drawing;
            msleep(200);
        }

        if (isKeyPressed(KEY_NSPIRE_C)) {
            clearScreen(screen_buffer, COLOR_BLACK);
            show_prediction = 0;
        }

        if (isKeyPressed(KEY_NSPIRE_P)) {
            vector<float> features = convertScreenToFeatures(screen_buffer);
            if (features.size() == weights.size()) {
                int prediction = perceptron.Predict(features);
                last_prediction = (prediction == 0) ? 'a' : 'b';
                show_prediction = 1;
                prediction_timer = 0;
            }
            msleep(200);
        }

        copyBuffer(display_buffer, screen_buffer);

        unsigned short cursor_color = drawing ? COLOR_GREEN : COLOR_RED;
        for (int i = -2; i <= 2; i++) {
            setPixel(display_buffer, x + i, y, cursor_color);
            setPixel(display_buffer, x, y + i, cursor_color);
        }

        if (show_prediction) {
            unsigned short pred_color = (last_prediction == 'a') ? COLOR_BLUE : COLOR_YELLOW;

            if (last_prediction == 'a') {
                // Draw a 'A' pattern
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
                // Draw a 'B' pattern
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
            if (prediction_timer > 100) {
                show_prediction = 0;
            }
        }

        lcd_blit(display_buffer, SCR_320x240_565);
        msleep(50);
    }

    return 0;
}