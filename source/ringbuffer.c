/**
 * @file ringbuffer.c
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Contains functions for ringbuffer.
 * 
 * @note A "standard" rb_pop() was not provided as it is not needed for use with the current implemenation.
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <ringbuffer.h>
#include <macros_kx132.h>
#include <trigger.h>


bool rb_init(ringbuffer_t *rb, int16_t *buffer, uint32_t size){

    rb->buffer  = buffer;
    rb->size    = size;
    rb->modulo  = rb->size - 1; /// because of power of two
    rb->index   = 0;

    if((rb->size & (rb->size - 1)) != 0) {
        printf("[ringbuffer][error] size of ringbuffer is not power of two\n");
        return false;
    }

    return true;
}


bool rb_xyz_init(ringbuffer_t *rb, int16_t **buffer, uint32_t size){

    if(!rb_init(&rb[X_INDEX], buffer[X_INDEX], size)){
        printf("[ringbuffer][error] Ringbuffer X could not be initialized.\n");
        return false;
    }
    if(!rb_init(&rb[Y_INDEX], buffer[Y_INDEX], size)){
        printf("[ringbuffer][error] Ringbuffer Y could not be initialized.\n");
        return false;
    }
    if(!rb_init(&rb[Z_INDEX], buffer[Z_INDEX], size)){
        printf("[ringbuffer][error] Ringbuffer Z could not be initialized.\n");
        return false;
    }

    return true;
}


void rb_reset(ringbuffer_t *rb){
    rb->index = 0;
}

uint32_t rb_size(ringbuffer_t *rb){
    return rb->size;
}


uint32_t rb_push(ringbuffer_t* rb, int16_t data){

    uint32_t localIndex     = rb->index & rb->modulo;
    rb->buffer[localIndex]  = data;

    rb->index++;

    return localIndex;
}


void rb_read_chunk(ringbuffer_t *rb, int16_t *buffer, trigger_info_t *triggerInfo){

    uint32_t sumOfSamples = triggerInfo->numberOfSamples;

    if(sumOfSamples > rb_size(rb)){
        printf("[ringbuffer][warning] Sum of requested samples exceeds size of buffer.\n");
        sumOfSamples = rb_size(rb);
    }

    uint32_t bufferIndex    = 0;
    uint32_t localIndex     = (triggerInfo->triggerIndex - triggerInfo->samplesBeforeTrig) & rb->modulo;

    while(bufferIndex < sumOfSamples){
        buffer[bufferIndex] = rb->buffer[localIndex];

        localIndex++;
        localIndex &= rb->modulo;
        bufferIndex++;
    }
}