#include "message.h"

#define FRAME_HEAD_1 0x7D
#define FRAME_HEAD_2 0x95

#define FRAME_TAIL_1 0x6A
#define FRAME_TAIL_2 0xE2

uint8_t CalculateChecksum(uint8_t* Bp_Buffer, int B_Size) 
{ 
    uint8_t B_Checksum = 0; 
    while (B_Size--) 
        B_Checksum += *((Bp_Buffer)++); 
    B_Checksum = ~B_Checksum; 
    return B_Checksum; 
} 

int load_msg(uint8_t *msg, uint8_t cmd, uint32_t target_address, uint32_t source_addr, uint8_t *content, uint8_t len)
{
    msg[0] = FRAME_HEAD_1;
    msg[1] = FRAME_HEAD_2;

    msg[2] = cmd;
    msg[3] = len;

    msg[4] = (target_address >> 24) & 0xFF;
    msg[5] = (target_address >> 16) & 0xFF;
    msg[6] = (target_address >> 8) & 0xFF;
    msg[7] = target_address & 0xFF;

    msg[8] = (source_addr >> 24) & 0xFF;
    msg[9] = (source_addr >> 16) & 0xFF;
    msg[10] = (source_addr >> 8) & 0xFF;
    msg[11] = source_addr & 0xFF;

    if (len > 0 && content != 0) {
        for (int i = 0; i < len; i++) {
            msg[12 + i] = content[i];
        }
    }

    msg[12 + len] = CalculateChecksum(msg, 12 + len);

    msg[12 + len + 1] = FRAME_TAIL_1;
    msg[12 + len + 2] = FRAME_TAIL_2;

    return len + 15;
}

int unload_msg(uint8_t *msg, uint8_t *cmd, uint32_t *target_address, uint32_t *source_addr, uint8_t *content, uint8_t *len)
{
    if (msg[0] != FRAME_HEAD_1 || msg[1] != FRAME_HEAD_2) {
        return 0; // Invalid frame head
    }

    if (msg[12 + msg[3] + 1] != FRAME_TAIL_1 || msg[12 + msg[3] + 2] != FRAME_TAIL_2) {
        return 0; // Invalid frame tail
    }

    *cmd = msg[2];
    *len = msg[3];

    *target_address = (msg[4] << 24) | (msg[5] << 16) | (msg[6] << 8) | msg[7];
    *source_addr = (msg[8] << 24) | (msg[9] << 16) | (msg[10] << 8) | msg[11];

    if (*len > 0 && content != 0) {
        for (int i = 0; i < *len; i++) {
            content[i] = msg[12 + i];
        }
    }

    return 1;
}

int parseRequestDeviceAddrMsg(uint8_t *msg, int len, uint32_t *password)
{
    if (len < 4) {
        return 0; // Invalid length
    }

    *password = (msg[0] << 24) | (msg[1] << 16) | (msg[2] << 8) | msg[3];
    return 1; // Success
}

int parseSetSensorModelMsg(uint8_t *content, int len, uint8_t *model, uint8_t *model_len)
{
    if (len < 1 || content == 0 || model == 0) {
        return 0; // Invalid parameters
    }

    *model_len = len;
    
    if (*model_len > 10)
        *model_len = 10;
    
    for (int i = 0; i < *model_len; i++)
    {
        model[i] = content[i];
    }
    
    return 1; // Success
}

int packetSetSensorModelReplyMsg(uint8_t *content, uint8_t reply, uint8_t *model, int len)
{
    if (content == 0) {
        return 0; // Invalid parameters
    }

    content[0] = reply;
    for (int i = 0; i < len; i++)
    {
        content[i+1] = model[i];
    }

    return 11; // Return the length of the content
}

int packetQueryDeviceInfoReplyMsg(uint8_t *content, uint32_t sn, uint32_t version, uint8_t sensor_num, uint8_t battery_level)
{
    content[0] = (sn >> 24) & 0xFF;
    content[1] = (sn >> 16) & 0xFF;
    content[2] = (sn >> 8) & 0xFF;
    content[3] = sn & 0xFF;

    content[4] = (version >> 24) & 0xFF;
    content[5] = (version >> 16) & 0xFF;
    content[6] = (version >> 8) & 0xFF;
    content[7] = version & 0xFF;

    content[8] = sensor_num;
    content[9] = battery_level;

    return 10; // Return the length of the content
}

int parseQueryDeviceInfoReplyMsg(uint8_t *content, int len, uint32_t *sn, uint32_t *version, uint8_t *sensor_num, uint8_t *battery_level)
{
    if (content == 0 || sn == 0 || version == 0 || sensor_num == 0 || battery_level == 0) {
        return 0; // Invalid parameters
    }

    if (len < 10) {
        return 0; // Invalid length
    }

    *sn = (content[0] << 24) | (content[1] << 16) | (content[2] << 8) | content[3];
    *version = (content[4] << 24) | (content[5] << 16) | (content[6] << 8) | content[7];
    *sensor_num = content[8];
    *battery_level = content[9];

    return 1; // Success
}


int packetQueryCollectionTimeMsg(uint8_t *content, uint8_t sensor_id)
{
    content[0] = sensor_id;
    return 1;
}

int packetQueryCollectionTimeReplyMsg(uint8_t *content, uint32_t start_time, uint32_t data_len)
{
    content[0] = (start_time >> 24) & 0xFF;
    content[1] = (start_time >> 16) & 0xFF;
    content[2] = (start_time >> 8) & 0xFF;
    content[3] = start_time & 0xFF;

    content[4] = (data_len >> 24) & 0xFF;
    content[5] = (data_len >> 16) & 0xFF;
    content[6] = (data_len >> 8) & 0xFF;
    content[7] = data_len & 0xFF;

    return 8; // Return the length of the content
}

int parseQueryCollectionTimeReplyMsg(uint8_t *content, int len, uint32_t *start_time, uint32_t *data_len)
{
    if (content == 0 || start_time == 0 || data_len == 0) {
        return 0; // Invalid parameters
    }

    if (len < 8) {
        return 0; // Invalid length
    }

    *start_time = (content[0] << 24) | (content[1] << 16) | (content[2] << 8) | content[3];
    *data_len = (content[4] << 24) | (content[5] << 16) | (content[6] << 8) | content[7];

    return 1; // Success
}

int packetQueryCollectionDataMsg(uint8_t *content, uint8_t sensor_id, uint32_t index_offest_end, uint16_t len)
{
    content[0] = sensor_id;
    content[1] = (index_offest_end >> 24) & 0xFF;
    content[2] = (index_offest_end >> 16) & 0xFF;
    content[3] = (index_offest_end >> 8) & 0xFF;
    content[4] = index_offest_end & 0xFF;

    content[5] = (len >> 8) & 0xFF;
    content[6] = len & 0xFF;

    return 7; // Return the length of the content
}

int parseQueryCollectionDataMsg(uint8_t *content, int content_len, uint8_t *sensor_id, uint32_t *index_offest_end, uint16_t *len)
{
    if (content == 0 || sensor_id == 0 || index_offest_end == 0 || len == 0) {
        return 0; // Invalid parameters
    }

    if (content_len < 7) {
        return 0; // Invalid length
    }

    *sensor_id = content[0];
    *index_offest_end = (content[1] << 24) | (content[2] << 16) | (content[3] << 8) | content[4];
    *len = (content[5] << 8) | content[6];

    return 1; // Success
}

int packetQueryCollectionTimeIntervalReplyMsg(uint8_t *content, uint16_t interval)
{
    content[0] = (interval >> 8) & 0xFF;
    content[1] = interval & 0xFF;

    return 2; // Return the length of the content
}

int packetSetCollectionIntervalMsg(uint8_t *content, uint16_t interval)
{
    content[1] = (interval >> 8) & 0xFF;
    content[2] = (interval >> 0) & 0xFF;

    return 2; // Return the length of the content
}

int parseSetCollectionIntervalMsg(uint8_t *content, int len, uint16_t *interval)
{
    if (len < 2 || content == 0 || interval == 0) {
        return 0; // Invalid parameters
    }

    *interval = (content[0] << 8) | content[1];

    return 1; // Success
}

int packetSetCollectionIntervalReplyMsg(uint8_t *content, uint8_t reply, uint16_t interval)
{
    content[0] = reply;
    content[1] = (interval >> 8) & 0xFF;
    content[2] = (interval >> 0) & 0xFF;

    return 3; // Return the length of the content
}

int parseSetCollectionIntervalReplyMsg(uint8_t *content, int len, uint8_t *reply, uint16_t *interval)
{
    if (len < 3 || content == 0 || reply == 0 || interval == 0) {
        return 0; // Invalid parameters
    }

    *reply = content[0];
    *interval = (content[1] << 8) | content[2];

    return 1; // Success
}

int packetQueryRtcReplyMsg(uint8_t *content, uint32_t rtc_val)
{
    content[0] = (rtc_val >> 24) & 0xFF;
    content[1] = (rtc_val >> 16) & 0xFF;
    content[2] = (rtc_val >> 8) & 0xFF;
    content[3] = rtc_val & 0xFF;

    return 4; // Return the length of the content
}

int parseSetRtcMsg(uint8_t *content, int len, uint32_t *rtc_val)
{
    if (len < 4 || content == 0 || rtc_val == 0) {
        return 0; // Invalid parameters
    }

    *rtc_val = (content[0] << 24) | (content[1] << 16) | (content[2] << 8) | content[3];
    return 1; // Success
}

int packetSetRtcReplyMsg(uint8_t *content, uint8_t reply, uint32_t rtc_val)
{
    content[0] = reply;
    content[1] = (rtc_val >> 24) & 0xFF;
    content[2] = (rtc_val >> 16) & 0xFF;
    content[3] = (rtc_val >> 8) & 0xFF;
    content[4] = rtc_val & 0xFF;

    return 5; // Return the length of the content
}

int packetQueryCollectionStartTimeReplyMsg(uint8_t *content, uint32_t start_time, uint32_t count)
{
    content[0] = (start_time >> 24) & 0xFF;
    content[1] = (start_time >> 16) & 0xFF;
    content[2] = (start_time >> 8) & 0xFF;
    content[3] = start_time & 0xFF;

    content[4] = (count >> 24) & 0xFF;
    content[5] = (count >> 16) & 0xFF;
    content[6] = (count >> 8) & 0xFF;
    content[7] = count & 0xFF;

    return 8; // Return the length of the content
}

int parseSetDeviceSettingMsg(uint8_t *content, int len, uint32_t *sn, uint8_t *sensor_num)
{
    if (len < 5 || content == 0 || sn == 0 || sensor_num == 0) {
        return 0; // Invalid parameters
    }

    *sn = (content[0] << 24) | (content[1] << 16) | (content[2] << 8) | content[3];
    *sensor_num = content[4];

    return 1; // Success
}

int packetSetDeviceSettingReplyMsg(uint8_t *content, uint8_t reply, uint32_t sn, uint32_t version, uint8_t sensor_num, uint8_t battery_level)
{
    content[0] = reply;
    content[1] = (sn >> 24) & 0xFF;
    content[2] = (sn >> 16) & 0xFF;
    content[3] = (sn >> 8) & 0xFF;
    content[4] = sn & 0xFF;
    content[5] = (version >> 24) & 0xFF;
    content[6] = (version >> 16) & 0xFF;
    content[7] = (version >> 8) & 0xFF;
    content[8] = version & 0xFF;
    content[9] = sensor_num;
    content[10] = battery_level;

    return 11; // Return the length of the content
}

int parseRestartMsg(uint8_t *content, int content_len, uint8_t *sensor_id)
{
    if (content_len < 1 || content == 0 || sensor_id == 0) {
        return 0; // Invalid parameters
    }

    *sensor_id = content[0];

    return 1; // Success
}

int packetRestartReplyMsg(uint8_t *content, uint8_t reply)
{
    content[0] = reply;

    return 1; // Return the length of the content
}


int packetErasePageMsg(uint8_t *content, uint32_t data_index)
{
    content[0] = UPGRADE_CMD_ERASE_PAGE;
    content[1] = (data_index >> 24) & 0xFF;
    content[2] = (data_index >> 16) & 0xFF;
    content[3] = (data_index >> 8) & 0xFF;
    content[4] = data_index & 0xFF;

    return 5; // Return the length of the content
}

int parseErasePageMsg(uint8_t *content, int len, uint32_t *data_index)
{
    if (len < 5 || content == 0 || data_index == 0) {
        return 0; // Invalid parameters
    }

    if (content[0] != UPGRADE_CMD_ERASE_PAGE) {
        return 0; // Invalid command type
    }

    *data_index = (content[1] << 24) | (content[2] << 16) | (content[3] << 8) | content[4];
    return 1; // Success
}

int packetErasePageReplyMsg(uint8_t *content, uint8_t reply)
{
    content[0] = UPGRADE_CMD_ERASE_PAGE;
    content[1] = reply;

    return 2; // Return the length of the content
}

int parseErasePageReplyMsg(uint8_t *content, int len, uint8_t *reply)
{
    if (len < 2 || content == 0 || reply == 0) {
        return 0; // Invalid parameters
    }

    if (content[0] != UPGRADE_CMD_ERASE_PAGE) {
        return 0; // Invalid command type
    }

    *reply = content[1];
    return 1; // Success
}

int packetWritePageMsg(uint8_t *content, uint32_t data_index, uint8_t *data, uint16_t data_len)
{
    content[0] = UPGRADE_CMD_WRITE_PAGE;
    content[1] = (data_index >> 24) & 0xFF;
    content[2] = (data_index >> 16) & 0xFF;
    content[3] = (data_index >> 8) & 0xFF;
    content[4] = data_index & 0xFF;

    for (int i = 0; i < data_len; i++)
    {
        content[5 + i] = data[i];
    }

    return 5 + data_len; // Return the length of the content
}

int parseWritePageMsg(uint8_t *content, int len, uint32_t *data_index, uint8_t *data, uint16_t *data_len)
{
    if (len < 5 || content == 0 || data_index == 0 || data == 0 || data_len == 0) {
        return 0; // Invalid parameters
    }

    if (content[0] != UPGRADE_CMD_WRITE_PAGE) {
        return 0; // Invalid command type
    }

    *data_index = (content[1] << 24) | (content[2] << 16) | (content[3] << 8) | content[4];

    *data_len = len - 5;
    for (int i = 0; i < *data_len; i++)
    {
        data[i] = content[5 + i];
    }

    return 1; // Success
}

int packetWritePageReplyMsg(uint8_t *content, uint8_t reply)
{
    content[0] = UPGRADE_CMD_WRITE_PAGE;
    content[1] = reply;

    return 2; // Return the length of the content
}

int parseWritePageReplyMsg(uint8_t *content, int len, uint8_t *reply)
{
    if (len < 2 || content == 0 || reply == 0) {
        return 0; // Invalid parameters
    }

    if (content[0] != UPGRADE_CMD_WRITE_PAGE) {
        return 0; // Invalid command type
    }

    *reply = content[1];
    return 1; // Success
}

int packetUpgradeEndMsg(uint8_t *content)
{
    content[0] = UPGRADE_CMD_END;
    return 1; // Return the length of the content
}

int packetUpgradeEndReplyMsg(uint8_t *content, uint8_t reply)
{
    content[0] = UPGRADE_CMD_END;
    content[1] = reply;

    return 2; // Return the length of the content
}
