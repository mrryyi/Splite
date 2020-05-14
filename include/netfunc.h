#pragma once

#include "pre.h"
#include "glm/glm.hpp"

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

static void write_int32(uint8** buffer, int32 i32) {
    memcpy(*buffer, &i32, sizeof( i32 ));
    *buffer += sizeof( i32 );
};


static void write_uint64(uint8** buffer, uint64 ui64) {
    memcpy(*buffer, &ui64, sizeof( ui64 ));
    *buffer += sizeof( ui64 );
};

static void write_int64(uint8** buffer, int64 i64) {
    memcpy(*buffer, &i64, sizeof( i64 ));
    *buffer += sizeof( i64 );
};

static void write_float32(uint8** buffer, float32 f32) {
    memcpy(*buffer, &f32, sizeof( f32 ));
    *buffer += sizeof( f32 );
};

static void write_float64(uint8** buffer, float64 f64) {
    memcpy(*buffer, &f64, sizeof( f64 ));
    *buffer += sizeof( f64 );
};

static void write_vec3f(uint8** buffer, glm::vec3 vec3f) {
    write_float32(buffer, vec3f.x);
    write_float32(buffer, vec3f.y);
    write_float32(buffer, vec3f.z);
}

static void write_player_input(uint8** buffer, Player::PlayerInput* input) {

    //  [var]       [compressed]  [how much to shift for desired bit position]
    //  forward     .... ...f     << 0
    //  backward    .... ..b.     << 1
    //  up          .... .u..     << 2
    //  down        .... d...     << 3
    //  left        ...l ....     << 4
    //  right       ..r. ....     << 5
    //  jump        .j.. ....     << 6
    //  Last bit unused.

    uint8 compressed_input = 
        (uint8)(input->forward)  ? 1 << 0 : 0  |
        (uint8)(input->backward) ? 1 << 1 : 0  |
        (uint8)(input->up        ? 1 << 2 : 0) |
        (uint8)(input->down      ? 1 << 3 : 0) |
        (uint8)(input->left      ? 1 << 4 : 0) |
        (uint8)(input->right     ? 1 << 5 : 0) |
        (uint8)(input->jump      ? 1 << 6 : 0);
    
    
    write_uint8(buffer, compressed_input);
    
    write_float32(buffer, input->yaw);
    write_float32(buffer, input->pitch);
};

// Not used.
static void write_player_input_verbose(uint8** buffer, Player::PlayerInput* input) {
    write_uint8(buffer, input->forward);
    write_uint8(buffer, input->backward);
    write_uint8(buffer, input->up);
    write_uint8(buffer, input->down);
    write_uint8(buffer, input->left);
    write_uint8(buffer, input->right);
    write_uint8(buffer, input->jump);

    write_float32(buffer, input->yaw);
    write_float32(buffer, input->pitch);
};

static void write_player_state(uint8** buffer, const Player::PlayerState* ps) {
    write_uint32(buffer, ps->id);
    write_vec3f(buffer, ps->position);
    write_vec3f(buffer, ps->velocity);
    write_float32(buffer, ps->yaw);
    write_float32(buffer, ps->pitch);
};

static void write_object(uint8** buffer, const Object* obj) {
    write_uint32(buffer, obj->id);
    write_vec3f(buffer, obj->position);
};

static void read_uint8(uint8** buffer, uint8* ui8) {
    // Set value of dereferenced pointer to value at buffer iterator.
    *ui8 = **buffer;
    ++(*buffer);
};

static void read_uint32(uint8** buffer, uint32* ui32) {
    // *buffer means the address of the value we want to start at.
    // We read bytes equal to the third argument.
    memcpy(ui32, *buffer, sizeof( *ui32 ));
    *buffer += sizeof( *ui32 );
};

static void read_int32(uint8** buffer, int32* i32) {
    // *buffer means the address of the value we want to start at.
    // We read bytes equal to the third argument.
    memcpy(i32, *buffer, sizeof( *i32 ));
    *buffer += sizeof( *i32 );
};

static void read_uint64(uint8** buffer, uint64* ui64) {
    memcpy(ui64, *buffer, sizeof( *ui64 ));
    *buffer += sizeof( *ui64 );
};

static void read_int64(uint8** buffer, int64* i64) {
    memcpy(i64, *buffer, sizeof( *i64 ));
    *buffer += sizeof( *i64 );
};

static void read_float32(uint8** buffer, float32* f32) {
    // *buffer means the address of the value we want to start at.
    // We read bytes equal to the third argument.
    memcpy(f32, *buffer, sizeof( *f32 ));
    *buffer += sizeof( *f32 );
};

static void read_float64(uint8** buffer, float64* f64) {
    memcpy(f64, *buffer, sizeof( *f64 ));
    *buffer += sizeof( *f64 );
};

static void read_vec3f(uint8** buffer, glm::vec3* vec3f) {
    read_float32(buffer, &vec3f->x);
    read_float32(buffer, &vec3f->y);
    read_float32(buffer, &vec3f->z);
}


static void read_player_input(uint8** buffer, Player::PlayerInput* input) {
    uint8 compressed = 0;
    read_uint8(buffer, &compressed);
    
    // Assign values to separate variables from compressed byte.
    input->forward      = compressed & 1;
	input->backward     = compressed & (1 << 1);
	input->up           = compressed & (1 << 2);
	input->down         = compressed & (1 << 3);
	input->left         = compressed & (1 << 4);
    input->right        = compressed & (1 << 5);
    input->jump         = compressed & (1 << 6);

    read_float32(buffer, &input->yaw);
    read_float32(buffer, &input->pitch);
};

// Not used.
static void read_player_input_verbose(uint8** buffer, Player::PlayerInput* input) {
    read_uint8(buffer, &input->forward);
    read_uint8(buffer, &input->backward);
    read_uint8(buffer, &input->up);
    read_uint8(buffer, &input->down);
    read_uint8(buffer, &input->left);
    read_uint8(buffer, &input->right);
    read_uint8(buffer, &input->jump);
    
    read_float32(buffer, &input->yaw);
    read_float32(buffer, &input->pitch);
};

static void read_player_state(uint8** buffer, Player::PlayerState* ps) {
    read_uint32(buffer, &ps->id);
    read_vec3f(buffer, &ps->position);
    read_vec3f(buffer, &ps->velocity);
    read_float32(buffer, &ps->yaw);
    read_float32(buffer, &ps->pitch);
};


static void read_object(uint8** buffer, Object* obj) {
    read_uint32(buffer, &obj->id);
    read_vec3f(buffer, &obj->position);
};

}