//
// Copyright 2016 Jeff Bush
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include "asm.h"
#include "list.h"
#include "vm_address_space.h"
#include "vm_translation_map.h"

#define MAX_HW_THREADS 32

struct process
{
    struct list_node list_entry;
    int id;
    spinlock_t lock;
    struct list_node thread_list;
    struct vm_address_space *space;
};

struct thread
{
    struct list_node queue_entry;
    struct list_node process_entry;

    int id;
    unsigned int *kernel_stack_ptr;
    unsigned int *current_stack;
    struct vm_area *kernel_stack_area;
    struct vm_area *user_stack_area;
    struct process *proc;
    void (*start_func)(void *param);
    void *param;
    enum
    {
        THREAD_READY,
        THREAD_RUNNING,
        THREAD_BLOCKED,
        THREAD_DEAD
    } state;
    char name[32];
};

struct thread_queue
{
    struct thread *head;
    struct thread *tail;
};

void bool_init_kernel_process(void);

// This must be called by each hardware thread to create a software
// thread context for itself.
void boot_init_thread(void);

struct thread *current_thread(void);
struct thread *spawn_user_thread(const char *name,
                                 struct process *proc,
                                 void (*start_function)(void *param),
                                 void *param);
struct thread *spawn_kernel_thread(const char *name,
                                   void (*start_function)(void *param),
                                   void *param);
void enqueue_thread(struct thread_queue*, struct thread*);
struct thread *dequeue_thread(struct thread_queue*);
void reschedule(void);
struct process *exec_program(const char *filename);
void __attribute__((noreturn)) thread_exit(int retcode);

void dump_process_list(void);

inline int current_hw_thread(void)
{
    return __builtin_nyuzi_read_control_reg(CR_CURRENT_HW_THREAD);
}
