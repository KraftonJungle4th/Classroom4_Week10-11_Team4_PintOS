#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void remove_file_descriptor(int fd);
struct lock file_lock;
void lock_acquire_if_available(const struct lock *);
void lock_release_if_available(const struct lock *);

#endif /* userprog/syscall.h */
