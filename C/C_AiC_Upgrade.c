#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <libssh/server.h>
#include <libssh/sftp.h>
#include <libssh/ssh2.h>
#include <string.h>

/*
    Author: Matictiny
    Date: 2023-06-08
    Usage: A complied binary executable file to upgrade AiC cu or du.
    Compile: gcc ssh_c.c -L /usr/include/libssh -lssh
*/

#define MAX_BUFFER_SIZE 256

char* exec_remote_cmd( const char *hostip, const char *username, const char *password, char *cmd );
//exec_remote_cmd_real_time_no_retu used to realtime
int exec_remote_cmd_real_time_no_return( const char *hostip, const char *username, const char *password, char *cmd );

void remove_dollar_sign(char* str);
void remove_newline(char *str);

int main(int argc, char **argv) {
    printf("Author: Maigctiny\nRelease Date:2023-06-08\nVersion:2023-0.1.0\nEmail: shijiu.lv@nkia-sbell.com\n");
    if (argc != 7) {
        printf("Usage:\n%s hostip username passwd 6.17.0 vcu 6.17.0 0.300.xxxx \n", argv[0]);
        return 1;
    }
    const char *hostip = argv[1];
    const char *username = argv[2];
    const char *password = argv[3];
    const char *kuafu = argv[4];
    const char *vxu = argv[5];
    const char *version = argv[6];

    printf("%s;%s;%s\n",hostip,username,password);
    char new_build_version[256];
    sprintf(new_build_version,"build_version=%s",version);
    printf("New version is:\n%s\n",version);

    char cmd_edit_version[256];
    sprintf(cmd_edit_version,"cd kuafu-v%s/artifacts/ ; sed -i 's/^build.*/%s/g' %s_config.ini" ,kuafu, new_build_version,vxu );

    char cmd_to_workdir[256];
    sprintf(cmd_to_workdir, "cd ; cd kuafu-v%s/scenarios/%s*", kuafu, vxu);

    char cmd_vcu[256];
    sprintf(cmd_vcu, "%s;./16*.sh;./20*.sh;./21*.sh;./42*.sh", cmd_to_workdir);

    char cmd_vdu[256];
    sprintf(cmd_vdu, "%s;./16*.sh && ./20*.sh && ./17*.sh update && ./21*.sh && ./42*.sh", cmd_to_workdir);
 
    char cmd_cp_scf_to_kuafu[256];
    sprintf(cmd_cp_scf_to_kuafu,"mv ~/xxx_scf_file.xml ~/kuafu-v%s/scf/%s/ && echo 'scf process done'",kuafu,vxu);

    // edit version;
    printf("Start to prepare version upgrade to:\n");
    exec_remote_cmd_real_time_no_return( hostip, username, password, cmd_edit_version );
    //copy scf to kuafu;
    printf("Start to process with scf, cmd is:\n");
    exec_remote_cmd_real_time_no_return( hostip, username, password, cmd_cp_scf_to_kuafu );
    //Start to upgrade;
    //Start to handle cu and du;
    printf("Start to upgrading process..cmd is : \n");
    if (strcmp(vxu, "vcu") == 0) {
        //printf("Start to upgrade %s, cmd is:\n%s\n",vxu,cmd_vcu);
        exec_remote_cmd_real_time_no_return( hostip, username, password, cmd_vcu );
    }
    else if (strcmp(vxu, "vdu") == 0) {
        //printf("Start to upgrade %s, cmd is:\n%s\n",vxu,cmd_vdu);
        exec_remote_cmd_real_time_no_return( hostip, username, password, cmd_vdu );        
    } 
    else {
        printf("your choise is %s not vcu or vdu, pls check again!\n" ,vxu);
        return 1;
    }

    printf("%s upgrade to %s has done. \nCheer~!\n",vxu,version);
    return 0;
}

char* exec_remote_cmd(const char *hostip, const char *username, const char *password, char *cmd) 
{
    ssh_session session;
    ssh_channel channel;
    int rc, nbytes;
    char buffer[MAX_BUFFER_SIZE];
    char *result = NULL;

    // create a new SSH session and connect to the remote host
    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Error creating SSH session.\n");
        exit(1);
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, hostip);
    ssh_options_set(session, SSH_OPTIONS_USER, username);
    //ssh_options_set(session, SSH_OPTIONS_PASSWORD, password);

    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to remote host: %s\n", ssh_get_error(session));
        exit(1);
    }

    rc = ssh_userauth_password(session, NULL, password);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
        exit(1);
    }

    // open a new SSH channel and execute the remote command
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating SSH channel.\n");
        exit(1);
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening SSH channel: %s\n", ssh_get_error(session));
        exit(1);
    }

    rc = ssh_channel_request_exec(channel, cmd);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error executing remote command: %s\n", ssh_get_error(session));
        exit(1);
    }

    // read the output of the remote command and store it in a string
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    if (nbytes < 0) {
        fprintf(stderr, "Error reading from SSH channel: %s\n", ssh_get_error(session));
        exit(1);
    }

    result = (char*) malloc(nbytes + 1); // allocate memory for the result string
    if (result == NULL) {
        fprintf(stderr, "Error allocating memory.\n");
        exit(1);
    }

    strncpy(result, buffer, nbytes); // copy the output to the result string
    result[nbytes] = '\0'; // add null terminator to the result string

    // close the SSH channel and session
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    return result;
}

int exec_remote_cmd_real_time_no_return( const char *hostip, const char *username, const char *password, char *cmd )
{
    ssh_session session;
    ssh_channel channel;
    int rc, nbytes;
    char buffer[MAX_BUFFER_SIZE];
    char *result = NULL;
    printf("%s\n",cmd);

    // create a new SSH session and connect to the remote host
    session = ssh_new();
    if (session == NULL) {
        fprintf(stderr, "Error creating SSH session.\n");
        exit(1);
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, hostip);
    ssh_options_set(session, SSH_OPTIONS_USER, username);
    //ssh_options_set(session, SSH_OPTIONS_PASSWORD, password);

    rc = ssh_connect(session);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error connecting to remote host: %s\n", ssh_get_error(session));
        exit(1);
    }

    rc = ssh_userauth_password(session, NULL, password);
    if (rc != SSH_AUTH_SUCCESS) {
        fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
        exit(1);
    }

    // open a new SSH channel and execute the remote command
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        fprintf(stderr, "Error creating SSH channel.\n");
        exit(1);
    }

    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error opening SSH channel: %s\n", ssh_get_error(session));
        exit(1);
    }

    rc = ssh_channel_request_exec(channel, cmd);
    if (rc != SSH_OK) {
        fprintf(stderr, "Error executing remote command: %s\n", ssh_get_error(session));
        exit(1);
    }

    while ((nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, nbytes, stdout);
    }

    if (nbytes < 0) {
        fprintf(stderr, "Error reading SSH channel: %s\n", ssh_get_error(session));
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        ssh_disconnect(session);
        ssh_free(session);
        exit(-1);
    }

    // Close channel and session
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);

    return 0;
}

void remove_dollar_sign(char* str) {
    char* dollar_ptr;
    char new_str[strlen(str)];
    int index = 0;
    
    // Find the first occurrence of '$' in the string
    dollar_ptr = strchr(str, '$');
    
    // Copy the characters before the dollar sign to the new string
    while (dollar_ptr != NULL) {
        strncpy(new_str + index, str, dollar_ptr - str);
        index += dollar_ptr - str;
        
        // Move the pointer past the dollar sign
        str = dollar_ptr + 1;
        
        // Find the next occurrence of '$'
        dollar_ptr = strchr(str, '$');
    }
    
    // Copy the remaining characters to the new string
    strcpy(new_str + index, str);
    
    // Update the original string with the new string
    strcpy(str, new_str);
}

void remove_newline(char *str) 
{
    int newline_pos = strcspn(str, "\n"); // 获取第一个 '\n' 的位置
    char new_str[newline_pos + 1]; // 定义新的字符串
    strncpy(new_str, str, newline_pos); // 复制到新的字符串中
    new_str[newline_pos] = '\0'; // 在新字符串末尾添加 '\0'
    strcpy(str, new_str); // 将新字符串复制回原来的字符串中
}