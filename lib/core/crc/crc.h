#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stdlib.h>

/** 
 * @brief Calculate CRC8 checksum
 * 
 *  This function computes the CRC8 checksum using the polynomial 0x07
 *  over the provided data buffer. It processes each byte of the input data
 *  and updates the CRC value accordingly.
 * 
 *  @param data Pointer to the data buffer
 *  @param len Length of the data buffer
 *  @return CRC8 checksum
 */
uint8_t crc8(const uint8_t* data, size_t len);

#endif