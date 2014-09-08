#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tins/tins.h>
#include "fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"


// the number of spaces in the below arrays for info and data
#define PACKET_SPACES                    20
  

using namespace Tins;

/***************************************Type definition *****************************************************************/

typedef struct {
    uint8_t* data_addr;
} dl_list; //Yet another wrap around for abstraction purpose (compatible with WARP board implementation)

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
receive_result* fragment_arrive(Tins::WARP_protocol::WARP_fragment_struct* info, dl_list* data, uint32_t data_length);

/**********************************************************************************************************************/
//Initialization
// here is where the buffer addresses for each packet being put together will be stored
    // this is fed into the fragment_arrive array such that it can match each
    // new fragment with any fragments whose buffer locations are stored in the array,
    // add the location of fragments from new packets to the array and remove completed
    // fragments
dl_list** checked_out_queue_addr = (dl_list**) calloc(PACKET_SPACES, sizeof(dl_list*));;
WARP_protocol::WARP_fragment_struct** info_addr = (WARP_protocol::WARP_fragment_struct**) calloc(PACKET_SPACES, sizeof(WARP_protocol::WARP_fragment_struct*));

/**********************************************************************************************************************/

uint8_t* get_data_buffer_from_queue(dl_list* input) {
    return input->data_addr;
}

WARP_protocol::WARP_fragment_struct* create_info(uint8_t id, uint8_t number, uint16_t byte_offset, uint8_t total_number_fragment) {
    WARP_protocol::WARP_fragment_struct* info = (WARP_protocol::WARP_fragment_struct*) calloc(sizeof(WARP_protocol::WARP_fragment_struct), 0);
    info->id = id;
    info->fragment_number = number;
    info->total_number_fragment = total_number_fragment;
    info->byte_offset = (uint16_t) byte_offset;
    return info;
}

receive_result* packet_receive(uint8_t* packet_buffer, uint32_t data_length) {
    uint8_t id = packet_buffer[FRAGMENT_ID_INDEX];
    uint8_t fragment_number = packet_buffer[FRAGMENT_NUMBER_INDEX];
    uint8_t total_number_fragment = packet_buffer[FRAGMENT_TOTAL_NUMBER_INDEX];
    uint16_t byte_offset = ((uint16_t)(packet_buffer[FRAGMENT_BYTE_OFFSET_MSB] << 8)) & (packet_buffer[FRAGMENT_BYTE_OFFSET_LSB]);
    WARP_protocol::WARP_fragment_struct* fragment_info = create_info(id, fragment_number, byte_offset, total_number_fragment);

    // printf("fragment id is %d\n", id);
    // printf("fragment fragment_number is %d\n", fragment_number);
    // printf("fragment total_number_fragment is %d\n", total_number_fragment);
    // printf("fragment byte_offset is %d\n", byte_offset);

    dl_list wrap_around;
    wrap_around.data_addr = packet_buffer + FRAGMENT_INFO_LENGTH;
    receive_result* output = fragment_arrive(fragment_info, &wrap_around, data_length - FRAGMENT_INFO_LENGTH);

    return output;
}

receive_result* fragment_arrive(WARP_protocol::WARP_fragment_struct* info, dl_list* checked_out_queue, uint32_t data_length) {
    // assemble and store data contained in buffers being read into this function.
    // When a fragment is received and its packet still requires more fragments
    // to be complete return WAITING_FOR_FRAGMENT. Because the function receives a pointer to the
    // addresses, this does not need to be returned, but rather can be accessed directly.
    // When a packet is completely reassembled, move its location in the array to addresses[0],
    // return READY_TO_SEND so that the main program knows that
    // the first element of the addresses array contains the address to the reassembled
    // packet
    uint8_t* data = get_data_buffer_from_queue(checked_out_queue);

    receive_result* frag_result = (receive_result*) calloc(sizeof(receive_result), 0);
    
    // printf("ID is %d, number is %d, offset is %d and length is %d\n", info->id, info->fragment_number, info->byte_offset, data_length);

    if (info->total_number_fragment == 1) {
        frag_result->status = READY_TO_SEND;
        frag_result->packet_address = get_data_buffer_from_queue(checked_out_queue);

        info->length = data_length;
        frag_result->info_address = (WARP_protocol::WARP_fragment_struct*)info;

        // printf("1\n");
    } else if (checked_out_queue_addr[0] == 0) {
        // there have been no addresses added yet, so we input the location
        // of the first data element of the incoming fragment
       
        // we keep track of how many fragments we are waiting for
        // by decrimenting the size. Thus, when size reaches 0, we know
        // that we have received all the fragments of this packet
        
        
        info->total_number_fragment--;
        info->length = data_length;
        
        info_addr[0] = info;
        
        checked_out_queue_addr[0] = checked_out_queue;
        
        frag_result->status = WAITING_FOR_FRAGMENT;
                            
        frag_result->packet_address = 0;
        frag_result->info_address = 0;     

    } else {
        // addresses array has been initiated
        uint8_t i = 0;
        
        for (i = 0; i < PACKET_SPACES; i++) {
            // run through the array of packets, seeing if the packet number
            // of a stored fragment matches the one in the incoming fragment
            // printf("loop: %d\n", i);
            
            if (checked_out_queue_addr[i] == NULL){
                // we've checked all the stored packet info and the new
                // fragment isn't a match to any of them. Thus, we store
                // this fragment in the first 0 element we see
                
                info->total_number_fragment--;
                info->length = data_length;
        
                info_addr[i] = info;
        
                checked_out_queue_addr[i] = checked_out_queue;
                
                frag_result->status = WAITING_FOR_FRAGMENT;
                            
                frag_result->packet_address = NULL;
                frag_result->info_address = NULL;     
                
                // printf("2\n");
                
                break;
                
            } else {
                // cross reference the first element of the i'th stored fragment
                // (the number of the packet it belongs to) with the first
                // element of the incoming data.
                
                dl_list* test_data = checked_out_queue_addr[i];
                WARP_protocol::WARP_fragment_struct* test_info = (WARP_protocol::WARP_fragment_struct*)info_addr[i];
                
                // printf("fragment id and number: %d %d\n", info->id, info->fragment_number);
                // printf("fragment id test info and number: %d %d\n", test_info->id, test_info->fragment_number);
                
                if (info->id == test_info->id) {
                    uint8_t* test_data_buffer = get_data_buffer_from_queue(test_data);
                    // printf("3: %d\n", i);

                    // we've found a match, therefore we look at the respective
                    // fragment numbers to see which is the lower packet number
                    // and add the data elements from the higher fragment
                    // number to the lower fragment number
                    
                    if (info->fragment_number < test_info->fragment_number) {
                    // printf("4 Copying to new buffer\n");
                    
                    // incoming fragment is the lower fragment number
                    
                        uint16_t rel_offset = test_info->byte_offset - info->byte_offset;
                        // find relative byte_offset of stored fragment from new
                        // fragment in order to know where to place data in
                        // new fragment
                        memmove(data + rel_offset, test_data_buffer, test_info->length);
                        info->length = test_info->byte_offset - info->byte_offset + test_info->length;

                        if (test_info->total_number_fragment - 1 == 0) {
                        // this new fragment contains the last pieces of the
                        // packet that we need. Provide the outside program 
                        // with the data and info addresses of the newly-modified
                        // "data" fragment, remove its address from the array
                        // and shift up each of the addresses after it
                         
                            frag_result->status = READY_TO_SEND;
                            
                            frag_result->packet_address = get_data_buffer_from_queue(checked_out_queue);
                            checked_out_queue_addr[i] = NULL;
                            
                            
                            uint8_t k = i;
                            
                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != NULL){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = NULL;
                                
                                k++;
                            }
            
                            frag_result->info_address = (WARP_protocol::WARP_fragment_struct*)info;

                            k = i;
                            free(info_addr[i]);
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != NULL){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = NULL;
                                
                                k++;
                            }
                        } else {      
                            // decrement the size in info for management 
                            // purposes and store the data and info addresses
                            // before letting the outside program know the result
                            info->total_number_fragment = test_info->total_number_fragment - 1;                   
                            
                            frag_result->status = WAITING_FOR_FRAGMENT;
                            frag_result->packet_address = get_data_buffer_from_queue(checked_out_queue_addr[i]);
                            frag_result->info_address = NULL;

                            checked_out_queue_addr[i] = checked_out_queue;
                            info_addr[i] = info;
                        }
                    } else {
                        // printf("5 Copying to old buffer\n");
                        uint16_t rel_offset = info->byte_offset - test_info->byte_offset;
                        memmove(test_data_buffer + rel_offset, data, data_length);
                        
                        if (info->byte_offset - test_info->byte_offset + data_length > test_info->length) {
                            test_info->length = info->byte_offset - test_info->byte_offset + data_length;
                        }

                        if (test_info->total_number_fragment - 1 == 0){                   
                            test_info->total_number_fragment = info->total_number_fragment;
                            frag_result->status = READY_TO_SEND;
                            frag_result->packet_address = get_data_buffer_from_queue(test_data);
                            checked_out_queue_addr[i] = NULL;
                            
                            

                            uint8_t k = i;
                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != NULL){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = NULL;
                                k++;
                            }                     
                            
                            frag_result->info_address = (WARP_protocol::WARP_fragment_struct*)test_info;  
                            
                            k = i;
                            free(info_addr[i]);
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != NULL){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = NULL;
                                
                                k++;
                            }
                            
                        } else {
                            test_info->total_number_fragment--;
                            frag_result->status = WAITING_FOR_FRAGMENT;

                            frag_result->packet_address = get_data_buffer_from_queue(checked_out_queue);
                            frag_result->info_address = NULL;                            
                        }
                    }
                    break;
                }
            }
        }
    }
    return frag_result;
}

/*
Test program below. Ignore when integrated
*/

int maain(void) {
    
    receive_result* fragResults;
    
    //{packet #, fragment #, # of fragments, packet length, fragment byte_offset, , data}
    uint8_t data_buff_A1[] = {0, 1, 2, 3, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A2[] = {4, 5, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A3[] = {6, 7, 8, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_A4[] = {9, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    dl_list dataA1, dataA2, dataA3, dataA4, dataB1, dataB2, dataB3, dataC1, dataC2, dataC3, dataC4, dataC5;
    dl_list dataD1;
    dataA1.data_addr = data_buff_A1;
    dataA2.data_addr = data_buff_A2;
    dataA3.data_addr = data_buff_A3;
    dataA4.data_addr = data_buff_A4;

    uint8_t data_buff_B1[] = {10, 11, 0, 0, 0};
    uint8_t data_buff_B2[] = {12, 0, 0, 0, 0};
    uint8_t data_buff_B3[] = {13, 14, 0, 0, 0};    
    dataB1.data_addr = data_buff_B1;
    dataB2.data_addr = data_buff_B2;
    dataB3.data_addr = data_buff_B3;
    
    uint8_t data_buff_C1[] = {15, 16, 17, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_C2[] = {19, 20, 21, 22, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_C3[] = {24, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_C4[] = {26, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t data_buff_C5[] = {28, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    dataC1.data_addr = data_buff_C1;
    dataC2.data_addr = data_buff_C2;
    dataC3.data_addr = data_buff_C3;
    dataC4.data_addr = data_buff_C4;
    dataC5.data_addr = data_buff_C5;

    uint8_t data_buff_D1[] = {65, 65, 65, 65};
    dataD1.data_addr = data_buff_D1;

    
    WARP_protocol::WARP_fragment_struct* infoA1 = create_info(222, 1, 0, 4);
    WARP_protocol::WARP_fragment_struct* infoA2 = create_info(222, 2, 4, 4);
    WARP_protocol::WARP_fragment_struct* infoA3 = create_info(222, 3, 6, 4);
    WARP_protocol::WARP_fragment_struct* infoA4 = create_info(222, 4, 9, 4);

    WARP_protocol::WARP_fragment_struct* infoB1 = create_info(211, 1, 0, 3);
    WARP_protocol::WARP_fragment_struct* infoB2 = create_info(211, 2, 2, 3);
    WARP_protocol::WARP_fragment_struct* infoB3 = create_info(211, 3, 3, 3);
    
    WARP_protocol::WARP_fragment_struct* infoC1 = create_info(225, 1, 0, 5);
    WARP_protocol::WARP_fragment_struct* infoC2 = create_info(225, 2, 4, 5);
    WARP_protocol::WARP_fragment_struct* infoC3 = create_info(225, 3, 9, 5);
    WARP_protocol::WARP_fragment_struct* infoC4 = create_info(225, 4, 11, 5);
    WARP_protocol::WARP_fragment_struct* infoC5 = create_info(225, 5, 13, 5);
    
    WARP_protocol::WARP_fragment_struct* infoD1 = create_info(220, 1, 0, 1);

    
     fragResults = fragment_arrive(infoB3, &dataB3, 2);
     fragResults = fragment_arrive(infoB2, &dataB2, 1);
    // fragResults = fragment_arrive(infoA4, &dataA4, 1);
    // fragResults = fragment_arrive(infoA3, &dataA3, 3);
    // fragResults = fragment_arrive(infoC4, &dataC4, 2);
    // fragResults = fragment_arrive(infoC5, &dataC5, 2);
    // fragResults = fragment_arrive(infoC1, &dataC1, 4);
    // fragResults = fragment_arrive(infoC2, &dataC2, 5);
    // fragResults = fragment_arrive(infoA1, &dataA1, 4);
    // fragResults = fragment_arrive(infoA2, &dataA2, 2);

    
    int i = 0;
    // printf("Packet length = %d\n", fragResults->info_address->length);
    // for (i = 0; i < fragResults->info_address->length; i++) {
    //     printf("%d, ", fragResults->packet_address[i]);
    // }
    // printf("\n");
    
    
     fragResults = fragment_arrive(infoB1, &dataB1, 2); 
     printf("Packet length = %d\n", fragResults->info_address->length);
     for (i = 0; i < (int)(fragResults->info_address->length); i++){
         printf("%d, ", fragResults->packet_address[i]);
     }
     printf("\n");
    
    
    // fragResults = fragment_arrive(infoC3, &dataC3, 2);
    // printf("Packet length = %d\n", fragResults->info_address->length);
    // for (i = 0; i < fragResults->info_address->length; i++){
    //     printf("%d, ", fragResults->packet_address[i]);
    // }
    // printf("\n");

    fragResults = fragment_arrive(infoD1, &dataD1, 4);
    printf("Packet length = %d\n", fragResults->info_address->length);
    for (i = 0; i < (int)(fragResults->info_address->length); i++){
        printf("%d, ", fragResults->packet_address[i]);
    }
    printf("\n");
}


