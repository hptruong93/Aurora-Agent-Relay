#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tins/tins.h>
#include "fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"
#include "../revised_version/util.h"


// the number of spaces in the below arrays for info and data
#define PACKET_SPACES                    20


using namespace Tins;

/***************************************Type definition *****************************************************************/

// typedef struct {
//     uint8_t* data_addr;
// } dl_list; //Yet another wrap around for abstraction purpose (compatible with WARP board implementation)

/***************************************Forward declaration ************************************************************/

/*
Return status READY_TO_SEND if all fragments have arrived (including this one)
Return status WAITING_FOR_FIRST_FRAGMENT if at least a fragment (including this one) has arrived, but the first fragment has not arrived
Return status WAITING_FOR_FRAGMENT if at least a fragment (including this one) has arrived, and the first fragment has arrived

Return frame_data is the pointer to the data buffer that is no longer used by the manager. Return null if nothing to return.
When completely reassembled, return pointer to buffer containing all data.

At all time, there should be only 1 frame buffer for each fragment id. This means
    If the fragment ID does not have a data buffer, use the pointer passed in as the buffer.
    If the fragment ID already had a data buffer, copy the newly arrived data into the buffer.

Note: Do not create (using malloc/ direct declaration) data buffer. Only use the buffer uint8_t* data in the input.
Creating buffer for management purpose (using malloc/ direct declaration) is 

If returned status is READ_TO_SEND, then all data must have been assembled in uint8_t* frame_data in the receive_result struct
Can safely assume that the pointer passed in will be able to hold all data
*/
void fragment_arrive(Tins::WARP_protocol::WARP_fragment_struct* info, uint8_t* data, uint32_t data_length, receive_result* result);

/**********************************************************************************************************************/
//Initialization
// here is where the buffer addresses for each packet being put together will be stored
    // this is fed into the fragment_arrive array such that it can match each
    // new fragment with any fragments whose buffer locations are stored in the array,
    // add the location of fragments from new packets to the array and remove completed
    // fragments
uint8_t packet_addresses[PACKET_SPACES][2048];
WARP_protocol::WARP_fragment_struct* info_addr[PACKET_SPACES];

/**********************************************************************************************************************/

void initialize_receiver() {
    uint8_t i = 0;
    for (i = 0; i < PACKET_SPACES; i++) {
        info_addr[i] = NULL;
    }
}

// uint8_t* get_data_buffer_from_queue(dl_list* input) {
//     return input->data_addr;
// }

void update_database(uint8_t index, WARP_protocol::WARP_fragment_struct* new_info) {
    info_addr[index] = new_info;
}

void purge_database() {
    uint8_t i;
    for (i = 0; i < PACKET_SPACES; i++) {
        update_database(i, NULL);
    }
}

WARP_protocol::WARP_fragment_struct* create_info(uint8_t id, uint8_t number, uint16_t byte_offset, uint8_t total_number_fragment) {
    WARP_protocol::WARP_fragment_struct* info = (WARP_protocol::WARP_fragment_struct*) malloc(sizeof(WARP_protocol::WARP_fragment_struct));

    info->id = id;
    info->fragment_number = number;
    info->total_number_fragment = total_number_fragment;
    info->byte_offset = (uint16_t) byte_offset;
    return info;
}

uint8_t find_packet_id(uint8_t id) {
    uint8_t i;
    uint8_t output = PACKET_SPACES; //Intentionally put invalid value so that if there is no space left the caller would know
    for (i = 0; i < PACKET_SPACES; i++) {
        WARP_protocol::WARP_fragment_struct* current = info_addr[i];
        if (current != NULL) {
            if (current->id == id) {
                return i;
            }
        } else {
            output = i;
        }
    }
    return output;
}

void assemble_result(uint8_t status, uint8_t* packet_address, WARP_protocol::WARP_fragment_struct* info_address, receive_result* result) {
    result->status = status;
    result->packet_address = packet_address;
    result->info_address = info_address;
}

void packet_receive(uint8_t* packet_buffer, uint32_t data_length, receive_result* result) {
    uint8_t id = packet_buffer[FRAGMENT_ID_INDEX];
    uint8_t fragment_number = packet_buffer[FRAGMENT_NUMBER_INDEX];
    uint8_t total_number_fragment = packet_buffer[FRAGMENT_TOTAL_NUMBER_INDEX];
    uint16_t byte_offset = ((uint16_t)(packet_buffer[FRAGMENT_BYTE_OFFSET_MSB] << 8)) + (packet_buffer[FRAGMENT_BYTE_OFFSET_LSB]);
    WARP_protocol::WARP_fragment_struct* fragment_info = create_info(id, fragment_number, byte_offset, total_number_fragment);

    // dl_list* wrap_around = (dl_list*) std::malloc(sizeof(dl_list));
    // wrap_around->data_addr = packet_buffer + FRAGMENT_INFO_LENGTH;
    fragment_arrive(fragment_info, packet_buffer + FRAGMENT_INFO_LENGTH, data_length - FRAGMENT_INFO_LENGTH, result);
}

void fragment_arrive(WARP_protocol::WARP_fragment_struct* new_info, uint8_t* new_packet, uint32_t data_length, receive_result* result) {
    // assemble and store data contained in buffers being read into this function.
    // When a fragment is received and its packet still requires more fragments
    // to be complete return WAITING_FOR_FRAGMENT. Because the function receives a pointer to the
    // addresses, this does not need to be returned, but rather can be accessed directly.
    // When a packet is completely reassembled, move its location in the array to addresses[0],
    // return READY_TO_SEND so that the main program knows that
    // the first element of the addresses array contains the address to the reassembled
    // packet
    printf("ID is %d, number is %d, total is %d, offset is %d and length is %d\n", new_info->id, new_info->fragment_number, new_info->total_number_fragment, new_info->byte_offset, data_length);

    if (new_info->total_number_fragment == 1) {
        //Bounce back immediately
        new_info->length = data_length;
        assemble_result(READY_TO_SEND, new_packet, new_info, result);
    } else {
        uint8_t found = find_packet_id(new_info->id);

        if (found == PACKET_SPACES) {//Database full...
            printf("Database is full!!! Something went wrong???\n");
            purge_database();
            //Try again
            fragment_arrive(new_info, new_packet, data_length, result);
        } else if (info_addr[found] == NULL) {
            //This is the first fragment of this given ID
            //Adjust offset
            memmove(packet_addresses[found] + new_info->byte_offset, new_packet, data_length);

            //Put this into memory
            update_database(found, new_info);

            //Update
            new_info->length = data_length;
            new_info->total_number_fragment--;
            new_info->byte_offset = 0; //Not really necessary

            //Assemble output
            assemble_result(WAITING_FOR_FRAGMENT, NULL, NULL, result);
        } else {
            //A previous packets have already arrived
            WARP_protocol::WARP_fragment_struct* previous_info = info_addr[found];
            uint8_t* previous_packet = packet_addresses[found];

            //Copy new data
            memmove(previous_packet + new_info->byte_offset, new_packet, data_length);

            //Update fragment information
            previous_info->length += data_length;
            previous_info->total_number_fragment--;

            //Check if last fragment and assemble
            if (previous_info->total_number_fragment == 0) {
                // uint8_t* output_data = (uint8_t*) malloc(previous_info->total_number_fragment * sizeof(uint8_t));
                // memcpy(output_data, previous_packet, previous_info->total_number_fragment);
                //Dangerous??? Perhaps uncomment the line above and return output_data instead??
                assemble_result(READY_TO_SEND, previous_packet, previous_info, result);
                update_database(found, NULL);
            } else {
                assemble_result(WAITING_FOR_FRAGMENT, NULL, NULL, result);
            }

            //Clean up
            free(new_info);
        }
    }
}

/*
Test program below. Ignore when integrated
*/

int maaain(void) {
    
    receive_result fragResults;
    
    //{packet #, fragment #, # of fragments, packet length, fragment byte_offset, , data}
    uint8_t data_buff_A1[] = {220, 0, 4, 0, 0,      0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A2[] = {220, 1, 4, 0, 4,      4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A3[] = {220, 2, 4, 0, 6,      6, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A4[] = {220, 3, 4, 0, 9,      9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t data_buff_B1[] = {221, 0, 2, 0, 0,      0, 1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_B2[] = {221, 1, 2, 0, 7,      7, 8, 9, 10, 11, 12, 0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t* bigC1 = (uint8_t*) malloc(2048);
    uint8_t* bigC2 = (uint8_t*) malloc(2048);

    uint16_t le = 5;
    for (; le < 505; le++) {
        bigC1[le] = le;
        bigC2[le] = 65000 - le;
    }
    bigC1[0] = 112;
    bigC1[1] = 0;
    bigC1[2] = 2;
    bigC1[3] = 0;
    bigC1[4] = 0;

    bigC2[0] = 112;
    bigC2[1] = 1;
    bigC2[2] = 2;
    bigC2[3] = 1;
    bigC2[4] = 244;

    uint8_t repeat = 0;

    // while (1 == 1) {
    //     if (repeat == 0) {
            // packet_receive(data_buff_B2, 5 + 6, &fragResults);
            // packet_receive(data_buff_B1, 5 + 7, &fragResults);
            // repeat = 1;
        // } else {
            packet_receive(data_buff_A1, 5 + 4, &fragResults);
            packet_receive(data_buff_A3, 5 + 3, &fragResults);
            packet_receive(data_buff_A2, 5 + 2, &fragResults);
            packet_receive(data_buff_A4, 5 + 1, &fragResults);
            // repeat = 0;
        // }

    // for (; repeat < 255; repeat++) {
    // packet_receive(bigC1, 5 + 500, &fragResults);
    // packet_receive(bigC2, 5 + 500, &fragResults);}

        if (fragResults.status == READY_TO_SEND) {
            uint8_t i;
            uint8_t* buffer = fragResults.packet_address;
            printf("Length is %d\n", fragResults.info_address->length);
            for (i = 0; i < fragResults.info_address->length; i++) {
                printf("%d-", buffer[i]);
            }

        }
        printf("\n");
    // }
}


