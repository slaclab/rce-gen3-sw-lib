/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - August 23, 2013
**
**  Revision History:
**	    None.
**
** --
*/
 
#ifndef SERVICE_NET_INTERFACE_LINUX
#define SERVICE_NET_INTERFACE_LINUX

#define LOOPBACK_NAME "lo"
#define AF_LINK        17

#define LLADDR(s) ((caddr_t)((s)->sdl_data + (s)->sdl_nlen))

/*
 * Structure of a Link-Level sockaddr:
 */
struct sockaddr_dl {
	u_char	sdl_len;	/* Total length of sockaddr */
	u_char	sdl_family;	/* AF_LINK */
	u_short	sdl_index;	/* if != 0, system given index for interface */
	u_char	sdl_type;	/* interface type */
	u_char	sdl_nlen;	/* interface name length, no trailing 0 reqd. */
	u_char	sdl_alen;	/* link level address length */
	u_char	sdl_slen;	/* link layer selector length */
    u_char  sdl_spare[4]; /* not present in common impl */
	char	sdl_data[42];	/* minimum work area, can be larger;
				   contains both if name and ll address */
};

#endif
