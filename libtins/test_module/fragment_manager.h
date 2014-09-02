/*
* fragment_manager.h
*
* Created on: 2014-09-02
* Author: Hoai Phuoc Truong & Alfred Kenny
*/
#ifndef FRAGMENT_MANAGER_H_
#define FRAGMENT_MANAGER_H_

#define READY_TO_SEND                           1
#define WAITING_FOR_FIRST_FRAGMENT              2
#define WAITING_FOR_FRAGMENT                    3

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct {
    u8* data_addr;
} dl_list;

typedef struct {
    u8 fragment_id;
    u8 fragment_number;
    u16 offset; //Offset of the current fragment
    u16 size; //Total size of the packet before fragmented
} fragment_info;

typedef struct {
    u8 status;
    dl_list* packet_address;
    fragment_info* info_address;
} manage_result;

/*
Return status READY_TO_SEND if all fragments have arrived (including this one)
Return status WAITING_FOR_FIRST_FRAGMENT if at least a fragment (including this one) has arrived, but the first fragment has not arrived
Return status WAITING_FOR_FRAGMENT if at least a fragment (including this one) has arrived, and the first fragment has arrived

Return frame_data is the pointer to the data buffer that is no longer used by the manager. Return null if nothing to return.
When completely reassembled, return pointer to buffer containing all data.

At all time, there should be only 1 frame buffer for each fragment id. This means
	If the fragment ID does not have a data buffer, use the pointer passed in as the buffer.
	If the fragment ID already had a data buffer, copy the newly arrived data into the buffer.

Note: Do not create (using malloc/ direct declaration) data buffer. Only use the buffer u8* data in the input.
Creating buffer for management purpose (using malloc/ direct declaration) is 

If returned status is READ_TO_SEND, then all data must have been assembled in u8* frame_data in the manage_result struct
Can safely assume that the pointer passed in will be able to hold all data
*/
manage_result fragment_arrive(fragment_info* info, dl_list* data, u16 data_length);

void clear_memory();

fragment_info create_info(u8 id, u8 number, u16 offset, u16 size);
#endif