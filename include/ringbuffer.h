/**
 * @file ringbuffer.h
 * @author awa
 * @date 19-02-2021
 * 
 * @brief Header for ringbuffer.c
 * 
 *  function declarations needed for ringbuffer.
 * 
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#include <trigger.h>

/// struct for ringbuffer
typedef struct{
    int16_t* buffer;    ///< buffer containing actual data
    uint32_t size;      ///< size of buffer
    uint32_t index;     ///< current index for writing
    uint32_t modulo;    ///< modulo-value for wrapping
} ringbuffer_t;


/**
 * @brief Initializes passed ringbuffer.
 * 
 * @param rb            pointer to ringbuffer
 * @param buffer        pointer to buffer to hold the actual data
 * @param size          size of buffer
 * @return true         if succes
 * @return false        if error
 */
bool rb_init(ringbuffer_t *rb, int16_t *buffer, uint32_t size);


/**
 * @brief Initializes an array of 3 ringbuffers.
 * 
 *  Function calls rb_init(...) 3 times
 * 
 * @param rb            pointer to array of 3 ringbuffers
 * @param buffer        pointer to 3 buffers to hold the actual data
 * @param size          size of buffers
 * @return true         if succes
 * @return false        if error
 */
bool rb_xyz_init(ringbuffer_t *rb, int16_t **buffer, uint32_t size);


/**
 * @brief Resets the Index of a ringbuffer to 0.
 * 
 * @param rb            pointer to ringbuffer
 */
void rb_reset(ringbuffer_t *rb);


/**
 * @brief Returns size of ringbuffer
 * 
 * @param rb            pointer to ringbuffer
 * @return              uint32_t size of passed ringbuffer
 */
uint32_t rb_size(ringbuffer_t *rb);


/**
 * @brief Pushes one value onto ringbuffer.
 * 
 * @param rb            pointer to ringbuffer
 * @param data          value to push onto ringbuffer
 * @return              local index so rb_read_chunk() knows which values to read in case of trigger-event
 */
uint32_t rb_push(ringbuffer_t* rb, int16_t data);


/**
 * @brief Reads chunk from ringbuffer.
 * 
 * @note Borders of chunk are based on triggerInfo->triggerIndex and triggerInfo->samplesBeforeTrig.
 * 
 * @param rb            pointer to ringbuffer
 * @param buffer        pointer to buffer where read data should be saved
 * @param triggerInfo   struct containing needed information about triggerIndex, numberOfSamples to be read and samplesBeforeTrig
 */
void rb_read_chunk(ringbuffer_t *rb, int16_t *buffer, trigger_info_t *triggerInfo);


#endif // RINGBUFFER_H_
