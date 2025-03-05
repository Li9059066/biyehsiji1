// 安全阈值定义
#define TEMP_MIN        -20    // 最低温度 -20℃
#define TEMP_MAX        50     // 最高温度 50℃
#define TEMP_THRESHOLD_MIN  15    // 温度阈值最小值
#define TEMP_THRESHOLD_MAX  45    // 温度阈值最大值

#define SMOKE_MIN       0      // 最低烟雾浓度 0ppm
#define SMOKE_MAX       1000   // 最高烟雾浓度 1000ppm
#define SMOKE_WARNING   400    // 烟雾警戒值 400ppm
#define SMOKE_DANGER    800    // 烟雾危险值 800ppm

#define CO_MIN          0      // 最低CO浓度 0ppm
#define CO_MAX          2000   // 最高CO浓度 2000ppm
#define CO_WARNING      200    // CO警戒值 200ppm
#define CO_DANGER       800    // CO危险值 800ppm

// 传感器值存储
typedef struct {
    float temp;          // 温度值
    float humi;          // 湿度值
    int smoke_ppm;  // 烟雾浓度(ppm)
    int co_ppm;     // CO浓度(ppm)
} sensor_data_t;

extern sensor_data_t sensor_data;