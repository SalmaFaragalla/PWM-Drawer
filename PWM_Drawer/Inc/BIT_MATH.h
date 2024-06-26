/**
 ******************************************************************************
 * @file    BIT_MATH.h
 * @author  Salma Faragalla
 * @brief   Header file for bit manipulation macros.
 ******************************************************************************
 */
#ifndef BIT_MATH_H_
#define BIT_MATH_H_

#define SET_BIT(reg, bit) (reg |= ( 0x01UL<< bit))
#define CLR_BIT(reg, bit) (reg &= (~(0x01UL << bit)))
#define TOG_BIT(reg, bit) (reg ^= (0x01UL << bit))
#define GET_BIT(reg, bit) ((reg >> bit) & (0x01UL))


#endif /* BIT_MATH_H_ */
