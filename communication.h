
#ifndef COMMINICATION_H
#define COMMINICATION_H

#define BITMAP_CMD(cmd)             (cmd    << 8)
#define BITMAP_SUBCMD(subcmd)       (subcmd << 6)

#define COMM_SYSTEM             BITMAP_CMD(0)
#define COMM_TRAIN_CONTROL      BITMAP_CMD(1)
#define COMM_RAIL_CONTROL0      BITMAP_CMD(2)
#define COMM_RAIL_CONTROL1      BITMAP_CMD(3)
#define COMM_POINT_CONTROL      BITMAP_CMD(4)
#define COMM_SIGNAL_CONTROL     BITMAP_CMD(5)
#define COMM_DEBUG              BITMAP_CMD(7)

#define COMM_SYS_SETTING_MODE    (COMM_SYSTEM         | BITMAP_SUBCMD(0))
#define COMM_SYS_PAUSE_STOP      (COMM_SYSTEM         | BITMAP_SUBCMD(1))
#define COMM_SYS_MOTOR_CFG_WRITE (COMM_SYSTEM         | BITMAP_SUBCMD(2))
#define COMM_SYS_EEPROM_INIT     (COMM_SYSTEM         | BITMAP_SUBCMD(3))
#define COMM_TRAIN_SPEED         (COMM_TRAIN_CONTROL  | BITMAP_SUBCMD(0))
#define COMM_RAIL_ROUTE_SET      (COMM_RAIL_CONTROL0  | BITMAP_SUBCMD(0))
#define COMM_RAIL_SPEED          (COMM_RAIL_CONTROL0  | BITMAP_SUBCMD(1))
#define COMM_RAIL_ON_TRAIN       (COMM_RAIL_CONTROL0  | BITMAP_SUBCMD(2))
#define COMM_RAIL_ENTRY_REQUEST  (COMM_RAIL_CONTROL1  | BITMAP_SUBCMD(0))
#define COMM_RAIL_ENTRY_ACCEPT   (COMM_RAIL_CONTROL1  | BITMAP_SUBCMD(1))
#define COMM_RAIL_ENTRY          (COMM_RAIL_CONTROL1  | BITMAP_SUBCMD(2))
#define COMM_RAIL_EXIT           (COMM_RAIL_CONTROL1  | BITMAP_SUBCMD(3))
#define COMM_POINT_SET           (COMM_POINT_CONTROL  | BITMAP_SUBCMD(0))
#define COMM_SIGNAL_SPEED        (COMM_SIGNAL_CONTROL | BITMAP_SUBCMD(0))
#define COMM_SIGNAL_SPEED_SET    (COMM_SIGNAL_CONTROL | BITMAP_SUBCMD(2))
#define COMM_SIGNAL_SET          (COMM_SIGNAL_CONTROL | BITMAP_SUBCMD(3))
#define COMM_DEBUG_CONTROL       (COMM_DEBUG          | BITMAP_SUBCMD(0))
#define COMM_DEBUG_MOTOR         (COMM_DEBUG          | BITMAP_SUBCMD(1))
#define COMM_DEBUG_MAINTENANCE   (COMM_DEBUG          | BITMAP_SUBCMD(3))

#define COMM_CMD_MASK           0x7C0
#define COMM_CTL_ID_MASK        0x03F

struct comm_data_sys_setting_mode {
    int8 mode;
};
struct comm_data_sys_pause_stop {
    int8 mode;
};
struct comm_data_sys_motor_cfg_write {
    int8 to_control_id;
    int8 addr;
    int8 size;
    int8 data[4];
};
struct comm_data_sys_eeprom_init {
    int8 to_control_id;
    int8 mode;
};
struct comm_data_train_speed {
    int8 train_id;
    int8 speed;
    int8 direction;
    int8 max_section;
    int16 length;
};
struct comm_data_rail_route_set {
    int8 rail_id;
    int8 set_control_id;
    int8 set_rail_id;
    int8 next_control_id;
    int8 next_rail_id;
};
struct comm_data_rail_speed {
    int8 train_id;
    int8 speed;
};
struct comm_data_rail_on_train {
    int8 rail_id;
    int8 train_id;
    int8 on_control_id;
    int8 on_rail_id;
    int8 max_section;
    bool head;
    bool tail;
};
struct comm_data_rail_entry_request {
    int8 rail_id;
    int8 owner_control_id;
    int8 owner_rail_id;
    int8 to_control_id;
    int8 to_rail_id;
    int8 train_id;
    int8 max_section;
    int8 rserve_num;
};
struct comm_data_rail_entry_accept {
    int8 rail_id;
    int8 owner_control_id;
    int8 owner_rail_id;
    int8 from_control_id;
    int8 from_rail_id;
    int8 train_id;
    int8 reserved_num;
};
struct comm_data_rail_entry {
    int8 rail_id;
    int8 from_control_id;
    int8 from_rail_id;
    int8 train_id;
};
struct comm_data_rail_exit {
    int8 rail_id;
    int8 to_control_id;
    int8 to_rail_id;
    int8 train_id;
};
struct comm_data_point_set {
    int8 pint_id;
    int8 curve;
};
struct comm_data_signal_speed {
    int8 signal_id;
    int8 signal;
};
struct comm_data_signal_speed_set {
    int8 signal_id;
    int8 speed[6];
};
struct comm_data_signal_set {
    int8 signal_id;
    int8 is_auto;
    int8 type;
};
struct comm_data_debug_control {
    int8 data[8];
};
struct comm_data_debug_motor {
    int8 data[8];
};
struct comm_data_debug_maintenance {
    int8 command;
    int8 data[7];
};


#endif /* COMMINICATION_H */

