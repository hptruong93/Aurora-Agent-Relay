#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fragment_manager.h"


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

// the number of spaces in the below arrays for info and data
#define PACKET_SPACES                    10

u8** checked_out_queue_addr;
fragment_info** info_addr;

u8* get_data_buffer_from_queue(dl_list* input) {
    return input->data_addr;
}

/*
Test program below --> test in a separate file
*/
fragment_info create_info(u8 id, u8 number, u16 offset, u16 size) {
	fragment_info info;
	info.fragment_id = id;
	info.fragment_number = number;
	info.offset	= (u16) offset;
    info.size = size;
	return info;
}



int main(void) {
    
    manage_result fragResults;
    
    //{packet #, fragment #, # of fragments, packet length, fragment offset, , data}
	u8 dataA1[] = {0, 1, 2, 3, 0, 0, 0, 0, 0, 0};
	u8 dataA2[] = {4, 5, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 dataA3[] = {6, 7, 8, 0, 0, 0, 0, 0, 0, 0};
	u8 dataA4[] = {9, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	u8 dataB1[] = {10, 11, 0, 0, 0};
	u8 dataB2[] = {12, 0, 0, 0, 0};
	u8 dataB3[] = {13, 14, 0, 0, 0};    
    
    u8 dataC1[] = {15, 16, 17, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u8 dataC2[] = {19, 20, 21, 22, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u8 dataC3[] = {24, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u8 dataC4[] = {26, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    u8 dataC5[] = {28, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	u8 dataA_assembled[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //For reference only. Not used
	u8 dataB_assembled[] = {10, 11, 12, 13, 14}; //For reference only. Not used
    
    // here is where the buffer addresses for each packet being put together will be stored
    // this is fed into the fragment_arrive array such that it can match each
    // new fragment with any fragments whose buffer locations are stored in the array,
    // add the location of fragments from new packets to the array and remove completed
    // fragments
    checked_out_queue_addr = (u8**) calloc(PACKET_SPACES, sizeof(u8*));
    info_addr = (fragment_info**) calloc(PACKET_SPACES, sizeof(fragment_info*));
    
	fragment_info infoA1 = create_info(222, 1, 0, 10);
	fragment_info infoA2 = create_info(222, 2, 4, 10);
	fragment_info infoA3 = create_info(222, 3, 6, 10);
	fragment_info infoA4 = create_info(222, 4, 9, 10);

	fragment_info infoB1 = create_info(211, 1, 0, 5);
	fragment_info infoB2 = create_info(211, 2, 2, 5);
	fragment_info infoB3 = create_info(211, 3, 3, 5);
    
    fragment_info infoC1 = create_info(225, 1, 0, 15);
    fragment_info infoC2 = create_info(225, 2, 4, 15);
	fragment_info infoC3 = create_info(225, 3, 9, 15);
	fragment_info infoC4 = create_info(225, 4, 11, 15);
    fragment_info infoC5 = create_info(225, 5, 13, 15);
    
    
	// fragResults = fragment_arrive(&infoB2, dataB2, 1);
    // fragResults = fragment_arrive(&infoC1, dataC1, 4);
    fragResults = fragment_arrive(&infoA4, dataA4, 1);
    // fragResults = fragment_arrive(&infoB3, dataB3, 2);
    // fragResults = fragment_arrive(&infoC4, dataC4, 2);
    fragResults = fragment_arrive(&infoA3, dataA3, 3);
    // fragResults = fragment_arrive(&infoC2, dataC2, 5);
    fragResults = fragment_arrive(&infoA1, dataA1, 4);
    // fragResults = fragment_arrive(&infoC5, dataC5, 2);
    fragResults = fragment_arrive(&infoA2, dataA2, 2);
    
    int i = 0;
    
    for (i = 0; i < fragResults.info_address->size; i++){
        printf("%d, ", fragResults.packet_address[i]);
    }
    printf("\n");
    
    // fragResults = fragment_arrive(&infoB1, dataB1, 2); 
    
    // for (i = 0; i < fragResults.info_address->size; i++){
    //     printf("%d, ", fragResults.packet_address[i]);
    // }
    
    // printf("\n");
    
    // fragResults = fragment_arrive(&infoC3, dataC3, 2);
    
    // for (i = 0; i < fragResults.info_address->size; i++){
    //     printf("%d, ", fragResults.packet_address[i]);
    // }
    
    // printf("\n");
}

manage_result fragment_arrive(fragment_info* info, u8* data, u16 data_length) {
    // assemble and store data contained in buffers being read into this function.
    // When a fragment is received and its packet still requires more fragments
    // to be complete return WAITING_FOR_FRAGMENT. Because the function receives a pointer to the
    // addresses, this does not need to be returned, but rather can be accessed directly.
    // When a packet is completely reassembled, move its location in the array to addresses[0],
    // return READY_TO_SEND so that the main program knows that
    // the first element of the addresses array contains the address to the reassembled
    // packet
    
    manage_result frag_result;
    
    if (checked_out_queue_addr[0] == 0) {
        // there have been no addresses added yet, so we input the location
        // of the first data element of the incoming fragment
       
        // we keep track of how many fragments we are waiting for
        // by decrimenting the size. Thus, when size reaches 0, we know
        // that we have received all the fragments of this packet
        info->size -= data_length;
        
        info_addr[0] = info;
        
        checked_out_queue_addr[0] = data;
        
        frag_result.status = WAITING_FOR_FRAGMENT;
                            
        frag_result.packet_address = 0;
        frag_result.info_address = 0;     

        
        //printf("1\n");
    } else {
        // addresses array has been initiated
        
        int i = 0;
        
        for (i = 0; i < PACKET_SPACES; i++){
            // run through the array of packets, seeing if the packet number
            // of a stored fragment matches the one in the incoming fragment
            
            //printf("loop: %d\n", i);
            
            if (checked_out_queue_addr[i] == 0){
                // we've checked all the stored packet info and the new
                // fragment isn't a match to any of them. Thus, we store
                // this fragment in the first 0 element we see
                
                info->size -= data_length;
        
                info_addr[i] = info;
        
                checked_out_queue_addr[i] = data;
                
                frag_result.status = WAITING_FOR_FRAGMENT;
                            
                frag_result.packet_address = 0;
                frag_result.info_address = 0;     
                
                //printf("2\n");
                
                break;
                
            } else {
                // cross reference the first element of the i'th stored fragment
                // (the number of the packet it belongs to) with the first
                // element of the incoming data.
                
                u8 * test_data = (u8*)checked_out_queue_addr[i];
                fragment_info * test_info = (fragment_info*)info_addr[i];
                
                //printf("fragment id: %d\n", info->fragment_id);
                //printf("fragment id test info: %d\n", test_info->fragment_id);
                
                if (info->fragment_id == test_info->fragment_id) {
                    
                    //printf("3: %d\n", i);
                    
                    // we've found a match, therefore we look at the respective
                    // fragment numbers to see which is the lower packet number
                    // and add the data elements from the higher fragment
                    // number to the lower fragment number
                    
                    if (info->fragment_number < test_info->fragment_number) {
                    //printf("4\n");
                    
                    // incoming fragment is the lower fragment number
                    
                        int rel_offset = test_info->offset - info->offset;
                        // find relative offset of stored fragment from new
                        // fragment in order to know where to place data in
                        // new fragment
    
                        int j = 0;
                        
                        // we want to move all the data from the stored
                        // fragment from its beginning to the last element
                        // of what will be the reassembled packet.
                        // Example:
                        //
                        // stored fragment: {1,2,4,1,0,0,1,0,0,0}
                        //
                        // The actual length of the packet data is 15
                        // and the offset of the stored fragment is 8,
                        // thus we take the first (15-8 = 7) elements of 
                        // the stored fragment and move them to the new fragment
                        memmove(data + rel_offset, test_data, info->size - test_info->offset);
                                                
                        if (test_info->size - data_length == 0){
                        // this new fragment contains the last pieces of the
                        // packet that we need. Provide the outside program 
                        // with the data and info addresses of the newly-modified
                        // "data" fragment, remove its address from the array
                        // and shift up each of the addresses after it
                         
                            frag_result.status = READY_TO_SEND;
                            
                            frag_result.packet_address = (u8*)data;
                            checked_out_queue_addr[i] = 0;
                            
                            int k = i;
                            
                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != 0){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = 0;
                                
                                k++;
                            }
            
                            frag_result.info_address = (fragment_info*)info;
                            info_addr[i] = 0;    
                            
                            k = i;
                            
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != 0){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = 0;
                                
                                k++;
                            }
                        } else {      
                            // decrement the size in info for management 
                            // purposes and store the data and info addresses
                            // before letting the outside program know the result
                            
                            info->size = test_info->size - data_length;                            
                            
                            checked_out_queue_addr[i] = data;
                            info_addr[i] = info;
                            
                            frag_result.status = WAITING_FOR_FRAGMENT;
                            
                            frag_result.packet_address = 0;
                            frag_result.info_address = 0;                            
                            
                        }
                        
                        
                        
                    } else {
                        //printf("5\n");
                        int rel_offset = info->offset - test_info->offset;
                        // int j = 0;
                        // for (j = 0; j < (data_length); j++){
                        //     test_data[j + rel_offset] = data[j];
                        // }
                        memmove(test_data + rel_offset, data, data_length);
                        
                        if (test_info->size - data_length == 0){                   
                            test_info->size = info->size;
                            frag_result.status = READY_TO_SEND;
                            frag_result.packet_address = (u8*)test_data;
                            checked_out_queue_addr[i] = 0;
                            
                            int k = i;
                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != 0){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = 0;
                                k++;
                            }                            
                            
                            frag_result.info_address = (fragment_info*)test_info;  
                            info_addr[i] = 0;                        
                            
                            k = i;
                            
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != 0){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = 0;
                                
                                k++;
                            }
                            
                        } else {
                            test_info->size -= data_length;
                            
                            frag_result.status = WAITING_FOR_FRAGMENT;
                            
                            frag_result.packet_address = 0;
                            frag_result.info_address = 0;                            
                        }
                    }
                    break;
                }
            }
        }
    }
    return frag_result;
}