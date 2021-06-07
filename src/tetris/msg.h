/*==============================================================================
** tetris.h -- Tetris Private Structs and Macros.
**
** MODIFY HISTORY:
**
** 2011-06-06 wdf Create.
==============================================================================*/

#ifndef  __TETRIS_MSG_H__
#define  __TETRIS_MSG_H__

/*======================================================================
  Tetris job_task can process these messages
======================================================================*/
typedef enum msg_job {
    TETRIS_MSG_QUIT,
    TETRIS_MSG_TURN,
    TETRIS_MSG_DOWN,
    TETRIS_MSG_RIGHT,
    TETRIS_MSG_LEFT,
    TETRIS_MSG_AUTO,
    TETRIS_MSG_BOTTOM,
    TETRIS_MSG_HOLD,
    TETRIS_MSG_NOP
} MSG_JOB;

/*======================================================================
  Tetris send to ui show message type
======================================================================*/
typedef enum msg_show {
    MSG_SHOW_SCORE,
    MSG_SHOW_COUNT,
    MSG_SHOW_SPEED,
    MSG_SHOW_TIME
} MSG_SHOW;

#endif /* __TETRIS_MSG_H__ */
/*==============================================================================
** FILE END
==============================================================================*/

