/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

/* Include PAM headers */
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void  cleanup(pam_handle_t *pamh, void *data, int error_status)
{
  if (data)
    free(data);
}

int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  int		retval;
  char		*pass;
  char    *part_path;
  char    *user;
  char    *mount_path;
  char    *device;
  DIR*    dir;
  char    cmd[4096];

  printf("open session\n");
  if (pam_get_data(pamh, "pam_module_pass", (const void **)&pass) != PAM_SUCCESS)
    return(PAM_SUCCESS);
  if (pass)
    {
      retval = pam_get_user(pamh, (const char **)&user, "Username: ");
      printf("%s\n", user);
      if (retval != PAM_SUCCESS) {
        return (retval);
      }
      if (!(part_path = malloc(sizeof(char) * (strlen(user) + 18))))
        return (PAM_SUCCESS);
      if (!(mount_path = malloc(sizeof(char) * (strlen(user) + 21))))
        return (PAM_SUCCESS);
      if (!(device = malloc(sizeof(char) * (strlen("/dev/mapper/device-") + strlen(user)))))
        return (PAM_SUCCESS);

      // fill cont_pass
          sprintf(part_path, "/home/%s/.partition", user);

      // fill mount_path
          sprintf(mount_path, "/home/%s/secure_data-rw", user);

      // fill device
          sprintf(device, "/dev/mapper/device-%s", user);

      // CALL FUNCTIONS SYSTEM
        // create partition
        dir = opendir(mount_path);
        if (dir)
        {
            // Other Time
            printf("when is already mounted one time\n");
            sprintf(cmd, "echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mount %s %s && sudo chown %s:users %s -R",
                          pass, part_path, user, device, mount_path, user, mount_path);
            system(cmd);
            closedir(dir);
            printf("when is already mounted one time\n");
        }
        else
          {
            // First Time
            sprintf(cmd, "fallocate -l 500MB %s && echo '%s' | sudo cryptsetup luksFormat %s && echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mkfs.ext3 %s && mkdir %s && sudo mount %s %s && sudo chown %s:users %s -R",
                          part_path, pass, part_path, pass, part_path, user, device, mount_path, device, mount_path, user, mount_path);
            system(cmd);
          }
    }
  return(PAM_SUCCESS);
}

int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        int		retval;
        char    *user;
        char    *mount_path;
        char    cmd[4096];
        printf("close session");
        // unmount container

        retval = pam_get_user(pamh, (const char **)&user, "Username: ");
        printf("%s\n", user);
        if (retval != PAM_SUCCESS) {
          return (retval);
        }
        if (!(mount_path = malloc(sizeof(char) * (strlen(user) + 21))))
          return (PAM_SUCCESS);

        // fill mount_path
            sprintf(mount_path, "/home/%s/secure_data-rw", user);

        printf("asasas\n");
        sprintf(cmd, "sudo umount %s && sudo cryptsetup luksClose device-%s",
                      mount_path, user);
        system(cmd);
        return(PAM_SUCCESS);
}

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        int		retval;
        char		*pass;

        printf("authenticate\n");
        if ((retval = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
            return (retval);
        if (pass)
          {
            pam_set_data(pamh, "pam_module_pass", strdup(pass), &cleanup);
          }
        return(PAM_SUCCESS);
}

/* PAM entry point for authentication token (password) changes */
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
        int		retval;
        char		*pass;

        printf("change password");
        if ((retval = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
            return (retval);
        if (pass)
          {
            //changement de mot de passe dans le container
            printf("%s\n", pass);
          }
        return(PAM_SUCCESS);
}
