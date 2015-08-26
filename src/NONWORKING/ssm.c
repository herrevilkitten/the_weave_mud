/******************************************************************************
 *  ssm.c   (shared string manager)                                           *
 *                                                                            *
 *  No copyright but names and headers should be preserved.                   *
 *  Written 1995 Melvin Smith  - Mercer University, Macon GA.                 *
 *  Some code is copied over from Furey's for the Merc port.                  *
 *  This code was scrapped from MUD++ in favor of a String class              *
 *  I converted it over to drop into Merc/Envy code by request of             *
 *  Jason Dinkel to go with OLC.  --Fusion                                    *
 *                                                                            *
 *  <msmith@falcon.mercer.peachnet.edu>                                       *
 *                                                                            *
 *                                                                            *
 *  ROM2.4 modifications by Tom Adriaenssen (Jan 1996) -- Wreck               *
 *                                                                            *
 *  <tadriaen@zorro.ruca.ua.ac.be>                                            *
 ******************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#if !defined( ultrix )
#include <memory.h>
#endif

typedef struct BE BufEntry;

/*
 * There is a temptation to change the 'long usage' to short or standard
 * int but this causes problems on at least 2 platforms that I know
 * of ( Sun and MIPS/Ultrix ) because of 32-bit alignment. The compiler
 * will pad the struct anyway so it probably woudln't make a difference.
 * I still believe this is better than using malloc which is pretty dumb
 * about chunk sizes.
 */
struct BE
{
	BufEntry *next;
	long int size;  /* size of the chunk (regardless of NULL CHAR) */ 
	long int usage;     /* keeps track of how many pointers to the string */ 
	char buf[1];    /* chunk starts here */
};

/*
 * This is for the temporary hashing of strings at bootup to speedup
 * comparison/crunching of the string space. The Hash table will
 * be freed after boot_done() is called.
 */
typedef struct TH TempHash;

struct TH
{
	TempHash *next;
	unsigned int len;
	char *str;
};

TempHash **temp_string_hash; 

/* These are the original Merc vars in db.c */
extern char  str_empty[];
extern char *string_space;
extern char *top_string;
extern int   nAllocString;
extern int   sAllocString;
extern bool  fBootDb;
int          numFree;
int          Full;

/*
 * These are the replacement functions,
 * comment them out in db.c
 */
char *str_dup		( const char * );
void free_string	( char * );
char *fread_string	( FILE * );

char *fread_word_dup	( FILE * );   /* Implement later to check words also */
void temp_hash_add      ( char * );
char *temp_hash_find    ( const char * );

/*
 * ssm_buf_head points to start of shared space,
 * ssm_buf_free points to next free block
 */ 
BufEntry *ssm_buf_head, *ssm_buf_free;

int   MAX_STRING = 2500000;
int   HEADER_SIZE;

/*
 * Not sure what is a good value for MAX_FREE 
 * If a dup fails str_dup will not defrag unless
 * the number of numFree >= MAX_FREE
 * numFree is NOT the current number of free blocks,
 * it is just a counter so defrag doesnt start dragging
 * the game in the case of a ton of failed dups
 */
#define   MAX_FREE     1000

void init_string_space()
{
	string_space = (char *)malloc( MAX_STRING );

	if( !string_space )
	{
		bug( "[SSM] Cant allocate shared string space.", 0 );
		exit(1);
	}

	top_string = string_space + MAX_STRING-1;
	ssm_buf_head = (BufEntry *)string_space;

	/*
	* Would be nice to use sizeof()-1 here but word alignment
	* adds some nice padding that puts it off. The REAL size of
	* the struct is probably 16 bytes, 8 words but since we are overlaying
	* the struct on the character space we need the exact offset.
	* 
	* Oct 24, 1995 - This is still true but probably has no effect now that
	* I am using a long (32-bit) for both ints, the struct should
	* line up nicely. Talen reports that his runs fine now after the
	* mod. Using short int causes SIGBUS on Sun, MIPS - Melvin
	*/

	HEADER_SIZE = (int)((char*)ssm_buf_head->buf-(char*)ssm_buf_head);
	ssm_buf_head->usage = 0;
	ssm_buf_head->size = MAX_STRING - HEADER_SIZE;
	ssm_buf_head->next = 0;
	ssm_buf_free = ssm_buf_head;
	temp_string_hash = (TempHash **)calloc( sizeof( TempHash * ), MAX_KEY_HASH );
}


int defrag_heap()
{
	/*
	* Walk through the shared heap and merge adjacent free blocks.
	* Free blocks are merged in str_free if free->next is free but
	* if the block preceding free is free, it stays unmerged. I would
	* rather not have the heap as a DOUBLE linked list for 2 reasons...
	*  (1) Extra 4 bytes per struct uses more mem
	*  (2) Speed - don't want to bog down str_ functions with heap management
	* The "orphaned" blocks will eventually be either merged or reused.
	* The str_dup function will call defrag if it cant allocate a buf.
	*/

	BufEntry *walk, *last_free, *next;
	int merges = 0;
	ssm_buf_free = 0;
	for( walk=ssm_buf_head,last_free=0; walk; walk = next )
	{
		next = walk->next;
		if( walk->usage > 0 )
		{
			/* this block is in use so set last_free to NULL */
			last_free = 0;
				continue;
		}
		else if( !last_free )
		{
			/* OK found a NEW free block, set last_free and move to next */
			last_free = walk;

			if( walk > ssm_buf_free )
				ssm_buf_free = walk; 

			continue; 
		}
		else
		{
			/* previous block is free so merge walk into last_free and move on */
			merges++;
			last_free->size += walk->size + HEADER_SIZE;
			last_free->next = walk->next;
		}
	}

	if( merges )
		bug( "[SSM] defrag_heap : made %d block merges.", merges );
	else
		bug( "[SSM] defrag_heap : resulted in 0 merges.", 0 ); 
  
	/* Start count over again */ 
	numFree = 0;
	return merges;
}


/*
 * Dup a string into shared space. If string exists, the usage count
 * gets incremented and the reference is returned. If the string does
 * not exist in heap, space is allocated and usage is 1.
 * This is a linked list first fit algorithm, so strings can be
 * freed. Upon bootup, there is a seperate hash table constructed in order
 * to do crunching, then the table is destroyed.
 */
char *str_dup( const char *str )
{
	BufEntry *ptr;
	int len;
	int rlen;
	char *str_new;

	if( !str || !*str )
		return &str_empty[0];

	if( str > string_space && str < top_string )
	{
		ptr = (BufEntry *)(str - HEADER_SIZE);
		if( ptr->usage <= 0 )
		{
			bug( "str_dup : invalid str", 0 );
			bug( str, 0 );
		}

		ptr->usage++;
		return (char *)str;
	}
  
	rlen = len = strlen( str ) + 1;

	/* 
	* Round up to word boundary if not already.
	* Don't remove this, because when the BufEntry struct is overlaid
	* the struct must be aligned on word boundarys.
	*/

	if( len & 3 ) 
		len += 4-(len&3);

	if( !ssm_buf_free )
	{
		/* A one time toggle just for bugging purposes */
		if( !Full )
		{
			bug( "[SSM] The shared string heap is full!", 0 );
			Full = 1;
		}
	
		str_new = (char *)malloc( rlen );
		strcpy( str_new, str ); 
		return str_new;
	} 
	else
	{ 
		RETRY:
			for( ptr = ssm_buf_free; ptr; ptr = ptr->next )
				if( ptr->usage <= 0 && ptr->size >= len )
					break;

		if( !ptr )
		{
			if( numFree >= MAX_FREE )
			{
				int merges;
				bug( "[SSM] Attempting to optimize shared string heap.", 0 );
				merges = defrag_heap();

				/* goto is fine because defrag will return 0 next time */
				if( merges )
					goto RETRY;
			}	

			str_new = (char *)malloc( rlen );
			strcpy( str_new, str ); 
			return str_new;
		}
		/* If there is at least header size excess break it up */
		else if( ptr->size-len >= HEADER_SIZE ) 
		{
			BufEntry *temp;
			/* WARNING! - DONT REMOVE THE CASTS BELOW! - Fusion */
			temp = (BufEntry*)((char *)ptr + HEADER_SIZE + len);
			temp->size = ptr->size - (len + HEADER_SIZE);
			temp->next = ptr->next;
			temp->usage = 0;
			ptr->size = len;
			ptr->next = temp;

			if( ptr == ssm_buf_free )
				ssm_buf_free = temp;
		} 

		ptr->usage = 1;
		str_new = ptr->buf;

		if( ptr == ssm_buf_free )
			for( ssm_buf_free = ssm_buf_head; ssm_buf_free; ssm_buf_free = ssm_buf_free->next )
				if( ssm_buf_free->usage <= 0 )
					break;

		strcpy( str_new, str ); 
		nAllocString++;
		sAllocString += len + HEADER_SIZE;
	}  

	return str_new;
}


/*
 * If string is in shared space, decrement usage, if usage then is 0,
 * free the chunk and attempt to merge with next node. Other
 * strings are freed with standard free.
 * Never call free/delete externally on a shared string.
 */
void free_string( char *str )
{
	BufEntry *ptr;

	if( !str || str == &str_empty[0] )
		return;

	if( str > string_space && str < top_string )
	{
		ptr = (BufEntry *)(str - HEADER_SIZE);

		if( --ptr->usage > 0 )
			return;
		else if( ptr->usage < 0 )
		{
			bug( "SSM: free_string() - multiple free bug.", 0 );
			bug( ptr->buf, 0 );
			return;
		}

		numFree++;
		sAllocString -= (ptr->size + HEADER_SIZE);
		nAllocString--;
		if( ptr->next && ptr->next->usage <= 0 )
		{
			ptr->size += ptr->next->size + HEADER_SIZE;
			ptr->next = ptr->next->next;   
		}

		if( ssm_buf_free > ptr )
			ssm_buf_free = ptr;
/*
This written if you ever need to free strings at bootup.
Without this, there would be invalid pointers in the
temp_hash_table. I have no need for it now but felt like writing it.

		if( fBootDb )
		{
			TempHash *ptr;
			TempHash *walk;
			int ihash = strlen( str ) % MAX_KEY_HASH;

			for( ptr = temp_string_hash[ ihash ]; ptr; ptr = ptr->next )
			{
				if( ptr->str != str )
					continue;
				else if( ptr == temp_string_hash[ ihash ] )
				{
					temp_string_hash[ ihash ] = ptr->next;
				}
				else
				for( walk = temp_string_hash[ ihash ]; walk; walk = walk->next )
				{
					if( walk->next == ptr )
					{
						walk->next = ptr->next;
						break;
					}
				}
         
				free( ptr );
				break; 
			}
		}
*/
		return;
	}

	free( str );
}


/*
 * Read and allocate space for a string from a file.
 * This replaces db.c fread_string
 * This is modified version of Furey's fread_string from Merc
 */
char *fread_string( FILE *fp )
{
	char buf[ MAX_STRING_LENGTH*4 ];
	char *ptr = buf;
	char  c;

	do
	{
		c = getc( fp );
	}
	while ( isspace( c ) );

	if ( ( *ptr++ = c ) == '~' )
		return &str_empty[0];

	for ( ;; )
	{
		switch ( *ptr = getc( fp ) )
		{
			default:
				ptr++;
				break;

			case EOF:
				bug( "Fread_string: EOF", 0 );
				exit( 1 );
				break;

			case '\n':
				ptr++;
				*ptr++ = '\r';
				break;

			case '\r':
				break;

			case '~':
				*ptr = '\0';
				if( fBootDb )
				{ 
					ptr = temp_hash_find( buf ); 
					if( ptr )
						return str_dup( ptr ); 

					ptr = str_dup( buf );
					temp_hash_add( ptr );
					return ptr;
				}

				return str_dup( buf );
		}
	}
}


/* 
 * This is a modified version of fread_string:
 * It reads till a '\n' or a '\r' instead of a '~' (like fread_string).
 * ROM uses this function to read in the socials.
 * -- Wreck
 */
char *fread_string_eol( FILE *fp )
{
	char buf[ MAX_STRING_LENGTH*4 ];
	char *ptr = buf;
	char  c;

	do
	{
		c = getc( fp );
	}
	while ( isspace( c ) );

	if ( ( *ptr++ = c ) == '\n' )
		return &str_empty[0];

	for ( ;; )
	{
		switch ( *ptr = getc( fp ) )
		{
			default:
				ptr++;
				break;

			case EOF:
				bug( "Fread_string: EOF", 0 );
				exit( 1 );
				break;

			case '\n':  case '\r':
				*ptr = '\0';
				if( fBootDb )
				{ 
					ptr = temp_hash_find( buf ); 
					if( ptr )
						return str_dup( ptr ); 

					ptr = str_dup( buf );
					temp_hash_add( ptr );
					return ptr;
				}

				return str_dup( buf );
		}
	}
}


/*
 * Read string into user supplied buffer.
 * Modified version of Furey's fread_string
 */
void temp_fread_string( FILE *fp, char *buf )
{
	char *ptr = buf;
	char  c;

	do
	{
		c = getc( fp );
	}
	while ( isspace( c ) );

	if ( ( *ptr++ = c ) == '~' )
	{   
		*buf = '\0';
		return;
	}

	for ( ;; )
	{
		switch ( *ptr = getc( fp ) )
		{
			default:
				ptr++;
				break;

			case EOF:
				bug( "Fread_string: EOF", 0 );
				exit( 1 );
				break;

			case '\n':
				ptr++;
				*ptr++ = '\r';
				break;

			case '\r':
				break;

			case '~':
				*ptr = '\0';
				return;
		}
	}
}


/* Lookup the string in the boot-time hash table. */
char *temp_hash_find( const char *str )
{
	TempHash *ptr;
	int len;
	int ihash;

	if( !fBootDb || !*str )
		return 0;

	len = strlen( str );
	ihash = len % MAX_KEY_HASH;

	for( ptr = temp_string_hash[ ihash ]; ptr; ptr = ptr->next )
	{
		if( *ptr->str != *str )
			continue;
		else if( strcmp( ptr->str, str ) )
			continue;
		else return ptr->str;
	}

	return 0;
}


/*
 * Add a reference in the temporary hash table.
 * String is still in the linked list structure but
 * reference is kept here for quick lookup at boot time;
 */
void temp_hash_add( char *str )
{
	int len;
	int ihash;
	TempHash *add;

	if( !fBootDb || !*str || ( str <= string_space && str >= top_string ) )
		return;

	len = strlen( str );
	ihash = len % MAX_KEY_HASH;
	add = (TempHash *)malloc( sizeof( TempHash ) );
	add->next = temp_string_hash[ ihash ];
	temp_string_hash[ ihash ] = add;
	add->len = len;
	add->str = str;
}


/* Free the temp boot string hash table */
void boot_done( void )
{
	TempHash *ptr, *next;
	int ihash;

	for( ihash = 0; ihash < MAX_KEY_HASH; ihash ++ )
	{
		for( ptr = temp_string_hash[ ihash ]; ptr; ptr = next )
		{
			next = ptr->next;
			free( ptr );
		}
	}
}
