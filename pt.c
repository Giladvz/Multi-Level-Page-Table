#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn);
uint64_t page_table_query(uint64_t pt, uint64_t vpn);

uint64_t get_last_node_ifexists(uint64_t pt, uint64_t vpn) {
    uint64_t * numToCompare = {0x1ff000000000, 0xff8000000, 0x7fc0000, 0x3fe00, 0x1ff};
    int i;

    uint64_t * node = phys_to_virt(pt << 12);
    uint64_t index = (vpn & numToCompare[0]) >> 36;

    for (i=1; i < 4; i++) {
        if (node[index] & 1 == 0) {
            return NO_MAPPING;
        }
        else {
            node = phys_to_virt(node[index] - 1);
            index = (vpn & numToCompare[i]) >> (36 - (9 * (i)));
        }
    }
    if (node[index] & 1 == 0) {
        return NO_MAPPING;
    }
    else {
        node = phys_to_virt(node[index] - 1);
        return node;
    }
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    int i;
    uint64_t * numToCompare = [0x1ff000000000, 0xff8000000, 0x7fc0000, 0x3fe00, 0x1ff];
    uint64_t index;
    uint64_t newnode;
    
    uint64_t alreadyExists = page_table_query(pt, vpn);
    index = (vpn & numToCompare[0]) >> 36;
    uint64_t * node = phys_to_virt(pt<<12);

    if (alreadyExists == NO_MAPPING){
        if (ppn == NO_MAPPING) {
            return;
        }
        // need to create
        else {
            for (i = 0; i < 4; i++) {
                if (node[index] & 1 == 0) {
                    // map
                    newnode =  alloc_page_frame() << 12;
                    node[index] = newnode + 1;
                }
                else {
                    node = node[index] -1 ;
                    index = (vpn & numToCompare[i+1]) >> (36 - (9 * (i+1)));

                }
            }
        }
    }
    else {
        // need to delete
        if (ppn == NO_MAPPING){
            node = get_last_node_ifexists(pt,vpn);
            index = vpn & 0x1ff;
            node[index] = node[index] - 1 ;
        }
        else {
            node = get_last_node_ifexists(pt,vpn);
            index = vpn & 0x1ff;
            node[index] = (ppn << 12) + 1;
        }
    }
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t index;
/*
    uint64_t * numToCompare = [0x1ff000000000, 0xff8000000, 0x7fc0000, 0x3fe00, 0x1ff];

    int i = 0;
    for (i=0; i < 4; i++) {
        if (node[index] & 1 == 0) {
            return NO_MAPPING;
        }
        else {
            node = * node[index] >> 12;
            index = vpn & numToCompare[i+1] >> (36 - (9 * (i+1)));
        }
    }
    if (node[index] & 1 == 0) {
        return NO_MAPPING;
    }
    else {
        return( * node[index ] >> 12);
    }
*/
    node = get_last_node_ifexists(pt,vpn);
    if (node == NO_MAPPING){
        return node;
    }
    index = vpn & 0x1ff;
    if (node[index] & 1 == 0) {
        return NO_MAPPING;
    }
    else {
        return node[index] >> 12;
    }
}