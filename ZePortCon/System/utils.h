/*
 * utils.h
 *
 * Created: 25.09.2017 11:34:04
 *  Author: kotenko_kg
 */ 


#ifndef UTILS_H_
#define UTILS_H_





#define interrupts() sei()
#define noInterrupts() cli()

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))






#endif /* UTILS_H_ */