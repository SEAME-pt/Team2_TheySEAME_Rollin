/**
 * @file SEGGER_SYSVIEW_ThreadX.c
 * @brief SEGGER SystemView <-> ThreadX execution profile bridge.
 *
 * Implements the four hooks that ThreadX calls when
 * TX_ENABLE_EXECUTION_CHANGE_NOTIFY is defined:
 *   _tx_execution_thread_enter  — a thread starts running
 *   _tx_execution_thread_exit   — a thread stops running (preempted / blocked)
 *   _tx_execution_isr_enter     — ISR starts
 *   _tx_execution_isr_exit      — ISR ends
 *
 * Also provides sysview_register_threads() to register all application
 * threads with SystemView so they appear with names in the timeline.
 */

#include "SEGGER_SYSVIEW.h"
#include "tx_api.h"
#include "tx_thread.h"

/* ------------------------------------------------------------------ */
/*  Initialization hook (called from _tx_initialize_kernel_enter)      */
/* ------------------------------------------------------------------ */

/**
 * Called once during ThreadX kernel initialisation when
 * TX_ENABLE_EXECUTION_CHANGE_NOTIFY is defined.
 */
void _tx_execution_initialize(void)
{
    /* Nothing extra needed — SystemView is already initialised
       before tx_kernel_enter() via SEGGER_SYSVIEW_Conf(). */
}

/* ------------------------------------------------------------------ */
/*  Thread-enter / thread-exit hooks (called from PendSV_Handler)     */
/* ------------------------------------------------------------------ */

/**
 * Called by the ThreadX scheduler whenever a thread begins executing.
 * _tx_thread_current_ptr already points to the new thread.
 */
void _tx_execution_thread_enter(void)
{
    TX_THREAD *p = _tx_thread_current_ptr;
    if (p != TX_NULL) {
        SEGGER_SYSVIEW_OnTaskStartExec((U32)p);
    }
}

/**
 * Called by the ThreadX scheduler when a thread is about to be preempted
 * or suspends.
 */
void _tx_execution_thread_exit(void)
{
    SEGGER_SYSVIEW_OnTaskStopExec();
}

/* ------------------------------------------------------------------ */
/*  ISR-enter / ISR-exit hooks (called from tx_thread_context_save/   */
/*  tx_thread_context_restore)                                        */
/* ------------------------------------------------------------------ */

void _tx_execution_isr_enter(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
}

void _tx_execution_isr_exit(void)
{
    SEGGER_SYSVIEW_RecordExitISR();
}

/* ------------------------------------------------------------------ */
/*  Thread registration helper                                        */
/* ------------------------------------------------------------------ */

/** Info structure kept alive for SystemView queries. */
static SEGGER_SYSVIEW_TASKINFO _aTaskInfo[8];
static unsigned                _NumTasks;

/**
 * Register a single ThreadX thread with SystemView.
 * Call after tx_thread_create() for each application thread.
 */
void sysview_register_thread(TX_THREAD *thread)
{
    if (_NumTasks >= (sizeof(_aTaskInfo) / sizeof(_aTaskInfo[0]))) {
        return; /* table full */
    }
    SEGGER_SYSVIEW_TASKINFO *pInfo = &_aTaskInfo[_NumTasks];
    pInfo->TaskID    = (U32)thread;
    pInfo->sName     = thread->tx_thread_name;
    pInfo->Prio      = thread->tx_thread_priority;
    pInfo->StackBase = (U32)thread->tx_thread_stack_start;
    pInfo->StackSize = (U32)((char *)thread->tx_thread_stack_end -
                             (char *)thread->tx_thread_stack_start);
    _NumTasks++;

    SEGGER_SYSVIEW_OnTaskCreate((U32)thread);
    SEGGER_SYSVIEW_SendTaskInfo(pInfo);
}

/**
 * Callback used by SystemView to re-send thread info on connection.
 * Must be registered with SEGGER_SYSVIEW_RegisterModule() OR called
 * from _cbSendSystemDesc / SEGGER_SYSVIEW_SendModule.
 */
void sysview_send_task_list(void)
{
    unsigned i;
    for (i = 0; i < _NumTasks; i++) {
        SEGGER_SYSVIEW_SendTaskInfo(&_aTaskInfo[i]);
    }
}
