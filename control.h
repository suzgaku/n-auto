#ifndef CONTROL_H
#define CONTROL_H

#include <define.h>

#define RAIL_MAX    7
#define RAIL_NOTIFICATION_CYCLE_MS      (100 / CYCLE_MS)

enum rail_state {
    RAIL_STATE_IDLE = 0,
    RAIL_STATE_ENTRY,
    RAIL_STATE_ON_TRAIN,
};

struct rail_debug_params {
    uint8 detect_sensor_speed;
    uint16 detect_sensor_sum;
};

struct led_config {
    uint8 blinkmap;
    uint8 onmap;
};

struct contrail_id {
    uint8 control_id;
    uint8 rail_id;
};

struct rail_request_config {
    int16 cycle_count;
    bool entry_request;
    bool rail_status_update;
};

struct rail_config {
    uint8 rail_id;
    enum rail_state state;
    bool head;
    bool tail;
    uint8 train_id;
    uint8 max_entry;
    uint8 reserved_num;
    struct contrail_id from;
    struct contrail_id to;
    struct rail_request_config req;
    uint8 sensor_calib;
    struct rail_debug_params debug;
};

struct control_config {
    struct contrail_id own;
    struct rail_config rail[RAIL_MAX];
    struct rail_request_config rail_req;
    struct led_config led;
};


#endif /* CONTROL_H */