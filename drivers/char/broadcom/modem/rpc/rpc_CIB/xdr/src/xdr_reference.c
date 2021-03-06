/*	$NetBSD: xdr_reference.c,v 1.13 2000/01/22 22:19:18 mycroft Exp $	*/

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#include "xdr.h"
#ifdef __weak_alias
__weak_alias(xdr_pointer, _xdr_pointer)
    __weak_alias(xdr_reference, _xdr_reference)
#endif
/*
 * XDR an indirect pointer
 * xdr_reference is for recursively translating a structure that is
 * referenced by a pointer inside the structure that is currently being
 * translated.  pp references a pointer to storage. If *pp is null
 * the  necessary storage is allocated.
 * size is the sizeof the referneced structure.
 * proc is the routine to handle the referenced structure.
 */
bool_t xdr_referenceEx(XDR *xdrs, caddr_t *pp,	/* the pointer to work on */
		       u_int size,	/* size of the object pointed to */
		       xdrproc_t proc, xdrproc_t xdr_user_obj)
{				/* xdr routine to handle the object */
	caddr_t loc = *pp;
	bool_t stat;

	if (loc == NULL)
		switch (xdrs->x_op) {
		case XDR_FREE:
			return (TRUE);

		case XDR_DECODE:
			*pp = loc = (caddr_t)XDR_ALLOC(xdrs, size);
			if (loc == NULL) {
				//warnx("xdr_reference: out of memory");
				return (FALSE);
			}
			memset(loc, 0, size);
			break;

		case XDR_ENCODE:
			break;
		}

	stat = (*proc) (xdrs, loc, xdr_user_obj);

	if (xdrs->x_op == XDR_FREE) {
		XDR_DEALLOC(xdrs, loc, size);
		*pp = NULL;
	}
	return (stat);
}

bool_t xdr_reference(XDR *xdrs, caddr_t *pp,	/* the pointer to work on */
		     u_int size,	/* size of the object pointed to */
		     xdrproc_t proc)
{				/* xdr routine to handle the object */
	return xdr_referenceEx(xdrs, pp, size, proc, NULL);
}

/*
 * xdr_pointer():
 *
 * XDR a pointer to a possibly recursive data structure. This
 * differs with xdr_reference in that it can serialize/deserialiaze
 * trees correctly.
 *
 *  What's sent is actually a union:
 *
 *  union object_pointer switch (boolean b) {
 *  case TRUE: object_data data;
 *  case FALSE: void nothing;
 *  }
 *
 * > objpp: Pointer to the pointer to the object.
 * > obj_size: size of the object.
 * > xdr_obj: routine to XDR an object.
 *
 */
bool_t
xdr_pointerEx(XDR *xdrs, char **objpp, u_int obj_size, xdrproc_t xdr_obj,
	      xdrproc_t xdr_user_obj)
{

	bool_t more_data;

	more_data = (*objpp != NULL);
	if (!xdr_bool(xdrs, &more_data)) {
		return (FALSE);
	}
	if (!more_data) {
		*objpp = NULL;
		return (TRUE);
	}
	return (xdr_referenceEx(xdrs, objpp, obj_size, xdr_obj, xdr_user_obj));
}

bool_t xdr_pointer(XDR *xdrs, char **objpp, u_int obj_size, xdrproc_t xdr_obj)
{
	return xdr_pointerEx(xdrs, objpp, obj_size, xdr_obj, NULL);
}
