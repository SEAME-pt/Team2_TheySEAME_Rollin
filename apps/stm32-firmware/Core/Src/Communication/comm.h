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

/* Communication Thread Entry Point */
void Communication_Thread_Entry(ULONG thread_input);

#endif /* COMM_H */
