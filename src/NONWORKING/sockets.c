#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "protos.h"
#include "structs.h"

#define OFF		0
#define ON		1
#define IP_PROTOCOL	0  /* Number of IP protocol entry in /etc/protocols */
#define MAXPORTNUMLEN	4

list *port_list;
list *conn_list;

/***************************************************************************
 * Function: void sprint_ipnum(char *buf, struct sockaddr_in *s_address)
 *
 * Description:
 *   Print the ip numeric address of the sockaddr_in parameter into
 * the spaced pointed to by `buf' in a human-friendly format.  `buf'
 * should point to an allocated space of at least 16 bytes.
 **************************************************************************/
static void sprint_ipnum(char *buf, struct sockaddr_in *s_address) {
  u8 *octets;

  octets = (u8 *)&(s_address->sin_addr.s_addr);
  sprintf(buf, "%03d.%03d.%03d.%03d", octets[0],octets[1],octets[2],octets[3]);
}

/***************************************************************************
 * Function: void fd_noblock(int fd)
 *
 * Description:
 *   Set a file descriptor non-blocking.  This means that future reads from
 * (or writes to) the descriptor will return immediately with an indicative
 * return value if the operation can't be performed right away, rather than
 * sleeping until the operation becomes possible.
 **************************************************************************/
static void fd_noblock(int fd) {
#ifdef O_NDELAY
  if (fcntl(fd, F_SETFL, O_NDELAY) < 0) {
    fprintf(stderr, "Error: Couldn't set fd %d non-blocking.\n", fd);
    perror("  fcntl(O_NDELAY)");
    exit(ERROR);
  }
#elif defined FNDELAY
  if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
    fprintf(stderr, "Error: Couldn't set fd %d non-blocking.\n", fd);
    perror("  fcntl(FNDELAY)");
    exit(ERROR);
  }
#endif
}

/***************************************************************************
 * Function: int get_connection(int port_fd)
 *
 * Description:
 *   Accept a new connection from the indicated port and add it to the list
 * of current connections.  Return TRUE if a connection was successfully
 * accepted, FALSE otherwise.
 **************************************************************************/
static int get_connection(int port_fd) {
  conn_data *conn;
  struct sockaddr_in s_address;
  int new_fd, s_addr_size;
  extern int errno;

/*
 * Prepare to accept a new connection by getting the local port's name.
 */
  s_addr_size = sizeof(s_address);
  if (getsockname(port_fd, (struct sockaddr *)&s_address, &s_addr_size) < 0) {
    fprintf(stderr, "Error: Couldn't get socket name from fd %d.\n", port_fd);
    perror("  get_connection(getsockname)");
    exit(ERROR);
  }

/*
 * Accept the new connection off of the port/socket.  If errno == EWOULDBLOCK,
 * there wasn't really a new connection to accept, so return FALSE.  If there
 * _was_ a new connection, set the file descriptor that has been associated
 * with it non_blocking.
 */
  if ((new_fd = accept(port_fd, (struct sockaddr *)&s_address,
       &s_addr_size)) < 0) {
    if (errno == EWOULDBLOCK) return(FALSE);
    fprintf(stderr, "Error: Couldn't accept new connection from fd %d.\n",
            port_fd);
    perror("  get_connection(accept)");
    exit(ERROR);
  }
  fd_noblock(new_fd);

/*
 * Create and initialize a conn_data object for the new connection, and
 * add it to the list of connections.
 */
  if ((conn = (conn_data *)malloc(sizeof(conn_data))) == NULL) {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    perror("  get_connection(malloc)");
    exit(ERROR);
  }
  bzero(conn, sizeof(conn_data));
  conn->fd = new_fd;
  sprint_ipnum(conn->addr, &s_address);
  time(&(conn->connect_time));
  conn->input = create_list();
  conn->output = create_list();
  add_data(conn_list, conn, PREPEND);

  fprintf(stderr, "Connect on fd %d from %s assigned fd %d (%s)\n",
          port_fd, conn->addr, conn->fd, ctime(&(conn->connect_time)));

  welcome(conn);

  return(TRUE);
}

/***************************************************************************
 * Function: void disconnect(conn_data *conn)
 *
 * Description:
 *   Remove the indicated connection from the list of connections,
 * close the file descriptor associated with it, and free all of the
 * memory it's using (including dereferencing of any strings still in
 * its input and output lists).
 **************************************************************************/
void disconnect(conn_data *conn) {
  find_data(conn_list, conn, ptrcmp, TRUE);
  close(conn->fd);
  destroy_list(conn->input, deref_string);
  destroy_list(conn->output, deref_string);
  free(conn);
}

/***************************************************************************
 * Function: void close_port(int port_fd)
 *
 * Description:
 *   Remove the indicated port from the list of ports, close the file
 * descriptor associated with it, and free all of the memory it is using.
 **************************************************************************/
void close_port(port_data *port) {
  find_data(port_list, port, ptrcmp, TRUE);
  close(port->fd);
  free(port);
}

/***************************************************************************
 * Function: int send_text(conn_data *conn)
 *
 * Description:
 *   Write any data waiting in the connection's output string list to its
 * file descriptor.  All of the text in the list should have been through
 * any relevant processing already.  If a write to the connection's file
 * descriptor returns -1 and errno is set to something other than EWOULDBLOCK,
 * assume the connection has broken and return ERROR.  Otherwise return the
 * number of bytes successfully sent.  Note that this is BSD specific.  The
 * man page for write says that Sys V returns 0 if the write would block,
 * not -1.  However, since a write that returns 0 will end up being treated
 * the same as one that returns -1 and sets errno to EWOULDBLOCK in the long
 * run, the difference is irrelevant.
 **************************************************************************/
static int send_text(conn_data *conn) {
  int     result, length, total;
  string  *str;
  list    *output;

  result = length = total = 0;
  output = conn->output;

  while (TRUE) {
    str = get_data(output, FALSE);
    if (conn->out_pos == NULL) conn->out_pos = str->text;
    length = strlen(conn->out_pos);
    if ((result = write(conn->fd, conn->out_pos, length)) < 0) {
#ifdef BSD
      if (errno == EWOULDBLOCK) return(total);
#endif
      return(ERROR);
    }

    total += result;

/*
 * If the entire text of the current string was successfully sent, remove
 * it from the connection's output list, dereference it, and leave the
 * output loop if there are no more strings on the output list.  If the
 * entire text was _not_ output, set the connection's out_pos pointer to
 * indicate the location where future output should resume.
 */
    if (result == length) {
      get_data(output, TRUE);
      deref_string(str);
      conn->out_pos = NULL;
      if (length_list(output) == 0) break;
    } else {
      conn->out_pos += result;
      break;
    }
  }
  return(total);
}

/***************************************************************************
 * Function: int recv_text(conn_data *conn)
 *
 * Description:
 *   Read data from the file descriptor associated with a connection and
 * add it to the connection's input buffer.  If the read returns -1 and either
 * 1) errno isn't set to EWOULDBLOCK (BSD), or 2) the OS is Sys V, an error has
 * occured and ERROR is returned.  If errno _is_ EWOULDBLOCK (under BSD), or the
 * return value was 0 (under Sys V), no data remains to be read from the
 * socket.  Under normal conditions return the number of bytes received.
 **************************************************************************/
static int recv_text(conn_data *conn) {
  int count, total = 0, result, length;

/*
 * Keep reading data from the socket until there's nothing left to read.
 */
  do {
    length = strlen(conn->input_buf);
    if ((count = read(conn->fd, conn->input_buf + length,
         MAX_INPUT_LEN - length)) < 0) {
#ifdef BSD
      if (errno == EWOULDBLOCK) return(total);
#endif
      return(ERROR);
    }
 
/*
 * If anything was read in, call filter_input to move it to the input list.
 */
    if (count != 0) {
      total += count;
      conn->input_buf[length + count] = '\0';
      result = filter_input(conn);
      if (result == ERROR) return(ERROR);
    }
  } while (length + count == MAX_INPUT_LEN);
  return(total);
}

/***************************************************************************
 * Function: void create_port(int portnum)
 *
 * Description:
 *   Open a new socket for receiving connections and add it to the port_list.
 **************************************************************************/
void create_port(int portnum) {
  port_data *new_port;
  int port_fd, socket_option;
  char host_name[MAXHOSTNAMELEN + 1];
  struct hostent *host_data;
  struct linger linger_option;
  struct sockaddr_in s_address;

/*
 * Create the socket and set it reusable and non-lingering.
 */
  if ((port_fd = socket(AF_INET, SOCK_STREAM, IP_PROTOCOL)) < 0) {
    fprintf(stderr, "Error: Couldn't create socket (fd = %d).\n", port_fd);
    perror("  create_port(socket)");
    exit(ERROR);
  }
  socket_option = ON;
  if (setsockopt(port_fd, SOL_SOCKET, SO_REUSEADDR,
			 &socket_option, sizeof(socket_option)) < 0) {
    fprintf(stderr, "Error: Couldn't set fd %d reusable.\n", port_fd);
    perror("  create_port(setsockopt:SO_REUSEADDR)");
    exit(ERROR);
  }
  linger_option.l_onoff = OFF;
  linger_option.l_linger = 0;
  if (setsockopt(port_fd, SOL_SOCKET, SO_LINGER,
			 &linger_option, sizeof(linger_option)) < 0) {
    fprintf(stderr, "Error: Couldn't set fd %d non-lingering.\n", port_fd);
    perror("  create_port(setsockopt:SO_LINGER)");
    exit(ERROR);
  }

/*
 * Now we have a bona fide UNIX socket, but there isn't any information about
 * addresses and such associated with it yet.  To fix that, we must first
 * obtain address information about our local machine.
 */
  if (gethostname(host_name, MAXHOSTNAMELEN + 1) < 0) {
    fprintf(stderr, "Error: Couldn't get host machine's name.\n");
    perror("  create_port(gethostname)");
    exit(ERROR);
  }
  if ((host_data = gethostbyname(host_name)) == NULL) {
    fprintf(stderr, "Error: Couldn't get host machine's entry.\n");
    perror("  create_port(gethostbyname)");
    exit(ERROR);
  }
  bzero(&s_address, sizeof(s_address));
  s_address.sin_family = host_data->h_addrtype;
  s_address.sin_port = htons(portnum);

/*
 * Now that we have our address information (machine address and port number),
 * associate the address with the socket (referenced by a file descriptor)
 * and set it up to listen for connections in a non-blocking fashion with a
 * maximum pending connection queue length of 5.
 */
  if (bind(port_fd, (struct sockaddr *)&s_address, sizeof(s_address)) < 0) {
    fprintf(stderr, "Error: Couldn't bind fd %d to address.\n", port_fd);
    perror("  create_port(bind)");
    exit(ERROR);
  }
  fd_noblock(port_fd);
  listen(port_fd, 5);

/*
 * Create a new port_data object to represent the socket and add it
 * to the list of ports.
 */
  if ((new_port = (port_data *)malloc(sizeof(port_data))) == NULL) {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    perror("  create_port(malloc)");
    exit(ERROR);
  }
  bzero(new_port, sizeof(port_data));
  new_port->portnum = portnum;
  new_port->fd = port_fd;
  add_data(port_list, new_port, PREPEND);
}

/***************************************************************************
 * Function: void gather_input()
 *
 * Description:
 *   Use select() to examine activity on all open sockets/file-descriptors.
 * This includes new connections on port sockets and input on connection
 * sockets (and, I suppose, exception states on either).  After eliminating
 * any sockets in exception states, process all input on remaining sockets.
 **************************************************************************/
void gather_input() {
  int fd;
  port_data *port;
  conn_data *conn;
  fd_set fd_inset, fd_excset;
  struct timeval sel_timeout;
  iterator *conn_iter = create_iter(conn_list);
  iterator *port_iter = create_iter(port_list);

/*
 * Initialize the input and exception fd_sets to the empty state, then add
 * all of the port and connection socket file descriptors.
 *
 */
  FD_ZERO(&fd_inset);
  FD_ZERO(&fd_excset);
  for (init_iter(port_iter); (port = peek_iter(port_iter));
       iterate_iter(port_iter)) {
    fd = port->fd;
    FD_SET(fd, &fd_inset);
    FD_SET(fd, &fd_excset);
  }
  for (init_iter(conn_iter); (conn = peek_iter(conn_iter));
       iterate_iter(conn_iter)) {
    fd = conn->fd;
    FD_SET(fd, &fd_inset);
    FD_SET(fd, &fd_excset);
  }

/*
 * Calling "select" will remove all sockets from each of the fd_sets to
 * which they don't actually belong.  In other words, if there isn't any
 * input on a given connection (or no new connection on a port), it is
 * removed from the input set.  Note that NOFILE is the maximum number of
 * open files per process, and, I suspect, is not a portable constant.  If
 * it isn't defined anywhere on your system, you'll probably have to use the
 * "getrlimit" system call to find out what that number is.  Also note that
 * select will sleep for the amount of time indicated in sel_timeout before
 * returning, which can be used to regulate how much time is consumed by
 * each game loop.  For now, though, the timeout is set to 0, since we're
 * using "usleep" in the main loop.
 */
  bzero(&sel_timeout, sizeof(struct timeval));
  if (select(NOFILE, &fd_inset, NULL, &fd_excset, &sel_timeout) <0 ) {
    fprintf(stderr, "Error: Selection of ready file descriptors failed.\n");
    perror("  gather_input(select)");
    exit(ERROR);
  }

/*
 * Process all port sockets, checking for new connections and exception states.
 * Since multiple connections may be pending on a single port, we keep calling
 * "get_connection" on each port in the input set until that function returns
 * FALSE, indicating that no more connections are pending on it.
 */
  for (init_iter(port_iter); (port = peek_iter(port_iter));
       iterate_iter(port_iter)) {
    fd = port->fd;
    if (FD_ISSET(fd, &fd_excset)) {
      fprintf(stderr, "Error: Exception state on port %d (fd %d).\n",
              port->portnum, fd);
      exit(ERROR);
    } else if (FD_ISSET(fd, &fd_inset))
      while (get_connection(fd));
  }

/*
 * Process connection sockets, checking for new input and exception conditions.
 * Note that it is necessary to iterate the iterator _before_ executing the
 * loop in case the current connection gets disconnected.
 */
  for (init_iter(conn_iter); (conn = peek_iter(conn_iter)); ) {
    iterate_iter(conn_iter);
    fd = conn->fd;

/*
 * Check to see if this connection is in an exception state.  If it is,
 * disconnect it.
 */
    if (FD_ISSET(fd, &fd_excset)) {
      FD_CLR(fd, &fd_inset);
      disconnect(conn);
      continue;
    }

/*
 * Check to see if this connection has pending input.  If it does, attempt to
 * read it all in.  If nothing can be read in, disconnect it.
 */
    if (FD_ISSET(fd, &fd_inset)) {
      if (recv_text(conn) <= 0) {
        disconnect(conn);
        continue;
      }
      time(&(conn->input_time));
    }
  }

  destroy_iter(port_iter);
  destroy_iter(conn_iter);
}

/***************************************************************************
 * Function: void flush_output()
 *
 * Description:
 *   Use select() to determine which connections that have output pending
 * are capable of accepting output at the moment, and send as much data to
 * those connections as they will accept.
 **************************************************************************/
void flush_output() {
  conn_data *conn;
  fd_set fd_outset;
  struct timeval sel_timeout;
  iterator *conn_iter = create_iter(conn_list);;

/*
 * Clear out the fd_set and add to it the fds of all connections with
 * output pending.
 */
  FD_ZERO(&fd_outset);
  for (init_iter(conn_iter); (conn = peek_iter(conn_iter));
       iterate_iter(conn_iter)) {
    if (length_list(conn->output) != 0) FD_SET(conn->fd, &fd_outset);
  }

/*
 * Use select to determine which fds actually belong in the set.
 * (NOFILE is the maximum number of open files per process, as described
 * previously)
 */
  bzero(&sel_timeout, sizeof(struct timeval));
  if (select(NOFILE, NULL, &fd_outset, NULL, &sel_timeout) <0 ) {
    fprintf(stderr, "Error: Selection of ready file descriptors failed.\n");
    perror("  flush_output(select)");
    exit(ERROR);
  }

/*
 * Process connections with pending output.  Note that it is necessary to
 * iterate the iterator _before_ executing the loop in case the connection
 * we're currently working on gets disconnected.
 */
  for (init_iter(conn_iter); (conn = peek_iter(conn_iter)); ) {
    iterate_iter(conn_iter);

    if (FD_ISSET(conn->fd, &fd_outset)) {
      if (send_text(conn) <= 0) {
        disconnect(conn);
        continue;
      }
    }
  }

  destroy_iter(conn_iter);
}
