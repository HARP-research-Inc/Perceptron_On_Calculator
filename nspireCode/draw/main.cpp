#include <os.h>
#include <libndls.h>

// Screen buffer - draw to this and blit it to screen
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

// Simple line drawing using Bresenham's algorithm
void drawLine(unsigned short* buffer, int x0, int y0, int x1, int y1, unsigned short color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        setPixel(buffer, x0, y0, color);
        
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

int main(void) {
    // Colors 
    const unsigned short COLOR_WHITE = 0xFFFF;
    const unsigned short COLOR_BLACK = 0x0000;
    const unsigned short COLOR_GREEN = 0x07E0;
    const unsigned short COLOR_RED = 0xF800;
    
    // Clear the screen buffer
    clearScreen(screen_buffer, COLOR_BLACK);
    
    // Drawing variables
    int x = 160, y = 120; // Start in center
    int prevX = x, prevY = y;
    int drawing = 0;
    
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
        
        // Blit display buffer to screen
        lcd_blit(display_buffer, SCR_320x240_565);
        
        msleep(50);
    }
    
    return 0;
}