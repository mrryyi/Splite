#pragma once

#include "pre.h"

namespace Network
{

static void write_uint8(uint8** buffer, uint8 ui8) {
    // Okay, so, we've a pointer to a uint8 array. (uint8** buffer)
    // In the implementation, we are sending an iterator acting as
    // the "reader" which is what we are increasing.

    // Set the value at reader iterator
    **buffer = ui8;
    // Increase the iterator
    ++(*buffer);
};

static void write_uint32(uint8** buffer, uint32 ui32) {
    // Same concept as write_uint8(), except we cannot do do a simple
    // assignment of the content to the spot. We need to define how to 
    // copy over the data, which memcpy does for us.
    memcpy(*buffer, &ui32, sizeof( ui32 ));
    *buffer += sizeof( ui32 );
};

static void write_uint64(uint8** buffer, uint64 ui64) {
    // ditto write_uint32()
    memcpy(*buffer, &ui64, sizeof( ui64 ));
    *buffer += sizeof( ui64 );
};

static void write_player_input(uint8** buffer, Player::PlayerInput* input) {

    //  [var]  [compressed]  [how much to shift for desired bit position]
    //  up     .... ...u     << 0
    //  down   .... ..d.     << 1
    //  left   .... .l..     << 2
    //  right  .... r...     << 3
    //  jump   ...j ....     << 4
    
    uint8 compressed_input = 
        (uint8)(input->up   ? 1 << 0 : 0) |
        (uint8)(input->down ? 1 << 1 : 0) |
        (uint8)(input->left ? 1 << 2 : 0) |
        (uint8)(input->right? 1 << 3 : 0) |
        (uint8)(input->jump ? 1 << 4 : 0);
    
    write_uint8(buffer, compressed_input);
    
};

static void read_uint8(uint8** buffer, uint8* ui8) {
    // Set value of dereferenced pointer to value at buffer iterator.
    *ui8 = **buffer;
    ++(*buffer);
};

static void read_uint32(uint8** buffer, uint32* ui32) {
    // *buffer means the address of the value we want to start at.
    // We read bytes equal to the third argument.
    memcpy(ui32, *buffer, sizeof( ui32 ));
    *buffer += sizeof( ui32 );
};

static void read_uint64(uint8** buffer, uint64* ui64) {
    memcpy(ui64, *buffer, sizeof( ui64 ));
    *buffer += sizeof( ui64 );
};

static void read_player_input(uint8** buffer, Player::PlayerInput* input) {
    uint8 compressed;
    read_uint8(buffer, &compressed);
    
    // Assign values to separate variables from compressed byte.
    input->up       = compressed & 1;
	input->down     = compressed & (1 << 1);
	input->left     = compressed & (1 << 2);
	input->right    = compressed & (1 << 3);
	input->jump     = compressed & (1 << 4);
}; 

}