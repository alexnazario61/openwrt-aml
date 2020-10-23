/*****************************************************************************
 **
 **  Name:           uipc_thread.c
 **
 **  Description:    Thread management API
 **
 **  Copyright (c) 2009-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "buildcfg.h"
#if defined (NSA_STANDALONE) && (NSA_STANDALONE == TRUE)
#include "nsa_api.h"
#include "nsa_int.h"
#else
#include "bsa_api.h"
#include "bsa_int.h"
#endif

#include "uipc_thread.h"

#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
#if defined(__FreeBSD__)
#define pthread_setname_np(t, n)  pthread_set_name_np((t), (n))
#endif
extern int __attribute__((weak)) pthread_setname_np(pthread_t thread, const char *name);
#define BSA_THREAD_NAME_LEN_MAX     16      /* Pthread Limitation */
#ifndef BSA_THREAD_NAME_PREFIX
#ifdef BSA_SERVER
#define BSA_THREAD_NAME_PREFIX  "BSA_Srv:"  /* Pthread Name Prefix */
#endif /* BSA_SERVER */
#ifdef BSA_CLIENT
#define BSA_THREAD_NAME_PREFIX  "BSA_Clt:"  /* Pthread Name Prefix */
#endif /* BSA_CLIENT */
#endif /* BSA_THREAD_NAME_PREFIX */
#endif /* BSA_SET_THREAD_NAME == TRUE */

typedef void *(*PTHREAD_ENTRY)(void*);

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
#include "gki_int.h"
#ifndef UIPC_THREAD_TM_MAX
#define UIPC_THREAD_TM_MAX  15
#endif
typedef struct
{
    BOOLEAN in_use;
    UINT16 stacksize;
    tTHREAD *p_thread;
    UINT8 name[BSA_TM_TASK_NAME_MAX];
}tUIPC_THREAD_TM;

tUIPC_THREAD_TM uipc_thread_tm_cb[UIPC_THREAD_TM_MAX];
#endif /* BSA_TM_INCLUDED */

/*******************************************************************************
 **
 ** Function         uipc_thread_create
 **
 ** Description      Create an UIPC thread.
 **
 ** Returns          void
 **
 *******************************************************************************/
int uipc_thread_create(THREAD_ENTRY task_entry, UINT8 *task_name,
        UINT16 *stack, UINT16 stacksize, tTHREAD *p_thread, void *arg)
{
    pthread_attr_t thread_attr;
    int status = 0;
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    int index;
#endif
#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    char thread_name[BSA_THREAD_NAME_LEN_MAX];
    int thread_name_len;
    char *p_name;
#endif

#ifdef UIPC_THREAD_DEBUG
    APPL_TRACE_DEBUG1("uipc_thread_create: task_name:%s", task_name);
#endif

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    /* Save the task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        if (uipc_thread_tm_cb[index].in_use == FALSE)
        {
            uipc_thread_tm_cb[index].in_use = TRUE;
            uipc_thread_tm_cb[index].stacksize = stacksize;
            uipc_thread_tm_cb[index].p_thread = p_thread;
            if (task_name)
            {
                strncpy((char *)uipc_thread_tm_cb[index].name,
                        (char *)task_name,
                        BSA_TM_TASK_NAME_MAX);
            }
            else
            {
                strncpy((char *)uipc_thread_tm_cb[index].name,
                        "UIPC",
                        BSA_TM_TASK_NAME_MAX);
            }
            uipc_thread_tm_cb[index].name[BSA_TM_TASK_NAME_MAX - 1] = '\0';
#ifdef UIPC_THREAD_DEBUG
            APPL_TRACE_DEBUG2("uipc_thread_create: add TM %s index:%d", uipc_thread_tm_cb[index].name, index);
#endif
            break;
        }
    }
    if (index >= UIPC_THREAD_TM_MAX)
    {
        APPL_TRACE_WARNING0("uipc_thread_create: no memory to save Thread info for TM !!!");
    }
#endif

    pthread_attr_init(&thread_attr);

    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(p_thread, &thread_attr, (PTHREAD_ENTRY) task_entry, (void *)arg) < 0)
    {
        APPL_TRACE_ERROR1("uipc_thread_create pthread_create failed:%d", errno);
        status = -1;
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
        if (index < UIPC_THREAD_TM_MAX)
        {
            uipc_thread_tm_cb[index].in_use = FALSE;
        }
#endif
    }

#if defined(BSA_SET_THREAD_NAME) && (BSA_SET_THREAD_NAME == TRUE)
    if (status == 0)
    {
        /* Change the pthread's name (for debug purpose) */
        BCM_STRNCPY_S(thread_name, sizeof(thread_name),
                BSA_THREAD_NAME_PREFIX, strlen(BSA_THREAD_NAME_PREFIX));
        thread_name_len = strlen(BSA_THREAD_NAME_PREFIX);
        thread_name[thread_name_len] = '\0';
        if (thread_name_len < (BSA_THREAD_NAME_LEN_MAX - 1))
        {
            /* Some task's name contains './' */
            p_name = strrchr((char *)task_name, '/');
            if (p_name == NULL)
            {
                p_name = (char *)task_name; /* No '/', use task_name */
            }
            else
            {
                p_name++;   /* Ship the '/' */
            }
            BCM_STRNCAT_S(thread_name, sizeof(thread_name),
                    p_name, BSA_THREAD_NAME_LEN_MAX - 1 - thread_name_len);
        }
        if (pthread_setname_np(*p_thread, thread_name) < 0)
        {
            APPL_TRACE_ERROR0("uipc_thread_create cannot change thread's name");
        }
    }
#endif

    return status;
}

/*******************************************************************************
 **
 ** Function         uipc_thread_stop
 **
 ** Description      Function that must be called when finishing a thread.  This
 **                  function does not end the thread but clears its entry from
 **                  the internal tables.
 **
 ** Parameters
 **
 ** Returns          The status of the execution.
 **
 *******************************************************************************/
int uipc_thread_stop(tTHREAD thread)
{
    int status = 0;
#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    int index;
#endif

    APPL_TRACE_DEBUG0("uipc_thread_stop");

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
    /* Remove the task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        if ((uipc_thread_tm_cb[index].in_use != FALSE) &&
            (pthread_equal(*(uipc_thread_tm_cb[index].p_thread), thread) != 0))
        {
            APPL_TRACE_DEBUG2("uipc_thread_stop: Free TM %s index:%d", uipc_thread_tm_cb[index].name, index);
            uipc_thread_tm_cb[index].in_use = FALSE;
            break;
        }
    }
    if (index >= UIPC_THREAD_TM_MAX)
    {
        APPL_TRACE_WARNING0("uipc_thread_stop: Thread info not retrieved from TM !!!");
    }
#endif

    APPL_TRACE_DEBUG0("INFO: uipc_thread_stop: this function does NOT really stop a thread");
    APPL_TRACE_DEBUG0("INFO: The thread ITSELF must explicitly end itself by returning calling from thread entry point");

    return status;
}

#if defined (BSA_TM_INCLUDED) && (BSA_TM_INCLUDED == TRUE)
/*******************************************************************************
 **
 ** Function         uipc_thread_get_usage
 **
 ** Description      Get thread usage (numbers, stack size, etc.).
 **
 ** Parameters
 **
 ** Returns          Number of thread when ok, -1 if missed tasks
 **
 *******************************************************************************/
int uipc_thread_get_usage(tBSA_TM_TASK_STATUS *p_task_status, UINT16 nb_element)
{
    int index;
    int nb_task = 0;

    /* Look for task information for Test module (TM) */
    for (index = 0; index < UIPC_THREAD_TM_MAX; index++)
    {
        /* If this thread is active */
        if (uipc_thread_tm_cb[index].in_use != FALSE)
        {
            if (nb_element > 0)
            {
                p_task_status->task_used = TRUE;
                /* Add 50 to do not confuse with GKI tasks */
                p_task_status->task_id = 50 + index;
                p_task_status->task_state = TASK_READY;
                p_task_status->stack_size = uipc_thread_tm_cb[index].stacksize;
                p_task_status->stack_used = 0; /* todo */
                strncpy((char *)p_task_status->task_name,
                        (char *)uipc_thread_tm_cb[index].name,
                        BSA_TM_TASK_NAME_MAX);
                /* make sure the string is terminated */
                p_task_status->task_name[BSA_TM_TASK_NAME_MAX-1] = 0;
                p_task_status++;
                nb_element--;
                nb_task++;
            }
            else
            {
                APPL_TRACE_WARNING0("uipc_thread_get_usage: task missed");
                nb_task = -1;
            }
        }
    }
    return nb_task;
}
#endif


