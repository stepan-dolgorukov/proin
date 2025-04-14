#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stepan Dolgorukov");

#include <linux/kernel.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/pid.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>

static int identifier_process = 0;

module_param(identifier_process, int, S_IRUSR);

static bool get_process_task(uid_t process,
                             struct task_struct* task);

static bool get_user_identifier(const struct task_struct* task,
                                uid_t* user);

static bool get_executable_path(const struct task_struct* task,
                                char* storage,
                                char** path);

static struct proc_dir_entry* entry_response = NULL;

static ssize_t response_read(struct file* file,
                             char* storage,
                             size_t size_storage,
                             loff_t* offset);

struct proc_ops operations_response =
{
  .proc_read = response_read
};

struct
{
  uid_t user;
  char* path_executable;
} response;

int init_module()
{
  pr_info("proin: identifier_process = %i\n", identifier_process);

  if (identifier_process <= 0)
  {
    return -1;
  }

  struct task_struct *task = kmalloc(sizeof(struct task_struct), GFP_KERNEL);

  if (!get_process_task(identifier_process, task))
  {
    pr_debug("%s\n", "get_process_task");
    kfree(task);

    return -1;
  }

  uid_t user = 0;

  if (!get_user_identifier(task, &user))
  {
    pr_debug("%s\n", "get_user_identifier");
    kfree(task);

    return -1;
  }

  pr_info("proin: user = %u\n", user);
  response.user = user;

  char* storage_path = kmalloc(256, GFP_KERNEL);
  char* path = NULL;

  if (!get_executable_path(task, storage_path, &path))
  {
    pr_debug("%s\n", "get_executable_path");
    kfree(task);
    path = NULL;
    kfree(storage_path);

    return -1;
  }

  pr_info("proin: executable = %s\n", path);
  response.path_executable = path;
  entry_response = proc_create("proin_response", 0, NULL, &operations_response);

  if (entry_response == NULL)
  {
    pr_debug("proin: %s\n", "proc_create");

    kfree(storage_path);
    path = NULL;
    kfree(task);

    return -1;
  }

  kfree(storage_path);
  path = NULL;
  kfree(task);

  return 0;
}

void cleanup_module()
{
  proc_remove(entry_response);

  pr_info("proin: exit\n");
}

bool get_process_task(uid_t process, struct task_struct* task)
{
  struct pid* const notion = find_get_pid(process);

  if (notion == NULL)
  {
    pr_debug("%s\n", "find_get_pid");

    return false;
  }

  const struct task_struct* const task_ = get_pid_task(notion, PIDTYPE_PID);

  if (task_ == NULL)
  {
    pr_debug("%s\n", "task_process");

    return false;
  }

  *task = *task_;

  return true;
}

bool get_user_identifier(const struct task_struct* task, uid_t* user)
{
  if (task == NULL)
  {
    pr_debug("%s\n", "task equals to NULL");

    return false;
  }

  if (user == NULL)
  {
    pr_debug("%s\n", "user equals to NULL");

    return false;
  }

  if (task->real_cred == NULL)
  {
    pr_debug("%s\n", "task_process->real_cred");

    return false;
  }

  *user = task->real_cred->uid.val;

  return true;
}

bool get_executable_path(const struct task_struct* task, char* storage, char** path)
{
  if (task == NULL)
  {
    return false;
  }

  if (path == NULL)
  {
    return false;
  }

  if (!task->mm)
  {
    return false;
  }

  if (!task->mm->exe_file)
  {
    return false;
  }

  *path = d_path(&(task->mm->exe_file->f_path), storage, 256);

  if (IS_ERR(*path))
  {
    *path = NULL;

    return false;
  }

  return true;
}

ssize_t response_read(struct file* file,
                      char* storage,
                      size_t size_storage,
                      loff_t* offset)
{
  char output[300] = {'\0'};
  static size_t size_output = 0;

  if (size_output != 0 && *offset >= size_output)
  {
    return 0;
  }

  sprintf(output, "%i;%u;%s\n", identifier_process, response.user, response.path_executable);
  size_output = strlen(output);

  if (copy_to_user(storage, output, size_output) != 0)
  {
    return 0;
  }

  *offset += size_output;

  return size_output;
}
