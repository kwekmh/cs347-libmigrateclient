#ifndef LIBMIGRATECLIENT_LIBMIGRATECLIENT_H_
#define LIBMIGRATECLIENT_LIBMIGRATECLIENT_H_

#define STR_VALUE(arg) #arg

#define MIGRATION_CLIENT_SOCKET_PATH STR_VALUE(/var/migrateclient/local-socket)

#define MSG_BUFFER_SIZE 256

typedef struct MigrateClient {
  int local_sock;
  int remote_sock;
  int service_identifier;
} MigrateClient;

MigrateClient * InitMigrateClient(int service_identifier);
void * HandleMigrateClient(void *c);
bool SendDescriptor(MigrateClient *migrate_client, int fd);
void SendRegistration(MigrateClient *migrate_client);

#endif
