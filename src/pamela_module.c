#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;
  const char  *user;
  const char  *pass;
  int         val;
  char        cmd[4096];
  char        *pathContainer = "/pamela/container";
  char        pathContainerFile[300];
  char        nameDevice[300];
  char        mountDevice[300];
  char        mountDest[300];
  DIR         *dir;

  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_data(pamh, "pamela_user_pass", (const void **)&pass)) != PAM_SUCCESS) // ESSAYER AVEC PAM_AUTHTOK
    pass = NULL;
  sprintf(pathContainerFile, "%s/%s", pathContainer, user);
  sprintf(nameDevice, "device-%s", user);
  sprintf(mountDevice, "/dev/mapper/device-%s", user);
  sprintf(mountDest, "/home/%s/secure_data-rw", user);
  dir = opendir(mountDest);
  if (dir) {
    sprintf(cmd, "echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mount %s %s && sudo chown %s:users %s -R",
            pass, pathContainerFile, user, mountDevice, mountDest, user, mountDest);
  }
  else {
    sprintf(cmd, "sudo mkdir -p /pamela && sudo mkdir -p /pamela/container && sudo fallocate -l 500MB %s && echo '%s' | sudo cryptsetup luksFormat %s && echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mkfs.ext3 %s && mkdir %s && sudo mount %s %s && sudo chown %s:users %s -R",
            pathContainerFile, pass, pathContainerFile, pass, pathContainerFile, user, mountDevice, mountDest, mountDevice, mountDest, user, mountDest);
  }
  system(cmd);
  return(PAM_SUCCESS);
}

int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;
  char        cmd[4096];
  char        nameDevice[300];
  char        mountDest[300];
  const char  *user;
  int         val;

  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  sprintf(nameDevice, "device-%s", user);
  sprintf(mountDest, "/home/%s/secure_data-rw", user);
  sprintf(cmd, "sudo umount %s && sudo cryptsetup luksClose %s", mountDest, nameDevice);
  system(cmd);
  return(PAM_SUCCESS);
}

int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)pamh;
  (void)flags;
  (void)argc;
  (void)argv;
  return(PAM_SUCCESS);
}

void my_cleanup(pam_handle_t *pamh, void *data, int error_status) {
  (void)pamh;
  (void)error_status;
  if (data != NULL) {
    bzero(data, strlen(data));
    free(data);
  }
}

int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;
  const char  *user;
  const char  *pass;
  int   val;

  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
    return (val);
  if ((val = pam_set_data(pamh, "pamela_user_pass", strdup(pass), &my_cleanup)) != PAM_SUCCESS)
    return (val);
  return(PAM_SUCCESS);
}

int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;
  (void)pamh;

  return(PAM_SUCCESS);
}

int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  (void)flags;
  (void)argc;
  (void)argv;
  (void)pamh;
  const char  *user;
  const char  *pass;
  const char  *oldPass;
  int         val;
  char        cmd[300];

  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_item(pamh, PAM_OLDAUTHTOK, (const void **)&oldPass)) != PAM_SUCCESS)
  	return (val);
  sprintf(cmd, "echo -e '%s\n%s\n%s' | sudo cryptsetup luksAddKey /pamela/container/%s && echo '%s' | sudo cryptsetup luksRemoveKey /pamela/container/%s", oldPass, pass, pass, user, oldPass, user);
  system(cmd);
  return(PAM_SUCCESS);
}
