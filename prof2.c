// Andrew Black
// June 21, 2016

// KNOWN ISSUES:
// 	Does not respect seek position. It would be more efficient to allow users
//	to open the file one time, and seek to the beginning to read the updated data.

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "prof.h"

struct state {
	struct task_struct *current_process;
	struct task_struct *current_subtask;

};

static void* prof_seq_start(struct seq_file *s, loff_t *pos) {
	struct state *cs;
	struct task_struct *cur_process;
	cs = (struct state*)s->private;
	if(cs->current_process == NULL) {
		return NULL; 
	} else {
		return 1;
	}
}

static void* prof_seq_next(struct seq_file *s, void *v, loff_t *pos) {
	return NULL;
}

static int prof_seq_show(struct seq_file *s, void *v) {
	struct state *cs;
	cs = (struct state*)s->private;
	seq_printf(s, "%d,%s\n", cs->current_process->pid, cs->current_process->comm);
	return 0;
}

static void prof_seq_stop(struct seq_file *s, void *v) {
	struct state *cs;
	cs = (struct state*)s->private;
	if(unlikely(cs->current_process == NULL)) {
		return;
	}
	cs->current_process = next_task(cs->current_process);
	if (unlikely(cs->current_process == &init_task)) {
		cs->current_process = NULL;	
	}
}

static struct seq_operations prof_seq_ops = {
	.start = prof_seq_start,
	.next = prof_seq_next,
	.stop = prof_seq_stop,
	.show = prof_seq_show
};

static int prof_open(struct inode *inode, struct file *file) {
	// TODO: Is there a memory leak here?
	//	What happens if seq_open fails, and we allocate our struct
	//	and put it in private_data of the file pointer we have.
	//	Is it possible for the file pointer to clean up its private_data?		
	int rval;
	struct seq_file *seq;
	struct state *state;	
	// Open seq_file and modify it, it gets stored in file->private_data
	rval = seq_open(file, &prof_seq_ops);
	seq = (struct seq_file*)file->private_data;	
	// Make our state struct and store it in the seq_file	
	state = kmalloc(sizeof(struct state), GFP_KERNEL);
	if (!state) {
		return -ENOMEM;
	}
	state->current_process = &init_task;
	state->current_subtask = NULL;
	seq->private = state;
	// Return what seq_open returned
	return rval;
}

static struct file_operations prof_file_ops = {
	.owner = THIS_MODULE,
	.open = prof_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static int __init proc_prof_init(void) {
	proc_create("prof", 0, NULL, &prof_file_ops);
	return 0;
}

static void __exit proc_prof_exit(void) {
	remove_proc_entry("prof", NULL);
}

module_init(proc_prof_init);
module_exit(proc_prof_exit);

