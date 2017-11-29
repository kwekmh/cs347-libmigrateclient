#ifndef LIBMIGRATECLIENT_LIBMIGRATECLIENT_H_
#define LIBMIGRATECLIENT_LIBMIGRATECLIENT_H_

#define STR_VALUE(arg) #arg

#define MIGRATION_CLIENT_SOCKET_PATH STR_VALUE(/var/migrateclient/local-socket)

bool SendDescriptor(int service_identifier, int fd);

#endif
