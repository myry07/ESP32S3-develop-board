#include <my_face_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define CAMERA_MODEL_ESP_EYE // Has PSRAM

// 摄像头引脚配置
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5

#define Y2_GPIO_NUM      11
#define Y3_GPIO_NUM      9
#define Y4_GPIO_NUM      8
#define Y5_GPIO_NUM      10
#define Y6_GPIO_NUM      12
#define Y7_GPIO_NUM      18
#define Y8_GPIO_NUM      17
#define Y9_GPIO_NUM      16

#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13

// 常量定义
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS   320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS   240
#define EI_CAMERA_FRAME_BYTE_SIZE         3

// 全局变量
static bool debug_nn = false; // 设置为 true 以查看调试信息
static bool is_initialised = false;
uint8_t *snapshot_buf = nullptr; // 指向捕获的图像数据
SemaphoreHandle_t camera_mutex = nullptr; // 用于保护摄像头资源的互斥锁

// 摄像头配置
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000, // 20MHz
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // JPEG 格式
    .frame_size = FRAMESIZE_QVGA,   // QVGA (320x240)
    .jpeg_quality = 12,             // 图像质量 (0-63, 值越小质量越高)
    .fb_count = 1,                  // 帧缓冲区数量
    .fb_location = CAMERA_FB_IN_PSRAM, // 使用 PSRAM 存储帧缓冲区
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// 函数声明
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

// 检测任务
void detectionTask(void *param) {
    while (true) {
        // 等待 5 毫秒
        if (ei_sleep(5) != EI_IMPULSE_OK) {
            continue;
        }

        // 分配内存（使用 PSRAM）
        snapshot_buf = (uint8_t*)heap_caps_malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE, MALLOC_CAP_SPIRAM);
        if (snapshot_buf == nullptr) {
            ei_printf("ERR: Failed to allocate snapshot buffer in PSRAM!\n");
            continue;
        }

        // 获取摄像头资源锁
        if (xSemaphoreTake(camera_mutex, portMAX_DELAY) == pdTRUE) {
            // 捕获图像
            if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
                ei_printf("Failed to capture image\r\n");
                heap_caps_free(snapshot_buf);
                xSemaphoreGive(camera_mutex);
                continue;
            }

            // 释放摄像头资源锁
            xSemaphoreGive(camera_mutex);
        }

        // 准备信号数据
        ei::signal_t signal;
        signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
        signal.get_data = &ei_camera_get_data;

        // 运行分类器
        ei_impulse_result_t result = { 0 };
        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
        if (err != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", err);
            heap_caps_free(snapshot_buf);
            continue;
        }

        // 打印预测结果
        ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                  result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
        ei_printf("Object detection bounding boxes:\r\n");
        for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
            ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
            if (bb.value == 0) continue;
            ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                      bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        }
#else
        ei_printf("Predictions:\r\n");
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            ei_printf("  %s: %.5f\r\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY
        ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

        // 释放内存
        heap_caps_free(snapshot_buf);
    }
}

// 初始化函数
void setup() {
    Serial.begin(115200);
    while (!Serial); // 等待串口连接
    Serial.println("Edge Impulse Inferencing Demo");

    // 初始化摄像头
    if (ei_camera_init() == false) {
        ei_printf("Failed to initialize Camera!\r\n");
    } else {
        ei_printf("Camera initialized\r\n");
    }

    // 创建互斥锁
    camera_mutex = xSemaphoreCreateMutex();
    if (camera_mutex == nullptr) {
        ei_printf("ERR: Failed to create mutex!\n");
        return;
    }

    // 启动检测任务
    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(2000);
    xTaskCreate(detectionTask, "detection", 1024 * 32, NULL, 1, NULL); // 增加任务堆栈大小
}

// 主循环
void loop() {
    // 空循环
}

// 摄像头初始化
bool ei_camera_init(void) {
    if (is_initialised) return true;

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // 初始化摄像头
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    // 配置摄像头传感器
    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // 垂直翻转
        s->set_brightness(s, 1); // 增加亮度
        s->set_saturation(s, 0); // 降低饱和度
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    s->set_awb_gain(s, 1);
#endif

    is_initialised = true;
    return true;
}

// 摄像头反初始化
void ei_camera_deinit(void) {
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        ei_printf("Camera deinit failed\n");
    }
    is_initialised = false;
}

// 捕获图像
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    if (!is_initialised) {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    // 获取帧缓冲区
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

    // 将 JPEG 转换为 RGB888
    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, out_buf);
    esp_camera_fb_return(fb);

    if (!converted) {
        ei_printf("Conversion failed\n");
        return false;
    }

    // 调整图像大小（如果需要）
    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        ei::image::processing::crop_and_interpolate_rgb888(
            out_buf, EI_CAMERA_RAW_FRAME_BUFFER_COLS, EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            out_buf, img_width, img_height);
    }

    return true;
}

// 获取图像数据
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // 将 BGR 转换为 RGB
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif