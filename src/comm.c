/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#include <sys/time.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "merc.h"
#include "mem.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_echo		);
DECLARE_DO_FUN(do_last		);
DECLARE_DO_FUN(do_unread	);


/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
#include <signal.h>
#if !defined( STDOUT_FILENO )
#define STDOUT_FILENO 1
#endif
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
/*
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
*/
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
/*
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
*/
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/*
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
*/
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
/*
int	setsockopt	args( ( int s, int level, int optname, caddr_toptval,
			    int optlen ) ); */
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
/*
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
*/
#else
/*
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
*/
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
/*
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
*/
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
time_t		    boot_time;		/* time of boot */	

int                 mudport;            /* mudport */
bool		    close_control = TRUE;

/*
 * OS-dependent local functions.
 */
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
DESCRIPTOR_DATA *set_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );


/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
bool    output_buffer           args( ( DESCRIPTOR_DATA *d ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );


int     group_cost	args( ( CHAR_DATA *ch, int gn ) );
void	give_spell	args( ( CHAR_DATA *ch ) );
void	give_weaves	args( ( CHAR_DATA *ch ) );
void	give_max	args( ( CHAR_DATA *ch ) );
void	give_talent	args( ( CHAR_DATA *ch ) );
void    handedness      args( ( CHAR_DATA *ch ) );

pid_t   wait		args( ( int *status ) );
pid_t   waitpid         args( ( pid_t pid, int *status, int options ) );
pid_t   fork            args( ( void ) );
int     kill            args( ( pid_t pid, int sig ) );
int     pipe            args( ( int filedes[2] ) );
int     dup2            args( ( int oldfd, int newfd ) );
int     execl           args( ( const char *path, const char *arg, ... ) );
void	restore_conns	args( ( void ) );

char	program_name[1024+1];
int	port_number;
int	control_socket;
bool	fileBoot = FALSE;

void save_area_list();
void save_area( AREA_DATA *pArea );
void save_guilds();
void save_helps();



void my_sig_handler( int signum )
{
    int status;
    pid_t pid;
    char buf[MAX_STRING_LENGTH];
  
    if ( (pid = wait( &status )) < 0)
    {
	perror( "Error: zombie child" );
    }
    else
    {
	sprintf(buf, "reaped child %d", pid);
	log_string( buf );
    }
}  

void seg_handler( int signum )
{
    CHAR_DATA *vch, *vch_next;
    AREA_DATA *pArea;
    DESCRIPTOR_DATA *d, *d_next;

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;

	send_to_char( "Your character is saved.\n\r", vch );
	interpret( vch, "save" );
    }

    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }

    save_helps( );
    save_guilds( );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        d_next = d->next;
	if ( signum == SIGSEGV )
	    write_to_buffer( d, "Segmentation fault!  MUD crashing.\n\r", 0 );
	else
	    write_to_buffer( d, "Floating point exception!  MUD crashing.\n\r", 0 );
        close_socket(d);
    }
    abort();
}

extern bool close_control; 
int execv( const char *path, char *const argv[]);

void hup_handler( int signum )
{
    AREA_DATA *pArea;
    extern char program_name[];
    extern int port_number, control_socket;
    FILE *file;
    char *argv[]  = { program_name, "", NULL };
    char buf[MAX_STRING_LENGTH];   
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    CHAR_DATA *vch, *vch_next;
    int temp;

    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;

	send_to_char( "Your character is saved.\n\r", vch );
	interpret( vch, "save" );
    }

    save_helps( );
    save_guilds( );

    system( "/home/weave/scripts/kill_resolv.pl" );
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
        d_next = d->next;
	write_to_buffer( d, "HUP signal sent.  MUD rebooting.\r\n", 0 );
    
        if (d->ipid > 0)          /* Kill any old resolve */
        {
            kill(d->ipid, SIGKILL);
            waitpid(d->ipid,&temp,0);
            close( d->ifd );
        }
    }
    
    merc_down = TRUE;
    if ( (file = fopen( "reboot.tmp", "w" )) == NULL )
    {
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;
            close_socket( d );
        }
        return;
    }
    close_control = FALSE;
    fprintf(file,"%d\n",control_socket);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {       
       if (d->connected == CON_PLAYING && CH(d) && CH(d)->in_room != NULL)
          fprintf(file,"CHAR %d %s~\n",
                       d->descriptor,CH(d)->name);
        
       else fprintf(file,"DESC %d\n",d->descriptor);
    }
    fprintf(file,"DESC -1\n");
    fclose( file );
        
    log_string( "Rebooting.");
    
    sprintf( buf, "%d", port_number );
    argv[1] = buf;
    execv( program_name, argv );
    exit( 1 );
}

void term_handler( int signum )
{
    CHAR_DATA *vch, *vch_next;
    AREA_DATA *pArea;
    DESCRIPTOR_DATA *d, *d_next;

    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
    {
        save_area( pArea );
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;

	send_to_char( "Your character is saved.\n\r", vch );
	interpret( vch, "save" );
    }

    save_helps( );
    save_guilds( );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        d_next = d->next;
	write_to_buffer( d, "TERM signal sent.  MUD shutting down.\n\r", 0 );
        close_socket(d);
    }
    exit( 0 );
}


int main( int argc, char **argv )
{
    struct timeval now_time;
    int port;

    int control;

    signal( SIGTERM, term_handler );
    signal( SIGHUP, hup_handler );

    strcpy( program_name, argv[0] );
    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    boot_time		= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 6060;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }
/*
    if ( port != 6063 )
    {
	signal( SIGSEGV, seg_handler );
	signal( SIGFPE, seg_handler );
    }	
*/
    /*
     * Run the game.
     */
    mudport = port;			/* <--- add this line */
    port_number = port;
    control = init_socket( port );
    control_socket = control;
    boot_db( );
    sprintf( log_buf, "ROM is ready to rock on port %d.", port );
    log_string( log_buf );
    restore_conns( );
    game_loop_unix( control );
    if (close_control)
	close (control);
    append_file( NULL, COMMAND_FILE, "MUD has rebooted." );

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;
    FILE *fp;

    if ( ( fp = fopen( "reboot.tmp", "r" ) ) != NULL )
    {
       fd  = fread_number( fp );
       fclose( fp );
       return fd;
    }

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}


/* 
 * Here comes the ident driver code.
 * - Wreck
 */

/*
 * Almost the same as read_from_buffer...
 */
bool read_from_ident( int fd, char *buffer )
{
    static char inbuf[MAX_STRING_LENGTH*2];
    int iStart, i, j, k;

    /* Check for overflow. */
    iStart = strlen( inbuf );
    if ( iStart >= sizeof( inbuf ) - 10 )
    {
	log_string( "Ident input overflow!!!" );
	return FALSE;
    }

    /* Snarf input. */
    for ( ; ; )
    {
	int nRead;

	nRead = read( fd, inbuf + iStart, sizeof( inbuf ) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( inbuf[iStart-2] == '\n' || inbuf[iStart-2] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_ident" );
	    return FALSE;
	}
    }

    inbuf[iStart] = '\0';

    /*
     * Look for at least one new line.
     */
    for ( i = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
    {
	if ( inbuf[i] == '\0' )
	    return FALSE;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
    {
	if ( inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii( inbuf[i] ) && isprint( inbuf[i] ) )
	    buffer[k++] = inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	buffer[k++] = ' ';
    buffer[k] = '\0';

    /*
     * Shift the input buffer.
     */
    while ( inbuf[i] == '\n' || inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( inbuf[j] = inbuf[i+j] ) != '\0'; j++ )
	;
    
    return TRUE;
}

/*
 * Process input that we got from the ident process.
 */
void process_ident( DESCRIPTOR_DATA *d )
{
    char buffer[MAX_INPUT_LENGTH];
    char address[MAX_INPUT_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    CHAR_DATA *ch=CH( d );
    char *user;
    sh_int results=0;
    int status;
    
    buffer[0]='\0';
    
    if ( !read_from_ident( d->ifd, buffer ) || IS_NULLSTR( buffer ) )
    	return;
    
    /* using first arg since we want to keep case */
    user=first_arg( buffer, address, FALSE );
    
    /* replace and set some states */
    if ( !IS_NULLSTR( user ) )
    {
        replace_string( d->ident, user );
        SET_BIT( results, 2 );
    }
    if ( !IS_NULLSTR( address ) )
    {
        replace_string( d->host, address );
        SET_BIT( results, 1 );
    }
    
    /* do sensible output */
    if ( results==1 ) /* address only */
    {
	/*
	 * Change the two lines below to your notification function...
	 * (wiznet, ..., whatever)
	 */
	if ( ch )
	{
	    sprintf( outbuf, "$N has address `4%s`n. Username unknown.", address );
	    wiznet( outbuf,ch,NULL,WIZ_LOGINS,0,0);
	    sprintf( log_buf, "%s has address %s.", ch->name, address );
	    log_string( log_buf );
	}
	else
	{
	    sprintf( outbuf, "Descriptor %d from address `4%s`n. Username unknown.",
		d->descriptor, address );
	    wiznet( outbuf,ch,NULL,WIZ_LOGINS,0,0);
	    sprintf( log_buf, "Descriptor %d has address %s.",
		d->descriptor, address );
	    log_string( log_buf );
	}
    }
    else if ( results==2 || results==3 ) /* ident only, or both */
    {
	/*
	 * Change the two lines below to your notification function...
	 * (wiznet, ..., whatever)
	 */
	if ( ch )
	{
	    sprintf( outbuf, "$N is `4%s`n@`4%s`n.", user, address );
	    wiznet( outbuf,ch,NULL,WIZ_LOGINS,0,0);
	    sprintf( log_buf, "%s is %s@%s.", ch->name, user, address );
	    log_string( log_buf );
	}
	else
	{
	    sprintf( outbuf, "Descriptor %d from `4%s`n@`4%s`n.",
		d->descriptor, user, address );
	    wiznet( outbuf,ch,NULL,WIZ_LOGINS,0,0);
	    sprintf( log_buf, "Descriptor %d from %s@%s.",
		d->descriptor, user, address );
	    log_string( log_buf );
	}
    }
    else
    {
	if ( ch )
	{
	    sprintf( log_buf, "%s could not be identified.", ch->name );
	    wiznet( log_buf,ch,NULL,WIZ_LOGINS,0,0);
            log_string( log_buf );
	}
	else
	{
	    sprintf( log_buf, "Descriptor %d could not be identified.",
		d->descriptor );
	    wiznet( log_buf,ch,NULL,WIZ_LOGINS,0,0);
            log_string( log_buf );
	}
    }
    
    /* close descriptor and kill ident process */
    /* 
     * we don't have to check here, 
     * cos the child is probably dead already. (but out of safety we do)
     * 
     * (later) I found this not to be true. The call to waitpid( ) is
     * necessary, because otherwise the child processes become zombie
     * and keep lingering around... The waitpid( ) removes them.
     */
    waitpid( d->ipid, &status, 0 );
    close( d->ifd );
    d->ifd=-1;

    d->ipid=-1;
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(d, BAN_ALL))
    {
        write_to_descriptor( d->descriptor,
            "Your site has been banned from this mud.\n\r", 0 );
        close( d->descriptor );
        free_descriptor(d);
        return;
    }

    if ( !d->got_ident )
	d->connected = CON_GET_NAME;
    d->got_ident = TRUE;

    if ( d->connected != CON_PLAYING )
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( d, help_greeting+1, 0 );
	else
	    write_to_buffer( d, help_greeting  , 0 );
    }

    return;    
}


void create_ident( DESCRIPTOR_DATA *d, long ip, sh_int port )
{
    int fds[2];
    pid_t pid;
    
    /* create pipe first */
    if ( pipe( fds )!=0 )
    {
        perror( "Create_ident: pipe: " );
    
	if ( !d->got_ident )
	    d->connected = CON_GET_NAME;
	if ( d->connected != CON_PLAYING )
    	{
	    extern char * help_greeting;
	    if ( help_greeting[0] == '.' )
		write_to_buffer( d, help_greeting+1, 0 );
	    else
		write_to_buffer( d, help_greeting  , 0 );
	}
        return;
    }
    
    if ( dup2( fds[1], STDOUT_FILENO )!=STDOUT_FILENO )
    {
        perror( "Create_ident: dup2(stdout): " );
	close( fds[0] );
    
	if ( !d->got_ident )
	    d->connected = CON_GET_NAME;
	if ( d->connected != CON_PLAYING )
    	{
	    extern char * help_greeting;
	    if ( help_greeting[0] == '.' )
		write_to_buffer( d, help_greeting+1, 0 );
	    else
		write_to_buffer( d, help_greeting  , 0 );
	}
        return;
    }
    
    if ( (pid=fork( ))>0 )
    {
    	/* parent process */
    	d->ifd=fds[0];
    	d->ipid=pid;
    }
    else if ( pid==0 )
    {
    	/* child process */
	char str_ip[64], str_local[64], str_remote[64];
        
    	d->ifd=fds[0]; 
    	d->ipid=pid;

	sprintf( str_local, "%d", mudport );
	sprintf( str_remote, "%d", port );
	sprintf( str_ip, "%ld", ip );
    	execl( "../src/resolve", "resolve", str_local, str_ip, str_remote, 0 );
    	/* Still here --> hmm. An error. */
    	log_string( "Exec failed; Closing child." );
	close( fds[0] );
    	d->ifd=-1;
    	d->ipid=-1;
    
	if ( !d->got_ident )
		d->connected = CON_GET_NAME;
	if ( d->connected != CON_PLAYING )
    	{
	    extern char * help_greeting;
	    if ( help_greeting[0] == '.' )
		write_to_buffer( d, help_greeting+1, 0 );
	    else
		write_to_buffer( d, help_greeting  , 0 );
	}
    	exit( 0 );
    }
    else 
    {
    	/* error */
    	perror( "Create_ident: fork" );
	close( fds[0] );
    
	if ( !d->got_ident )
	    d->connected = CON_GET_NAME;
	if ( d->connected != CON_PLAYING )
    	{
	    extern char * help_greeting;
	    if ( help_greeting[0] == '.' )
		write_to_buffer( d, help_greeting+1, 0 );
	    else
		write_to_buffer( d, help_greeting  , 0 );
	}
    }
    close( fds[1] );
}

void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	    if ( d->ifd!=-1 && d->ipid!=-1 )
	    {
	    	maxdesc = UMAX( maxdesc, d->ifd );
	    	FD_SET( d->ifd, &in_set );
	    }
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    if ( errno == EINTR )
		continue;

	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
/*	if ( FD_ISSET( control, &in_set ) )
	    set_descriptor( control );*/

    if ( FD_ISSET( control, &in_set ) )
    {
          struct sockaddr_in sock;
          socklen_t size;
          int desc;

          size = sizeof( sock );
          getsockname( control, (struct sockaddr *) &sock, &size );

          if ( ( desc = accept( control, (struct sockaddr *) &sock,
            &size ) ) < 0 ) {
              perror( "Set_descriptor: accept" );
              goto skip;
          }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

          if ( fcntl( desc, F_SETFL, FNDELAY ) < 0 ) {
              perror( "Set_descriptor: fcntl: FNDELAY" );
              goto skip;
          }

          set_descriptor( desc );
      }
    skip:



	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character )
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		{
		    d->character->timer = 0;
		    d->idle = 0;
		}
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }
	    /* check for input from the ident */
	    if ( ( d->connected == CON_IDENT_WAIT
	    ||     CH(d)!=NULL
	    ||     d->connected == CON_PLAYING ) && 
	         d->ifd!=-1 && FD_ISSET( d->ifd, &in_set ) )
	        process_ident( d );


	    if ( d->character != NULL && d->character->wait > 0 )
		continue;

            if ( d->delayed[0] != '\0') /* To fix alias thingie */
            {
               char *point;
               char buf[MAX_INPUT_LENGTH];
               for (point = d->delayed;*point != '\0';point++)
               {
                  if (*point == ';' && *(point + 1) == ';')
                  {
                     *point++ = '\0';
                     *point++ = '\0';
                     break;
                  }
               }
               if ( !d->pString && !run_olc_editor( d, d->delayed ) )
                  interpret(d->character,d->delayed);

               strcpy(buf,point);           /* Need to be enhanced */
               strcpy(d->delayed,buf);

               if (*point == '\0')
                  *(d->delayed) = 0;

               continue;
            }


	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point )
                    show_string( d, d->incomm );
        	else
        	    if ( d->pString )
            		string_add( d->character, d->incomm );
        	    else
            		switch ( d->connected )
            		{
               	 	    case CON_PLAYING:
                    		if ( !run_olc_editor( d, d->incomm ) )
                        	substitute_alias( d, d->incomm );
                    		break;
                	    default:
                    		nanny( d, d->incomm );
                    		break;
            		}

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    if ( errno == EINTR )
			continue;

		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}



DESCRIPTOR_DATA *set_descriptor( int desc )
{
    static DESCRIPTOR_DATA d_zero;
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    socklen_t size;

    /*
     * Cons a new descriptor.
     */
    dnew		= new_descriptor( );

    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->connected	= CON_IDENT_WAIT;
    if ( fileBoot )
    {
	dnew->connected	= CON_PLAYING;
	dnew->got_ident = TRUE;
    }
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->pEdit         = NULL;                 /* OLC */
    dnew->pString       = NULL;                 /* OLC */
    dnew->editor        = 0;                    /* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );
    /* Add the following three */
    dnew->ident		= str_dup( "???" );
    dnew->ifd		= -1;
    dnew->ipid		= -1;
    size = sizeof(sock);

    if ( !fileBoot )
	write_to_buffer( dnew,
	    "Doing hostname lookup.  This will take one minute at the most.\r\n",
	    0 );

    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "Set_descriptor: getpeername" );
	free_string( dnew->host );
	dnew->host = str_dup( "(unknown)" );
	if ( !dnew->got_ident )
	    dnew->connected = CON_GET_NAME;
	if ( dnew->connected != CON_PLAYING )
    	{
	    extern char * help_greeting;
	    if ( help_greeting[0] == '.' )
		write_to_buffer( dnew, help_greeting+1, 0 );
	    else
		write_to_buffer( dnew, help_greeting  , 0 );
	}
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	create_ident( dnew, sock.sin_addr.s_addr, ntohs( sock.sin_port ) );
	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
        /* Use just IP address for now. */
	free_string( dnew->host );
	dnew->host=str_dup( buf );
	dnew->port=ntohs( sock.sin_port );
	sprintf( log_buf, "New socket %d from %s", dnew->descriptor, buf );
	log_string( log_buf );
    }

    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    return dnew;
}

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->ipid>-1 ) 
    {
	int status;

	kill( dclose->ipid, SIGKILL );
	waitpid( dclose->ipid, &status, 0 );
	close( dclose->ifd );
	dclose->ifd = -1;
    }
    if ( dclose->ifd > -1 )
    	close( dclose->ifd );

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( (ch = dclose->character) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	if ( dclose->connected == CON_PLAYING && !merc_down )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	}
	else
	{
	    if ( dclose->character->char_made == FALSE )
	    {
		char strsave[MAX_INPUT_LENGTH];
		sprintf( strsave, "%s%c/%s", PLAYER_DIR,
		    LOWER(ch->name[0]), correct_name(ch->name) );
		unlink(strsave);
	    }
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_string( dclose->host );
    free_string( dclose->ident );
    /* RT socket leak fix -- I hope */
    free_mem(dclose->outbuf,dclose->outsize);
    free_string(dclose->showstr_head); 
    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    if ( d->connected == CON_IDENT_WAIT )
	return TRUE;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 20 && d->character
	    &&   d->connected == CON_PLAYING )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
                wiznet("Spam spam spam $N spam spam spam spam spam!",
		    d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
                if (d->incomm[0] == '!')
                    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0, 
                        get_trust(d->character));
                else
                    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
                        get_trust(d->character));
		WAIT_STATE(d->character, d->repeat - 15);
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
    {
        if ( d->showstr_point )
            write_to_buffer( d, "`n[Hit Return to continue]\n\r", 0 );
        else if ( fPrompt && d->pString
	     &&   (d->connected == CON_PLAYING || d->connected == CON_MAKE_DESCRIPTION) )
	    write_to_buffer( d, "> ", 2 );
        else if ( fPrompt && d->connected == CON_PLAYING )
	{
   	    CHAR_DATA *ch;
	    CHAR_DATA *victim;

	    ch = d->character;

	    if ( !ch )
	    {
		bug( "Process_output: NULL ch", 0 );
		return FALSE;
	    }

	    if ( ch->fighting != NULL && IS_SET(ch->act, PLR_FOECOND) )
	    {
		int percent;
		char wound[100];
		char buf[MAX_STRING_LENGTH];

		victim = ch->fighting;

		if (victim->max_hit > 0)
                    percent = victim->hit * 100 / victim->max_hit;
		else
                    percent = -1;

		if (percent >= 100)
		    sprintf(wound,"is in excellent condition.");
		else if (percent >= 90)
		    sprintf(wound,"has a few scratches.");
		else if (percent >= 75)
		    sprintf(wound,"has some small wounds and bruises.");
		else if (percent >= 50)
		    sprintf(wound,"has quite a few wounds.");
		else if (percent >= 30)
		    sprintf(wound,"has some big nasty wounds and scratches.");
		else if (percent >= 15)
		    sprintf(wound,"looks pretty hurt.");
		else if (percent >= 0)
		    sprintf(wound,"is in awful condition.");
		else
		    sprintf(wound,"is bleeding to death.");

		sprintf(buf,"`3%s %s`n \n\r", 
		    IS_NPC(victim) ? victim->short_descr : victim->name,wound);

		buf[0] = UPPER(buf[0]);

		send_to_char( buf, ch );
	    }
	    if ( IS_SET(ch->comm, COMM_TANKCOND)
	    &&   ch->in_room != NULL )
	    {
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *vch;
		int val1 = 0;

	    
		for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
		{
		    if ( ch == vch )
			continue;

		    if ( ch == vch->fighting )
			continue;

		    if ( vch == NULL || vch->fighting == NULL )
			continue;

		    if ( !is_same_group( ch, vch->fighting ) )
			continue;

		    if ( vch->fighting->max_hit > 0 )
                	val1 = vch->fighting->hit * 100 / vch->fighting->max_hit;

		    sprintf( buf, "`n%s [%d%%] is fighting %s%s%s`n.\n\r",
			PERS(vch->fighting,ch), val1,
			(vch->fighting->fighting == ch->fighting) ? "`3*" : "",
			(vch->fighting->fighting != NULL) ?
			PERS(vch->fighting->fighting,ch) : "no one",
			(vch == ch->fighting) ? "*" : "" );
		    send_to_char( buf, ch );
		}
	    }

	    ch = d->original ? d->original : d->character;

	    if ( IS_SET(ch->comm, COMM_PROMPT) )
		bust_a_prompt( d->character );

	    if (IS_SET(ch->comm,COMM_TELNET_GA))
		write_to_buffer(d,go_ahead_str,0);
	}
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;


    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL && d->connected == CON_PLAYING )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name, 0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    /*   Old stuff 
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
    */
    /*
     * OS-dependent output.
     *
     * now done at output_buffer( ) to deal with color codes.
     * - Wreck
     */
    return output_buffer( d );

}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( txt == NULL )
	return;

    if ( d == NULL )
	return;

    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize > 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, txt );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next, *d_new;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    NOTE_DATA *note;
    char *pwdnew;
    char *p;
    int i, num_on = 0;
    int race;
    bool fOld;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_IDENT_WAIT:
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    if ( d->character )
		free_char (d->character);
	    d->character = NULL;
	    return;
	}

	sprintf( log_buf, "%s is from %s%s%s",
	    argument,
	    !str_cmp( d->ident, "???" ) ? "" : d->ident,
	    !str_cmp( d->ident, "???" ) ? "" : "@",
	    d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,get_trust(ch));

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    wiznet(log_buf,NULL,NULL,WIZ_LOGINS,0,get_trust(ch));
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

        if (check_ban(d,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
        {
            write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
            close_socket(d);
            return;
        }


	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
                close_socket( d );
                return;
            }

            if (check_ban(d,BAN_NEWBIES))
            {
                write_to_buffer(d,
                    "New players are not allowed from your site.\n\r",0);
                close_socket(d);
                return;
            }

	    sprintf( log_buf, "New player: %s", ch->name );
	    log_string( log_buf );
	    wiznet(log_buf,NULL,NULL,WIZ_CREATE,0,get_trust(ch));

	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_PASSWORD_2;
	    return;
	}
 
	if (IS_NULLSTR(ch->pcdata->pwd) )
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	for ( d_new = descriptor_list; d_new != NULL; d_new = d_new->next )
            if ( d_new->connected == CON_PLAYING )
                num_on++;

	if ( num_on > max_on )
	    max_on = num_on;

	if ( check_reconnect(d, ch->name, TRUE) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;

	sprintf( log_buf, "%s (%s%s%s) has connected.", ch->name, 
		!str_cmp( d->ident, "???" ) ? "" : d->ident,
		!str_cmp( d->ident, "???" ) ? "" : "@",
		d->host );
	log_string( log_buf );
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
	if ( ch->level == -1 )
	{
	    send_to_char( "This character has been marked for recreation.\n\r", ch );
            write_to_buffer(d,"What is your last name (enter for none): ",0);
	    d->connected = CON_GET_LAST_NAME;
	    break;
	}
	if ( ch->pcdata->email == NULL )
	{
	    send_to_char( "You have no email address registered with us.\n\r",
		ch );
	    send_to_char( "We ask for your email address in order to be able\n\r",
		ch );
	    send_to_char( "to send you news on downtimes and changes.\n\r", ch );
	    send_to_char( "If you do not wish to give us your address, please\n\r",
		ch );
	    send_to_char( "enter your name and press enter, otherwise,\n\r",
		ch );
	    send_to_char( "enter your email address:  ", ch );
	    d->connected = CON_EMAIL;
	    break;
	}
	if ( IS_HERO(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;
    case CON_EMAIL:
	if ( ch->pcdata->email )
	    free_string( ch->pcdata->email );
	ch->pcdata->email = str_dup( argument );
	if ( IS_HERO(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;


/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( log_buf, "%s (%s%s%s) is a new player.", ch->name,
                !str_cmp( d->ident, "???" ) ? "" : d->ident,
                !str_cmp( d->ident, "???" ) ? "" : "@",
                d->host );
	    wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
	    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	    sprintf( buf, "New character: %s", ch->name );
	    log_string( buf );

	    write_to_buffer( d,
"\r\n"
"NOTE: Stat hunting (constant recreation in order to gain better stats)\r\n"
"is NOT allowed at the Weave.  Testing classes, trying different skills,\r\n"
"etc., may be inferred as being this.  You will be warned if you are\r\n"
"creating too many times.  If you feel you are being warned in error, or\r\n"
"if you have any questions about this, please ask any immortals who are on\r\n"
"at the time.\r\n"
"- Artanin\r\n\r\n", 0 );

	    sprintf( buf, "New character.\n\r\n\r"
"The Weave is a fantasy oriented MUD, and because of this, has certain\n\r" 
"name restrictions.  Names using book characters, or not fitting in with\n\r"
"the fantasy genre, are not allowed.  Examples of good names: Ororo, Yotian\n\r"
"Examples of bad names: MajorPower, DeathRider.  If you feel your name\n\r"
"is not appropriate, type NO.  Otherwise, type YES.  If you choose YES,\n\r"
"and we feel your name is not acceptable, you may be asked to change it.\n\r"
"Is your name acceptable? " );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_NAME_IS_FANTASY;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_NAME_IS_FANTASY:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	if ( !IS_NULLSTR(ch->name) )
	    pwdnew = crypt( argument, ch->name );
	else
	    pwdnew = "*()*";
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

        write_to_buffer(d,"\n\rWhat is your last name (enter for none): ",0);
        d->connected = CON_GET_LAST_NAME;
        break;

    case CON_GET_LAST_NAME:
	do_last( ch, argument );
	if ( strlen( ch->pcdata->last_name ) < 1 )
	    sprintf( buf, "Are you sure you don't want a last name? " );
	else
	    sprintf( buf, "Are you sure you want%s for a last name? ",
		ch->pcdata->last_name );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_CONFIRM_LAST_NAME;
	break;

    case CON_CONFIRM_LAST_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "Okay, you are named %s%s.\n\r",
		ch->name,
		ch->pcdata->last_name );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_QUESTION;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what would like for a last name? ", 0 );
	    d->connected = CON_GET_LAST_NAME;
	    return;

	default:
	    write_to_buffer( d, "Please type Yes or No.", 0 );
	    if ( ch->pcdata->last_name[0] == '\0' )
		sprintf( buf, "Are you sure you don't want a last name? " );
	    else
		sprintf( buf, "Are you sure you want %s for a last name? ",
		    ch->pcdata->last_name );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_LAST_NAME;
	    return;
	}
	write_to_buffer( d,
	    "\n\rYou must now choose a race for your character.  The following races", 0 );
	write_to_buffer( d,
	    "\n\rare available.  Please note: Choosing some races may restrict you from", 0 );
	write_to_buffer( d,
	    "\n\rjoining certain guilds.  I.e., no Warders who are Aiel (from the", 0 );
	write_to_buffer( d,
	    "\n\rThree Fold Land).  Type HELP for more info.\n\r", 0 );
	sprintf( buf,"\n\rRacial Bonuses\n\r%-15s  STR  INT  WIS  DEX  CON  CHR  LUC  AGI\n\r",
	    "Race" );
	write_to_buffer( d, buf, 0 );
        for ( race = 1; race_table[race].name != NULL; race++ )
        {
            if (!race_table[race].pc_race)
                break;
	    sprintf(buf, "%-15s  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d\n\r",
		race_table[race].name,
		pc_race_table[race].stats[0],
		pc_race_table[race].stats[1],
		pc_race_table[race].stats[2],
		pc_race_table[race].stats[3],
		pc_race_table[race].stats[4],
		pc_race_table[race].stats[5],
		pc_race_table[race].stats[6],
		pc_race_table[race].stats[7] );
	    write_to_buffer( d, buf, 0 );
        }
	write_to_buffer( d, "\n\r", 0 );
	write_to_buffer( d,
	    "What is your race (help for more information)? ",0); 
	d->connected = CON_GET_RACE;
	break;
    case CON_GET_RACE:
        one_argument(argument,arg);

        if (!strcmp(arg,"help"))
        {
            argument = one_argument(argument,arg);
            if (argument[0] == '\0')
                do_help(ch,"race help");
            else
                do_help(ch,argument);
            write_to_buffer(d,
                "What is your race (help for more information)? ",0); 
            break;
        }
        race = race_lookup(argument);

        if (race == 0 || !race_table[race].pc_race)
        {
            write_to_buffer(d,"That is not a valid race.\n\r",0);
	    write_to_buffer( d,
		"\n\rYou must now choose a race for your character.  The following races", 0 );
	    write_to_buffer( d,
		"\n\rare available.  Please note: Choosing some races may restrict you from", 0 );
	    write_to_buffer( d,
		"\n\rjoining certain guilds.  I.e., no Warders who are Aiel (from the", 0 );
	    write_to_buffer( d,
		"\n\rThree Fold Land).  Type HELP for more info.\n\r", 0 );
	sprintf( buf,"\n\rRacial Bonuses\n\r%-15s  STR  INT  WIS  DEX  CON  CHR LUC AGI\n\r",
	    "Race" );
	write_to_buffer( d, buf, 0 );
        for ( race = 1; race_table[race].name != NULL; race++ )
        {
            if (!race_table[race].pc_race)
                break;
	    sprintf(buf, "%-15s  %3d  %3d  %3d  %3d  %3d  %3d  %3d  %3d\n\r",
		race_table[race].name,
		pc_race_table[race].stats[0],
		pc_race_table[race].stats[1],
		pc_race_table[race].stats[2],
		pc_race_table[race].stats[3],
		pc_race_table[race].stats[4],
		pc_race_table[race].stats[5],
		pc_race_table[race].stats[6],
		pc_race_table[race].stats[7] );
	    write_to_buffer( d, buf, 0 );
        }
	write_to_buffer( d, "\n\r", 0 );
            write_to_buffer(d,
                "What is your race? (help for more information) ",0);
            break;
        }

	sprintf( buf, "Okay, you are of the %s race.\n\r",
	    race_table[race].name );
	send_to_char( buf, ch );

        ch->race = race;
        /* initialize stats */
        for (i = 0; i < MAX_STATS; i++)
            ch->perm_stat[i] = 7 + pc_race_table[race].stats[i];
        ch->affected_by = ch->affected_by|race_table[race].aff;
        ch->imm_flags   = ch->imm_flags|race_table[race].imm;
        ch->res_flags   = ch->res_flags|race_table[race].res;
        ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
        ch->form        = race_table[race].form;
        ch->parts       = race_table[race].parts;
             
        /* add skills */
        for (i = 0; i < 5; i++)
        {
            if (pc_race_table[race].skills[i] == NULL)
                break;
            group_add(ch,pc_race_table[race].skills[i],FALSE);
        }
        /* add cost */
        ch->pcdata->points = pc_race_table[race].points + 20;
        ch->size = pc_race_table[race].size;

	write_to_buffer( d,
	    "\n\rWhat is your age? ", 0 );
	d->connected = CON_GET_AGE;
	break;
    case CON_GET_AGE:
	if ( !is_number(argument) )
	{
	    write_to_buffer( d, "\n\rYou must give a number.\n\r", 0);
	    write_to_buffer( d, "How old is your character? ", 0 );
	    break;
	}

	ch->start_age = atoi(argument);
	if ( ch->start_age < 1
	||  ch->start_age > pc_race_table[ch->race].max_age )
	{
	    sprintf( buf, "\n\rThat age is out of bounds.  Your age must be between 1 and %d.\n\r", pc_race_table[ch->race].max_age );
	    write_to_buffer( d, buf, 0 );
	    write_to_buffer( d, "How old is your character? ", 0 );
	    break;
	}

	send_to_char( "\n\rYou will now be given the chance to enter a description\n\r", ch );
	send_to_char( "for your character.  Descriptions are mandatory at the weave,\n\r", ch );
	send_to_char( "and some players may require you to have a description\n\r", ch );
	send_to_char( "before roleplaying with them.  Please try to put a lot of\n\r", ch );
	send_to_char( "thought into your description.  Your description may be changed\n\r", ch );
	send_to_char( "at any time while you play by typing DESCRIPTION.\n\r\n\r", ch );
	send_to_char( "@ or ~ on a BLANK line will end your description.\n\r", ch );
	send_to_char( ".h or /h will show you the help.\n\r", ch );
	d->connected = CON_MAKE_DESCRIPTION;
	string_append( ch, &ch->description );
	break;

    case CON_GET_QUESTION:
	switch ( *argument )
	{
	case 'a': case 'A':
	    ch->class = 0; 
	    ch->gold = dice(1, 4) * 100;
	    group_add( ch, "warrior-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the warrior.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	case 'b': case 'B':
	    ch->class = 1;
	    ch->gold = dice(1, 3) * 150;
	    group_add( ch, "rogue-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the rogue.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	case 'c': case 'C':
	    ch->class = 2;
	    ch->gold = dice(2, 3) * 75;
	    group_add( ch, "scholar-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the scholar.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	case 'd': case 'D':
	    ch->class = 2;
	    ch->gold = dice(1, 4) * 80;
	    group_add( ch, "wisdom-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the wisdom.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	case 'e': case 'E':
	    ch->class = 0;
	    ch->gold = dice(1, 8) * 50;
	    group_add( ch, "hunter-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the hunter.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	case 'f': case 'F':
	    ch->class = 1;
	    ch->gold = dice(4, 6) * 50;
	    group_add( ch, "merchant-type", FALSE );
	    write_to_buffer( d, "\n\rYou have learned the skills of the merchant.\n\r", 0 );
	    write_to_buffer( d, "You have 10 tries to roll stats.\n\r", 0 );
	    d->connected = CON_CONFIRM_STATS;
	    break;

	default:
	    write_to_buffer( d, "Please choose a, b, c, d, e, or f.", 0 );
	    d->connected = CON_GET_QUESTION;
	    return;
	}


	/* The fun part, where we get to screw with the players :) */
        /* initialize stats */

        for (i = 0; i < MAX_STATS; i++)
            ch->perm_stat[i] = 7 + dice( 2, 3 )
			     + number_fuzzy( pc_race_table[ch->race].stats[i] );

	sprintf( buf, "\n\rStrength: %2d Intelligence: %2d Wisdom: %2d Dexterity %2d\n\r",
	    ch->perm_stat[0], ch->perm_stat[1], ch->perm_stat[2], ch->perm_stat[3] ); 
	write_to_buffer( d, buf, 0 );
	sprintf( buf, "Constitution: %2d Charisma: %2d Luck: %2d Agility: %2d\n\r",
	    ch->perm_stat[4], ch->perm_stat[5], ch->perm_stat[6], ch->perm_stat[7] );
	write_to_buffer( d, buf, 0 );

	write_to_buffer( d, "\n\rAre these stats acceptable?\n\r", 0 );
	d->connected = CON_CONFIRM_STATS;
	break;

    case CON_CONFIRM_STATS:
	ch->pcdata->stat_count--;

	if ( ch->pcdata->stat_count > 0 )
	{
	switch ( *argument )
	{
	case 'y': case 'Y':
	    write_to_buffer( d,
	"You now have 10 points to place among your stats.  To place something,\n\r", 0 );
	    write_to_buffer( d,
	"type the name of the stat you wish to improve.  I.e. strength, wisdom,\n\r", 0);
	    write_to_buffer( d,
	"wis, etc.  If you wish to save points (for extra trains, etc.) then\n\r", 0 );
	    write_to_buffer( d,
	"type done.\n\r", 0 );
	    d->connected = CON_PLACE_POINTS;
	    break;

	case 'n': case 'N':
	    if ( ch->pcdata->stat_count > 1 )
		sprintf( buf, "You have %d tries remaining.\n\r",
		    ch->pcdata->stat_count );
	    else
		sprintf( buf, "This set of stats will be taken.\n\r" );

	    write_to_buffer( d, buf, 0 );	

            for (i = 0; i < MAX_STATS; i++)
                ch->perm_stat[i] = 7 + dice( 2, 3 )
				 + number_fuzzy( pc_race_table[ch->race].stats[i] );

	    sprintf( buf, "\n\rStrength: %2d Intelligence: %2d Wisdom: %2d Dexterity %2d\n\r",
	        ch->perm_stat[0], ch->perm_stat[1], ch->perm_stat[2], ch->perm_stat[3] ); 
	    write_to_buffer( d, buf, 0 );
	    sprintf( buf, "Constitution: %2d Charisma: %2d Luck: %2d Agility: %2d\n\r",
	        ch->perm_stat[4], ch->perm_stat[5], ch->perm_stat[6], ch->perm_stat[7] );
	    write_to_buffer( d, buf, 0 );

	    if ( ch->pcdata->stat_count > 1 )
	        write_to_buffer( d, "\n\rAre these stats acceptable?\n\r", 0 );
	    else
		write_to_buffer( d, "\n\rThese stats will be taken.\n\r", 0 );

	    d->connected = CON_CONFIRM_STATS;
	    break;

	default:
	    ch->pcdata->stat_count++;
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	}
	else
	{
	    write_to_buffer( d,
	"You now have 10 points to place among your stats.  To place something,\n\r", 0 );
	    write_to_buffer( d,
	"type the name of stat you wish to improve.  I.e. strength, wisdom,\n\r", 0);
	    write_to_buffer( d,
	"wis, etc.  If you wish to save points (for extra trains, etc.) then\n\r", 0 );
	    write_to_buffer( d,
	"type done.\n\r", 0 );
	    d->connected = CON_PLACE_POINTS;
	}
	break;

    case CON_PLACE_POINTS:
	if ( ch->pcdata->stat_point > 0 )
	{
	    if ( !str_prefix( argument, "strength" ) )
	    {
		if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
		{
		    ch->perm_stat[STAT_STR]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase strength anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "intelligence" ) )
	    {
		if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT)) 
		{
		    ch->perm_stat[STAT_INT]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase intelligence anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "dexterity" ) )
	    {
		if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX)) 
		{
		    ch->perm_stat[STAT_DEX]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase dexterity anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "wisdom" ) )
	    {
		if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
		{
		    ch->perm_stat[STAT_WIS]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase wisdom anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "constitution" ) )
	    {
		if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON)) 
		{
		    ch->perm_stat[STAT_CON]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase constitution anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "charisma" ) )
	    {
		if ( ch->perm_stat[STAT_CHR] < get_max_train(ch,STAT_CHR)) 
		{
		    ch->perm_stat[STAT_CHR]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase charisma anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "luck" ) )
	    {
		if ( ch->perm_stat[STAT_LUK] < get_max_train(ch,STAT_LUK)) 
		{
		    ch->perm_stat[STAT_LUK]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase luck anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "agility" ) )
	    {
		if ( ch->perm_stat[STAT_AGI] < get_max_train(ch,STAT_AGI)) 
		{
		    ch->perm_stat[STAT_AGI]++;
		    ch->pcdata->stat_point--;
		}
		else
		    write_to_buffer( d, "You cannot increase agility anymore.\n\r", 0 );
	    }
	    else if ( !str_prefix( argument, "help" ) )
	    {
		do_help( ch, "create-stat" );
	    }
	    else if ( !str_cmp( argument, "done" ) )
	    {
		write_to_buffer( d, "\n\rOkay, your remaining points will be used to give you additional\n\r", 0 );
		write_to_buffer( d, "training points.\n\r", 0 );
		write_to_buffer( d, "What is your sex (Male/Female)? ", 0 );

		d->connected = CON_GET_NEW_SEX;
		break;
	    }
	    else
	    {
		write_to_buffer( d, "That is not a stat or the word done.\n\r", 0);
		write_to_buffer( d, "str int dex wis con cha luc help done\n\r", 0);
	    }

	    sprintf( buf, "\n\rStrength: %2d Intelligence: %2d Wisdom: %2d Dexterity %2d\n\r",
	    	ch->perm_stat[0], ch->perm_stat[1], ch->perm_stat[2], ch->perm_stat[3] ); 
	    write_to_buffer( d, buf, 0 );
	    sprintf( buf, "Constitution: %2d Charisma: %2d Luck: %2d Agility: %2d\n\r",
	    	ch->perm_stat[4], ch->perm_stat[5], ch->perm_stat[6], ch->perm_stat[7] );
	    write_to_buffer( d, buf, 0 );

	    if ( ch->pcdata->stat_point < 1 )
	    {
            	write_to_buffer( d, "What is your sex (Male/Female)? ", 0 );
            	d->connected = CON_GET_NEW_SEX;
	    	break;
	    }
	    write_to_buffer( d, "\n\rYou can increase:\n\r", 0 );
	    if ( ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
		write_to_buffer( d, "str ", 0 );
	    if ( ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT)) 
		write_to_buffer( d, "int ", 0 );
	    if ( ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
		write_to_buffer( d, "wis ", 0 );
	    if ( ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX)) 
		write_to_buffer( d, "dex ", 0 );
	    if ( ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON)) 
		write_to_buffer( d, "con ", 0 );
	    if ( ch->perm_stat[STAT_CHR] < get_max_train(ch,STAT_CHR)) 
		write_to_buffer( d, "cha ", 0 );
	    if ( ch->perm_stat[STAT_LUK] < get_max_train(ch,STAT_LUK)) 
		write_to_buffer( d, "luck ", 0 );
	    if ( ch->perm_stat[STAT_AGI] < get_max_train(ch,STAT_AGI)) 
		write_to_buffer( d, "agi ", 0 );
	    write_to_buffer( d, "\n\r", 0 );

	    if ( ch->pcdata->stat_point != 1 )
	        sprintf( buf, "\n\rYou have %d points left to spend.\n\r",
		    ch->pcdata->stat_point );
	    else
		sprintf( buf, "\n\rThis is your last point to spend.\n\r" );

	    write_to_buffer( d, buf, 0 );
	}
	else
	{
            write_to_buffer( d, "What is your sex (M/F)? ", 0 );
            d->connected = CON_GET_NEW_SEX;
	    break;
	}
        break;
        
    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    d->connected = CON_GET_NEW_SEX;
	    return;
	}

	write_to_buffer(d,"\n\r",0);

	write_to_buffer(d,"axe dagger flail mace polearm spear sword whip staff\n\r",0);
	write_to_buffer(d,"Which weapon are you skilled with? ",0);
	d->connected = CON_WEAPON_CHOICE;
	break;

case CON_WEAPON_CHOICE:
	if ( !str_prefix( argument, "axe" ) )
	{
	    group_add( ch, "axe", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_AXE;
	}
	else if ( !str_prefix( argument, "dagger" ) )
	{
	    group_add( ch, "dagger", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_DAGGER;
	}
	else if ( !str_prefix( argument, "flail" ) )
	{
	    group_add( ch, "flail", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_FLAIL;
	}
	else if ( !str_prefix( argument, "mace" ) )
	{
	    group_add( ch, "mace", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_MACE;
	}
	else if ( !str_prefix( argument, "polearm" ) )
	{
	    group_add( ch, "polearm", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_POLEARM;
	}
	else if ( !str_prefix( argument, "spear" ) )
	{
	    group_add( ch, "spear", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_SPEAR;
	}
	else if ( !str_prefix( argument, "sword" ) )
	{
	    group_add( ch, "sword", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_SWORD;
	}
	else if ( !str_prefix( argument, "whip" ) )
	{
	    group_add( ch, "whip", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_WHIP;
	}
	else if ( !str_prefix( argument, "staff" ) )
	{
	    group_add( ch, "staff", FALSE );
	    ch->pcdata->weapon = OBJ_VNUM_STAFF;
	}
	else
	{
	    write_to_buffer(d,"That is not a weapon.\n\r", 0 );
	    write_to_buffer(d,"axe dagger flail mace polearm spear sword whip staff\n\r",0);
	    write_to_buffer(d,"Which weapon are you skilled with? ",0);
	    d->connected = CON_WEAPON_CHOICE;
	    return;
	}

	ch->pcdata->teach_skill = dice( 1, 10 );
	ch->pcdata->learn_skill = dice( 1, 10 );

	if ( ch->race == race_lookup("Ogier") )
	{
            do_help( ch, "motd" );
            d->connected = CON_READ_MOTD;
            break;
	}
	d->connected = CON_MAKE_CHOICE;
	give_max( ch );
	handedness( ch );
	write_to_buffer( d, "\n\rNow you must decide the following.\n\r", 0 );
	write_to_buffer( d, "If you wish your character to start off as a\n\r", 0 );
	write_to_buffer( d, "channeler, it will cost an additional 25\n\r", 0 );
	write_to_buffer( d, "creation points.  If you do not choose yes,\n\r", 0 );
	write_to_buffer( d, "there is still a chance that you may start off\n\r", 0 );
	write_to_buffer( d, "one.  There are many skills (like healing,\n\r", 0 );
	write_to_buffer( d, "brewing, etc) that can be used instead of the\n\r", 0 );
	write_to_buffer( d, "One Power.\n\r", 0 );
	write_to_buffer( d, "NOTE: ALL CHANNELING IS CONSIDERED IN CHARACTER.\n\r", 0 );
	write_to_buffer( d, "Do you wish to be a channeler? ", 0 );
	break;
    case CON_MAKE_CHOICE:
	switch ( argument[0] )
	{
	case 'N': case 'n': 
		break;
	case 'Y': case 'y':
		ch->pcdata->points += 30;
		give_weaves( ch );
		give_spell( ch );
		group_add(ch, "channeling", FALSE );
		ch->pcdata->learned[gsn_tie_weave] = 20;

		give_talent( ch );

		if ( ch->pcdata->talent[tn_dreamwalking] )
		{
		    SET_BIT(ch->act, PLR_DREAMWALKER);
		    send_to_char( "You have a strange ability .. and sometimes .. your dreams seem almost REAL.\n\r", ch );
		}

		if ( !can_channel(ch, 0)
		&&   !IS_SET(ch->act, PLR_DREAMWALKER)
		&&   number_percent() > 95 )
		{
		    group_add(ch, "summon wolf", FALSE );
		    SET_BIT( ch->act, PLR_WOLFKIN );
		    ch->pcdata->learned[gsn_summon_wolf] = 25;
		    send_to_char( "As you grew, your eyes turned a golden color .. and you dreamed .. about wolves.\n\r", ch );
		}

		send_to_char( "\n\r", ch );
		ch->gen_data = new_gen_data( );
	        ch->gen_data->points_chosen = ch->pcdata->points;
	        do_help(ch,"group header");   
	        list_group_costs(ch);
	        write_to_buffer(d,"You already have the following skills:\n\r",0);
		write_to_buffer(d,"NOTE: You do not have to gain skills here.  You can gain them later.\n\r", 0 );
		write_to_buffer(d,"in the game, using TRAINs.\n\r", 0 );
		remove_skill( ch );
	        do_skills(ch,"");
	        do_help(ch,"menu choice");
		if ( ch->pcdata->talent[tn_blademastery] )
		{
		    group_add( ch, "blademaster", FALSE );
		    ch->pcdata->learned[gsn_feint]		= 10;
		    ch->pcdata->learned[gsn_riposte]		= 10;
		    ch->pcdata->learned[gsn_switch_opponent]	= 10;
		    ch->pcdata->learned[gsn_heroic_rescue]	= 10;
		    ch->pcdata->learned[gsn_blindfighting]	= 10;
		    ch->pcdata->learned[gsn_forms]		= 10;
		}
		d->connected = CON_GEN_GROUPS;
		return;
	default:
		write_to_buffer( d, "Do you wish to be a channeler? ", 0 );
		return;
        }
	if ( ch->sex == SEX_MALE )
	{
	    int luck;
	    int chance;
	    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;
	    chance = number_percent() + luck;
	    if ( chance >= 85 )		/* The guy CAN channel */
	    {
		ch->pcdata->learned[gsn_earth]	= 0;
		ch->pcdata->learned[gsn_air]	= 0;
		ch->pcdata->learned[gsn_fire]	= 0;
		ch->pcdata->learned[gsn_water]	= 0;
		ch->pcdata->learned[gsn_spirit]	= 0;
	    }
	}
	else
	{
	    int luck;
	    int chance;
	    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod;
	    chance = number_percent() + luck;
	    if ( chance >= 75 )		/* The lady CAN channel */
	    {
		ch->pcdata->learned[gsn_earth]	= 0;
		ch->pcdata->learned[gsn_air]	= 0;
		ch->pcdata->learned[gsn_fire]	= 0;
		ch->pcdata->learned[gsn_water]	= 0;
		ch->pcdata->learned[gsn_spirit]	= 0;
	    }
	}

	if ( can_channel(ch,0) && number_percent() +
	    (luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod/2) >= 35 )
	{
	    give_weaves( ch );
	    ch->pcdata->points += 10;
	    give_spell( ch );
	    group_add(ch, "channeling", FALSE );
	    ch->pcdata->learned[gsn_tie_weave] = 20;
	}
	give_talent( ch );

	if ( ch->pcdata->talent[tn_dreamwalking] )
	{
	    SET_BIT(ch->act, PLR_DREAMWALKER);
	    send_to_char( "You have a strange ability .. and sometimes .. your dreams seem almost REAL.\n\r", ch );
	}

	if ( !can_channel(ch, 0)
	&&   !IS_SET(ch->act, PLR_DREAMWALKER)
	&&   number_percent() > 95 )
	{
	    group_add(ch, "summon wolf", FALSE );
	    SET_BIT( ch->act, PLR_WOLFKIN );
	    ch->pcdata->learned[gsn_summon_wolf] = 25;
	    send_to_char( "As you grew, your eyes turned a golden color .. and you dreamed .. about wolves.\n\r", ch );
	}

	send_to_char( "\n\r", ch );
	ch->gen_data = new_gen_data( );
        ch->gen_data->points_chosen = ch->pcdata->points;
        do_help(ch,"group header");   
        list_group_costs(ch);
        write_to_buffer(d,"You already have the following skills:\n\r",0);
	remove_skill( ch );
        do_skills(ch,"");
	write_to_buffer(d,"NOTE: You do not have to gain skills here.  You can gain them later.\n\r", 0 );
	write_to_buffer(d,"in the game, using TRAINs.\n\r", 0 );
        do_help(ch,"menu choice");
	if ( ch->pcdata->talent[tn_blademastery] )
	{
	    group_add( ch, "blademaster", FALSE );
	    ch->pcdata->learned[gsn_feint]		= 10;
	    ch->pcdata->learned[gsn_riposte]		= 10;
	    ch->pcdata->learned[gsn_switch_opponent]	= 10;
	    ch->pcdata->learned[gsn_heroic_rescue]	= 10;
	    ch->pcdata->learned[gsn_blindfighting]	= 10;
	    ch->pcdata->learned[gsn_forms]		= 10;
	}
	d->connected = CON_GEN_GROUPS;
	break;

    case CON_GEN_GROUPS:
        send_to_char("\n\r",ch);  
        if (!str_cmp(argument,"done"))   
        {
            sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
            send_to_char(buf,ch);
            sprintf(buf,"Experience per level: %d\n\r",
                    exp_per_level(ch,ch->gen_data->points_chosen));
            if (ch->pcdata->points < 40)
                ch->train = (40 - ch->pcdata->points + 1) / 2;
            send_to_char(buf,ch);
            write_to_buffer( d, "\n\r", 2 );

	    for ( d_new = descriptor_list; d_new != NULL; d_new = d_new->next )
                if ( d_new->connected == CON_PLAYING )
		    if ( d_new->character != NULL
		    &&	 d_new->character->level >= LEVEL_IMMORTAL
		    &&   !IS_WRITING(d_new->character) )
			send_to_char(
			"A new character has been created.\n\r", d_new->character );
	    sprintf( log_buf, "%s (%s%s%s) has connected.", ch->name,
                !str_cmp( d->ident, "???" ) ? "" : d->ident,
                !str_cmp( d->ident, "???" ) ? "" : "@",
                d->host );
	    log_string( log_buf );
	    free_gen_data( ch->gen_data );
	    ch->gen_data = NULL;

            do_help( ch, "motd" );
            d->connected = CON_READ_MOTD;
            break;
        }
            
        if (!parse_gen_groups(ch,argument))
        send_to_char(
        "Choices are: list,learned,premise,add,drop,info,help, and done.\n\r"
        ,ch);
            
        do_help(ch,"menu choice");
        break;

    case CON_GET_PASSWORD_2:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd )) 
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	if ( ch->pcdata->pwd[0] == '\0' )
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}

	write_to_buffer( d, echo_on_str, 0 );

	for ( d_new = descriptor_list; d_new != NULL; d_new = d_new->next )
            if ( d_new->connected == CON_PLAYING )
                num_on++;

	if ( num_on > max_on )
	    max_on = num_on;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;
		    
	sprintf( log_buf, "%s (%s%s%s) has connected.", ch->name, 
		!str_cmp( d->ident, "???" ) ? "" : d->ident,
		!str_cmp( d->ident, "???" ) ? "" : "@",
		d->host );
	log_string( log_buf );
	if ( ch->level == -1 )
	{
	    send_to_char( "This character has been marked for recreation.\n\r", ch );
            write_to_buffer(d,"What is your last name (enter for none): ",0);
	    d->connected = CON_GET_LAST_NAME;
	    break;
	}
	if ( ch->pcdata->email == NULL )
	{
	    send_to_char( "You have no email address registered with us.\n\r",
		ch );
	    send_to_char( "We ask for your email address in order to be able\n\r",
		ch );
	    send_to_char( "to send you news on downtimes and changes.\n\r", ch );
	    send_to_char( "If you do not wish to give us your address, please\n\r",
		ch );
	    send_to_char( "enter your name and press enter, otherwise,\n\r",
		ch );
	    send_to_char( "enter your email address:  ", ch );
	    d->connected = CON_EMAIL;
	    break;
	}
	if ( IS_HERO(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

    case CON_MAKE_DESCRIPTION:
	break;
    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
        do_help( ch, "motd" );
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	write_to_buffer( d, 
    "\n\rWelcome to the Weave.  May the Pattern enfold you.\n\r", 0 );
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;
/*
	if ( IS_IMMORTAL(ch) && ch->pcdata->login[0] != '\0'
	&&  !IS_SET(ch->act, PLR_WIZINVIS) )
	    do_echo( ch, ch->pcdata->login );
*/
	if ( IS_IN_GUILD(ch, guild_lookup("aes sedai")) )
	{
	    char *temp;
	    char name[MAX_INPUT_LENGTH];
	    CHAR_DATA *warder;

	    temp = one_argument(ch->pcdata->guild->warder, name);
	    while ( !IS_NULLSTR(temp) && !IS_NULLSTR(name) )
	    {
		if ( (warder = get_char_sedai( ch, name )) == NULL )
		{
		    temp = one_argument(temp, name);
		    continue;
		}
		act( "Your bond tugs at you and you feel $n once more.", ch,
		    NULL, warder, TO_VICT );
		temp = one_argument(temp, name);
	    }
	}

	if ( !IS_NULLSTR(ch->pcdata->sedai) )
	{
	    CHAR_DATA *sedai;
             
	    if ( (sedai = get_char_sedai( ch, ch->pcdata->sedai )) != NULL )
	    {
		act( "Your bond tugs at you and you feel $n once more.", ch,
		    NULL, sedai, TO_VICT );
	    }
	}     
	reset_char(ch);

	if ( ch->level == 0 )
	{
	    ch->pcdata->perm_hit = 3 * number_fuzzy( class_table[ch->class].hp )
			         + dice( 6, 3 ) + 60;
	    ch->pcdata->perm_stamina = 5 * number_fuzzy( get_curr_stat(ch, STAT_CON) )
			    + number_range( 1, get_curr_stat(ch, STAT_STR) )
			    + dice( 2, 10 ) + 60;
	    ch->max_hit = ch->pcdata->perm_hit;
	    ch->max_stamina = ch->pcdata->perm_stamina;
	    ch->level	= 1;
	    ch->exp	= exp_per_level(ch,ch->pcdata->points);
	    ch->hit	= ch->max_hit;
	    ch->stamina	= ch->max_stamina;
	    ch->train	+= ch->pcdata->stat_point;
	    ch->practice = 3;
	    ch->char_made = TRUE;

	    do_outfit(ch,"");
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

 	    ch->pcdata->learned[get_weapon_sn(ch)]= 40;

	    char_to_room( ch, get_room_index( ROOM_VNUM_RECALL) );
	    send_to_char("\n\r",ch);
	    do_help(ch,"NEWBIE INFO");
	    send_to_char("\n\r",ch);

	    for ( note = note_list; note; note = note->next )
		ch->pcdata->last_note = note->date_stamp + 1;
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "moved" );

        wiznet("$N has left real life behind.",ch,NULL,
                WIZ_LOGINS,WIZ_SITES,get_trust(ch));

	if ( !IS_IMMORTAL(ch)
	||   (int) (current_time - ch->pcdata->last_tax) < 43200 )
	{
	    int bank = 0, gold = 0;

	    if ( ch->bank > 1500000 )
		bank = (ch->bank - 1500000) / 6;
	    if ( ch->gold > 2500000 )
		gold = (ch->gold - 2500000) / 4;

	    if ( bank > 0 || gold > 0 )
	    {
		sprintf( buf, "You have been taxed %d coins to pay for the guards.\n\r", 
		    bank + gold );
		send_to_char( buf, ch );
		if ( gold > 0 )
		{
		    sprintf( buf, "  %d coin%s from your body.\n\r", gold,
			gold != 1 ? "s" : "" );
		    send_to_char( buf, ch );
		}

		if ( bank > 0 )
		{
		    sprintf( buf, "  %d coin%s from your account.\n\r", bank,
			bank != 1 ? "s" : "" );
		    send_to_char( buf, ch );
		}

		ch->gold -= gold;
		ch->bank -= bank;
		ch->pcdata->last_tax = (int) current_time;
	    }
	}
	
	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}

	/* check notes */
	do_unread( ch, "" );

	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    int i;
    NODE_DATA *list;
    const char * reserved_words[] =
    {
	"all", "auto", "immortal", "self", "someone", "something", "the",
	"you", "guild", "warder", "sedai", "aes", "darkfriend", "seanchan",
	"tinker", "children", "of", "the", "light",
	NULL
    };
    /*
     * Reserved words.
     */
    for ( i = 0; reserved_words[i] != NULL; i++ )
    {
	if ( !str_cmp(name, reserved_words[i]) )
	    return FALSE;
    }

    /*
     * Check for PCs
     */
    for ( list = pc_list; list != NULL; list = list->next )
    {
	CHAR_DATA *person;

	if ( list->data_type != NODE_PC )
	    continue;

	person = (CHAR_DATA *) list->data;

	if ( person->desc == NULL
	||   person->desc->connected == CON_PLAYING )
	    continue;
	if ( !str_cmp(person->name, name) )
	    return FALSE;
    }
    /*
     * Length restrictions.
     */     
    if ( strlen(name) <  2 )
	return FALSE;

    if ( strlen(name) > 12 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) && *pc != '\'' && *pc != '-' )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     *
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_full_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }
     */
    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		if ( buf_string(ch->pcdata->buffer)[0] != '\0' )
		    send_to_char( "You have tells waiting.  Use REPLAY to see them.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s (%s%s%s) has reconnected.", ch->name, 
		!str_cmp( d->ident, "???" ) ? "" : d->ident,
		!str_cmp( d->ident, "???" ) ? "" : "@", d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
                    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;
	
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    if ( !d || !input )
    {
	bug( "Show_string: NULL desriptor or string.", 0 );
	return;
    }

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++) {
              /* Find the first non-whitespace */;
            }
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_string(d->showstr_head);
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	
/* ansi color stuff */
void ansi_color( const char *txt, CHAR_DATA *ch )
{
    if ( !ch || !ch->desc )
	return;

    if ( txt != NULL && ch->desc != NULL )
    {
     if ( !IS_SET(ch->act,PLR_ANSI) ) return;
/*      {
         if ( !str_cmp(txt, GREEN )
           || !str_cmp(txt, RED )
          || !str_cmp(txt, BLUE )
           || !str_cmp(txt, BLACK )
           || !str_cmp(txt, CYAN )
           || !str_cmp(txt, GREY )
           || !str_cmp(txt, YELLOW )
           || !str_cmp(txt, PURPLE ) ) return;
      }  Stuff VT100 can't use -- for later reference */
     write_to_buffer( ch->desc, txt, strlen(txt) );
     return;
    }
}
     

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RECLINING);
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act_new: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ( to->desc == NULL || (to->position < min_pos &&
	    !(to->position == POS_MOUNTED && min_pos == POS_STANDING)))
            continue;

	if ( IS_WRITING(to) )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;

	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;

	if ( type == TO_ROOM && to == ch )
	    continue; 

	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	if ( type == TO_GROUP && !is_same_group(to, ch) )
	    continue;

	if ( type == TO_CHARVICT && to != ch && to != vch )
	    continue;
 
        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' 
		 && *str >= '0' && *str <= '9' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
		/* Color code -- I hope */
		case '1':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_AUCTION]].code;
		    break;
		case '2':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_OOC]].code;
		    break;
		case '3':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_IMMTALK]].code;
		    break;
		case '4':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_BARD]].code;
		    break;
		case '5':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_TELL]].code;
		    break;
		case '6':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_SAY]].code;
		    break;
		case '7':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_GUILD]].code;
		    break;
		case '8':
		    i = "";
		    if ( IS_SET( to->act, PLR_ANSI ))
			i = color_table[to->colors[COLOR_SPECIAL]].code;
		    break;
                /* Thx alex for 't' idea */
                case 't': i = (char *) arg1;                            break;
                case 'T': i = (char *) arg2;                            break;
                case 'n': i = PERS( ch,  to  );
			if ( ch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'N': i = PERS( vch, to  ); 
			if ( vch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'y': i = PERS( ch,  to  );
			if ( ch == to )
			    i = "you";
			break;
                case 'Y': i = PERS( vch, to  ); 
			if ( vch == to )
			    i = "you";
			break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "your";
			break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "your";
			break;
 		case 'o':
			sprintf (buf1, "%s's", PERS (ch, to));
			if (ch == to)
			    i = "your";
			else
			    i = buf1;
			break;
 		case 'O':
			sprintf (buf1, "%s's", PERS (vch, to));
			if (vch == to)
			    i = "your";
			else
			    i = buf1;
			break;
		case '%':
			if (ch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '^':
			if (vch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '&':
			if (ch == to)
			    i = "";
			else
			    i = "es";
			break;
		case '*':
			if (vch == to)
			    i = "";
			else
			    i = "es";
			break;
		case 'i':
			if (ch == to)
			    i = "are";
			else
			    i = "is";
			break;
		case 'I':
			if (vch == to)
			    i = "are";
			else
			    i = "is";
			break;
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
		case 'C':
		    {
			++str;
			if ( !*str
			||   (*str < '0' || *str > '4') )
			{
			    bug( "Act: bad channeling code %d.", *str );
			    i = " <@@@> ";
			    break;
			}
			switch( *str )
			{
			    default:
				bug( "Act: Unknown power %d.", *str );
				i = " <@@@> ";
				break;
			    case '0':
				if ( ch == to
				||   check_power(to, POWER_EARTH) )
				    i = "earth";
				else
				    i = "something unknown";
				break;
			    case '1':
				if ( ch == to
				||   check_power(to, POWER_AIR) )
				    i = "air";
				else
				    i = "something unknown";
				break;
			    case '2':
				if ( ch == to
				||   check_power(to, POWER_FIRE) )
				    i = "fire";
				else
				    i = "something unknown";
				break;
			    case '3':
				if ( ch == to 
				||   check_power(to, POWER_WATER) )
				    i = "water";
				else
				    i = "something unknown";
				break;
			    case '4':
				if ( ch == to
				||   check_power(to, POWER_SPIRIT) )
				    i = "spirit";
				else
				    i = "something unknown";
				break;
			}
		    }
		    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point++ = '\0';
        buf[0]   = UPPER(buf[0]);
        write_to_buffer( to->desc, buf, (point - buf) -1 );
    }
 
    return;
}

void act_fight( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    int min_pos = POS_RECLINING;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act_fight: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ( to->desc == NULL || (to->position < min_pos &&
	    !(to->position == POS_MOUNTED && min_pos == POS_STANDING)))
            continue;

	if ( to->desc != NULL )
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch || to->desc->pString != NULL) )
            	continue;
            if ( type == TO_ROOM && (to == ch || to->desc->pString != NULL) )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch || to->desc->pString != NULL) )
            	continue;
	    if ( type == TO_ALL && to->desc->pString != NULL )
		continue;
	    if ( type == TO_CHARVICT && to != ch && to != vch )
		continue;
	}
	else
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch ) )
            	continue;
            if ( type == TO_ROOM && to == ch )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch) )
            	continue;
	    if ( type == TO_CHARVICT && to != ch && to != vch )
		continue;
	}

	if ( IS_SET(to->comm, COMM_NOSPAM) )
	    continue;

	if ( IS_NPC(to) )
	    continue;

        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' 
		 && *str >= '0' && *str <= '9' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;


                /* Thx alex for 't' idea */
                case 't': i = (char *) arg1;                            break;
                case 'T': i = (char *) arg2;                            break;
                case 'n': i = PERS( ch,  to  );
			if ( ch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'N': i = PERS( vch, to  ); 
			if ( vch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'y': i = PERS( ch,  to  );
			if ( ch == to )
			    i = "you";
			break;
                case 'Y': i = PERS( vch, to  ); 
			if ( vch == to )
			    i = "you";
			break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "your";
			break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "your";
			break;
 		case 'o':
			sprintf (buf1, "%s's", PERS (ch, to));
			if (ch == to)
			    i = "your";
			else
			    i = buf1;
			break;
 		case 'O':
			sprintf (buf1, "%s's", PERS (vch, to));
			if (vch == to)
			    i = "your";
			else
			    i = buf1;
			break;
		case '%':
			if (ch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '^':
			if (vch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '&':
			if (ch == to)
			    i = "";
			else
			    i = "es";
			break;
		case '*':
			if (vch == to)
			    i = "";
			else
			    i = "es";
			break;
		case 'i':
			if (ch == to)
			    i = "are";
			else
			    i = "is";
			break;
		case 'I':
			if (vch == to)
			    i = "are";
			else
			    i = "is";
			break;
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
		case 'C':
		    {
			++str;
			if ( !*str
			||   (*str < '0' || *str > '4') )
			{
			    bug( "Act: bad channeling code %d.", *str );
			    i = " <@@@> ";
			    break;
			}
			switch( *str )
			{
			    default:
				bug( "Act: Unknown power %d.", *str );
				i = " <@@@> ";
				break;
			    case '0':
				if ( ch == to
				||   check_power(to, POWER_EARTH) )
				    i = "earth";
				else
				    i = "something unknown";
				break;
			    case '1':
				if ( ch == to
				||   check_power(to, POWER_AIR) )
				    i = "air";
				else
				    i = "something unknown";
				break;
			    case '2':
				if ( ch == to
				||   check_power(to, POWER_FIRE) )
				    i = "fire";
				else
				    i = "something unknown";
				break;
			    case '3':
				if ( ch == to 
				||   check_power(to, POWER_WATER) )
				    i = "water";
				else
				    i = "something unknown";
				break;
			    case '4':
				if ( ch == to
				||   check_power(to, POWER_SPIRIT) )
				    i = "spirit";
				else
				    i = "something unknown";
				break;
			}
		    }
		    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point++ = '\0';
        buf[0]   = UPPER(buf[0]);
        write_to_buffer( to->desc, buf, (point - buf) -1 );
    }
 
    return;
}


void act_channel( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    int min_pos = POS_RECLINING;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act_magic: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ( to->desc == NULL || (to->position < min_pos &&
	    !(to->position == POS_MOUNTED && min_pos == POS_STANDING)))
            continue;

	if ( to->desc != NULL )
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch || to->desc->pString != NULL) )
            	continue;
            if ( type == TO_ROOM && (to == ch || to->desc->pString != NULL) )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch || to->desc->pString != NULL) )
            	continue;
	    if ( type == TO_ALL && to->desc->pString != NULL )
		continue;
	    if ( type == TO_CHARVICT && to != ch && to != vch )
		continue;
	}
	else
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch ) )
            	continue;
            if ( type == TO_ROOM && to == ch )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch) )
            	continue;
	    if ( type == TO_CHARVICT && to != ch && to != vch )
		continue;
	}

	if ( !can_channel(to, 1) )
	    continue;

	if ( TRUE_SEX(to) != TRUE_SEX(ch) )
	    continue;

	if ( !can_see(ch, to) )
	    continue;

	if ( IS_NPC(to) )
	    continue;

        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' 
		 && *str >= '0' && *str <= '9' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
                case 't': i = (char *) arg1;                            break;
                case 'T': i = (char *) arg2;                            break;
                case 'n': i = PERS( ch,  to  );
			if ( ch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'N': i = PERS( vch, to  ); 
			if ( vch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'y': i = PERS( ch,  to  );
			if ( ch == to )
			    i = "you";
			break;
                case 'Y': i = PERS( vch, to  ); 
			if ( vch == to )
			    i = "you";
			break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "your";
			break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "your";
			break;
 		case 'o':
			sprintf (buf1, "%s's", PERS (ch, to));
			if (ch == to)
			    i = "your";
			else
			    i = buf1;
			break;
 		case 'O':
			sprintf (buf1, "%s's", PERS (vch, to));
			if (vch == to)
			    i = "your";
			else
			    i = buf1;
			break;
		case '%':
			if (ch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '^':
			if (vch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '&':
			if (ch == to)
			    i = "";
			else
			    i = "es";
			break;
		case '*':
			if (vch == to)
			    i = "";
			else
			    i = "es";
			break;
		case 'i':
			if (ch == to)
			    i = "are";
			else
			    i = "is";
			break;
		case 'I':
			if (vch == to)
			    i = "are";
			else
			    i = "is";
			break;
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
		case 'C':
		    {
			++str;
			if ( !*str
			||   (*str < '0' || *str > '4') )
			{
			    bug( "Act: bad channeling code %d.", *str );
			    i = " <@@@> ";
			    break;
			}
			switch( *str )
			{
			    default:
				bug( "Act: Unknown power %d.", *str );
				i = " <@@@> ";
				break;
			    case '0':
				if ( ch == to
				||   check_power(to, POWER_EARTH) )
				    i = "earth";
				else
				    i = "something unknown";
				break;
			    case '1':
				if ( ch == to
				||   check_power(to, POWER_AIR) )
				    i = "air";
				else
				    i = "something unknown";
				break;
			    case '2':
				if ( ch == to
				||   check_power(to, POWER_FIRE) )
				    i = "fire";
				else
				    i = "something unknown";
				break;
			    case '3':
				if ( ch == to 
				||   check_power(to, POWER_WATER) )
				    i = "water";
				else
				    i = "something unknown";
				break;
			    case '4':
				if ( ch == to
				||   check_power(to, POWER_SPIRIT) )
				    i = "spirit";
				else
				    i = "something unknown";
				break;
			}
		    }
		    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point++ = '\0';
        buf[0]   = UPPER(buf[0]);
        write_to_buffer( to->desc, buf, (point - buf) -1 );
    }
 
    return;
}





void act_immortal( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( vch == NULL )
        {
            bug( "Act_immortal: null vch with TO_VICT.", 0 );
            return;
        }

	if (vch->in_room == NULL)
	    return;

        to = vch->in_room->people;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
        if ( to->desc == NULL || !IS_IMMORTAL(to) )
            continue;

	if ( to->desc != NULL )
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch || IS_WRITING(to)) )
            	continue;
            if ( type == TO_ROOM && (to == ch || to->desc->pString != NULL) )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch || to->desc->pString != NULL) )
            	continue;
	    if ( type == TO_ALL && to->desc->pString != NULL )
		continue;
	}
	else
	{
            if ( type == TO_CHAR && to != ch )
            	continue;
            if ( type == TO_VICT && ( to != vch || to == ch ) )
            	continue;
            if ( type == TO_ROOM && to == ch )
            	continue; 
            if ( type == TO_NOTVICT && (to == ch || to == vch) )
            	continue;
	}

	if ( !can_see(ch, to) )
	    continue;

	if ( IS_NPC(to) )
	    continue;

        point   = buf;
        str     = format;
        while ( *str != '\0' )
        {
            if ( *str != '$' )
            {
                *point++ = *str++;
                continue;
            }
            ++str;
 
            if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' 
		 && *str >= '0' && *str <= '9' )
            {
                bug( "Act: missing arg2 for code %d.", *str );
                i = " <@@@> ";
            }
            else
            {
                switch ( *str )
                {
                default:  bug( "Act: bad code %d.", *str );
                          i = " <@@@> ";                                break;
                /* Thx alex for 't' idea */
                case 't': i = (char *) arg1;                            break;
                case 'T': i = (char *) arg2;                            break;
                case 'n': i = PERS( ch,  to  );
			if ( ch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'N': i = PERS( vch, to  ); 
			if ( vch == to && type != TO_CHAR )
			    i = "you";
			break;
                case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "you";
			break;
                case 'M': i = him_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "you";
			break;
                case 's': i = his_her [URANGE(0, ch  ->sex, 2)];
			if (ch == to)
			    i = "your";
			break;
                case 'S': i = his_her [URANGE(0, vch ->sex, 2)];
			if (vch == to)
			    i = "your";
			break;
 		case 'o':
			sprintf (buf1, "%s's", PERS (ch, to));
			if (ch == to)
			    i = "your";
			else
			    i = buf1;
			break;
 		case 'O':
			sprintf (buf1, "%s's", PERS (vch, to));
			if (vch == to)
			    i = "your";
			else
			    i = buf1;
			break;
		case '%':
			if (ch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '^':
			if (vch == to)
			    i = "";
			else
			    i = "s";
			break;
		case '&':
			if (ch == to)
			    i = "";
			else
			    i = "es";
			break;
		case '*':
			if (vch == to)
			    i = "";
			else
			    i = "es";
			break;
		case 'i':
			if (ch == to)
			    i = "are";
			else
			    i = "is";
			break;
		case 'I':
			if (vch == to)
			    i = "are";
			else
			    i = "is";
			break;
                case 'p':
                    i = can_see_obj( to, obj1 )
                            ? obj1->short_descr
                            : "something";
                    break;
 
                case 'P':
                    i = can_see_obj( to, obj2 )
                            ? obj2->short_descr
                            : "something";
                    break;
 
                case 'd':
                    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
                    {
                        i = "door";
                    }
                    else
                    {
                        one_argument( (char *) arg2, fname );
                        i = fname;
                    }
                    break;
                }
            }
 
            ++str;
            while ( ( *point = *i ) != '\0' )
                ++point, ++i;
        }
 
        *point++ = '\n';
        *point++ = '\r';
	*point++ = '\0';
        buf[0]   = UPPER(buf[0]);
        write_to_buffer( to->desc, buf, (point - buf) -1 );
    }
 
    return;
}



/*
 * output_buffer( descriptor )
 * this function sends output down a socket. Color codes are stripped off
 * is the player is not using color, or converted to ANSI color sequences
 * to provide colored output.
 * When using ANSI, the buffer can become a lot larger due to the (sometimes)
 * lengthy ANSI sequences, thus potentially overflowing the buffer. Therefor
 * *new* buffer is send in chunks.
 * The 'bzero's may seem unnecessary, but i didn't want to take risks.
 *
 * - Wreck
 */

bool output_buffer( DESCRIPTOR_DATA *d )
{
    char        buf[MAX_STRING_LENGTH * 2];
    char        buf2[128];
    const char  *str;
    char        *i;
    char        *point;
    bool        flash=FALSE, o_flash,
                bold=FALSE, o_bold;
    bool        act=FALSE, ok=TRUE, color_code=FALSE;
    int         color=7, o_color;

    /* discard NULL descriptor */
    if ( d==NULL )
        return FALSE;

/*    bzero( buf, MAX_STRING_LENGTH ); */
    point=buf;
    str=d->outbuf;
    o_color=color;
    o_bold=bold;
    o_flash=flash;

    while ( *str != '\0' && (str-d->outbuf)<d->outtop )
    {
        if ( *str != '`' )
        {
            color_code=FALSE;
            *point++ = *str++;
            continue;
        }

        if ( !color_code && *(str+1)!='<' )
        {
            o_color=color;
            o_bold=bold;
            o_flash=flash;
        }
        color_code=TRUE;

        act=FALSE;
        str++;
        switch ( *str )
        {
            default:    sprintf( buf2, "`%c", *str );                  break;
            case 'x':   sprintf( buf2, "`" );                          break;
            case '-':   sprintf( buf2, "~" );                          break;
            case '<':   color=o_color; bold=o_bold; flash=o_flash;
                                                             act=TRUE; break;
            case '0':   color=0;                             act=TRUE; break;
            case '1':   color=1;                             act=TRUE; break;
            case '2':   color=2;                             act=TRUE; break;
            case '3':   color=3;                             act=TRUE; break;
            case '4':   color=4;                             act=TRUE; break;
            case '5':   color=5;                             act=TRUE; break;
            case '6':   color=6;                             act=TRUE; break;
            case '7':   color=7;                             act=TRUE; break;
            case ')':   color=0; bold = TRUE;                act=TRUE; break;
            case '*':   color=0; bold = TRUE;                act=TRUE; break;
            case '!':   color=1; bold = TRUE;                act=TRUE; break;
            case '@':   color=2; bold = TRUE;                act=TRUE; break;
            case '#':   color=3; bold = TRUE;                act=TRUE; break;
            case '$':   color=4; bold = TRUE;                act=TRUE; break;
            case '%':   color=5; bold = TRUE;                act=TRUE; break;
            case '^':   color=6; bold = TRUE;                act=TRUE; break;
            case '&':   color=7; bold = TRUE;                act=TRUE; break;
            case 'B':   bold=TRUE;                           act=TRUE; break;
            case 'b':   bold=FALSE;                          act=TRUE; break;
            case 'n':   if ( d->character && IS_ANSI( d->character ) )
                            sprintf( buf2, "%s", NTEXT );
                        else
                            buf2[0]='\0';
                        bold=FALSE; color=7; flash=FALSE;       break;
        }
        if ( act )
        {
            if ( d->character && IS_ANSI( d->character ) )
            {
                sprintf( buf2, "%s", color_value_string( color, bold, flash ) );
                color_code=TRUE;
            }
            else
                buf2[0]='\0';
        }

        i=buf2;
        str++;
        while ( ( *point = *i ) != '\0' )
            ++point, ++i;

        if ( (int)(point-buf)>=MAX_STRING_LENGTH-32 )
        {
            /* buffer is full, so send it through the socket */
            *point++='\0';
            if ( !(ok=write_to_descriptor( d->descriptor,
                                           buf,
                                           strlen( buf ) )) )
                break;
            bzero( buf, MAX_STRING_LENGTH );
            point=buf;
        }
    }

    *point++='\0';
    ok=ok && (write_to_descriptor( d->descriptor, buf, strlen( buf ) ));
    d->outtop=0;

    return ok;
}

void give_spell( CHAR_DATA *ch )
{
    int gsn = 0;
    int i;
    int total;
    bool gFound = FALSE;
    const char *groups[] =
	{
	"charm/illusion",
	"combat",
	"creation",
	"detection",
	"enchantment",
	"healing",
	"maladictions",
	"protective",
	"transportation",
	"weather"
	};

    total = 2 + number_fuzzy( 0 );

    while( total > 0 )
    {
	gFound = FALSE;
	while ( gFound == FALSE )
	{
	    i = number_range( 0, 9 );
	    gsn = group_lookup( groups[i] );
	    if ( number_range(1,36) >= group_cost(ch, gsn) * 2
	    &&  !ch->pcdata->group_known[gsn] )
	    {
		group_add( ch, groups[i], FALSE );
		total--;
		gFound = TRUE;
	    }
	}
    }

    group_add( ch, "general weaves", FALSE );
    send_to_char( "As you grew, you realized that you had many innate powers .. to channel.\n\r\n\r", ch );
    return;
}

void restore_conns( void )
{
    FILE *file;
    char *word;
    int desc = 1;
    DESCRIPTOR_DATA *d;

    if ( ( file = fopen( "reboot.tmp", "r" ) ) == NULL )
        return;

    fileBoot = TRUE;

    fread_number( file );
    while ( desc > 0 )
    {
        word = fread_word( file );

        if (!str_cmp(word,"DESC"))
        {
           if ( (desc = fread_number( file )) > 0 )
              set_descriptor( desc );
        }
        else if (!str_cmp(word,"CHAR"))
        {
           d = NULL;
           if ( (desc = fread_number( file )) > 0 )
              d = set_descriptor( desc );

           word = fread_string(file);

           if (d)
           {
              write_to_buffer(d,"\r\n\r\n", 0);
              if ( !check_parse_name( word ) )
              {
                write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
		if ( d->character )
		    free_char (d->character);
		d->character = NULL;
                d->connected = CON_GET_NAME;
              }
              else
              {
                 if (!load_char_obj( d, word ))
                 {
                    free_char(d->character);
                    d->character = NULL;
		    d->connected = CON_GET_NAME;
                 }
                 else
                 {
                    CHAR_DATA *ch = d->character;

                    sprintf( log_buf, "%s [%s] is back from reboot", CH(d)->name, d->host );
                    log_string( log_buf );

                    write_to_buffer(d,"Re-loading after reboot, successful.\n\r", 0 );
                    write_to_buffer( d,
                 "\n\rWelcome back to the Weave.  May the pattern enfold you.\n\r",
                                   0 );
                    d->character->next  = char_list;
                    char_list           = d->character;
                    d->connected        = CON_PLAYING;
                    reset_char(ch);
                    if ( ch->in_room != NULL )
                       char_to_room( ch, ch->in_room );
                    else if ( IS_IMMORTAL(ch) )
                       char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
                    else
                       char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );

                    act( "$n is back from the reboot.", ch, NULL, NULL, TO_ROOM);
                    do_look( ch, "moved" );
                    if (ch->pet != NULL)
                       char_to_room(ch->pet,ch->in_room);

		    do_unread( ch, "" );
                 }
              }
           }
           free_string( word );
        }
    }
    fclose( file );
    unlink( "reboot.tmp" );
    fileBoot = FALSE;
}




void send_to_char_new( CHAR_DATA *ch, char *format, ... )
{  
    va_list args;
    char str[MAX_STRING_LENGTH];

    if ( ch == NULL || ch->desc == NULL
    || ch->desc->connected != CON_PLAYING )
        return;

    va_start( args, format );
    vsprintf( str, format, args );
    write_to_buffer( ch->desc, str, 0 );
    va_end( args );
}  

void give_max( CHAR_DATA *ch )
{
    int luck;
    int i, j;

    if ( IS_NPC(ch) )
	return;

    luck = luk_app[get_curr_stat( ch, STAT_LUK )].percent_mod / 4;

    /* Find the person's strongest power :) */
    if ( TRUE_SEX(ch) == SEX_MALE )
	i = number_fuzzy( number_range(0, 2) ) - 1;
    else
	i = number_fuzzy( number_range(2, 4) );
    switch( i )
    {
	default:
		j = 4;
		break;
	case 0:
		if ( number_percent() >= 85 )
		    j = UMAX(number_fuzzy(0), 0);
		else
		    j = 0;
		break;
	case 1:
		if ( number_percent() >= 85 )
		    j = number_fuzzy(2);
		else
		    j = 2;
		break;
	case 2:
		if ( number_percent() >= 85 )
		    j = number_fuzzy(1);
		else
		    j = 1;
		break;
	case 3:
		if ( number_percent() >= 85 )
		    j = UMIN(number_fuzzy(3), 3);
		else
		    j = 3;
		break;
    }
    if ( j < 0 || j > 4 )
	j = 4;
    ch->channel_max[j] = number_range( 55, 80 );

    /* Get the rest */
    for ( i = 0; i < 5; i++ )
    {
	if ( ch->channel_max[i] == 0 )
	{
	    ch->channel_max[i] = number_range( 20, 55 )
			       + number_range( 0, luck );
	}
	if ( TRUE_SEX(ch) == SEX_MALE )
	    ch->channel_max[i] += number_range( 1, 4 );
    }
    return;
}

void give_weaves( CHAR_DATA *ch )
{
    int i;

    i = dice(2, 3) * UMAX(1, number_fuzzy(2));
    ch->pcdata->learned[gsn_earth] = UMAX( 5, i );
    i = dice(2, 3) * UMAX(1, number_fuzzy(2));
    ch->pcdata->learned[gsn_air] = UMAX( 5, i );
    i = dice(2, 3) * UMAX(1, number_fuzzy(2));
    ch->pcdata->learned[gsn_fire] = UMAX( 5, i );
    i = dice(2, 3) * UMAX(1, number_fuzzy(2));
    ch->pcdata->learned[gsn_water] = UMAX( 5, i );
    i = dice(2, 3) * UMAX(1, number_fuzzy(2));
    ch->pcdata->learned[gsn_spirit] = UMAX( 5, i );
    return;
}

void give_talent( CHAR_DATA *ch )
{
    int total;

    if ( IS_NPC(ch) )
	return;

    for ( total = 0; total < MAX_TALENT; total++ )
	ch->pcdata->talent[total] = FALSE;

    total = UMAX( 0, number_fuzzy(1) + number_fuzzy(0) );
    if ( !can_channel(ch, 0) )
	total = UMAX( 0, total - 1 );
    while( total > 0 )
    {
	int roll, tn;
	roll = number_percent();
	tn = number_range( 0, MAX_TALENT+9 );

	if ( talent_table[tn].name == NULL )
	{
	    total--;
	    continue;
	}

	if ( talent_table[tn].non_chan == FALSE
	&&   !can_channel(ch, 0) )
	    continue;

	if ( talent_table[tn].non_chan == TRUE
	&&   can_channel(ch, 0) )
	    roll = roll * 3 / 4;

	if ( roll >= talent_table[tn].diff )
	{
	    ch->pcdata->talent[tn] = TRUE;
	    total--;
	}
    }
    if ( can_channel(ch, 0) && ch->pcdata->talent[tn_channeler] )
    {
	int i;

	for ( i = 0; i < 5; i++ )
	    ch->channel_max[i] += number_range(1, 3);
	ch->pcdata->learned[gsn_earth]	+= number_range(1, 2);
	ch->pcdata->learned[gsn_air]	+= number_range(1, 2);
	ch->pcdata->learned[gsn_fire]	+= number_range(1, 2);
	ch->pcdata->learned[gsn_water]	+= number_range(1, 2);
	ch->pcdata->learned[gsn_spirit]	+= number_range(1, 2);
    }
    if ( can_channel(ch, 0) && ch->pcdata->talent[tn_powerful_channeler] )
    {
	int i;

	for ( i = 0; i < 5; i++ )
	    ch->channel_max[i] += number_range(1, 10);
	ch->pcdata->learned[gsn_earth]	+= number_range(1, 6);
	ch->pcdata->learned[gsn_air]	+= number_range(1, 6);
	ch->pcdata->learned[gsn_fire]	+= number_range(1, 6);
	ch->pcdata->learned[gsn_water]	+= number_range(1, 6);
	ch->pcdata->learned[gsn_spirit]	+= number_range(1, 6);
    }
    check_taveren( ch );
    return;
}

void handedness( CHAR_DATA *ch )
{
    int roll;
    if ( IS_NPC(ch) )
	return;

    roll = number_percent();

    if ( roll >= 20 )
	ch->pcdata->hand = 0;
    else if ( roll >= 3 )
	ch->pcdata->hand = 1;
    else
	ch->pcdata->hand = 2;
    return;
}


void broadcast( const char *str, CHAR_DATA *ch, int type)
{
    DESCRIPTOR_DATA *d, *char_d;

    if ( (type == BC_ROOM || type == BC_CHAR)
    &&   ch == NULL )
	return;

    if ( type == BC_ROOM && ch->in_room == NULL )
	return;

    char_d = ch->desc;
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->character && IS_WRITING(d->character) )
	    continue;

	if ( d->connected != CON_PLAYING )
	    continue;

	if ( type == BC_ROOM
	&&   d->character
	&&   d->character->in_room != ch->in_room )
	    continue;

	if ( type == BC_CHAR
	&&   d->character != ch )
	    continue;

	write_to_buffer( d, str, 0 );
    }
    return;
};

/*
 * Bust a prompt (player settable prompt)   
 * coded by Morgenes for Aldara Mud
 */             
void bust_a_prompt( CHAR_DATA *ch )
{
    int hp_status;
    int st_status;
    const char *hp_color;
    const char *st_color;

    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;    
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit; 
    bool found;
    const char *dir_name[] = {"N","E","S","W","U","D"};
    int door;
        
    point = buf;
    str = ch->prompt;

    hp_status = health_status( ch );
    if ( hp_status >= 3 )
	hp_color = "`2";
    else if ( hp_status == 2 )
	hp_color = "`B`6";
    else
	hp_color = "`3";

    st_status = stamina_status( ch );
    if ( st_status >= 2 )
	st_color = "`2";
    else if ( st_status == 1 )
	st_color = "`B`6";
    else
	st_color = "`3";

    if ( IS_NULLSTR(str) )
    {           
        sprintf( buf, "`n<%s%d`nhp %s%d`nst> ",
	    hp_color,
	    ch->hit,
	    st_color,
	    ch->stamina );
        send_to_char( buf, ch );
        return;
    }

    if ( !IS_SET(ch->comm, COMM_COMPACT) )
	write_to_buffer( ch->desc, "\r\n", 0 );    
    while( *str != '\0' )
    {
	if( *str != '%' )
	{
	    *point++ = *str++;
	    continue;
	}

	++str;
	switch( *str )
	{
	    default :
		i = " "; break;
	    case 'e':
		found = FALSE;
		doors[0] = '\0';
		for (door = 0; door < 6; door++)
		{
		    if ((pexit = ch->in_room->exit[door]) != NULL
		    &&  pexit ->u1.to_room != NULL
		    &&  can_see_room(ch,pexit->u1.to_room)
		    &&  !IS_SET(pexit->exit_info,EX_CLOSED))
		    {
			found = TRUE;
			strcat(doors,dir_name[door]);
		    }
		}
		if (!found)
		    strcat(buf,"none");
		sprintf(buf2,"%s",doors);
		i = buf2; break;
	    case 'c' :
		sprintf(buf2,"%s","\n\r");
		i = buf2; break;
	    case 'h' :
		sprintf( buf2, "%s%d`n", hp_color, ch->hit );
		i = buf2; break;
	    case 'H' :
		sprintf( buf2, "%d", ch->max_hit );
		i = buf2; break;
	    case 's' :
		sprintf( buf2, "%s%d`n", st_color, ch->stamina );
		i = buf2; break;
	    case 'S' :
		sprintf( buf2, "%d", ch->max_stamina );
		i = buf2; break;
	    case 'x' :
		sprintf( buf2, "%d", ch->exp );
		i = buf2; break;
	    case 'X' :
		sprintf(buf2, "%d", EXP_TO_LEVEL(ch) );
		i = buf2; break;
	    case 'g' :
		sprintf( buf2, "%ld", ch->gold);
		i = buf2; break;
	    case 'G' :
		sprintf( buf2, "%ld", ch->bank);
		i = buf2; break;
	    case 'r' :
		if( ch->in_room != NULL )
		    sprintf( buf2, "%s",
			((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
			(!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room)))
			? ch->in_room->name : "darkness");
		else
		    sprintf( buf2, " " );
		i = buf2; break;
	    case 'R' :
		if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
		    sprintf( buf2, "%d", ch->in_room->vnum );
		else
		    sprintf( buf2, " " );
		i = buf2; break;
	    case 'z' :
		if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
		    sprintf( buf2, "%s", ch->in_room->area->name );
		else
		    sprintf( buf2, " " );
		i = buf2; break;
	    case '%' :
		sprintf( buf2, "%%" );
		i = buf2; break;
	}
	++str;
	while( (*point = *i) != '\0' )
	    ++point, ++i;
    }
    write_to_buffer( ch->desc, buf, point - buf );

    if ( IS_SET(ch->comm, COMM_AFK) )
	write_to_buffer( ch->desc, "`#[AFK]`n ", 0 );

    if ( IS_SET(ch->comm, COMM_COMPACT) )
    {
	write_to_buffer( ch->desc, "(`n", 0 );
	if ( IS_AFFECTED(ch, AFF_SNEAK) )
	    write_to_buffer( ch->desc, "s", 0 );
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	    write_to_buffer( ch->desc, "h", 0 );
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	    write_to_buffer( ch->desc, "i", 0 );
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS) )
	    write_to_buffer( ch->desc, "`&w`n", 0 );
	if ( IS_GRASPING(ch) )
	    write_to_buffer( ch->desc, "`&g`n", 0 );
	write_to_buffer( ch->desc, ") ", 0 );
   }
   else
   {
	write_to_buffer( ch->desc, "`n", 0 );
	if ( IS_AFFECTED(ch, AFF_SNEAK) )
	    write_to_buffer( ch->desc, "(s) ", 0 );
	if ( IS_AFFECTED(ch, AFF_HIDE) )
	    write_to_buffer( ch->desc, "(h) ", 0 );
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	    write_to_buffer( ch->desc, "(i) ", 0 );
	if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS) )
	    write_to_buffer( ch->desc, "`&(wizi)`n ", 0 );
	if ( IS_GRASPING(ch) )
	    write_to_buffer( ch->desc, "`&(grasp)`n ", 0 );
    }
    return;
}

