#ifndef DEFINE_H
#define DEFINE_H

#define CONTROL
#define USING_STATIC_CONFIG
#define USING_SENSOR_CALIBRATION
#define USING_DEBUG_COMM_LOG

#define bool char
#define uint8 unsigned char
#define uint16 unsigned long
#define uint32 unsigned long long

#define TRAIN_INFO_TABLE_MAX    16
#define CYCLE_MS                10

#define RAIL_CYCLE_PROCESS_MS  100


#define SPEED_MAX               100
#define DATA_UNKNOWN            255

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

#define CONTRAIL_ID(contol, rail)       ((control << 8) | rail)
#define CONTROL_ID(contrail)            (contrail >> 8)
#define RAIL_ID(contrail)               (contrail & 0x7)

#endif /* DEFINE_H */

