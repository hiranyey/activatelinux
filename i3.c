#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define I3_IPC_MAGIC "i3-ipc"
#define I3_IPC_MESSAGE_TYPE_COMMAND 0
#define I3_IPC_MESSAGE_TYPE_SUBSCRIBE 2

// Function to read the socket path
char *get_socket_path() {
  FILE *fp = popen("i3 --get-socketpath", "r");
  if (fp == NULL) {
    perror("popen failed");
    return NULL;
  }

  static char path[256];
  if (fgets(path, sizeof(path), fp) == NULL) {
    perror("Failed to get i3 socket path");
    pclose(fp);
    return NULL;
  }

  path[strcspn(path, "\n")] = 0; // Remove the newline character
  pclose(fp);
  return path;
}

// Function to send a message to i3's IPC
int send_ipc_message(int sockfd, uint32_t type, const char *payload) {
  uint32_t len = strlen(payload);
  char header[14];
  memcpy(header, I3_IPC_MAGIC, 6);
  memcpy(header + 6, &len, sizeof(len));
  memcpy(header + 10, &type, sizeof(type));

  if (write(sockfd, header, sizeof(header)) == -1 ||
      write(sockfd, payload, len) == -1) {
    perror("Failed to send IPC message");
    return -1;
  }
  return 0;
}

void find_and_move_window(int target_workspace, int sockfd) {
  char command[128];
  snprintf(command, sizeof(command),
           "[title=\"ActivateLinux\"] move container to workspace %d",
           target_workspace);
  send_ipc_message(sockfd, I3_IPC_MESSAGE_TYPE_COMMAND, command);
}

int main() {
  // Get the i3 socket path
  char *socket_path = get_socket_path();
  if (socket_path == NULL) {
    fprintf(stderr, "Failed to get i3 socket path\n");
    return 1;
  }

  // Create a Unix socket
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    return 1;
  }

  // Connect to the i3 socket
  struct sockaddr_un addr = {0};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    close(sockfd);
    return 1;
  }

  // Subscribe to workspace and window events
  const char *subscribe_payload = "{\"subscribe\":[\"workspace\"]}";
  if (send_ipc_message(sockfd, I3_IPC_MESSAGE_TYPE_SUBSCRIBE,
                       subscribe_payload) == -1) {
    close(sockfd);
    return 1;
  }

  // Read events from the socket
  while (1) {
    char header[14];
    if (read(sockfd, header, sizeof(header)) != sizeof(header)) {
      perror("Failed to read IPC header");
      break;
    }

    uint32_t len, type;
    memcpy(&len, header + 6, sizeof(len));
    memcpy(&type, header + 10, sizeof(type));

    char *payload = malloc(len + 1);
    if (payload == NULL) {
      perror("malloc");
      break;
    }

    if (read(sockfd, payload, len) != len) {
      perror("Failed to read IPC payload");
      free(payload);
      break;
    }
    payload[len] = 0;
    struct json_object *parsed_json = json_tokener_parse(payload);
    if (parsed_json) {
      json_object *current = json_object_object_get(parsed_json, "current");
      json_object *workspace_name = json_object_object_get(current, "name");
      const char *workspace = json_object_get_string(workspace_name);
      if (workspace != NULL) {
        find_and_move_window(atoi(workspace), sockfd);
      }
      json_object_put(parsed_json);
    }

    free(payload);
  }

  close(sockfd);
  return 0;
}
