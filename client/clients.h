#ifndef CLIENTS_H
#define CLIENTS_H
#include "client/client.h"
#ifdef NACL
#include "client/pp.h"
#else
#include "client/tcp.h"
#endif
#endif // CLIENTS_H
