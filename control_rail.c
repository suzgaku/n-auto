
#include <communication.h>
#include <control.h>
#include <train_info.h>

#ifdef  USING_STATIC_CONFIG
struct static_rail_config {
    uint8 rail_id;
    uint8 next_control_id;
    uint8 next_rail_id;
};

#define STATIC_RAIL_CONFIG_NUM   6
const struct static_rail_config st_rail_config[STATIC_RAIL_CONFIG_NUM] = {
    {0,0,1},
    {1,0,2},
    {2,0,3},
    {3,0,4},
    {4,0,5},
    {5,0,0}
};

#endif /* USING_STATIC_CONFIG */


extern struct control_config ctrl;

void rail_clear_config(uint8 rid)
{
    struct rail_config *rail = &ctrl.rail[rid];

    //memset((void)rail, 0, sizeof(struct rail_config));
    //memset(rail, 0, sizeof(struct rail_config));
    rail->rail_id         = rid;
    rail->head            = FALSE;
    rail->tail            = FALSE;
    rail->train_id        = DATA_UNKNOWN;
    rail->max_entry       = 0;
    rail->reserved_num    = 0;

    rail->from.control_id = DATA_UNKNOWN;
    rail->from.rail_id    = DATA_UNKNOWN;
//    rail->to.control_id   = DATA_UNKNOWN;
//    rail->to.rail_id      = DATA_UNKNOWN;
    
    rail->req.entry_request      = FALSE;
    rail->req.rail_status_update = FALSE;
    
    state_change(rid, RAIL_STATE_IDLE);

#ifdef USING_STATIC_CONFIG
    int i;
    
    for (i = 0; i < STATIC_RAIL_CONFIG_NUM; i++) {
        if (rail->rail_id == st_rail_config[i].rail_id) {
            rail->to.control_id = st_rail_config[i].next_control_id;
            rail->to.rail_id    = st_rail_config[i].next_rail_id;
        }
    }
#endif /* USING_STATIC_RAIL_CONFIG */    
}
void rail_initialize(void)
{
    int i;
    
    for (i = 0; i < RAIL_MAX; i++) {
        rail_clear_config(i);
    }
}

/*
 * "入線要請"の繰り返し送信
 */
static void rail_req_entry_request(struct rail_config *rail)
{
    struct comm_data_rail_entry_request req; 

    req.rail_id          = rail->rail_id;
    req.owner_control_id = ctrl.own.control_id;
    req.owner_rail_id    = rail->rail_id;
    req.to_control_id    = rail->to.control_id;
    req.to_rail_id       = rail->to.rail_id;
    req.train_id         = rail->train_id;
    req.max_section      = rail->max_entry;
    req.rserve_num       = 0;

    send_comm(COMM_RAIL_ENTRY_REQUEST,
        (uint8 *)&req, sizeof(struct comm_data_rail_entry_request));    
}

#if 0
void rail_request(uint8 rid)
{
    struct rail_config *rail = ctrl.rail[rid];
    
    if (rail->req.entry_request) {
        rail_req_entry_request(rail);
    }
}
#endif

/* 
 * "TRAIN速度"受信
 */
void rail_receive_train_speed(
    uint8 ctl_id, struct comm_data_train_speed *rcv)
{
    train_info_save_train_speed(rcv->train_id, rcv->speed, rcv->direction);
}


/* 
 * "経路登録"受信
 */
void rail_receive_rail_route_set(
    uint8 ctl_id, struct comm_data_rail_route_set *rcv)
{
    uint8 rid = rcv->set_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];  

    rail->to.control_id = rcv->next_control_id;
    rail->to.rail_id    = rcv->next_rail_id;
}

/* 
 * "RAIL速度"受信
 */
void rail_receive_rail_speed(
    uint8 ctl_id, struct comm_data_rail_speed *rcv)
{
    train_info_save_rail_speed(rcv->train_id, rcv->speed);
}

/* 
 * "車両載せ"受信
 */
void rail_receive_on_train(
    uint8 ctl_id, struct comm_data_rail_on_train *rcv)
{
    uint8 rid = rcv->on_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];

    if (rcv->train_id == DATA_UNKNOWN) {
        /* 車両を消す命令の場合 */
        rail_clear_config(rid);
        return;
    }
    
    rail->train_id = rcv->train_id;
    rail->max_entry = rcv->max_section;
    rail->head = rcv->head;
    rail->tail = rcv->tail;
    
    if (rail->head) {
        rail->req.entry_request = TRUE;
    }
    state_change(rid, RAIL_STATE_ON_TRAIN);
}

/*
 * "入線要請"受信
 */
void rail_receive_rail_entry_request(
    uint8 ctl_id, struct comm_data_rail_entry_request *rcv)
{
    uint8 rid = rcv->to_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];
    
    struct comm_data_rail_entry_accept accept;
    struct comm_data_rail_entry_request req; 
   
    if ((rail->train_id != DATA_UNKNOWN) && (rcv->train_id != rail->train_id)) { 
        return;
    }
        
    if (rail->train_id == DATA_UNKNOWN) {
        /* 入線を受け入れる為の登録作業 */
        rail->train_id        = rcv->train_id;
        rail->from.control_id = ctl_id;
        rail->from.rail_id    = rcv->rail_id;
        rail->max_entry       = rcv->max_section;
        rail->req.entry_request = TRUE;
        
        /* "入線受け入れ"の送信 */
        accept.rail_id          = rid;
        accept.owner_control_id = rcv->owner_control_id;
        accept.owner_rail_id    = rcv->owner_rail_id;
        accept.from_control_id  = rail->from.control_id;
        accept.from_rail_id     = rail->from.rail_id;
        accept.train_id         = rcv->train_id;
        accept.reserved_num     = rcv->rserve_num + 1;

        send_comm(COMM_RAIL_ENTRY_ACCEPT,
            (uint8 *)&accept, sizeof(struct comm_data_rail_entry_accept));
        
        state_change(rid, RAIL_STATE_ENTRY);
    }
    /*
     * "入線要請"は区間を複数確保する場合に"入線受け入れ"を行った後にも発行される。
     * その場合は"入線受け入れ"を返す必要はなく、自分より先の区間の"入線要請"を
     * 行う。
     */
    if (rcv->max_section > accept.reserved_num) {
        /* "入線要請の送信" */
        req.rail_id          = rid;
        req.owner_control_id = rcv->owner_control_id;
        req.owner_rail_id    = rcv->owner_rail_id;
        req.to_control_id    = rail->to.control_id;
        req.to_rail_id       = rail->to.rail_id;
        req.train_id         = rail->train_id;
        req.max_section      = rcv->max_section;
        req.rserve_num      = rcv->rserve_num + 1;

        send_comm(COMM_RAIL_ENTRY_REQUEST,
            (uint8 *)&req, sizeof(struct comm_data_rail_entry_request));
    }
}

/*
 * "入線受け入れ"受信
 */
/* 隣接区間/遠くの区間共通(検討した結果一緒になった) */
void rail_receive_rail_entry_accept(
    uint8 ctl_id, struct comm_data_rail_entry_accept *rcv)
{
    uint8 rid = rcv->owner_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];
    
    rail->reserved_num = MAX(rail->reserved_num, rcv->reserved_num);
    rail->req.rail_status_update = TRUE;

    if (rcv->reserved_num >= rail->max_entry) {
        rail->req.entry_request = FALSE;
    }
}

/*
 * "入線"受信
 */
void rail_receive_rail_entry(
    uint8 ctl_id, struct comm_data_rail_entry *rcv)
{
    uint8 rid = rcv->from_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];
    
    rail->head = FALSE;
}

/*
 * "出線"受信
 */
void rail_receive_rail_exit(
    uint8 ctl_id, struct comm_data_rail_exit *rcv)
{
    uint8 rid = rcv->to_rail_id;
    struct rail_config *rail = &ctrl.rail[rid];
    
    rail->tail = TRUE;
}

/*
 * 周期処理
 */
void rail_cycle_process(void)
{
    static uint16 cycle_count = 0;
    static uint8 rid = 0;
    uint8 speed;
    bool dir;
    
    
    cycle_count++;
    
    //if (cycle_count < (RAIL_CYCLE_PROCESS_MS / CYCLE_MS)) {
    //    return;
    //}
    cycle_count = 0;
    
    {
        
        struct rail_config *rail = &ctrl.rail[rid];
        speed = train_info_load_speed(rail->train_id, &dir);
       
        switch (rail->state) {
        case RAIL_STATE_IDLE:
            break;
        case RAIL_STATE_ENTRY:
            if ((speed > 4) && sensor_is_detect(rid)) {
                /* 車両を検知 */
                state_change(rid, RAIL_STATE_ON_TRAIN);
                rail->head = TRUE;
                rail->debug.detect_sensor_speed = speed;
                rail->debug.detect_sensor_sum = sensor_get_raw_sum(rid);
                
                /* "入線"通知 */
                struct comm_data_rail_entry entry;
                entry.rail_id         = rail->rail_id;
                entry.from_control_id = rail->from.control_id;
                entry.from_rail_id    = rail->from.rail_id; 
                entry.train_id        = rail->train_id;
                send_comm(COMM_RAIL_ENTRY,
                    (uint8 *)&entry, sizeof(struct comm_data_rail_entry));
            }
            break;
        case RAIL_STATE_ON_TRAIN:
            if (rail->tail) {
                if((speed > 4) && !sensor_is_detect(rid) && !rail->head) {            
                    /* 車両を確認できない */
                    
                    /* "出線"通知 */
                    struct comm_data_rail_exit exit;
                    exit.rail_id       = rail->rail_id;
                    exit.to_control_id = rail->to.control_id;
                    exit.to_rail_id    = rail->to.rail_id;
                    exit.train_id      = rail->train_id;
                    send_comm(COMM_RAIL_EXIT,
                        (uint8 *)&exit, sizeof(struct comm_data_rail_exit));
                    
                    /* 情報削除(RAIL_STATE_IDLEへ) */
                    rail_clear_config(rid);
                }
            }
            if (rail->head) {
                if (rail->req.entry_request) {
                //    rail->req.cycle_count++;
                //    if (rail->req.cycle_count >= RAIL_NOTIFICATION_CYCLE_MS) {
                //        rail->req.cycle_count = 0;
                    
                        /* "入線要請"通知 */
                        rail_req_entry_request(rail);
                //    }
                }            
            }
            break;
        }
    }
    
    rid++;
    if (rid >= RAIL_MAX) {
        rid = 0;
    }
}

void sensor_calibration(void)
{
    int i, j;
    uint8 sample_max, sample;
     
    for (i = 0; i < RAIL_MAX; i++) {
        ctrl.rail[i].sensor_calib = 0;
    }
    
    send_comm_debug_maintenance_pwm(10);
    
    delay_ms(1000);
    delay_ms(RAIL_MAX * SENSOR_LOG_MAX * 1);

    for (i = 0; i < RAIL_MAX; i++) {
        sample_max = 0;
        for (j = 0; j < SENSOR_LOG_MAX; j++) {
            sample = sensor_get_raw(i, j);
            sample_max = MAX(sample_max, sample);
        }
        ctrl.rail[i].sensor_calib = sample_max;
    }

    disable_interrupts(GLOBAL);
    for (i = 0; i < RAIL_MAX; i++) {
        write_eeprom(0x10 + i, ctrl.rail[i].sensor_calib);
    }
    enable_interrupts(GLOBAL);

    send_comm_debug_maintenance_pwm(0);
}