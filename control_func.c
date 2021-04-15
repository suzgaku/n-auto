

#include <communication.h>
#include <control.h>
#include <can-pic18f_ecan.h>

#define SENSOR_LOG_MAX      50
#define OWM_COMM_STACK      20

extern struct control_config ctrl;

struct comm_payload {
    uint32 cmd;
    uint8 data[8];
};
struct own_comm_ring_buf {
    int w;
    int r;
    struct comm_payload pay[OWM_COMM_STACK];
};

struct own_comm_ring_buf own_comm_buf = {0};
static uint8 sensor_log[RAIL_MAX][SENSOR_LOG_MAX];

#ifdef USING_DEBUG_COMM_LOG
#define COMM_LOG_SIZE   10
struct comm_log_buf {
    uint8 index;
    struct comm_payload pay[COMM_LOG_SIZE];
};
struct comm_log_buf comm_log = {0};

void comm_loop_buff_save(uint32 cmd, uint8 *data)
{
    int i;
    
    comm_log.pay[comm_log.index].cmd = cmd;
    for (i = 0; i < 8; i++) {
        comm_log.pay[comm_log.index].data[i] = data[i];
    }
    comm_log.index++;
    if (comm_log.index >= COMM_LOG_SIZE) {
        comm_log.index = 0;
    }
}
#endif /* USING_DEBUG_COMM_LOG */

void led_set_on(uint8 rid)
{
    ctrl.led.onmap    |=   1 << rid;
    ctrl.led.blinkmap &= ~(1 << rid);
}
void led_set_on_blinking(uint8 rid)
{
    ctrl.led.onmap    |= 1 << rid;
    ctrl.led.blinkmap |= 1 << rid;
}
void led_set_off(uint8 rid)
{
    ctrl.led.onmap    &= ~(1 << rid);
    ctrl.led.blinkmap &= ~(1 << rid);
}

void state_change(uint8 rid, uint8 state)
{
    switch (state) {
        case RAIL_STATE_IDLE:
            led_set_off(rid);
            break;
        case RAIL_STATE_ENTRY:
            led_set_on_blinking(rid);
            break;
        case RAIL_STATE_ON_TRAIN:
            led_set_on(rid);
            break;
        default:
            break;
    }
    ctrl.rail[rid].state = state;
}

void spi_initialize(void )
{
   	output_high(PIN_A4);
	output_drive(PIN_A4);
	output_low(PIN_A4);
    delay_ms(1);
   	output_high(PIN_A4);
    delay_ms(1);    
    
    IO_INIT();
    
    /* POINT */
    IO_SET_TRIS_A(IO_DEVICE_0, 0x00);
    IO_SET_TRIS_B(IO_DEVICE_0, 0x00);

    /* LED */
    IO_SET_TRIS_A(IO_DEVICE_2, 0x00);
    IO_OUTPUT_FLOAT(IO_DEVICE_2, IO_PIN_B0);
}

bool own_comm_send_buffer_is_empty(void)
{
    return (own_comm_buf.w == 0);
}

void own_comm_send_buffer_write(uint32 cmd, uint8 *data)
{
    int i;
    
    own_comm_buf.pay[own_comm_buf.w].cmd = cmd;
    for (i = 0; i < 8; i++) {
        own_comm_buf.pay[own_comm_buf.w].data[i] = data[i];
    }
    own_comm_buf.w++;
    if (own_comm_buf.w >= OWM_COMM_STACK) {
        while(1);
    }
}

bool own_comm_send_buffer_read(CAN_RX_HEADER *rHeader, uint8 *data)
{
    int i;
    
    if (own_comm_send_buffer_is_empty()) {
        return FALSE;
    }
    rHeader->Id = own_comm_buf.pay[own_comm_buf.r].cmd;
    for (i = 0; i < 8; i++) {
        data[i] =  own_comm_buf.pay[own_comm_buf.r].data[i];
    }
    own_comm_buf.r++;
    
    if (own_comm_buf.r == own_comm_buf.w) {
         own_comm_buf.w = 0;
         own_comm_buf.r = 0;
    }
            
    return TRUE;
}

void send_comm(uint16 cmd, uint8 *data, uint8 len)
{
    CAN_TX_HEADER tHeader;

    tHeader.Id = cmd | ctrl.own.control_id;
    tHeader.Length = len;
    tHeader.Priority = 1;
    tHeader.ext = TRUE;
    tHeader.rtr = FALSE;

    while (!can_tbe()) {
            ;
    }
    
    comm_loop_buff_save(tHeader.Id, data);
    own_comm_send_buffer_write(tHeader.Id, data);

    can_putd(&tHeader, data);
    
}

void sensor_initialize(void)
{
    int i;
    
    setup_adc_ports(sAN16 | sAN17 | sAN18 | sAN19 | sAN20 | sAN21 |sAN22);//C0-C6
    
    setup_adc(ADC_CLOCK_INTERNAL);
    
#if 0    
    for (i = 0; i < RAIL_MAX; i++) {
        ctrl.rail[i].sensor_calib = read_eeprom(0x10 + i);
    }
#else
    uint8 calib[7] = {3, 3, 8, 5, 3, 4, 4};
    
    for (i = 0; i < RAIL_MAX; i++) {
        //ctrl.rail[i].sensor_calib = calib[i];
    }
#endif
    
}

inline uint8 sensor_get_raw(uint8 ch, uint8 index)
{
    return sensor_log[ch][index];
}

uint16 sensor_get_raw_sum(uint8 ch)
{
    int i;
    uint16 sum = 0;
    
    for (i = 0; i < SENSOR_LOG_MAX; i++) {
        sum += (uint16)sensor_log[ch][i];
    }
    return sum;
}


inline void sensor_sampling(void)
{
    static uint8 now_ch = 0;
    static uint8 now_count = 0;
    uint8 sample;
    static uint8 skip = 0;
    
    if (skip < (1 * 1)) { /* 0:x1 1:x2  */
        skip++;
        return;
    }
    skip = 0;
   
    sample = read_adc(adc_read_only);
#if 0
    sensor_log[now_ch][now_count] = (sample > ctrl.rail[now_ch].sensor_calib) ?
            sample - ctrl.rail[now_ch].sensor_calib : 0;

    now_ch++;
    if (now_ch >= RAIL_MAX) {
        now_ch = 0;

        now_count++;
        if (now_count >= SENSOR_LOG_MAX) {
            now_count = 0;
        }
    }
#else
    sensor_log[now_ch][now_count] = sample;
    
        now_count++;
        if (now_count >= SENSOR_LOG_MAX) {
            now_count = 0;
        }
#endif

    
    //set_adc_channel(now_ch);
    set_adc_channel(16 + now_ch);
    read_adc(adc_start_only);
}
#if 1
bool sensor_is_detect(int8 ch)
{
    uint16 sum = 0;
    int i;
    
    for (i = 0; i < SENSOR_LOG_MAX; i++) {
        sum += sensor_log[ch][i];
        if (sum > 60) { // 20 40 80x 120x 100x
            return TRUE;
        }
    }
    return FALSE;
}
#else
bool sensor_is_detect(int8 ch)
{
    return TRUE;
}
#endif
/*
 * void "RAIL速度"の送信
 */
void send_comm_rail_speed(uint8 tr_id, uint8 speed)
{
    struct comm_data_rail_speed r_speed;
    
    r_speed.train_id = tr_id;
    r_speed.speed    = speed;
    
    send_comm(COMM_RAIL_SPEED | ctrl.own.control_id,
        (uint8 *)&r_speed, sizeof(struct comm_data_rail_speed));
}

/*
 * void "TRAIN速度"の送信
 */
void send_comm_train_speed(uint8 tr_id, uint8 speed, bool dir, uint8 max_sec, uint8 len)
{
    struct comm_data_train_speed tr_speed;
    
    tr_speed.train_id = tr_id;
    tr_speed.speed    = speed;
    tr_speed.direction = dir;
    tr_speed.max_section = max_sec;
    tr_speed.length = len;
    
    send_comm(COMM_TRAIN_SPEED | ctrl.own.control_id,
        (uint8 *)&tr_speed, sizeof(struct comm_data_train_speed));
}

/*
 * void "経路登録"の送信
 */
void send_comm_rail_route_set(uint8 rid, uint8 nx_ctl_id, uint8 nx_rid)
{
    struct comm_data_rail_route_set route;
    
    route.rail_id         = rid;
    route.next_control_id = nx_ctl_id;
    route.next_rail_id    = nx_rid;
    
    send_comm(COMM_RAIL_ROUTE_SET | ctrl.own.control_id,
        (uint8 *)&route, sizeof(struct comm_data_rail_route_set));
}

/*
 * void "車両載せ"の送信
 */
void send_comm_rail_on_train(uint8 rid, uint8 on_ctl_id, uint8 on_rid, uint8 train_id, uint8 max_sec, bool head, bool tail)
{
    struct comm_data_rail_on_train on_tr;
    
    on_tr.rail_id         = rid;
    on_tr.on_control_id   = on_ctl_id;
    on_tr.on_rail_id      = on_rid;
    on_tr.train_id        = train_id;
    on_tr.max_section     = max_sec;
    on_tr.head            = head;
    on_tr.tail            = tail;
    
    send_comm(COMM_RAIL_ON_TRAIN | ctrl.own.control_id,
        (uint8 *)&on_tr, sizeof(struct comm_data_rail_on_train));
}

void send_comm_debug_maintenance_pwm(uint8 speed)
{
    struct comm_data_debug_maintenance data;
    data.command = 0x01;
    data.data[0] = speed;
    
    send_comm(COMM_DEBUG_MAINTENANCE,
        (uint8 *)&data, sizeof(struct comm_data_debug_maintenance));
}

#define LED_BLINKING_CYCLE_MS   200
void led_cycle_process(void)
{
    static uint8 blinking_count;
    uint8 data;
    
    data = (blinking_count < (LED_BLINKING_CYCLE_MS / CYCLE_MS)) ?
        (ctrl.led.onmap & ~ctrl.led.blinkmap) : ctrl.led.onmap;
    
    IO_OUTPUT_A(IO_DEVICE_2, data);
    
    blinking_count++;
    
    if (blinking_count > ((LED_BLINKING_CYCLE_MS/CYCLE_MS) * 2)) {
        blinking_count = 0;
    }
}

uint8 train_to_rail_id(uint8 train_id)
{
    int8 i;
    
    for (i = 0; i < RAIL_MAX; i++) {
        if (ctrl.rail[i].train_id == train_id) {
            return i;
        }
    }
    return DATA_UNKNOWN;
}

void point_set(uint8 point_id, bool curve)
{
#if 0
    //IO_OUTPUT_A(IO_DEVICE_0, 0xFF);
    //IO_OUTPUT_B(IO_DEVICE_0, 0xFF);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 0);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 1);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 2);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 3);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 4);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 5);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 6);
    IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + 7);
#else
    if (curve) {
        IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_A0 + point_id);
    } else {
        IO_OUTPUT_HIGH(IO_DEVICE_0, IO_PIN_B0 + point_id);
    }
    delay_ms(15);//100 ok 10ok   5ng  7ng  8ng  10ng  20ok
    IO_OUTPUT_LOW(IO_DEVICE_0, IO_PIN_A0 + point_id);
    IO_OUTPUT_LOW(IO_DEVICE_0, IO_PIN_B0 + point_id);
#endif
}

