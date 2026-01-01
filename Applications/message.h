#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>
// Your code here

#define ADDR_DEVICE 0x04004200
#define ADDR_BOARDCAST 0x7F7F7F7F
#define ADDR_UPPER 0x1F162C6A
#define ADDR_ADMIN 0xA62E59D7

#define CMD_RESET_DEVICE 0x04

#define CMD_QUERY_DEVICE_ADDR 0x10
#define CMD_QUERY_DEVICE_ADDR_REPLY 0x80

#define CMD_QUERY_DEVICE_INFO 0x11
#define CMD_QUERY_DEVICE_INFO_REPLY 0x81

#define CMD_QUERY_SENSOR_MODEL 0x12
#define CMD_QUERY_SENSOR_MODEL_REPLY 0x82

#define CMD_QUERY_RTC 0x13
#define CMD_QUERY_RTC_REPLY 0x83

#define CMD_QUERY_TIME_INTERVAL 0x14
#define CMD_QUERY_TIME_INTERVAL_REPLY 0x84

#define CMD_QUERY_REAL_TIME_SENSOR_DATA 0x15
#define CMD_QUERY_REAL_TIME_SENSOR_DATA_REPLY 0x85

#define CMD_QUERY_COLLECTION_START_TIME 0x16
#define CMD_QUERY_COLLECTION_START_TIME_REPLY 0x86

#define CMD_QUERY_COLLECTION_DATA 0x17
#define CMD_QUERY_COLLECTION_DATA_REPLY 0x87

#define CMD_SET_DEVICE_SETTING 0x21
#define CMD_SET_DEVICE_SETTING_REPLY 0x91

#define CMD_SET_SENSOR_MODEL 0x22
#define CMD_SET_SENSOR_MODEL_REPLY 0x92

#define CMD_SET_RTC 0x23
#define CMD_SET_RTC_REPLY 0x93

#define CMD_SET_TIME_INTERVAL 0x24
#define CMD_SET_TIME_INTERVAL_REPLY 0x94

#define CMD_COLLECTION_RESTART 0x25
#define CMD_COLLECTION_RESTART_REPLY 0x95

#define CMD_CLEAR_COLLECTION_DATA 0x26
#define CMD_CLEAR_COLLECTION_DATA_REPLY 0x96

#define CMD_UPGRADE_DEVICE 0x50
#define CMD_UPGRADE_DEVICE_REPLY 0xC0



#define PASSWORD 0x23F209C3



int load_msg(uint8_t *msg, uint8_t cmd, uint32_t target_address, uint32_t source_addr, uint8_t *content, uint8_t len);
int unload_msg(uint8_t *msg, uint8_t *cmd, uint32_t *target_address, uint32_t *source_addr, uint8_t *content, uint8_t *len);

int parseRequestDeviceAddrMsg(uint8_t *msg, int len, uint32_t *password);

int parseSetSensorModelMsg(uint8_t *content, int len, uint8_t *model, uint8_t *model_len);
int packetSetSensorModelReplyMsg(uint8_t *content, uint8_t reply, uint8_t *model, int len);

int packetQueryDeviceInfoReplyMsg(uint8_t *content, uint32_t sn, uint32_t version, uint8_t sensor_num, uint8_t battery_level);
int parseQueryDeviceInfoReplyMsg(uint8_t *content, int len, uint32_t *sn, uint32_t *version, uint8_t *sensor_num, uint8_t *battery_level);

int packetQueryCollectionTimeIntervalReplyMsg(uint8_t *content, uint16_t interval);

int packetSetCollectionIntervalMsg(uint8_t *content, uint16_t interval);
int parseSetCollectionIntervalMsg(uint8_t *content, int len, uint16_t *interval);

int packetSetCollectionIntervalReplyMsg(uint8_t *content, uint8_t reply, uint16_t interval);
int parseSetCollectionIntervalReplyMsg(uint8_t *content, int len, uint8_t *reply, uint16_t *interval);

int packetQueryRtcReplyMsg(uint8_t *content, uint32_t rtc_val);

int parseSetRtcMsg(uint8_t *content, int len, uint32_t *rtc_val);
int packetSetRtcReplyMsg(uint8_t *content, uint8_t reply, uint32_t rtc_val);

int packetQueryCollectionStartTimeReplyMsg(uint8_t *content, uint32_t start_time, uint32_t count);

int parseQueryCollectionDataMsg(uint8_t *content, int content_len, uint8_t *sensor_id, uint32_t *index_offest_end, uint16_t *len);


int parseSetDeviceSettingMsg(uint8_t *content, int len, uint32_t *sn, uint8_t *sensor_num);
int packetSetDeviceSettingReplyMsg(uint8_t *content, uint8_t reply, uint32_t sn, uint32_t version, uint8_t sensor_num, uint8_t battery_level);

int parseRestartMsg(uint8_t *content, int content_len, uint8_t *sensor_id);
int packetRestartReplyMsg(uint8_t *content, uint8_t reply);

#define UPGRADE_CMD_ERASE_PAGE 1
int packetErasePageMsg(uint8_t *content, uint32_t data_index);
int parseErasePageMsg(uint8_t *content, int len, uint32_t *data_index);

int packetErasePageReplyMsg(uint8_t *content, uint8_t reply);
int parseErasePageReplyMsg(uint8_t *content, int len, uint8_t *reply);

#define UPGRADE_CMD_WRITE_PAGE 2
#define UPGRADE_DATA_MAX_LEN 128
int packetWritePageMsg(uint8_t *content, uint32_t data_index, uint8_t *data, uint16_t data_len);
int parseWritePageMsg(uint8_t *content, int len, uint32_t *data_index, uint8_t *data, uint16_t *data_len);

int packetWritePageReplyMsg(uint8_t *content, uint8_t reply);
int parseWritePageReplyMsg(uint8_t *content, int len, uint8_t *reply);

#define UPGRADE_CMD_END 3
int packetUpgradeEndMsg(uint8_t *content);
int packetUpgradeEndReplyMsg(uint8_t *content, uint8_t reply);


#endif
