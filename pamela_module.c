/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

/* Include PAM headers */
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

/* PAM entry point for session creation */
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  write(1, "salut", strlen("salut"));
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

  write(1, "coucou", strlen("coucou"));

  //recuperer login et mdp et decrypter conteneur grace a ca (et le monter je crois)
  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_data(pamh, "pamela_user_pass", (const void **)&pass)) != PAM_SUCCESS) // ESSAYER AVEC PAM_AUTHTOK
    pass = NULL;

  write(1, "batard", strlen("batard"));
  //creation path
  sprintf(pathContainerFile, "%s/%s", pathContainer, user);
  sprintf(nameDevice, "device-%s", user);
  sprintf(mountDevice, "/dev/mapper/device-%s", user);
  sprintf(mountDest, "/home/%s/secure_data-rw", user);

  write(1, "bite", strlen("bite"));
  // sh pour ouvrir et monter conteneur
  dir = opendir(mountDest);
  if (dir) {
    write(1, "baba", strlen("baba"));
    sprintf(cmd, "echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mount %s %s && sudo chown %s:users %s -R",
            pass, pathContainerFile, user, mountDevice, mountDest, user, mountDest);
  }
  else {
    write(1, "rata", strlen("rata"));
    sprintf(cmd, "sudo mkdir -p /pamela && sudo mkdir -p /pamela/container && sudo fallocate -l 500MB %s && echo '%s' | sudo cryptsetup luksFormat %s && echo '%s' | sudo cryptsetup luksOpen %s device-%s && sudo mkfs.ext3 %s && mkdir %s && sudo mount %s %s && sudo chown %s:users %s -R",
            pathContainerFile, pass, pathContainerFile, pass, pathContainerFile, user, mountDevice, mountDest, mountDevice, mountDest, user, mountDest);
  }
  write(1, "test", strlen("test"));
  system(cmd);
  write(1, "retest", strlen("retest"));

  return(PAM_SUCCESS);
}

/* PAM entry point for session cleanup */
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  char        cmd[4096];
  char        nameDevice[300];
  char        mountDest[300];
  const char        *user;
  int         val;

  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  sprintf(nameDevice, "device-%s", user);
  sprintf(mountDest, "/home/%s/secure_data-rw", user);

  sprintf(cmd, "sudo umount %s && sudo cryptsetup luksClose %s", mountDest, nameDevice);
  system(cmd);
  return(PAM_SUCCESS);

  //log et mdp ET crypter conteneur et demonter
}

/* PAM entry point for accounting */
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

void my_cleanup(pam_handle_t *pamh, void *data, int error_status) {

  if (data != NULL) {
    bzero(data, strlen(data));
    free(data);
  }
}

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {

  //log et mdp ET decrypter conteneur (creer si y'a pas)
  write(1, "auth", strlen("auth"));
  const char  *user;
  const char  *pass;
  int   val;

  write(1, "auth2", strlen("auth2"));
  if ((val = pam_get_user(pamh, &user, "Username: ")) != PAM_SUCCESS)
    return (val);
  if ((val = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
    return (val);
  if ((val = pam_set_data(pamh, "pamela_user_pass", strdup(pass), &my_cleanup)) != PAM_SUCCESS)
    return (val);
  write(1, "auth3", strlen("auth3"));
  return(PAM_SUCCESS);
}

/*
   PAM entry point for setting user credentials (that is, to actually
   establish the authenticated user's credentials to the service provider)
 */
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  return(PAM_SUCCESS);
}

/* PAM entry point for authentication token (password) changes */
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  // recuperer ancien mdp, decrypter conteneur, puis reencrypter avec nouveau mdp

  const char  *oldPass;
  const char  *pass;
  int         val;

  if ((val = pam_get_item(pamh, PAM_AUTHTOK, (const void **)&pass)) != PAM_SUCCESS)
    return (val);
  return(PAM_SUCCESS);
}
