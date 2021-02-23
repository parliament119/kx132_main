/**
 * @file tcp.h
 * @author awa
 * @date 20-02-2021
 * 
 * @brief header for tcp.h
 * 
 *  Contains funtion declarations needed for tcp.h
 * 
 */

#ifndef TCP_H
#define TCP_H



/**
 * @brief Establishes TCP-connection to client.
 * 
 * Creates TCP-socket and waits for connection to client.
 * 
 * @note based on: "https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/"" [20.02.2021]
 * 
 * @return true             if TCP-connection could be established
 * @return false            if TCP-connection could not be established
 */
bool tcp_server_init(void);


/**
 * @brief Terminates the TCP-connection.
 * 
 */
void tcp_server_close(void);


/**
 * @brief Sends an array of formatted axes-data over TCP to client.
 * 
 * @note used for streaming mode
 * @attention Originally this was sending the whole array directly. This however often lead to shifting of data inside the array. Sending the data seperately prevents this.
 * 
 * @param xyzFormatted      pointer to array holding signed 16-Bit axis values
 */
void tcp_send(int16_t* xyzFormatted);


/**
 * @brief Sends normalized axis data, size of read samples, trigger index and array of formatted axes-data over TCP to client.
 * 
 * @note used for trigger mode
 * 
 * @param xyzFormatted      pointer to array holding arrays of signed 16-Bit axis values
 * @param triggerInfo       pointer to struct holding info about number of samples and trigger index
 * @param normalizedData    pointer to array hlding normalized axes data
 */
void tcp_send_trig_buffer(int16_t **xyzFormatted, trigger_info_t *triggerInfo, int16_t *normalizedData);


/**
 * @brief Reads incoming TCP data from client.
 * 
 * @warning On client side make sure 256 bytes are sent.
 * 
 * @param data              pointer to string where input should be stored
 */
void tcp_recv(char* data);





#endif //TCP_H
