/*==============================================================================
** os.h -- tetris os interface header file.
**
** MODIFY HISTORY:
**
** 2011-09-29 wdf Create.
==============================================================================*/

#ifndef __TETRIS_OS_H__
#define __TETRIS_OS_H__

/*======================================================================
  Select OS
======================================================================*/
#define OS_DFEWOS           0    /* this is for DfewOS */
#define OS_VXWORKS          0    /* this is for VxWorks */
#define OS_SYLIXOS          1    /* this is for SylixOS */

#if (OS_VXWORKS > 0) || (OS_SYLIXOS > 0) /* VXWORKS */

#include <vxWorks.h>
#include <taskLib.h>
#include <tickLib.h>
#include <sysLib.h>
#include <msgQLib.h>
#include <memLib.h>
#define OS_TASK_ID          int
#define OS_TASK_ID_ERROR    ERROR
#define OS_MSG_Q_ID         MSG_Q_ID

#elif (OS_DFEWOS > 0)                    /* DFEWOS */

#include <dfewos.h>
#include "../../string.h"
#define OS_TASK_ID          OS_TCB*
#define OS_TASK_ID_ERROR    NULL
#define OS_MSG_Q_ID         MSG_QUE*

#elif (OS_PPOS > 0)                      /* PPOS */

#error This OS Need Yaozhan Complete it!

#endif                                   /* END OS */

/*======================================================================
  os interface function declare
======================================================================*/
OS_TASK_ID  os_task_create(char         *name,
                           unsigned int  stack_size,
                           unsigned char task_priority,
                           void         *task_entry,
                           int arg1, int arg2);
void        os_task_delay(int ms);
void        os_task_delete(OS_TASK_ID task_id);

OS_MSG_Q_ID os_msgQ_create (int max_num, int max_length);
int         os_msgQ_send (OS_MSG_Q_ID msgQ_id, char *msg_buf, int msg_len);
void        os_msgQ_receive (OS_MSG_Q_ID msgQ_id, char *buf, int buf_len);
void        os_msgQ_delete (OS_MSG_Q_ID msgQ_id);
void        os_msgQ_flush (OS_MSG_Q_ID msgQ_id);

int         os_time_get ();

#endif /* __TETRIS_OS_H__ */

/*==============================================================================
** FILE END
==============================================================================*/

