#pragma once
EXTERN_C __declspec(selectany) const GUID ProviderGuid = {0x27b66b66, 0x6df9, 0x4e63, {0xb6, 0xc0,0xdd,0x18,0x81,0xc8,0xbc,0xf9}};
#define CHANNEL_RNR 0x10
#define READ_KEYWORD 0x1
#define WRITE_KEYWORD 0x2
#define LOCAL_KEYWORD 0x4
#define REMOTE_KEYWORD 0x8
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR GenericDataEvent = {0x1, 0x0, 0x10, 0x4, 0x0, 0x0, 0x8000000000000005};
#define GenericDataEvent_value 0x1
#define MSG_Event_WhenToTransfer         0xB0000001L

#define MSG_Map_Download                 0xD0000001L

#define MSG_Map_Friday                   0xF0000006L

#define MSG_Map_Monday                   0xF0000002L

#define MSG_Map_Saturday                 0xF0000007L

#define MSG_Map_Sunday                   0xF0000001L

#define MSG_Map_Thursday                 0xF0000005L

#define MSG_Map_Tuesday                  0xF0000003L

#define MSG_Map_Upload                   0xD0000002L

#define MSG_Map_UploadReply              0xD0000003L

#define MSG_Map_Wednesday                0xF0000004L

