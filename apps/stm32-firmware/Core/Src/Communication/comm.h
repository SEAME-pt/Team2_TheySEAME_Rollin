/**
 ******************************************************************************
 * @file    comm.h
 * @brief   Communication module header
 * @note    Independent CAN communication thread
 ******************************************************************************
 */

#ifndef COMM_H
#define COMM_H

#include "main.h"

/**
 * @brief Entry point for the communication thread.
 *
 * This function is the thread entry called by ThreadX/RTOS and runs the
 * communication loop handling CAN bus interaction and message processing.
 *
 * @param thread_input Thread input parameter provided by the RTOS (typically a pointer or id). Can be NULL if unused.
 * @return void
 */
void Communication_Thread_Entry(ULONG thread_input);

#endif /* COMM_H */
