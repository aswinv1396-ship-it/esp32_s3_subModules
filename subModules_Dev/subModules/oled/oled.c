#include "oled.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "OLED_DRIVER";
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  400000     // Fast 400kHz I2C bus speeds

/* --- Basic 5x7 ASCII Minimalist Embedded Font Map Matrix --- */
static const uint8_t font_map[95][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0xfa,0x00,0x00},{0x00,0xe0,0x00,0xe0,0x00}, // Space, !, "
    {0x28,0xfe,0x28,0xfe,0x28},{0x24,0x54,0xfe,0x54,0x48},{0xc4,0xc8,0x10,0x26,0x46}, // #, $, %
    {0x6c,0x92,0xaa,0x44,0x0a},{0x00,0xa0,0xc0,0x00,0x00},{0x00,0x38,0x44,0x82,0x00}, // &, ', (
    {0x00,0x82,0x44,0x38,0x00},{0x28,0x10,0x7c,0x10,0x28},{0x10,0x10,0x7c,0x10,0x10}, // ), *, +
    {0x00,0x0a,0x0d,0x00,0x00},{0x10,0x10,0x10,0x10,0x10},{0x00,0x06,0x06,0x00,0x00}, // ,, -, .
    {0x04,0x08,0x10,0x20,0x40},{0x7c,0x8a,0x92,0xa2,0x7c},{0x00,0x42,0xfe,0x02,0x00}, // /, 0, 1
    {0x42,0x86,0x8a,0x92,0x62},{0x44,0x82,0x92,0x92,0x6c},{0x18,0x28,0x48,0xfe,0x08}, // 2, 3, 4
    {0xe4,0xa2,0xa2,0xa2,0x9c},{0x3c,0x4a,0x92,0x92,0x0c},{0x80,0x86,0x98,0x60,0x80}, // 5, 6, 7
    {0x6c,0x92,0x92,0x92,0x6c},{0x30,0x4a,0x92,0x92,0x7c},{0x00,0x6c,0x6c,0x00,0x00}, // 8, 9, :
    {0x00,0xaa,0x2c,0x00,0x00},{0x10,0x28,0x44,0x82,0x00},{0x24,0x24,0x24,0x24,0x24}, // ;, <, =
    {0x00,0x82,0x44,0x28,0x10},{0x40,0x80,0x8a,0x90,0x60},{0x42,0x42,0x7e,0x42,0x42}, // >, ?, @
    {0x7c,0x88,0x88,0x88,0x7c},{0xfe,0x92,0x92,0x92,0x6c},{0x3c,0x42,0x42,0x42,0x24}, // A, B, C
    {0xfe,0x42,0x42,0x42,0x3c},{0xfe,0x92,0x92,0x92,0x82},{0xfe,0x90,0x90,0x90,0x80}, // D, E, F
    {0x3c,0x42,0x42,0x4e,0x2e},{0xfe,0x10,0x10,0x10,0xfe},{0x00,0x42,0xfe,0x42,0x00}, // G, H, I
    {0x06,0x02,0x02,0x02,0xfc},{0xfe,0x10,0x28,0x44,0x82},{0xfe,0x02,0x02,0x02,0x02}, // J, K, L
    {0xfe,0x40,0x20,0x40,0xfe},{0xfe,0x20,0x10,0x08,0xfe},{0x3c,0x42,0x42,0x42,0x3c}, // M, N, O
    {0xfe,0x90,0x90,0x90,0x60},{0x3c,0x42,0x46,0x44,0x3a},{0xfe,0x90,0x98,0x94,0x62}, // P, Q, R
    {0x62,0x92,0x92,0x92,0x8c},{0x80,0x80,0xfe,0x80,0x80},{0xfc,0x02,0x02,0x02,0xfc}, // S, T, U
    {0xf8,0x04,0x02,0x04,0xf8},{0xfc,0x02,0x3c,0x02,0xfc},{0xc6,0x28,0x10,0x28,0xc6}, // V, W, X
    {0xc0,0x20,0x1f,0x20,0xc0},{0x46,0x4a,0x52,0x62,0x42}  // Y, Z
};

static esp_err_t oled_send_cmd(uint8_t cmd) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x00, true); // Control byte command indicator flag
    i2c_master_write_byte(link, cmd, true);
    i2c_master_stop(link);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, link, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(link);
    return ret;
}

esp_err_t oled_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_I2C_SDA_PIN,
        .scl_io_num = OLED_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) return ret;
    
    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;

    // Standard sequence layout initialization registers for SSD1306 128x64 display panels
        // Updated sequence layout initialization registers for SSD1306 128x64 display panels
    uint8_t init_cmds[] = {
        0xAE, // Turn off display screen
        0xD5, 0x80, // Set clock divide ratio
        0xA8, 0x3F, // Set multiplex ratio (1 to 64)
        0xD3, 0x00, // Set display offset shift map to zero
        0x40,       // Set start line memory address to zero

        0xA0,       // Changed from 0xA1: Set segment re-map normal direction
        0xC0,       // Changed from 0xC8: Set COM output scan normal direction
        
        0xDA, 0x12, // Set COM pins hardware configuration maps
        0x81, 0xCF, // Set Screen Contrast level adjustment threshold
        0xD9, 0xF1, // Set pre-charge period cycles
        0xDB, 0x40, // Set VCOMH Deselect level
        0xA4,       // Entire Display ON output trace
        0xA6,       // Set standard Normal/Inverse configuration visibility
        0xAF        // Power display panel explicitly ON
    };


    for (size_t i = 0; i < sizeof(init_cmds); i++) {
        oled_send_cmd(init_cmds[i]);
    }
    
    ESP_LOGI(TAG, "OLED panel framework initialized successfully.");
    oled_clear();
    return ESP_OK;
}

esp_err_t oled_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        oled_send_cmd(0xB0 + page); // Step sequence through pages
        oled_send_cmd(0x00);       // Reset lower column start address
        oled_send_cmd(0x10);       // Reset upper column start address
        
        for (uint8_t col = 0; col < 128; col++) {
            i2c_cmd_handle_t link = i2c_cmd_link_create();
            i2c_master_start(link);
            i2c_master_write_byte(link, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write_byte(link, 0x40, true); // Data mode tracking block indicator
            i2c_master_write_byte(link, 0x00, true); // Write black byte
            i2c_master_stop(link);
            i2c_master_cmd_begin(I2C_MASTER_NUM, link, pdMS_TO_TICKS(10));
            i2c_cmd_link_delete(link);
        }
    }
    return ESP_OK;
}

esp_err_t oled_write_line(uint8_t line, const char *text) {
    if (line > 7) return ESP_ERR_INVALID_ARG;
    
    oled_send_cmd(0xB0 + line); // Target specific page row channel line selection
    oled_send_cmd(0x00);       
    oled_send_cmd(0x10);       

    size_t length = strlen(text);
    for (size_t i = 0; i < length && i < 20; i++) { // Max 20 printable bounds across width
        uint8_t c = text[i] - 32; // Offset tracking against standard printable boundary index
        if (c >= 95) c = 0;       // Out-of-bounds safety fallback
        
        i2c_cmd_handle_t link = i2c_cmd_link_create();
        i2c_master_start(link);
        i2c_master_write_byte(link, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(link, 0x40, true);
        
        i2c_master_write(link, (uint8_t *)font_map[c], 5, true);
        i2c_master_write_byte(link, 0x00, true); // Print single empty 1px padding column separator space
        i2c_master_stop(link);
        i2c_master_cmd_begin(I2C_MASTER_NUM, link, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(link);
    }
    return ESP_OK;
}

